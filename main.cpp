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
#include <functional>

#define INFINITE 10000000
#define LOG(...) fprintf(stderr, __VA_ARGS__)

typedef struct
{
	int cluster;
	std::unordered_set<int> adj;
} node_data;

typedef struct
{
	std::unordered_map<int, node_data> graph;
	std::vector<std::unordered_set<int>> clusters;
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

	LOG("### BFS START ###\n");
	LOG("start node: %d\n", s);
	LOG("cluster: %d\n", i_cluster);

	while (!queue.empty())
	{
		int p = queue.front();
		queue.pop_front();
		cost += visited[p] - 1;

		LOG("d(%d, %d) = %d \n", s, p, visited[p] - 1);

		for (int q : g.graph[p].adj)
		{
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

	LOG("cost: %d\n", cost);
	LOG("### BFS END ###\n");

	return cost;
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

	// 2^k
	uint64_t k_subset_size = static_cast<uint64_t>(1) << k_size;

	int start_node = 1;

	// GRAPH
	ClusterGraph g;

	// CLUSTERS
	std::vector<std::unordered_set<int>> clusters(k_size);
	g.clusters = clusters;

	// OPT[ V ][ s ];
	std::vector<uint64_t>
		uint_init(v_size, INFINITE);
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

		// Clusters indexes go from [0, k-1]
		cluster--;

		g.graph[v].cluster = cluster;
		g.clusters[cluster].insert(v);

		eta[1 << cluster]++;
	}

	// Calculate BFS of Vi for each node of Vi
	for (uint64_t i = 1, j = 0; i < k_subset_size; i <<= 1, j++)
	{
		for (int v : g.clusters[j])
		{
			OPT[i][v] = BFS_Vi(g, v);
		}
	}

	// Debug print
	LOG("### l(v, v') ###\n");
	for (int i = 0; i < v_size; i++)
	{
		for (int j = 0; j < v_size; j++)
		{
			if (g.lvv[i][j] != INFINITE)
			{
				LOG("l(%d, %d) = %d\n", i, j, (int)g.lvv[i][j]);
			}
		}
	}

	std::vector<bool> s_flags(k_size);
	std::vector<int> s_vec;
	int number_of_clusters = 0;

	// For each subset S in V
	for (uint64_t s_binary = 0; s_binary < k_subset_size; s_binary++)
	{
		int ith_cluster = 0;

		// OPT of S with |S|=1 already calculated
		if (number_of_clusters > 1)
		{
			// For each node v
			for (auto map_pair : g.graph)
			{
				int v_node = map_pair.first;
				uint64_t min = INFINITE;

				// Support vector of S
				std::vector<int> s_vec_tmp(s_vec);

				// Finds the cluster of the v_node in S
				std::vector<int>::iterator it = std::lower_bound(s_vec_tmp.begin(), s_vec_tmp.end(), g.graph[v_node].cluster, std::greater<int>());

				// If the cluster of v_node is not in S => OPT[S][v_node] = INF
				if (it != s_vec_tmp.end())
				{
					// Removes the cluster of v_node from S
					if (*it == g.graph[v_node].cluster)
					{
						s_vec_tmp.erase(it);
					}

					std::vector<int> s1_vec;
					std::vector<bool> s1_flags(s_vec_tmp.size());
					uint64_t s1_subset_size = static_cast<uint64_t>(1) << s_vec_tmp.size();

					// Vars init
					s1_flags[0] = 1;
					s1_vec.push_back(s_vec_tmp[0]);
					u_int64_t s1_eta = g.clusters[s_vec_tmp[0]].size();
					u_int64_t s1_binary = static_cast<uint64_t>(1) << s_vec_tmp[0];

					// For each S' in S
					for (uint64_t i = 1; i < s1_subset_size; i++)
					{
						int j = 0;
						u_int64_t s_not_s1_binary = s_binary & ~s1_binary;

						// For each v in C | C subset of S'
						for (int cluster : s1_vec)
						{
							for (int v1 : g.clusters[cluster])
							{
								uint64_t tmp = INFINITE;
								if (g.lvv[v_node][v1] != INFINITE &&
									OPT[s1_binary][v1] != INFINITE &&
									OPT[s_not_s1_binary][v_node] != INFINITE)
								{
									// l(v, v')η(S') + OPT[v', S'] + OPT[v, S \ S']
									tmp = g.lvv[v_node][v1] * s1_eta + OPT[s1_binary][v1] + OPT[s_not_s1_binary][v_node];

									if (tmp < min)
									{
										min = tmp;
									}
								}
							}
						}

						while (s1_flags[j])
						{
							s1_flags[j] = false;
							s1_vec.pop_back();
							s1_eta -= g.clusters[s_vec_tmp[j]].size();
							s1_binary &= ~(1 << s_vec_tmp[j]);
							j++;
						}

						s1_flags[j] = true;
						s1_vec.push_back(s_vec_tmp[j]);
						s1_eta += g.clusters[s_vec_tmp[j]].size();
						s1_binary |= 1 << s_vec_tmp[j];
					}
				}

				OPT[s_binary][v_node] = min;
			}
		}

		while (s_flags[ith_cluster])
		{
			s_flags[ith_cluster] = false;
			s_vec.pop_back();
			number_of_clusters--;
			ith_cluster++;
		}

		s_flags[ith_cluster] = true;

		s_vec.push_back(ith_cluster);
		number_of_clusters++;
	}

	// Print the cost of CLUBFS from start_node
	std::cout << OPT[k_subset_size - 1][start_node] << std::endl;
}
