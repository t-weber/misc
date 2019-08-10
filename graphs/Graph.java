/**
 * graph tests
 *
 * @author Tobias Weber
 * @date 10-aug-19
 * @license: see 'LICENSE.GPL' file
 */

import java.util.Queue;
import java.util.PriorityQueue;
import java.util.Vector;
import java.util.List;
import java.util.LinkedList;
import java.util.Set;
import java.util.HashSet;
import java.lang.Math;


public class Graph
{
	// pair
	public class Pair<t_first, t_second>
	{
		public t_first first;
		public t_second second;
		
		public Pair(t_first t1, t_second t2)
		{
			this.first = t1;
			this.second = t2;
		}
	}


	// vertex of a graph
	public class Vertex
	{
		private String name;

		public Vertex(String name)
		{
			this.name = name;
		}

		@Override public String toString() { return name; }
	}

	
	// edge of a graph
	public class Edge
	{
		private Vertex vertex_from, vertex_to;
		private Double dist = 0.;

		public Edge(Vertex vertex_from, Vertex vertex_to, Double dist)
		{
			this.vertex_from = vertex_from;
			this.vertex_to = vertex_to;
			this.dist = dist;
		}
		
		public Vertex getVertexFrom() { return vertex_from; }
		public Vertex getVertexTo() { return vertex_to; }
		public Double getDist() { return dist; }

		@Override public String toString()
		{
			if(vertex_from == null)
				return vertex_to.toString();
			if(vertex_to == null)
				return vertex_from.toString();

			return vertex_from.toString() + " -> " 
				+ vertex_to.toString() + " (" + dist + ")";
		}
	}

	
	private Vector<Vertex> m_vertices;
	private Vector<Edge> m_edges;
	boolean m_directed = false;	// directed graph?


	public Graph(boolean directed)
	{
		m_directed = directed;
		m_vertices = new Vector<Vertex>();
		m_edges = new Vector<Edge>();
	}


	public Graph(Vector<Vertex> vertices, Vector<Edge> edges, boolean directed)
	{
		this(directed);
		this.m_vertices = vertices;
		this.m_edges = edges;
	}


	public Vertex addVertex(String name)
	{
		Vertex v = new Vertex(name);
		m_vertices.add(v);
		return v;
	}

	
	public Edge addEdge(Vertex vert_from, Vertex vert_to, Double dist)
	{
		Edge e = new Edge(vert_from, vert_to, dist);
		m_edges.add(e);
		return e;
	}


	public Vector<Pair<Vertex, Double>> getNeighbours(Vertex v)
	{
		Vector<Pair<Vertex, Double>> vec = new Vector<Pair<Vertex, Double>>();

		for(Edge e : m_edges)
		{
			if(e.getVertexFrom() == v)
				vec.add(new Pair<Vertex, Double>(e.getVertexTo(), e.getDist()));
			// also look the other direction for undirected graphs
			else if(!m_directed && e.getVertexTo() == v)
				vec.add(new Pair<Vertex, Double>(e.getVertexFrom(), e.getDist()));
		}
		
		return vec;
	}


	public List<Edge> dijk_path(Vertex start)
	{
		// priority queue
		Queue<Edge> prio = new PriorityQueue<Edge>(64,
			(Edge d1, Edge d2) ->
			{
				final double eps = 1e-8;
				if(Math.abs(d1.getDist()-d2.getDist()) < eps) return 0;
				if(d1.getDist() < d2.getDist()) return -1;
				return 1;
			});
		
		// predecessor list
		List<Edge> predecessors = new LinkedList<Edge>();
		
		// visited vertices
		Set<Vertex> visited = new HashSet<Vertex>();

		// add start vertex, which has no predecessor
		prio.add(new Edge(null, start, 0.));

		// iterate priority queue
		Edge nextprio = null;
		while((nextprio = prio.poll()) != null)
		{
			if(visited.contains(nextprio.vertex_to))
				continue;
			visited.add(nextprio.vertex_to);
			predecessors.add(nextprio);
			
			for(Pair<Vertex, Double> pair : getNeighbours(nextprio.vertex_to))
			{
				Double dist = pair.second + nextprio.getDist();
				prio.add(new Edge(nextprio.vertex_to, pair.first, dist));
			}
		}
		
		return predecessors;
	}

	
	// like dijk(), but without priority queue, ignoring distances
	public List<Vertex> breadth_first_order(Vertex start)
	{
		// to be visited vertices
		Queue<Vertex> queue = new LinkedList<Vertex>();

		// visited vertices
		List<Vertex> visited = new LinkedList<Vertex>();

		// add start vertex
		queue.add(start);

		// iterate queue
		Vertex nextprio = null;
		while((nextprio = queue.poll()) != null)
		{
			if(visited.contains(nextprio))
				continue;
			visited.add(nextprio);
			
			for(Pair<Vertex, Double> pair : getNeighbours(nextprio))
				queue.add(pair.first);
		}
		
		return visited;
	}


	public void __depth_first_order(Vertex start, List<Vertex> visited)
	{
		// add start vertex
		visited.add(start);

		for(Pair<Vertex, Double> pair : getNeighbours(start))
		{
			if(visited.contains(pair.first))
				continue;
			__depth_first_order(pair.first, visited);
		}
	}

	
	public List<Vertex> depth_first_order(Vertex start)
	{
		// visited vertices
		List<Vertex> visited = new LinkedList<Vertex>();
		__depth_first_order(start, visited);
		return visited;
	}


	public static void main(String[] args)
	{
		Graph graph = new Graph(false);

		// vertices
		Graph.Vertex A = graph.addVertex("A");
		Graph.Vertex B = graph.addVertex("B");
		Graph.Vertex C = graph.addVertex("C");
		Graph.Vertex D = graph.addVertex("D");

		// edges
		graph.addEdge(A, B, 1.);
		graph.addEdge(A, D, 5.);
		graph.addEdge(B, C, 10.);
		graph.addEdge(B, D, 2.);
		graph.addEdge(D, C, 1.);
		graph.addEdge(C, A, 0.5);

		List<Edge> dists_dijk = graph.dijk_path(A);
		System.out.println("dijk: " + dists_dijk);

		List<Vertex> verts_breath = graph.breadth_first_order(A);
		System.out.println("breadth-first: " + verts_breath);

		List<Vertex> verts_depth = graph.depth_first_order(A);
		System.out.println("depth-first: " + verts_depth);
	}
}
