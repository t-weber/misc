/**
 * track segment test
 * @author Tobias Weber
 * @date 24 November 2024
 * @license see 'LICENSE' file
 *
 * g++ -std=c++20 -march=native -O2 -I../libs -Wall -Wextra -Weffc++ -o track track.cpp
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace ptree = boost::property_tree;

#include <boost/geometry.hpp>
namespace geo = boost::geometry;



/**
 * haversine
 * @see https://en.wikipedia.org/wiki/Versine#Haversine
 */
template<typename t_real = double>
t_real havsin(t_real th)
{
	return t_real(0.5) - t_real(0.5)*std::cos(th);
}



/**
 * arcaversin
 * @see https://en.wikipedia.org/wiki/Versine#Haversine
 */
template<typename t_real = double>
t_real arcaversin(t_real x)
{
	return std::acos(t_real(1) - t_real(2)*x);
}



/**
 * haversine formula
 * @see https://en.wikipedia.org/wiki/Haversine_formula
 */
template<typename t_real = double>
t_real geo_dist(t_real lat1, t_real lat2, t_real lon1, t_real lon2, t_real elev)
{
	t_real rad = 6.3781e6;  // see: https://en.wikipedia.org/wiki/Earth_radius
	rad += elev;

	t_real h =
		havsin<t_real>(lat2 - lat1) +
		havsin<t_real>(lon2 - lon1) * std::cos(lat1)*std::cos(lat2);

	return rad * arcaversin<t_real>(h);
}



/**
 * geographic distance
 * @see https://github.com/BoostGSoC18/geometry/blob/develop/test/strategies/thomas.cpp
 * @see https://github.com/boostorg/geometry/blob/develop/example/ml02_distance_strategy.cpp
 */
template<typename t_real = double>
t_real geo_dist_2(t_real lat1, t_real lat2, t_real lon1, t_real lon2, [[__maybe_unused__]] t_real elev)
{
	//t_real rad = 6.3781e6;  // see: https://en.wikipedia.org/wiki/Earth_radius
	//rad += elev;

	using t_pt = geo::model::point<t_real, 2, geo::cs::geographic<geo::radian>>;

	//using t_strat = geo::strategy::distance::haversine<geo::srs::spheroid<t_real>>;
	//using t_strat = geo::strategy::distance::vincenty<geo::srs::spheroid<t_real>>;
	using t_strat = geo::strategy::distance::thomas<geo::srs::spheroid<t_real>>;

	t_pt pt1{lon1, lat1};
	t_pt pt2{lon2, lat2};

	return geo::distance<t_pt, t_pt, t_strat>(pt1, pt2, t_strat{/*rad*/});
}



/**
 * loads a gpx track file
 * @see https://en.wikipedia.org/wiki/GPS_Exchange_Format
 */
bool test_track(const std::string& trackfile)
{
	using t_real = double;
	using t_clk = std::chrono::system_clock;
	using t_timept = std::chrono::time_point<t_clk>;

	ptree::ptree track;
	ptree::read_xml(trackfile, track);

	const auto& gpx = track.get_child_optional("gpx");
	if(!gpx)
	{
		std::cerr << "Error: " << trackfile << " is not a gpx file." << std::endl;
		return false;
	}

	std::cout << "File version: " << gpx->get<std::string>("<xmlattr>.version") << std::endl;
	std::cout << "File creator: " << gpx->get<std::string>("<xmlattr>.creator") << std::endl;
	std::cout << std::endl;

	const auto& tracks = gpx->get_child_optional("");
	if(!tracks)
	{
		std::cerr << "Error: No tracks available." << std::endl;
		return false;
	}

	std::optional<t_real> lattitude_last, longitude_last;
	std::optional<t_timept> time_pt_last;

	t_real total_dist = 0.;
	t_real total_time = 0.;

	for(const auto& track : *tracks)
	{
		if(track.first != "trk")
			continue;

		const auto& segs = track.second.get_child_optional("");
		if(!segs)
			continue;

		for(const auto& seg : *segs)
		{
			if(seg.first != "trkseg")
				continue;

			const auto& pts = seg.second.get_child_optional("");
			if(!pts)
				continue;

			for(const auto& pt : *pts)
			{
				if(pt.first != "trkpt")
					continue;

				t_real lattitude_deg = pt.second.get<t_real>("<xmlattr>.lat");
				t_real longitude_deg = pt.second.get<t_real>("<xmlattr>.lon");
				t_real elevation = pt.second.get<t_real>("ele");
				std::string time_str = pt.second.get<std::string>("time");

				t_real lattitude = lattitude_deg / t_real(180) * t_real(M_PI);
				t_real longitude = longitude_deg / t_real(180) * t_real(M_PI);

				// time of track point
				t_timept time_pt;
				std::istringstream{time_str} >>
					std::chrono::parse("%4Y-%2m-%2dT%2H:%2M:%2SZ", time_pt);

				// elapsed seconds since last track point
				t_real elapsed = 0.;
				if(time_pt_last)
					elapsed = std::chrono::duration<t_real>{time_pt - *time_pt_last}.count();

				t_real dist = 0.;
				t_real dist2 = 0.;
				if(lattitude_last && longitude_last)
				{
					dist = geo_dist<t_real>(
						*lattitude_last, lattitude, *longitude_last, longitude, elevation);

					dist2 = geo_dist_2<t_real>(
						*lattitude_last, lattitude, *longitude_last, longitude, elevation);
				}

				std::cout
					<< std::left << std::setw(30) << time_str << " "
					<< std::left << std::setw(15) << lattitude_deg << " "
					<< std::left << std::setw(15) << longitude_deg << " "
					<< std::left << std::setw(15) << elevation << " "
					<< std::left << std::setw(15) << elapsed << " "
					<< std::left << std::setw(15) << dist << " "
					<< std::left << std::setw(15) << dist2 << " "
					<< std::endl;

				total_time += elapsed;
				total_dist += dist;

				lattitude_last = lattitude;
				longitude_last = longitude;
				time_pt_last = time_pt;
			}
		}
	}

	std::cout << std::endl;
	std::cout << "Total distance: " << total_dist / 1000. << " km" << std::endl;
	std::cout << "Total time: " << total_time / 60. << " min" << std::endl;
	std::cout << "Speed: " << total_dist / total_time << " m/s"
		<< " = " << (total_dist / 1000.) / (total_time / 60. / 60.) << " km/h"
		<< std::endl;
	std::cout << "Pace: " << (total_time / 60.) / (total_dist / 1000.)
		<< " min/km" << std::endl;
	std::cout << std::endl;

	return true;
}



int main(int argc, char **argv)
{
	if(argc <= 1)
	{
		std::cerr << "Please give a track file." << std::endl;
		return -1;
	}

	try
	{
		test_track(argv[1]);
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
