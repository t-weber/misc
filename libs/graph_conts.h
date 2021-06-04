/**
 * graph containers
 * @author Tobias Weber (orcid: 0000-0002-7230-1932)
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
#include <memory>
#include <optional>
#include <type_traits>

#include "math_conts.h"
#include "math_algos.h"


template<class T, class t_val = typename std::tuple_element<0, T>::type>
constexpr bool is_pair =
	std::is_same_v<T, std::pair<t_val, t_val>> ||
	std::is_same_v<T, std::tuple<t_val, t_val>>;


/**
 * adjacency matrix
 * @see (FUH 2021), Kurseinheit 4, pp. 3-5
 * @see https://en.wikipedia.org/wiki/Adjacency_matrix
 */
template<class _t_data = unsigned int, class _t_weight = _t_data>
class adjacency_matrix
{
public:
	using t_data = _t_data;
	using t_weight = _t_weight;
	using t_mat = m::mat<t_data, std::vector>;


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


	void RemoveVertex(std::size_t idx)
	{
		m_mat = m::submat<t_mat>(m_mat, idx, idx);
		m_vertexidents.erase(m_vertexidents.begin() + idx);
	}


	void RemoveVertex(const std::string& id)
	{
		std::size_t idx = GetVertexIndex(id);
		RemoveVertex(idx);
	}


	/**
	 * set weight (or flux)
	 */
	void SetWeight(std::size_t idx1, std::size_t idx2, t_weight w)
	{
		if constexpr(is_pair<t_data, t_weight>)
			std::get<0>(m_mat(idx1, idx2)) = w;
		else
			m_mat(idx1, idx2) = w;
	}


	/**
	 * set weight (or flux)
	 */
	void SetWeight(const std::string& vert1, const std::string& vert2, t_weight w)
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			SetWeight(*idx1, *idx2, w);
	}


	/**
	 * get weight (or flux)
	 */
	t_weight GetWeight(std::size_t idx1, std::size_t idx2) const
	{
		if constexpr(is_pair<t_data, t_weight>)
			return std::get<0>(m_mat(idx1, idx2));
		else
			return m_mat(idx1, idx2);
	}


	/**
	 * get weight (or flux)
	 */
	t_weight GetWeight(const std::string& vert1, const std::string& vert2) const
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			return GetWeight(*idx1, *idx2);

		return t_weight{};
	}


	/**
	 * set capacity (if t_data is a pair)
	 */
	void SetCapacity(std::size_t idx1, std::size_t idx2, t_weight c)
	{
		if constexpr(is_pair<t_data, t_weight>)
			std::get<1>(m_mat(idx1, idx2)) = c;
	}


	/**
	 * set capacity (if t_data is a pair)
	 */
	void SetCapacity(const std::string& vert1, const std::string& vert2, t_weight c)
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			SetCapacity(*idx1, *idx2, c);
	}


	/**
	 * get capacity (if t_data is a pair)
	 */
	t_weight GetCapacity(std::size_t idx1, std::size_t idx2) const
	{
		if constexpr(is_pair<t_data, t_weight>)
			return std::get<1>(m_mat(idx1, idx2));
		else
			return t_weight{};
	}


	/**
	 * get capacity (if t_data is a pair)
	 */
	t_weight GetCapacity(const std::string& vert1, const std::string& vert2) const
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			return GetWeight(*idx1, *idx2);

		return t_weight{};
	}


	void AddEdge(std::size_t idx1, std::size_t idx2, t_weight w=0)
	{
		SetWeight(idx1, idx2, w);
	}


	void AddEdge(const std::string& vert1, const std::string& vert2, t_weight w=0)
	{
		SetWeight(vert1, vert2, w);
	}


	std::vector<std::tuple<std::size_t, std::size_t, t_data>> GetEdges() const
	{
		std::vector<std::tuple<std::size_t, std::size_t, t_data>> edges;
		edges.reserve(m_mat.size1() * m_mat.size2());

		for(std::size_t i=0; i<m_mat.size1(); ++i)
		{
			for(std::size_t j=0; j<m_mat.size2(); ++j)
			{
				// include edges which have a capacity
				if constexpr(is_pair<t_data, t_weight>)
				{
					if(GetCapacity(i, j))
						edges.emplace_back(std::make_tuple(i, j, m_mat(i, j)));
				}

				// include edges which have a weight
				else
				{
					if(GetWeight(i, j))
						edges.emplace_back(std::make_tuple(i, j, m_mat(i, j)));
				}
			}
		}

		return edges;
	}


	void RemoveEdge(const std::string& vert1, const std::string& vert2)
	{
		SetWeight(vert1, vert2, t_weight{});
	}


	bool IsAdjacent(std::size_t idx1, std::size_t idx2) const
	{
		return GetWeight(idx1, idx2) != t_weight{};
	}


	bool IsAdjacent(const std::string& vert1, const std::string& vert2) const
	{
		return GetWeight(vert1, vert2) != t_weight{};
	}


	std::vector<std::size_t> GetNeighbours(std::size_t idx, bool outgoing_edges=true) const
	{
		std::vector<std::size_t> neighbours;

		// neighbour vertices on outgoing edges
		if(outgoing_edges)
		{
			for(std::size_t idxOther=0; idxOther<m_mat.size2(); ++idxOther)
			{
				if(GetWeight(idx, idxOther))
					neighbours.push_back(idxOther);
			}
		}

		// neighbour vertices on incoming edges
		else
		{
			for(std::size_t idxOther=0; idxOther<m_mat.size1(); ++idxOther)
			{
				if(GetWeight(idxOther, idx))
					neighbours.push_back(idxOther);
			}
		}

		return neighbours;
	}


	std::vector<std::string> GetNeighbours(const std::string& vert, bool outgoing_edges=true) const
	{
		auto iter = std::find(m_vertexidents.begin(), m_vertexidents.end(), vert);
		if(iter == m_vertexidents.end())
			return {};
		std::size_t idx = iter - m_vertexidents.begin();

		std::vector<std::string> neighbours;

		// neighbour vertices on outgoing edges
		if(outgoing_edges)
		{
			for(std::size_t idxOther=0; idxOther<m_mat.size2(); ++idxOther)
			{
				if(GetWeight(idx, idxOther))
					neighbours.push_back(m_vertexidents[idxOther]);
			}
		}

		// neighbour vertices on incoming edges
		else
		{
			for(std::size_t idxOther=0; idxOther<m_mat.size1(); ++idxOther)
			{
				if(GetWeight(idxOther, idx))
					neighbours.push_back(m_vertexidents[idxOther]);
			}
		}

		return neighbours;
	}


private:
	std::vector<std::string> m_vertexidents{};
	t_mat m_mat{};
};



/**
 * adjacency list
 * @see (FUH 2021), Kurseinheit 4, pp. 3-5
 * @see https://en.wikipedia.org/wiki/Adjacency_list
 */
template<class _t_weight = unsigned int>
class adjacency_list
{
public:
	using t_weight = _t_weight;


public:
	adjacency_list() = default;
	~adjacency_list() = default;


	std::size_t GetNumVertices() const
	{
		return m_vertexidents.size();
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
		m_vertexidents.push_back(id);
		m_nodes.push_back(nullptr);
	}


	void RemoveVertex(std::size_t idx)
	{
		m_vertexidents.erase(m_vertexidents.begin() + idx);
		m_nodes.erase(m_nodes.begin() + idx);

		for(std::size_t idx1=0; idx1<m_nodes.size(); ++idx1)
		{
			std::shared_ptr<AdjNode> node = m_nodes[idx1];
			std::shared_ptr<AdjNode> node_prev;

			while(node)
			{
				if(node->idx == idx)
				{
					if(node_prev)
						node_prev->next = node->next;
					else
						m_nodes[idx1] = node->next;
					break;
				}

				node_prev = node;
				node = node->next;
			}
		}
	}


	void RemoveVertex(const std::string& id)
	{
		std::size_t idx = GetVertexIndex(id);
		RemoveVertex(idx);
	}


	void SetWeight(std::size_t idx1, std::size_t idx2, t_weight w)
	{
		std::shared_ptr<AdjNode> node = m_nodes[idx1];

		while(node)
		{
			if(node->idx == idx2)
			{
				node->weight = w;
				break;
			}

			node = node->next;
		}
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
		std::shared_ptr<AdjNode> node = m_nodes[idx1];

		while(node)
		{
			if(node->idx == idx2)
				return node->weight;

			node = node->next;
		}

		return t_weight{};
	}


	t_weight GetWeight(const std::string& vert1, const std::string& vert2) const
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);

		if(idx1 && idx2)
			return GetWeight(*idx1, *idx2);

		return t_weight{};
	}


	void AddEdge(std::size_t idx1, std::size_t idx2, t_weight w=0)
	{
		std::shared_ptr<AdjNode> node = m_nodes[idx1];
		m_nodes[idx1] = std::make_shared<AdjNode>();
		m_nodes[idx1]->next = node;
		m_nodes[idx1]->idx = idx2;
		m_nodes[idx1]->weight = w;
	}


	void AddEdge(const std::string& vert1, const std::string& vert2, t_weight w=0)
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);
		if(!idx1 || !idx2)
			return;

		AddEdge(*idx1, *idx2, w);
	}


	void RemoveEdge(const std::string& vert1, const std::string& vert2)
	{
		auto idx1 = GetVertexIndex(vert1);
		auto idx2 = GetVertexIndex(vert2);
		if(!idx1 || !idx2)
			return;

		std::shared_ptr<AdjNode> node = m_nodes[*idx1];
		std::shared_ptr<AdjNode> node_prev;

		while(node)
		{
			if(node->idx == *idx2)
			{
				if(node_prev)
					node_prev->next = node->next;
				else
					m_nodes[*idx1] = node->next;
				break;
			}

			node_prev = node;
			node = node->next;
		}
	}


	bool IsAdjacent(std::size_t idx1, std::size_t idx2) const
	{
		return GetWeight(idx1, idx2) != t_weight{};
	}


	bool IsAdjacent(const std::string& vert1, const std::string& vert2) const
	{
		return GetWeight(vert1, vert2) != t_weight{};
	}


	std::vector<std::size_t> GetNeighbours(std::size_t idx, bool outgoing_edges=true) const
	{
		std::vector<std::size_t> neighbours;
		neighbours.reserve(GetNumVertices());

		// neighbour vertices on outgoing edges
		if(outgoing_edges)
		{
			std::shared_ptr<AdjNode> node = m_nodes[idx];

			while(node)
			{
				neighbours.push_back(node->idx);
				node = node->next;
			}
		}

		// neighbour vertices on incoming edges
		else
		{
			for(std::size_t i=0; i<m_nodes.size(); ++i)
			{
				std::shared_ptr<AdjNode> node = m_nodes[i];

				while(node)
				{
					if(node->idx == idx)
					{
						neighbours.push_back(i);
						break;
					}
					node = node->next;
				}
			}
		}

		return neighbours;
	}


	std::vector<std::string> GetNeighbours(const std::string& vert, bool outgoing_edges=true) const
	{
		std::size_t idx = GetVertexIndex(vert);
		std::vector<std::size_t> neighbour_indices = GetNeighbours(idx, outgoing_edges);

		std::vector<std::string> neighbours;
		neighbours.reserve(neighbour_indices.size());

		for(std::size_t neighbour_index : neighbour_indices)
		{
			const std::string& id = GetVertexIdent(neighbour_index);
			neighbours.push_back(id);
		}

		return neighbours;
	}


private:
	struct AdjNode
	{
		std::size_t idx{};
		t_weight weight{};

		std::shared_ptr<AdjNode> next{};
	};

	std::vector<std::string> m_vertexidents{};
	std::vector<std::shared_ptr<AdjNode>> m_nodes{};
};


#endif
