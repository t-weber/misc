/**
 * distance vector algo, see e.g. https://de.wikipedia.org/wiki/Distanzvektoralgorithmus
 *
 * @author Tobias Weber
 * @date 23-jun-19
 * @license: see 'LICENSE.EUPL' file
 */

#include <vector>
#include <tuple>
#include <unordered_map>
#include <limits>
#include <string>
#include <iostream>
#include <iomanip>
#include <thread>

#include <boost/signals2/signal.hpp>
namespace sig = boost::signals2;


using t_real = double;


class Node
{
private:
	const std::string& m_name;

	std::unordered_map<std::string, t_real> m_neighbours;
	std::unordered_map<std::string, t_real> m_route;

	using t_sigAnnounceRoute = sig::signal<void(const std::string&, const std::string&, t_real)>;
	t_sigAnnounceRoute m_sigAnnounceRoute;


public:
	Node(const std::string& name) : m_name(name)
	{
		// route to self
		m_neighbours[m_name] = 0.;
		m_route[m_name + "->" + m_name] = 0.;
	}


	void SetNeighbour(const std::string& name, t_real dist)
	{
		m_neighbours[name] = dist;
		m_route[m_name + "->" + name] = dist;
	}


	void AnnounceNeighbourDistances()
	{
		for(const auto& pair : m_neighbours)
		{
			m_sigAnnounceRoute(m_name, pair.first, pair.second);
		}
	}


	/**
	 * find minimal distance to target node
	 */
	t_real GetMinDistTo(const std::string& node)
	{
		t_real dist = std::numeric_limits<t_real>::max();

		for(const auto& pair : m_route)
		{
			std::size_t iPos = pair.first.find("->")+2;
			std::string nodeTo = pair.first.substr(iPos);
			if(nodeTo == node)
				dist = std::min(dist, pair.second);
		}

		return dist;
	}


	void ReceiveRoute(const std::string& nameVia, const std::string& nameTo, t_real dist)
	{
		// check if the sending node is a known neighbour and get the distance to it
		t_real distToSendingNode = 0.;

		if(auto iter = m_neighbours.find(nameVia); iter != m_neighbours.end())
		{
			distToSendingNode = iter->second;
		}
		else
		{
			// ignore message from non-neighbour node
			return;
		}


		std::string strRouteName = nameVia + "->" + nameTo;

		// does this route already exist?
		if(auto iter = m_route.find(strRouteName); iter != m_route.end())
		{
			// more efficient route found?
			if(distToSendingNode + dist < iter->second)
			{
				bool bNewMinDist = distToSendingNode + dist < GetMinDistTo(nameTo);

				// update distance
				iter->second = distToSendingNode + dist;

				std::cout << "Updated route for node " << m_name << ": "
					<< strRouteName << ", distance: " << distToSendingNode + dist
					<< std::endl;

				// announce updated route
				if(bNewMinDist)
					m_sigAnnounceRoute(m_name, nameTo, distToSendingNode + dist);
			}
		}
		// new route
		else
		{
			bool bNewMinDist = distToSendingNode + dist < GetMinDistTo(nameTo);

			m_route[strRouteName] = distToSendingNode + dist;

			std::cout << "New route for node " << m_name << ": "
				<< strRouteName << ", distance: " << distToSendingNode + dist
				<< std::endl;

			// announce new route
			if(bNewMinDist)
				m_sigAnnounceRoute(m_name, nameTo, distToSendingNode + dist);
		}
	}


	template<class t_slot>
	void ConnectToRouteAnnouncer(const t_slot& slot)
	{
		m_sigAnnounceRoute.connect(slot);
	}


	friend std::ostream& operator<<(std::ostream& ostr, const Node& node)
	{
		ostr << "Routing table for node " << node.m_name << "\n";

		for(const auto& pair : node.m_route)
			ostr << pair.first << ": " << pair.second << "\n";

		return ostr;
	}
};


int main()
{
	// example graph
	// vertices
	Node a("a");
	Node b("b");
	Node c("c");

	// edges
	a.SetNeighbour("b", 1.);
	b.SetNeighbour("a", 1.);
	b.SetNeighbour("c", 2.);
	c.SetNeighbour("b", 2.);


	// connect the signals of every node to every other node
	for(Node* node1 : {&a, &b, &c})
	{
		for(Node* node2 : {&a, &b, &c})
		{
			if(node1 == node2)
				continue;

			node1->ConnectToRouteAnnouncer(
				[node2](const std::string& nameVia, const std::string& nameTo, t_real dist)
				{ node2->ReceiveRoute(nameVia, nameTo, dist); });
		}
	}


	// begin annoucing initial distances
	for(Node* node : {&a, &b, &c})
		node->AnnounceNeighbourDistances();


	// print final routing tables
	std::cout << "\n";
	for(Node* node : {&a, &b, &c})
		std::cout << *node << std::endl;


	return 0;
}
