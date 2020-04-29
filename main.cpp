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

#define INFINITE 10000000

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
	std::vector<std::vector<uint64_t>> lvv;
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

	std::cout << "### BFS START ###" << std::endl;
	std::cout << "start node: " << s << std::endl;
	std::cout << "cluster: " << i_cluster << std::endl;

	while (!queue.empty())
	{
		int p = queue.front();
		queue.pop_front();
		cost += visited[p] - 1;

		for (int k = 0; k < visited[p]; k++)
			std::cout << "--";
		std::cout << p << " d(" << s << "," << p << "):" << visited[p] - 1 << std::endl;

		for (int q : g.graph[p].adj)
		{
			// std::cout << g.graph[q].cluster << " " << i_cluster << " " << g.lvv[s] << std::endl;
			if (g.graph[q].cluster != i_cluster && g.lvv[s][q] == INFINITE)
			{
				g.lvv[s][q] = visited[p];
			}

			if (!visited[q] && g.graph[q].cluster == i_cluster)
			{
				visited[q] = visited[p] + 1;
				queue.push_back(q);
			}
		}
	}

	std::cout << "cost: " << cost << std::endl;
	std::cout << "### BFS END ###" << std::endl;

	return cost;
}

uint64_t OPT_REC(int, ClusterSet, ClusterGraph, std::vector<std::vector<uint64_t>> &);

uint64_t OPT_CALC(int s, ClusterGraph g, ClusterSet cs, std::vector<std::vector<uint64_t>> &OPT)
{
	uint64_t min_cost = INFINITE;
	int s_cluster = g.graph[s].cluster;
	std::cout << "s_cluster: " << s_cluster << std::endl;

	ClusterSet S1 = cs.nextSubset(s_cluster);
	while (cs.canIterate())
	{
		ClusterSet notS1 = cs.subsetExcludingLast();

		std::cout << "S ";
		S1.print();
		std::cout << std::endl;
		std::cout << "NS ";
		notS1.print();
		std::cout << std::endl;

		for (int cluster : S1.activeSet())
		{
			if (cluster != s_cluster)
				for (int v : g.cluster_map[cluster])
				{
					if (g.lvv[s][v] != INFINITE)
					{
						std::cout << "START OPT[" << v << "][";
						S1.print();
						std::cout << "]" << std::endl;
						uint64_t opt_v1_s1 = OPT_REC(v, S1, g, OPT);
						std::cout << "END OPT[" << v << "][";
						S1.print();
						std::cout << "]" << std::endl;

						std::cout << "START OPT[" << s << "][";
						notS1.print();
						std::cout << "]" << std::endl;
						uint64_t opt_s_nots1 = OPT_REC(s, notS1, g, OPT);
						std::cout << "END OPT[" << s << "][";
						notS1.print();
						std::cout << "]" << std::endl;

						uint64_t n_s_v1 = S1.nodesNumber();

						uint64_t cost = (g.lvv[s][v] * n_s_v1) + opt_v1_s1 + opt_s_nots1;
						std::cout << "( l(" << s << "," << v << ")*n(";
						S1.print();
						std::cout << ") + OPT[" << v << "][";
						S1.print();
						std::cout << "] + OPT[" << s << "][";
						notS1.print();
						std::cout << "] ) = " << cost << std::endl;
						std::cout << "( " << g.lvv[s][v] << "*" << n_s_v1 << " + " << opt_v1_s1 << " + " << opt_s_nots1 << ") = " << cost << std::endl;

						if (cost < min_cost)
							min_cost = cost;
					}
				}
		}
		std::cout << "end S ";
		S1.print();
		std::cout << std::endl;

		S1 = cs.nextSubset(s_cluster);
	}

	// std::cout << " min: " << min_cost << std::endl;

	return min_cost;
}

uint64_t OPT_REC(int v, ClusterSet cs, ClusterGraph g, std::vector<std::vector<uint64_t>> &OPT)
{
	uint64_t S = cs.activeUint64_t();
	std::bitset<5> a(S);

	if (OPT[S][v] != INFINITE)
	{
		return OPT[S][v];
	}

	if (cs.activeSize() == 1)
		OPT[S][v] = INFINITE;
	else
		OPT[S][v] = OPT_CALC(v, g, cs, OPT);

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
	std::vector<uint64_t> uint_init(v_size, INFINITE);
	std::vector<std::vector<uint64_t>> OPT(k_subset_size, uint_init);

	// l(v, v');
	std::vector<std::vector<uint64_t>> lvv(v_size, uint_init);
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
		for (int j = 0; j < v_size; j++)
			std::cout << "lvv[" << i << "][" << j << "] = " << g.lvv[i][j] << std::endl;

	std::cout << std::endl;
	for (uint64_t i = 1; i < k_subset_size; i++)
	{
		std::bitset<5> asd(i);
		for (int j = 0; j < v_size; j++)
			if (OPT[i][j] != INFINITE)
				std::cout << "opt[" << asd << "][" << j << "] = " << OPT[i][j] << std::endl;
	}

	std::vector<int> active_clu(g.cluster_vec);
	int s = 0;
	// int s_cluster = g.graph[s].cluster;

	ClusterSet cs(k_size, cluster_vec_sp, cluster_set_sp, s2v_map_sp, eta_sp);

	std::cout << std::endl;

	OPT[k_subset_size - 1][s] = OPT_CALC(0, g, cs, OPT);

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
