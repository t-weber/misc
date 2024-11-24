/**
 * track segment test
 * @author Tobias Weber
 * @date 24 November 2024
 * @license see 'LICENSE' file
 *
 * g++ -std=c++20 -march=native -O2 -Wall -Wextra -Weffc++ -o track track.cpp
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <cmath>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
namespace ptree = boost::property_tree;

#include <boost/geometry.hpp>
namespace geo = boost::geometry;

#define HAS_CHRONO_PARSE 0



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
 * earth radius
 * @see https://en.wikipedia.org/wiki/Earth_radius
 */
template<typename t_real = double>
t_real earth_radius(t_real lat)
{
	t_real rad_pol = 6.3567523e6;
	t_real rad_equ = 6.3781370e6;

	t_real c = std::cos(lat);
	t_real s = std::sin(lat);

	t_real num =
		std::pow(rad_equ*rad_equ*c, t_real(2)) +
		std::pow(rad_pol*rad_pol*s, t_real(2));
	t_real den =
		std::pow(rad_equ*c, t_real(2)) +
		std::pow(rad_pol*s, t_real(2));

	return std::sqrt(num / den);
}



/**
 * haversine formula
 * @see https://en.wikipedia.org/wiki/Haversine_formula
 */
template<typename t_real = double>
t_real geo_dist(t_real lat1, t_real lat2,
	t_real lon1, t_real lon2,
	t_real elev1, t_real elev2)
{
	t_real rad = earth_radius<t_real>((lat1 + lat2) / t_real(2));
	rad += (elev1 + elev2) * t_real(0.5);

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
t_real geo_dist_2(t_real lat1, t_real lat2,
	t_real lon1, t_real lon2,
	[[__maybe_unused__]] t_real elev1, [[__maybe_unused__]] t_real elev2)
{
	//t_real rad = earth_radius<t_real>((lat1 + lat2) / t_real(2));
	//rad += (elev1 + elev2) * t_real(0.5);

	using t_pt = geo::model::point<t_real, 2, geo::cs::geographic<geo::radian>>;

	//using t_strat = geo::strategy::distance::haversine<geo::srs::spheroid<t_real>>;
	//using t_strat = geo::strategy::distance::vincenty<geo::srs::spheroid<t_real>>;
	using t_strat = geo::strategy::distance::thomas<geo::srs::spheroid<t_real>>;

	t_pt pt1{lon1, lat1};
	t_pt pt2{lon2, lat2};

	return geo::distance<t_pt, t_pt, t_strat>(pt1, pt2, t_strat{/*rad*/});
}



template<class t_clk, class t_timept = typename t_clk::time_point>
t_timept get_timepoint(const std::string& time_str)
{
	t_timept time_pt{};

#if HAS_CHRONO_PARSE != 0
	std::istringstream{time_str} >>
		std::chrono::parse("%4Y-%2m-%2dT%2H:%2M:%2SZ", time_pt);
#else
	std::tm t{};
	t.tm_year = std::stoi(time_str.substr(0, 4));
	t.tm_mon = std::stoi(time_str.substr(5, 2)) + 1;
	t.tm_mday = std::stoi(time_str.substr(8, 2));
	t.tm_hour = std::stoi(time_str.substr(11, 2));
	t.tm_min = std::stoi(time_str.substr(14, 2));
	t.tm_sec = std::stoi(time_str.substr(17, 2));

	time_pt = t_clk::from_time_t(std::mktime(&t));
#endif

	return time_pt;
}



/**
 * loads a gpx track file
 * @see https://en.wikipedia.org/wiki/GPS_Exchange_Format
 */
bool test_track(const std::string& trackfile)
{
	using t_real = double;
	using t_clk = std::chrono::system_clock;
	using t_timept = typename t_clk::time_point;

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

	t_real assume_dt = 3.;
	int field_width = std::cout.precision() + 2;
	bool timing_error_shown = false;

	std::optional<t_real> latitude_last, longitude_last, elevation_last;
	std::optional<t_timept> time_pt_last;

	std::size_t total_pts = 0;
	t_real total_dist = 0.;
	t_real total_time = 0.;
	t_real min_elev = std::numeric_limits<t_real>::max();
	t_real max_elev = -min_elev;

	std::cout
		<< std::left << std::setw(field_width) << "Lat." << " "
		<< std::left << std::setw(field_width) << "Lon." << " "
		<< std::left << std::setw(field_width) << "h" << " "
		<< std::left << std::setw(field_width) << "\xce\x94t" << "  "
		<< std::left << std::setw(field_width) << "\xce\x94s" << "  "
		<< std::left << std::setw(field_width) << "\xce\x94s_2" << "  "
		<< std::left << std::setw(field_width) << "t" << " "
		<< std::left << std::setw(field_width) << "s";
	std::cout << std::endl;

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

				t_real latitude_deg = pt.second.get<t_real>("<xmlattr>.lat");
				t_real longitude_deg = pt.second.get<t_real>("<xmlattr>.lon");
				t_real elevation = pt.second.get<t_real>("ele");
				std::string time_str;
				bool has_time = false;
				if(auto time_opt = pt.second.get_optional<std::string>("time"))
				{
					time_str = *time_opt;
					has_time = true;
				}
				else if(!timing_error_shown)
				{
					std::cerr << "No timing information, assuming \xce\x94t = "
						<< assume_dt << " s." << std::endl;
					timing_error_shown = true;
				}

				t_real latitude = latitude_deg / t_real(180) * t_real(M_PI);
				t_real longitude = longitude_deg / t_real(180) * t_real(M_PI);

				// time of track point
				t_timept time_pt;
				if(has_time)
					time_pt = get_timepoint<t_clk>(time_str);

				// elapsed seconds since last track point
				t_real elapsed = 0.;
				if(time_pt_last)
				{
					if(has_time)
						elapsed = std::chrono::duration<t_real>{time_pt - *time_pt_last}.count();
					else
						elapsed = assume_dt;
				}

				t_real dist = 0.;
				t_real dist2 = 0.;
				if(latitude_last && longitude_last && elevation_last)
				{
					dist = geo_dist<t_real>(
						*latitude_last, latitude,
						*longitude_last, longitude,
						*elevation_last, elevation);

					dist2 = geo_dist_2<t_real>(
						*latitude_last, latitude,
						*longitude_last, longitude,
						*elevation_last, elevation);
				}

				// cumulative values
				total_time += elapsed;
				total_dist += dist;
				++total_pts;
				max_elev = std::max(max_elev, elevation);
				min_elev = std::min(min_elev, elevation);

				std::cout
					<< std::left << std::setw(field_width) << latitude_deg << " "
					<< std::left << std::setw(field_width) << longitude_deg << " "
					<< std::left << std::setw(field_width) << elevation << " "
					<< std::left << std::setw(field_width) << elapsed << " "
					<< std::left << std::setw(field_width) << dist << " "
					<< std::left << std::setw(field_width) << dist2 << " "
					<< std::left << std::setw(field_width) << total_time << " "
					<< std::left << std::setw(field_width) << total_dist << " ";

				if(has_time)
				{
					std::cout
						<< std::left << std::setw(25) << time_str << " ";
				}

				std::cout << std::endl;

				// save last values
				latitude_last = latitude;
				longitude_last = longitude;
				elevation_last = elevation;
				time_pt_last = time_pt;
			}
		}
	}

	std::cout << std::endl;
	std::cout << "Number of track points: " << total_pts << std::endl;
	std::cout << "Elevation range: [ " << min_elev << ", " << max_elev << " ] m" << std::endl;
	std::cout << "Height difference: " << max_elev - min_elev << " m" << std::endl;
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
