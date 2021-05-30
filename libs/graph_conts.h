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
#include <optional>

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


	std::size_t GetNumVertices() const
	{
		return m_mat.size1();
	}


	const std::string& GetVertexIdent(std::size_t i) const
	{
		return m_vertexidents[i];
	}


	std::optional<std::size_t> GetVertexIndex(const std::string& vert) const
	{
		auto iter = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert);
		if(iter == m_vertexidents.end())
			return std::nullopt;

		return iter - m_vertexidents.begin();
	}


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


	void SetWeight(std::size_t idx1, std::size_t idx2, t_weight w)
	{
		m_mat(idx1, idx2) = w;
	}


	void SetWeight(const std::string& vert1, const std::string& vert2, t_weight w)
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			SetWeight(*idx1, *idx2, w);
	}


	t_weight GetWeight(std::size_t idx1, std::size_t idx2) const
	{
		return m_mat(idx1, idx2);
	}


	t_weight GetWeight(const std::string& vert1, const std::string& vert2) const
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
		return GetWeight(*idx1, *idx2);

		return t_weight{0};
	}


	void AddEdge(const std::string& vert1, const std::string& vert2, t_weight w)
	{
		SetWeight(vert1, vert2, w);
	}


	void RemoveEdge(const std::string& vert1, const std::string& vert2)
	{
		SetWeight(vert1, vert2, t_weight{0});
	}


	bool IsAdjacent(std::size_t idx1, std::size_t idx2) const
	{
		return GetWeight(idx1, idx2) != t_weight{0};
	}


	bool IsAdjacent(const std::string& vert1, const std::string& vert2) const
	{
		return GetWeight(vert1, vert2) != t_weight{0};
	}


	std::vector<std::size_t> GetNeighbours(std::size_t idx) const
	{
		std::vector<std::size_t> neighbours;

		for(std::size_t idxOther=0; idxOther<m_mat.size2(); ++idxOther)
		{
			if(GetWeight(idx, idxOther))
				neighbours.push_back(idxOther);
		}

		return neighbours;
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


private:
	std::vector<std::string> m_vertexidents{};
	t_mat m_mat{};
};


#endif
