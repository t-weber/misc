/**
 * graph tests using Grenoble tram system
 * @author Tobias Weber
 * @date 03-dec-17
 * @license: see 'LICENSE' file
 *
 * References:
 *  * https://github.com/boostorg/graph/tree/develop/example
 *  * http://www.boost.org/doc/libs/1_65_1/libs/graph/doc/table_of_contents.html
 *
 * gcc -o graph_gre graph_gre.cpp -std=c++17 -lstdc++ -lm
 */

#include <iostream>
#include <fstream>
#include <string>
#include <unordered_set>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/algorithm/string.hpp>



// ----------------------------------------------------------------------------
// types

using t_real = double;

using t_col = boost::default_color_type;

struct t_vertex
{
	std::string name;
	//std::vector<std::string> lines;

	int idx = -1;

	t_vertex() = default;
	t_vertex(const std::string& strName) : name(strName) {}
};

struct t_edge
{
	t_edge() = default;
	t_edge(t_real weight) : weight(weight) {}

	t_real weight = 1.;
};

using t_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, t_vertex, t_edge>;
using t_vertex_iter = boost::graph_traits<t_graph>::vertex_iterator;
using t_edge_descr = boost::graph_traits<t_graph>::edge_descriptor;

// ----------------------------------------------------------------------------


/**
 * build Grenoble tram graph
 * map: https://upload.wikimedia.org/wikipedia/commons/2/20/Tram_Grenoble-01.svg
 */
t_graph mk_tram()
{
	t_graph tram;

	std::vector<t_vertex> line_A
	{{
		t_vertex("Fontaine - La Poya"),
		t_vertex("Charles Michels"),
		t_vertex("Fontaine Hôtel de Ville"),
		t_vertex("Louis Maisonnat"),
		t_vertex("Les Fontainades"),
		t_vertex("Berriat Le Magasin"),
		t_vertex("Saint-Bruno"),
		t_vertex("Gares"),
		t_vertex("Alsace-Lorraine"),
		t_vertex("Victor Hugo"),
		t_vertex("Hubert Dubedout - Maison du Tourisme"),
		t_vertex("Verdun Préfecture"),
		t_vertex("Chavant"),
		t_vertex("Albert 1er de Belgique"),
		t_vertex("Mounier"),
		t_vertex("MC2 - Maison de la Culture"),
		t_vertex("Malherbe"),
		t_vertex("La Bruyère"),
		t_vertex("Arlequin"),
		t_vertex("Grand’ Place"),
		t_vertex("Pôle Sud - Alpexpo"),
		t_vertex("Les Granges"),
		t_vertex("Surieux"),
		t_vertex("Essarts - La Butt"),
		t_vertex("Échirolles - Gare"),
		t_vertex("La Rampe - Centre-ville"),
		t_vertex("Marie Curie"),
		t_vertex("Auguste Delaune"),
		t_vertex("Échirolles - Denis Papin"),
	}};

	std::vector<t_vertex> line_B
	{{
		t_vertex("Grenoble - Presqu'île"),
		t_vertex("CEA - Cambridge"),
		t_vertex("Cité Internationale"),
		t_vertex("Palais de Justice"),
		t_vertex("Saint-Bruno"),
		t_vertex("Gares"),
		t_vertex("Alsace-Lorraine"),
		t_vertex("Victor Hugo"),
		t_vertex("Hubert Dubedout - Maison du Tourisme"),
		t_vertex("Sainte-Claire - Les Halles"),
		t_vertex("Notre-Dame Musée"),
		t_vertex("Île Verte"),
		t_vertex("La Tronche - Hôpital"),
		t_vertex("Michallon"),
		t_vertex("Grand Sablon"),
		t_vertex("Saint-Martin-d’Hères - Les Taillés Universités"),
		t_vertex("Gabriel Fauré"),
		t_vertex("Bibliothèques Universitaires"),
		t_vertex("Saint-Martin-d’Hères - Condillac Universités"),
		t_vertex("Mayencin - Champ Roman"),
		t_vertex("Gières Gare - Universités"),
		t_vertex("Gières - Plaine des Sports"),
	}};

	std::vector<t_vertex> line_C
	{{
		t_vertex("Seyssins - Le Prisme"),
		t_vertex("Mas des Îles"),
		t_vertex("Grand Pré"),
		t_vertex("Fauconnière"),
		t_vertex("Seyssinet-Pariset Hôtel de Ville"),
		t_vertex("Vallier - Catane"),
		t_vertex("Vallier - Dr Calmette"),
		t_vertex("Vallier - Libération"),
		t_vertex("Foch-Ferrié"),
		t_vertex("Gustave Rivet"),
		t_vertex("Chavant"),
		t_vertex("Grenoble Hôtel de Ville"),
		t_vertex("Flandrin Valmy"),
		t_vertex("Péri Brossolette"),
		t_vertex("Neyrpic - Belledonne"),
		t_vertex("Hector Berlioz Universités"),
		t_vertex("Gabriel Fauré"),
		t_vertex("Bibliothèques Universitaires"),
		t_vertex("Saint-Martin-d’Hères - Condillac Universités"),
	}};

	std::vector<t_vertex> line_D
	{{
		t_vertex("Saint-Martin-d’Hères - Les Taillés Universités"),
		t_vertex("Neyrpic - Belledonne"),
		t_vertex("Maison Communale"),
		t_vertex("Édouard Vaillant"),
		t_vertex("Parc Jo Blanchon"),
		t_vertex("Saint-Martin-d’Hères - Etienne Grappe"),
	}};

	std::vector<t_vertex> line_E
	{{
		t_vertex("Le Fontanil - Palluel"),
		t_vertex("Rafour"),
		t_vertex("Karben"),
		t_vertex("La Pinéa - Saint-Robert"),
		t_vertex("Pont de Vence"),
		t_vertex("Muret"),
		t_vertex("Fiancey - Prédieu"),
		t_vertex("Néron"),
		t_vertex("Horloge"),
		t_vertex("Saint-Martin-Le-Vinoux Hôtel de Ville"),
		t_vertex("Casamaures Village"),
		t_vertex("Esplanade"),
		t_vertex("Alsace-Lorraine"),
		t_vertex("Condorcet"),
		t_vertex("Vallier - Libération"),
		t_vertex("Alliés"),
		t_vertex("Grenoble - Louise Michel"),
	}};


	auto lines = std::vector<decltype(&line_A)>{{&line_A, &line_B, &line_C, &line_D, &line_E}};


	auto find_station_idx = [&lines](const std::string& strName) -> int
	{
		for(auto& line : lines)
		{
			for(auto& vert : *line)
			{
				if(vert.name == strName && vert.idx >= 0)
					return vert.idx;
			}
		}

		return -1;
	};


	// add vertices
	std::unordered_set<std::string> setAlreadySeen;

	int iIdx = 0;
	for(auto& line : lines)
	{
		for(auto& vert : *line)
		{
			if(setAlreadySeen.find(vert.name) == setAlreadySeen.end())
			{
				boost::add_vertex(vert, tram);
				setAlreadySeen.insert(vert.name);
				vert.idx = iIdx++;
			}
		}
	}


	// connect stations of the lines
	for(const auto& line : lines)
	{
		for(auto iter=line->begin(); iter!=line->end(); ++iter)
		{
			auto iterNext = std::next(iter);
			if(iterNext == line->end())
				break;

			int iIdx = iter->idx;
			int iIdxNext = iterNext->idx;

			// shared station?
			if(iIdx < 0) iIdx = find_station_idx(iter->name);
			if(iIdxNext < 0) iIdxNext = find_station_idx(iterNext->name);

			if(iIdx < 0) std::cerr << "Invalid station: " << iter->name << std::endl;
			if(iIdxNext < 0) std::cerr << "Invalid station: " << iterNext->name << std::endl;

			if(auto [edge, bInserted] = boost::add_edge(boost::vertex(iIdx, tram),
				boost::vertex(iIdxNext, tram), t_edge(1.), tram); !bInserted)
			{
				std::cerr << "Edge (" << iIdx << ", " << iIdxNext
					<< ") not inserted!" << std::endl;
			}
		}
	}


/*	// old code to connect lines at common stations
	for(std::size_t iLine=0; iLine<lines.size(); ++iLine)
	{
		const auto& line = lines[iLine];

		for(auto iter=line->begin(); iter!=line->end(); ++iter)
		{
			const std::string& strName = iter->name;

			for(std::size_t iLineOther=iLine+1; iLineOther<lines.size(); ++iLineOther)
			{
				const auto& lineOther = lines[iLineOther];

				auto iterOther = std::find_if(lineOther->begin(), lineOther->end(),
					[&strName](const t_vertex& vert) -> bool
						{ return vert.name == strName; });

				// stations with the same name on both lines?
				if(iterOther != lineOther->end())
				{
					//std::cout << "Connecting station: " << strName << "\n";
					// connect stations
					int iIdx = iter->idx;
					int iIdxOther = iterOther->idx;
					if(auto [edge, bInserted] = boost::add_edge(boost::vertex(iIdx, tram),
						boost::vertex(iIdxOther, tram), t_edge(1.), tram); !bInserted)
					{
						std::cerr << "Edge (" << iIdx << ", " << iIdxOther
							<< ") not inserted!" << std::endl;
					}
				}
			}
		}
	}*/


	return tram;
}


int main()
{
	std::ios_base::sync_with_stdio(false);

	t_graph graph = mk_tram();


	// access to members
	auto vert_idx = boost::get(boost::vertex_index, graph);
	auto name = boost::get(&t_vertex::name, graph);
	auto weight = boost::get(&t_edge::weight, graph);



	// write graph to an svg file
	std::ofstream ofgraph("gre.graph");
	boost::write_graphviz(ofgraph, graph, boost::make_label_writer(name));
	std::system("dot -Tsvg tst.graph -o gre.svg");



	// get station index from name
	auto find_station_idx = [&graph, &name, &vert_idx](const std::string& str) -> int
	{
		for(auto [vert_iter, vert_iter_end] = boost::vertices(graph);
			vert_iter!=vert_iter_end; ++vert_iter)
		{
			const std::string& _strName = boost::get(name, *vert_iter);
			std::string strName = boost::to_lower_copy(_strName);

			int iIdx = boost::get(vert_idx, *vert_iter);

			if(strName.find(str) != std::string::npos)
				return iIdx;
		}

		return -1;
	};


	// shortest path
	while(1)
	{
		std::string strStart, strEnd;
		int idx_start=-1, idx_end=-1;

		while(1)
		{
			std::cout << "Starting at station: ";
			std::cin >> strStart;
			boost::trim(strStart);
			boost::to_lower(strStart);

			idx_start = find_station_idx(strStart);
			if(idx_start >= 0)
				break;
			std::cerr << "Station not found!" << std::endl;
		}

		while(1)
		{
			std::cout << "Ending at station: ";
			std::cin >> strEnd;
			boost::trim(strEnd);
			boost::to_lower(strEnd);

			idx_end = find_station_idx(strEnd);
			if(idx_end >= 0)
				break;
			std::cerr << "Station not found!" << std::endl;
		}


		std::vector<int> vecPred(boost::num_vertices(graph));
		dijkstra_shortest_paths(graph, boost::vertex(idx_end, graph),
			boost::predecessor_map(boost::make_iterator_property_map(vecPred.begin(), vert_idx)).
			weight_map(weight));

		std::cout << "\nShortest way from \"" << name[idx_start]
			<< "\" to \"" << name[idx_end] << "\":\n";

		std::cout << "----------------------------------------\n";
		int idx_cur = idx_start;
		std::size_t iStep = 1;
		while(1)
		{
			std::cout << "(" << (iStep++) << ") " << name[idx_cur] << "\n";
			if(idx_cur == idx_end)
				break;
			idx_cur = vecPred[idx_cur];
		}
		std::cout << "----------------------------------------\n";
		std::cout << "\n";
	}


	return 0;
}
