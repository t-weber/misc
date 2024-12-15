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
#include <tuple>
#include <unordered_map>
#include <unordered_set>

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
	std::list<t_size> segment_inner_ids{};
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



protected:
	bool ImportVertex(const boost::property_tree::ptree& node)
	{
		auto id = node.get_optional<t_size>("<xmlattr>.id");
		auto lon = node.get_optional<t_real>("<xmlattr>.lon");
		auto lat = node.get_optional<t_real>("<xmlattr>.lat");
		auto vis = node.get_optional<bool>("<xmlattr>.visible");
		if(vis && !*vis)
			return false;

		if(!id || !lon || !lat)
			return false;

		t_vertex vertex
		{
			.longitude = *lon,
			.latitude = *lat,
		};

		if(auto tags = node.get_child_optional(""))
		{
			for(const auto& tag : *tags)
			{
				if(tag.first != "tag")
					continue;

				auto key = tag.second.get_optional<std::string>("<xmlattr>.k");
				auto val = tag.second.get_optional<std::string>("<xmlattr>.v");

				if(!key || !val)
					continue;

				//std::cout << *id << ": " << *key << " = " << *val << std::endl;
				vertex.tags.emplace(std::make_pair(*key, *val));
			}
		}

		// vertex ranges
		m_min_latitude = std::min(m_min_latitude, vertex.latitude);
		m_max_latitude = std::max(m_max_latitude, vertex.latitude);
		m_min_longitude = std::min(m_min_longitude, vertex.longitude);
		m_max_longitude = std::max(m_max_longitude, vertex.longitude);

		m_vertices.emplace(std::make_pair(*id, std::move(vertex)));
		return true;
	}



	bool ImportSegment(const boost::property_tree::ptree& node)
	{
		auto id = node.get_optional<t_size>("<xmlattr>.id");
		auto vis = node.get_optional<bool>("<xmlattr>.visible");
		if(vis && !*vis)
			return false;

		t_segment seg;
		bool is_background = false;

		if(auto tags = node.get_child_optional(""))
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

					if(*key == "landuse")
						is_background = true;

					//std::cout << *id << ": " << *key << " = " << *val << std::endl;
					seg.tags.emplace(std::make_pair(*key, *val));
				}
			}
		}

		if(seg.vertex_ids.size() >= 2 && *seg.vertex_ids.begin() == *seg.vertex_ids.rbegin())
			seg.is_area = true;
		if(is_background)
			m_segments_background.emplace(std::make_pair(*id, std::move(seg)));
		else
			m_segments.emplace(std::make_pair(*id, std::move(seg)));
		return true;
	}



	bool ImportMultiSegment(const boost::property_tree::ptree& node)
	{
		auto id = node.get_optional<t_size>("<xmlattr>.id");
		auto vis = node.get_optional<bool>("<xmlattr>.visible");
		if(vis && !*vis)
			return false;

		t_multisegment seg;

		if(auto tags = node.get_child_optional(""))
		{
			for(const auto& tag : *tags)
			{
				if(tag.first == "member")
				{
					auto seg_ty = tag.second.get_optional<std::string>("<xmlattr>.type");
					auto seg_ref = tag.second.get_optional<t_size>("<xmlattr>.ref");
					auto seg_role = tag.second.get_optional<std::string>("<xmlattr>.role");
					if(!seg_ty || !seg_ref)
						continue;

					if(*seg_ty == "node")
						seg.vertex_ids.push_back(*seg_ref);
					else if(*seg_ty == "way" && seg_role && *seg_role == "inner")
						seg.segment_inner_ids.push_back(*seg_ref);
					else if(*seg_ty == "way")
						seg.segment_ids.push_back(*seg_ref);

				}
				else if(tag.first == "tag")
				{
					auto key = tag.second.get_optional<std::string>("<xmlattr>.k");
					auto val = tag.second.get_optional<std::string>("<xmlattr>.v");

					if(!key || !val)
						continue;

					//std::cout << *id << ": " << *key << " = " << *val << std::endl;
					seg.tags.emplace(std::make_pair(*key, *val));
				}
			}
		}

		m_multisegments.emplace(std::make_pair(*id, std::move(seg)));
		return true;
	}



public:
	/**
	 * import a map from an osm file
	 * @see https://wiki.openstreetmap.org/wiki/OSM_XML
	 * @see https://wiki.openstreetmap.org/wiki/Elements
	 */
	bool Import(const std::string& mapname)
	{
		namespace fs = __map_fs;
		namespace ptree = boost::property_tree;

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

		// reset vertex ranges
		m_min_latitude = std::numeric_limits<t_real>::max();
		m_max_latitude = -m_min_latitude;
		m_min_longitude = std::numeric_limits<t_real>::max();
		m_max_longitude = -m_min_latitude;

		const auto& map_nodes = osm->get_child_optional("");
		if(!map_nodes)
			return false;

		for(const auto& node : *map_nodes)
		{
			//std::cout << name << std::endl;

			// node is a vertex
			if(node.first == "node")
				ImportVertex(node.second);

			// node is a line segment or area
			else if(node.first == "way")
				ImportSegment(node.second);

			// node is a relation
			else if(node.first == "relation")
				ImportMultiSegment(node.second);
		}  // node iteration

		return true;
	}



	/**
	 * @see https://wiki.openstreetmap.org/wiki/Key:surface
	 */
	std::tuple<bool, std::string>
	GetSurfaceColour(const std::string& key, const std::string& val,
		const std::string& def_col) const
	{
		if(key == "surface" && val == "asphalt")
			return std::make_tuple(true, "#222222");
		else if(key == "surface" && val == "concrete")
			return std::make_tuple(true, "#333333");
		else if(key == "natural" && val == "shingle")
			return std::make_tuple(true, "#5555ff");
		else if((key == "natural" || key == "surface") && val == "wood")
			return std::make_tuple(true, "#009900");
		else if(key == "natural" && val == "water")
			return std::make_tuple(true, "#4444ff");
		else if(key == "landuse" && val == "residential")
			return std::make_tuple(true, "#aaaaaa");
		else if(key == "landuse" && val == "retail")
			return std::make_tuple(true, "#ff4444");
		else if(key == "landuse" && val == "industrial")
			return std::make_tuple(true, "#4444ff");
		else if(key == "landuse" && val == "forest")
			return std::make_tuple(true, "#009900");
		else if(key == "landuse" && (val == "grass" || val == "meadow"))
			return std::make_tuple(true, "#44ff44");
		else if(key == "waterway" && val == "river")
			return std::make_tuple(true, "#5555ff");
		else if(key == "building" /*&& val == "yes"*/)
			return std::make_tuple(true, "#dddddd");
		else if(key == "leisure" && (val == "park" || val == "garden"))
			return std::make_tuple(true, "#55ff55");
		else if(key == "leisure" && val == "pitch")
			return std::make_tuple(true, "#55bb55");

		return std::make_tuple(false, def_col);
	}



	/**
	 * @see https://wiki.openstreetmap.org/wiki/Key:highway
	 */
	std::tuple<bool, t_real>
	GetLineWidth(const std::string& key, const std::string& val,
		t_real def_line_width) const
	{
		if(key == "highway" && val == "motorway")
			return std::make_tuple(true, 70.);
		else if(key == "highway" && val == "motorway_link")
			return std::make_tuple(true, 65.);
		else if(key == "highway" && val == "trunk")
			return std::make_tuple(true, 60.);
		else if(key == "highway" && val == "primary")
			return std::make_tuple(true, 50.);
		else if(key == "highway" && val == "secondary")
			return std::make_tuple(true, 40.);
		else if(key == "highway" && val == "tertiary")
			return std::make_tuple(true, 30.);
		else if(key == "highway" && val == "residential")
			return std::make_tuple(true, 20.);
		else if(key == "highway" && val == "track")
			return std::make_tuple(true, 10.);

		return std::make_tuple(false, def_line_width);
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

		t_svg svg(ofstr,
			64. / (m_max_longitude - m_min_longitude),
			64. / (m_max_latitude - m_min_latitude));


		// draw area
		std::unordered_set<t_size> seg_already_drawn;
		auto draw_seg = [this, &seg_already_drawn, &svg](t_size id, const t_segment *seg = nullptr)
		{
			auto id_iter = seg_already_drawn.find(id);
			if(id_iter != seg_already_drawn.end())
				return;
			seg_already_drawn.insert(id);

			if(!seg)
			{
				auto iter = m_segments.find(id);
				if(iter == m_segments.end())
					return;

				seg = &iter->second;
			}

			if(!seg->is_area)
				return;
			t_poly poly;

			for(const t_size vert_id : seg->vertex_ids)
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
			bool found = false;

			for(const auto& [ tag_key, tag_val ] : seg->tags)
			{
				//std::cout << id << ": " << tag_key << " = " << tag_val << std::endl;

				std::tie(found, fill_col) =
					GetSurfaceColour(tag_key, tag_val, "#ffffff");
				if(found)
					break;
			}

			svg.add(poly);
			svg.map(poly, "stroke:" + line_col +
				"; stroke-width:" + std::to_string(line_width) + "px" +
				"; fill:" + fill_col + ";", 1.);
		};

		// draw background areas
		for(const auto& [ id, seg ] : m_segments_background)
			draw_seg(id, &seg);

		// draw multi-areas
		for(const auto& [ multiseg_id, multiseg ] : m_multisegments)
		{
			for(const t_size id : multiseg.segment_ids)
				draw_seg(id);
			for(const t_size id : multiseg.segment_inner_ids)
				draw_seg(id);
		}

		// draw areas
		for(const auto& [ id, seg ] : m_segments)
			draw_seg(id, &seg);

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
			bool found_width = false, found_col = false;

			for(const auto& [ tag_key, tag_val ] : seg.tags)
			{
				//std::cout << id << ": " << tag_key << " = " << tag_val << std::endl;

				if(!found_width)
					std::tie(found_width, line_width) =
						GetLineWidth(tag_key, tag_val, 8.);

				if(!found_col)
					std::tie(found_col, line_col) =
						GetSurfaceColour(tag_key, tag_val, "#000000");

				if(found_width && found_col)
					break;
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
	std::unordered_map<t_size, t_segment> m_segments_background{};
	std::unordered_map<t_size, t_multisegment> m_multisegments{};

	t_real m_min_latitude{}, m_max_latitude{};
	t_real m_min_longitude{}, m_max_longitude{};
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
