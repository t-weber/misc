/**
 * shortest path in graph
 * @author Tobias Weber
 * @date 15-jun-19
 * @license: see 'LICENSE.GPL' file
 */

import java.util.*;


public class dijk
{
	private boolean m_debug = false;

	public void SetDebug(boolean debug) { m_debug = debug; }


	// edge of a graph
	public class Edge
	{
		public String vertex_from, vertex_to;
		public Double dist;

		public Edge(String vertex_from, String vertex_to, Double dist)
		{
			this.vertex_from = vertex_from;
			this.vertex_to = vertex_to;
			this.dist = dist;
		}
	}


	// entry into distance map
	public class Dist
	{
		public String vertex, predecessor;
		public Double dist;

		public Dist(String vertex, String predecessor, Double dist)
		{
			this.vertex = vertex;
			this.predecessor = predecessor;
			this.dist = dist;
		}
	}


	/**
	 * calculates the shortest path
	 * @param edge edges of the graph
	 * @param startvertex starting point
	 */
	public Map<String, Dist> calc(Vector<Edge> edges, String startvertex)
	{
		String vertcur = startvertex;
		Double curdist = 0.;
		int curiter = 1;
		Map<String, Dist> distmap = new HashMap<String, Dist>();


		// get (unvisited) vertices from the edge endpoints
		Set<String> unvisited = new HashSet<String>();
		Set<String> visited = new HashSet<String>();
		for(Edge edge : edges)
		{
			unvisited.add(edge.vertex_from);
			unvisited.add(edge.vertex_to);
		}

	
		while(unvisited.size() != 0)
		{
			// iterate all paths starting from current vertex
			for(Edge edge : edges)
			{
				if(!edge.vertex_from.equals(vertcur))
					continue;
				
				Dist distto = distmap.get(edge.vertex_to);
				if(distto == null)
				{
					// new entry
					distmap.put(edge.vertex_to, new Dist(edge.vertex_to, vertcur, curdist+edge.dist));
				}
				else
				{
					// update existing entry if the distance is lower
					if(curdist+edge.dist < distto.dist)
					{
						distto.vertex = edge.vertex_to;
						distto.predecessor = vertcur;
						distto.dist = curdist + edge.dist;
					}
				}
			}


			// mark current vertex as visited
			visited.add(vertcur);
			unvisited.remove(vertcur);


			// find closest unvisited vertex
			Double closestdist = Double.MAX_VALUE;
			for(String vert : unvisited)
			{
				Dist distto = distmap.get(vert);
				if(distto == null)
					continue;

				if(distto.dist < closestdist)
				{
					curdist = closestdist = distto.dist;
					vertcur = vert;
				}
			}


			// output
			if(m_debug)
			{
				System.out.println("Iteration " + curiter++);
				System.out.println("Current vertex: " + vertcur);

				System.out.print("Visited:");
				for(String vertex : visited)
					System.out.print(" " + vertex);
				System.out.println();

				System.out.printf("%15s %15s %15s\n", 
					"Vertex", 
					"Distance", 
					"Predecessor");

				for(Map.Entry<String, Dist> dist : distmap.entrySet())
				{
					System.out.printf("%15s %15s %15s\n", 
						dist.getKey(), 
						dist.getValue().dist, 
						dist.getValue().predecessor);
				}
				System.out.println();
			}
		}


		return distmap;
	}


	public static void main(String[] args)
	{
		dijk d = new dijk();
		d.SetDebug(true);

		// edges
		Vector<Edge> edges = new Vector<Edge>();
		edges.add(d.new Edge("A", "B", 1.));
		edges.add(d.new Edge("A", "D", 5.));
		edges.add(d.new Edge("B", "C", 10.));
		edges.add(d.new Edge("B", "D", 2.));
		edges.add(d.new Edge("D", "C", 1.));

		d.calc(edges, "A");
	}
}
