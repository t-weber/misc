/**
 * map file loading and svg saving test
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
 * @date 14 December 2024
 * @license see 'LICENSE.GPL' file
 *
 * g++ -std=c++20 -march=native -O2 -Wall -Wextra -Weffc++ -o maps maps.cpp
 */

#include <iostream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <string>
#include <list>
#include <vector>
#include <unordered_map>

#if __has_include(<filesystem>)
	#include <filesystem>
	namespace __map_fs = std::filesystem;
#else
	#include <boost/filesystem.hpp>
	namespace __map_fs = boost::filesystem;
#endif

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/geometry.hpp>



template<class t_real = double>
struct MapVertex
{
	t_real longitude{};
	t_real latitude{};

	std::unordered_map<std::string, std::string> tags{};
};



template<class t_size = std::size_t>
struct MapSegment
{
	std::list<t_size> vertex_ids{};
	bool is_area{false};

	std::unordered_map<std::string, std::string> tags{};
};



template<class t_size = std::size_t>
struct MapMultiSegment
{
	std::list<t_size> vertex_ids{};
	std::list<t_size> segment_ids{};

	std::unordered_map<std::string, std::string> tags{};
};



template<class t_real = double, class t_size = std::size_t>
class Map
{
public:
	using t_vertex = MapVertex<t_real>;
	using t_segment = MapSegment<t_size>;
	using t_multisegment = MapMultiSegment<t_size>;


public:
	Map() = default;
	~Map() = default;



	/**
	 * import a map from an osm file
	 * @see https://wiki.openstreetmap.org/wiki/OSM_XML
	 * @see https://wiki.openstreetmap.org/wiki/Elements
	 */
	bool Import(const std::string& mapname)
	{
		namespace ptree = boost::property_tree;
		namespace num = std::numbers;
		namespace fs = __map_fs;

		fs::path mapfile{mapname};
		if(!fs::exists(mapfile))
			return false;

		ptree::ptree map_root;
		ptree::read_xml(mapname, map_root);

		const auto& osm = map_root.get_child_optional("osm");
		if(!osm)
			return false;

		m_filename = mapfile.filename().string();
		m_version = osm->get<std::string>("<xmlattr>.version", "<unknown>");
		m_creator = osm->get<std::string>("<xmlattr>.generator", "<unknown>");

		const auto& map_nodes = osm->get_child_optional("");
		if(!map_nodes)
			return false;

		for(const auto& node : *map_nodes)
		{
			//std::cout << node.first << std::endl;

			// node is a vertex
			if(node.first == "node")
			{
				auto id = node.second.get_optional<t_size>("<xmlattr>.id");
				auto lon = node.second.get_optional<t_real>("<xmlattr>.lon");
				auto lat = node.second.get_optional<t_real>("<xmlattr>.lat");
				auto vis = node.second.get_optional<bool>("<xmlattr>.visible");
				if(vis && !*vis)
					continue;

				if(!id || !lon || !lat)
					continue;

				t_vertex vertex
				{
					.longitude = *lon,
					.latitude = *lat,
				};

				if(auto tags = node.second.get_child_optional(""))
				{
					for(const auto& tag : *tags)
					{
						if(tag.first != "tag")
							continue;

						auto key = tag.second.get_optional<std::string>("<xmlattr>.k");
						auto val = tag.second.get_optional<std::string>("<xmlattr>.v");

						if(!key || !val)
							continue;

						//std::cout << *key << " = " << *val << std::endl;
						vertex.tags.emplace(std::make_pair(*key, *val));
					}
				}

				m_vertices.emplace(std::make_pair(*id, std::move(vertex)));
			}

			// node is a line segment
			else if(node.first == "way")
			{
				auto id = node.second.get_optional<t_size>("<xmlattr>.id");
				auto vis = node.second.get_optional<bool>("<xmlattr>.visible");
				if(vis && !*vis)
					continue;

				t_segment seg;

				if(auto tags = node.second.get_child_optional(""))
				{
					for(const auto& tag : *tags)
					{
						if(tag.first == "nd")
						{
							auto vertex_id = tag.second.get_optional<t_size>("<xmlattr>.ref");
							if(!vertex_id)
								continue;

							seg.vertex_ids.push_back(*vertex_id);
						}

						else if(tag.first == "tag")
						{
							auto key = tag.second.get_optional<std::string>("<xmlattr>.k");
							auto val = tag.second.get_optional<std::string>("<xmlattr>.v");

							if(!key || !val)
								continue;

							//std::cout << *key << " = " << *val << std::endl;
							seg.tags.emplace(std::make_pair(*key, *val));
						}
					}
				}

				if(seg.vertex_ids.size() >= 2 && *seg.vertex_ids.begin() == *seg.vertex_ids.rbegin())
					seg.is_area = true;
				m_segments.emplace(std::make_pair(*id, std::move(seg)));
			}

			// node is a relation
			else if(node.first == "relation")
			{
				auto id = node.second.get_optional<t_size>("<xmlattr>.id");
				auto vis = node.second.get_optional<bool>("<xmlattr>.visible");
				if(vis && !*vis)
					continue;

				t_multisegment seg;

				if(auto tags = node.second.get_child_optional(""))
				{
					for(const auto& tag : *tags)
					{
						if(tag.first == "member")
						{
							auto seg_ty = tag.second.get_optional<std::string>("<xmlattr>.type");
							auto seg_ref = tag.second.get_optional<t_size>("<xmlattr>.ref");
							if(!seg_ty || !seg_ref)
								continue;

							if(*seg_ty == "node")
								seg.vertex_ids.push_back(*seg_ref);
							else if(*seg_ty == "way")
								seg.segment_ids.push_back(*seg_ref);

						}

						else if(tag.first == "tag")
						{
							auto key = tag.second.get_optional<std::string>("<xmlattr>.k");
							auto val = tag.second.get_optional<std::string>("<xmlattr>.v");

							if(!key || !val)
								continue;

							seg.tags.emplace(std::make_pair(*key, *val));
						}
					}
				}

				m_multisegments.emplace(std::make_pair(*id, std::move(seg)));
			}
		}  // node iteration

		return true;
	}



	/**
	 * write an svg file
	 * @see https://github.com/boostorg/geometry/tree/develop/example
	 */
	bool ExportSvg(const std::string& filename) const
	{
		namespace geo = boost::geometry;

		using t_vert = geo::model::point<t_real, 2, geo::cs::cartesian>;
		using t_line = geo::model::linestring<t_vert, std::vector>;
		using t_poly = geo::model::polygon<t_vert, true /*cw*/, false /*closed*/, std::vector>;
		using t_svg = geo::svg_mapper<t_vert>;

		std::ofstream ofstr(filename);
		if(!ofstr)
			return false;

		t_svg svg(ofstr, 128, 128);

		// TODO: draw multi-areas
		/*for(const auto& [ id, seg ] : m_multisegments)
		{
			for(const auto& [ tag_key, tag_val ] : seg.tags)
			{
				std::cout << tag_key << " = " << tag_val << std::endl;
			}
			std::cout << std::endl;
		}*/

		// draw areas
		for(const auto& [ id, seg ] : m_segments)
		{
			if(!seg.is_area)
				continue;
			t_poly poly;

			for(const t_size vert_id : seg.vertex_ids)
			{
				auto iter = m_vertices.find(vert_id);
				if(iter == m_vertices.end())
					continue;

				t_vert vert{iter->second.longitude, iter->second.latitude};
				poly.outer().push_back(vert);
			}

			std::string line_col = "#000000";
			std::string fill_col = "#ffffff";
			t_real line_width = 2.;
			for(const auto& [ tag_key, tag_val ] : seg.tags)
			{
				//std::cout << tag_key << " = " << tag_val << std::endl;
				if(tag_key == "landuse" && tag_val == "residential")
				{
					fill_col = "#aaaaaa";
					break;
				}
				else if(tag_key == "building" && tag_val == "yes")
				{
					fill_col = "#ffffff";
					break;
				}
				else if(tag_key == "landuse" && tag_val == "forest")
				{
					fill_col = "#009900";
					break;
				}
			}

			svg.add(poly);
			svg.map(poly, "stroke:" + line_col +
				"; stroke-width:" + std::to_string(line_width) + "px" +
				"; fill:" + fill_col + ";", 1.);
		}

		// draw streets
		for(const auto& [ id, seg ] : m_segments)
		{
			if(seg.is_area)
				continue;
			t_line line;

			for(const t_size vert_id : seg.vertex_ids)
			{
				auto iter = m_vertices.find(vert_id);
				if(iter == m_vertices.end())
					continue;

				t_vert vert{iter->second.longitude, iter->second.latitude};
				line.push_back(vert);
			}

			std::string line_col = "#000000";
			std::string fill_col = "none";
			t_real line_width = 8.;

			for(const auto& [ tag_key, tag_val ] : seg.tags)
			{
				//std::cout << tag_key << " = " << tag_val << std::endl;

				if(tag_key == "highway" && tag_val == "motorway")
				{
					std::string fill_col = "#000000";
					line_width = 52.;
				}
				else if(tag_key == "highway" && tag_val == "trunk")
				{
					std::string fill_col = "#000000";
					line_width = 48.;
				}
				else if(tag_key == "highway" && tag_val == "primary")
				{
					std::string fill_col = "#000000";
					line_width = 40.;
				}
				else if(tag_key == "highway" && tag_val == "secondary")
				{
					std::string fill_col = "#000000";
					line_width = 32.;
				}
				else if(tag_key == "highway" && tag_val == "tertiary")
				{
					std::string fill_col = "#000000";
					line_width = 24.;
				}
				else if(tag_key == "highway" && tag_val == "residential")
				{
					std::string fill_col = "#000000";
					line_width = 16.;
				}
				else if(tag_key == "highway" && tag_val == "track")
				{
					std::string fill_col = "#000000";
					line_width = 8.;
				}
			}

			svg.add(line);
			svg.map(line, "stroke:" + line_col +
				"; stroke-width:" + std::to_string(line_width) + "px"
				"; fill:" + fill_col + ";", 1.);
		}

		return true;
	}



private:
	std::string m_filename{};
	std::string m_version{};
	std::string m_creator{};

	std::unordered_map<t_size, t_vertex> m_vertices{};
	std::unordered_map<t_size, t_segment> m_segments{};
	std::unordered_map<t_size, t_multisegment> m_multisegments{};
};



int main(int argc, char **argv)
{
	if(argc <= 2)
	{
		std::cerr << "Please give an osm input and an svg output file name." << std::endl;
		return -1;
	}

	try
	{
		Map map{};

		if(!map.Import(argv[1]))
		{
			std::cerr << "Could not read \"" << argv[1] << "\"." << std::endl;
			return -1;
		}

		if(!map.ExportSvg(argv[2]))
		{
			std::cerr << "Could not write \"" << argv[2] << "\"." << std::endl;
			return -1;
		}
	}
	catch(const std::exception& ex)
	{
		std::cerr << ex.what() << std::endl;
		return -1;
	}

	return 0;
}
