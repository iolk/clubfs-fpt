#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <ios>
#include <unordered_map>
#include <unordered_set>
#include <list>
#include <math.h>
#include <bitset>
#include <cstdint>
#include "includes/ClusterSet.h"

typedef struct
{
	int cluster;
	std::unordered_set<int> adj;
} node_data;

typedef struct
{
	std::unordered_map<int, node_data> graph;
	std::unordered_map<int, std::unordered_set<int>> cluster_map;
	std::unordered_set<int> cluster_set;
	std::vector<int> cluster_vec;
	std::unordered_map<int, int> s2v_map;
	std::vector<int> lvv;
} ClusterGraph;

/**
 * BFS_Vi[s]
 */
int BFS_Vi(ClusterGraph &g, int s)
{
	std::vector<int> visited((int)g.graph.size());
	std::list<int> queue;

	queue.push_back(s);
	visited[s] = 1;
	int i_cluster = g.graph[s].cluster;
	int cost = 0;

#ifdef DEBUG
	std::cout << "### BFS START ###" << std::endl;
	std::cout << "start node: " << s << std::endl;
	std::cout << "cluster: " << i_cluster << std::endl;
#endif

	while (!queue.empty())
	{
		int p = queue.front();
		queue.pop_front();
		cost += visited[p] - 1;

#ifdef DEBUG
		for (int k = 0; k < visited[p]; k++)
			std::cout << "--";
		std::cout << p << " d(" << s << "," << p << "):" << visited[p] - 1 << std::endl;
#endif

		for (int q : g.graph[p].adj)
		{
			// std::cout << g.graph[q].cluster << " " << i_cluster << " " << g.lvv[s] << std::endl;
			if (g.graph[q].cluster != i_cluster && g.lvv[s] == 0)
			{
				g.lvv[s] = visited[p] + 1;
			}

			if (!visited[q] && g.graph[q].cluster == i_cluster)
			{
				visited[q] = visited[p] + 1;
				queue.push_back(q);
			}
		}
	}

#ifdef DEBUG
	std::cout << "cost: " << cost << std::endl;
	std::cout << "### BFS END ###" << std::endl;
#endif

	return cost;
}

uint64_t OPT_REC(int, ClusterSet, ClusterGraph, std::vector<std::vector<int>> &);

uint64_t OPT_CALC(ClusterGraph g, ClusterSet cs, std::vector<std::vector<int>> &OPT)
{
	uint64_t min_cost = 10000000;

	ClusterSet S1 = cs.nextSubset();
	while (cs.canIterate())
	{
		ClusterSet notS1 = cs.subsetExcludingLast();

		for (int cluster : S1.activeSet())
		{
			for (int v : g.cluster_map[cluster])
			{
				uint64_t cost = g.lvv[v] * S1.nodesNumber() + OPT_REC(v, S1, g, OPT) + OPT_REC(v, notS1, g, OPT);
				if (cost < min_cost)
					min_cost = cost;
			}
		}

		S1 = cs.nextSubset();
	}

	return min_cost;
}

uint64_t OPT_REC(int v, ClusterSet cs, ClusterGraph g, std::vector<std::vector<int>> &OPT)
{
	uint64_t S = cs.activeUint64_t();
	// std::bitset<5> a(S);

	if (OPT[S][v] != -1)
	{
		return OPT[S][v];
	}

	if (cs.activeSize() == 1)
		OPT[S][v] = BFS_Vi(g, v);
	else
		OPT[S][v] = OPT_CALC(g, cs, OPT);

	// std::cout << "OPT[" << a << "][" << v << "]= " << OPT[S][v] << std::endl;
	return OPT[S][v];
}

int main(int argc, char *argv[])
{
	freopen("results.out", "w", stdout);
	freopen(argv[1], "r", stdin);
	std::ios_base::sync_with_stdio(false);

	// |V|, |E|, |C|
	int v_size, m_size, k_size;
	std::cin >> v_size >> m_size >> k_size;

	if (k_size > 63)
		return -1;

	uint64_t k_subset_size = (1 << k_size);

	// GRAPH
	ClusterGraph g;

	// OPT[ V ][ s ];
	std::vector<int> OPT_INIT(v_size, -1);
	std::vector<std::vector<int>> OPT(k_subset_size, OPT_INIT);

	// l(v, v');
	std::vector<int> lvv(v_size);
	g.lvv = lvv;

	// η(S);
	std::vector<int> eta(k_subset_size);

	for (int i = 0; i < m_size; i++)
	{
		int v, u;
		std::cin >> v >> u;

		g.graph[v].adj.insert(u);
		g.graph[u].adj.insert(v);
	}

	for (int i = 0; i < v_size; i++)
	{
		int v, cluster;
		std::cin >> v >> cluster;

		auto res = g.cluster_set.insert(cluster);
		if (res.second)
		{
			g.cluster_vec.push_back(cluster);
			g.s2v_map[cluster] = g.cluster_vec.size() - 1;
		}

		g.cluster_map[cluster].insert(v);
		g.graph[v].cluster = cluster;

		eta[1 << (cluster - 1)]++;
	}

	auto cluster_set_sp = std::make_shared<std::unordered_set<int>>(g.cluster_set);
	auto cluster_vec_sp = std::make_shared<std::vector<int>>(g.cluster_vec);
	auto s2v_map_sp = std::make_shared<std::unordered_map<int, int>>(g.s2v_map);
	auto eta_sp = std::make_shared<std::vector<int>>(eta);

	for (uint64_t i = 1, j = 0; i < k_subset_size; i <<= 1, j++)
	{
		for (int v : g.cluster_map[g.cluster_vec[j]])
		{
			OPT[i][v] = BFS_Vi(g, v);
		}
	}

	for (int i = 0; i < v_size; i++)
		std::cout << "lvv[" << i << "] = " << g.lvv[i] << std::endl;

	std::cout << std::endl;
	for (uint64_t i = 1; i < k_subset_size; i++)
	{
		std::bitset<5> asd(i);
		for (int j = 0; j < v_size; j++)
			if (OPT[i][j] != -1)
				std::cout << "opt[" << asd << "][" << j << "] = " << OPT[i][j] << std::endl;
	}

	std::vector<int> active_clu(g.cluster_vec);
	int s = 0;
	int s_cluster = g.graph[s].cluster;
	active_clu.erase(active_clu.begin() + g.s2v_map[s_cluster]);

	ClusterSet cs(active_clu, k_size, cluster_vec_sp, cluster_set_sp, s2v_map_sp, eta_sp);

	std::cout << std::endl;

	OPT[k_subset_size - 1][s] = OPT_CALC(g, cs, OPT);

	for (uint64_t i = 1; i < k_subset_size; i++)
	{
		std::bitset<5> asd(i);
		std::cout << "eta[" << asd << "] = " << (*eta_sp)[i] << std::endl;
	}

	std::cout << std::endl;

	for (uint64_t i = 1; i < k_subset_size; i++)
	{
		std::bitset<5> asd(i);
		for (int j = 0; j < v_size; j++)
			std::cout << "opt[" << asd << "][" << j << "] = " << OPT[i][j] << std::endl;
	}
	std::cout << std::endl
			  << std::endl
			  << "opt[" << k_subset_size - 1 << "][" << s << "] = " << OPT[k_subset_size - 1][s] << std::endl;
}
