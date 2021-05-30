/**
 * graph containers
 * @author Tobias Weber
 * @date may-2021
 * @license see 'LICENSE.EUPL' file
 *
 * references:
 *   - (FUH 2021) "Effiziente Algorithmen" (2021), Kurs 1684, Fernuni Hagen
 *                (https://vu.fernuni-hagen.de/lvuweb/lvu/app/Kurs/01684).
 */

#ifndef __GRAPH_CONTS_H__
#define __GRAPH_CONTS_H__

#include <string>
#include <vector>
#include <iostream>

#include "math_conts.h"
#include "math_algos.h"


/**
 * adjacency matrix
 * @see (FUH 2021), Kurseinheit 4, pp. 3-5
 * @see https://en.wikipedia.org/wiki/Adjacency_matrix
 */
template<class _t_weight = unsigned int>
class adjacency_matrix
{
public:
	using t_weight = _t_weight;
	using t_mat = m::mat<t_weight, std::vector>;


public:
	adjacency_matrix() = default;
	~adjacency_matrix() = default;


	void AddVertex(const std::string& id)
	{
		t_mat matNew = m::zero<t_mat>(m_mat.size1()+1, m_mat.size2()+1);

		for(std::size_t i=0; i<m_mat.size1(); ++i)
			for(std::size_t j=0; j<m_mat.size2(); ++j)
				matNew(i,j) = m_mat(i,j);

		m_mat = std::move(matNew);
		m_vertexidents.push_back(id);
	}


	void RemoveVertex(const std::string& id)
	{
		auto iter = std::find(m_vertexidents.begin(), m_vertexidents.end(), id);
		if(iter == m_vertexidents.end())
			return;

		m_vertexidents.erase(iter);
	}


	void SetWeight(const std::string& vert1, const std::string& vert2, t_weight w)
	{
		auto iter1 = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert1);
		if(iter1 == m_vertexidents.end())
			return;

		auto iter2 = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert2);
		if(iter2 == m_vertexidents.end())
			return;

		std::size_t idx1 = iter1 - m_vertexidents.begin();
		std::size_t idx2 = iter2 - m_vertexidents.begin();

		m_mat(idx1, idx2) = w;
	}


	t_weight GetWeight(const std::string& vert1, const std::string& vert2) const
	{
		auto iter1 = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert1);
		if(iter1 == m_vertexidents.end())
			return t_weight{};

		auto iter2 = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert2);
		if(iter2 == m_vertexidents.end())
			return t_weight{};

		std::size_t idx1 = iter1 - m_vertexidents.begin();
		std::size_t idx2 = iter2 - m_vertexidents.begin();

		return m_mat(idx1, idx2);
	}


	void AddEdge(const std::string& vert1, const std::string& vert2, t_weight w)
	{
		SetWeight(vert1, vert2, w);
	}


	void RemoveEdge(const std::string& vert1, const std::string& vert2)
	{
		SetWeight(vert1, vert2, t_weight{0});
	}


	bool IsAdjacent(const std::string& vert1, const std::string& vert2) const
	{
		return GetWeight(vert1, vert2) != t_weight{0};
	}


	std::vector<std::string> GetNeighbours(const std::string& vert) const
	{
		std::vector<std::string> neighbours;

		auto iter = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert);
		if(iter == m_vertexidents.end())
			return neighbours;

		std::size_t idx = iter - m_vertexidents.begin();
		for(std::size_t idxOther=0; idxOther<m_mat.size2(); ++idxOther)
		{
			if(GetWeight(idx, idxOther))
				neighbours.push_back(m_vertexidents[idxOther]);
		}

		return neighbours;
	}


	/**
	 * export to dot
	 * @see https://graphviz.org/doc/info/lang.html
	 */
	void Print(std::ostream& ostr = std::cout) const
	{
		ostr << "digraph my_graph\n{\n";

		ostr << "\t// vertices\n";
		for(std::size_t i=0; i<m_vertexidents.size(); ++i)
			ostr << "\t" << i << " [label=\"" << m_vertexidents[i] << "\"];\n";

		ostr << "\n";
		ostr << "\t// edges and weights\n";
		for(std::size_t i=0; i<m_mat.size1(); ++i)
		{
			for(std::size_t j=0; j<m_mat.size2(); ++j)
			{
				t_weight w = m_mat(i, j);
				if(!w)
					continue;

				ostr << "\t" << i << " -> " << j << " [label=\"" << w << "\"];\n";
			}
		}

		ostr << "}\n";
	}


private:
	std::vector<std::string> m_vertexidents{};
	t_mat m_mat{};
};


#endif
