#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <ios>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <list>
#include <cstdint>
#include <functional>
#include <chrono>

#define INFINITE 184467440737095516
#define LOG(...) fprintf(stderr, __VA_ARGS__)

typedef struct
{
	int cluster;
	std::unordered_set<int> adj;
} node_data;

typedef struct
{
	uint64_t distance;
	std::pair<int,int> link;
} lvv_data;

typedef struct
{
	std::vector<node_data> graph;
	std::vector<std::unordered_set<int>> clusters;
	std::vector<std::vector<std::pair<int,int>>> bfs_vi;
	std::vector<std::vector<lvv_data>> lvv;
} ClusterGraph;

void print_solution(uint64_t node, uint64_t subset, std::vector<std::vector<std::pair<uint64_t, uint64_t>>> PREV, ClusterGraph g);

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

	// LOG("### BFS START ###\n");
	// LOG("start node: %d\n", s);
	// LOG("cluster: %d\n", i_cluster);

	while (!queue.empty())
	{
		int p = queue.front();
		queue.pop_front();
		cost += visited[p] - 1;
		
		// LOG("d(%d, %d) = %d \n", s, p, visited[p] - 1);

		for (int q : g.graph[p].adj)
		{
			if (g.graph[q].cluster != i_cluster && g.lvv[s][q].distance == INFINITE)
			{
				g.lvv[s][q].distance = visited[p];
				g.lvv[s][q].link = std::make_pair(p, q);
			}

			if (!visited[q] && g.graph[q].cluster == i_cluster)
			{
				visited[q] = visited[p] + 1;
				g.bfs_vi[s].push_back(std::make_pair(p, q));
				queue.push_back(q);
			}
		}
	}

	// LOG("cost: %d\n", cost);
	// LOG("### BFS END ###\n");

	return cost;
}

int main(int argc, char *argv[])
{
	auto start_with_io = std::chrono::high_resolution_clock::now();
	freopen(argv[2], "w", stdout);
	freopen("debug.txt", "w", stderr);
	freopen(argv[1], "r", stdin);
	std::ios_base::sync_with_stdio(false);

	// |V|, |E|, |C|
	int v_size, m_size, k_size;
	std::cin >> v_size >> m_size >> k_size;

	if (k_size > 63)
		return -1;

	// 2^k
	uint64_t k_subset_size = static_cast<uint64_t>(1) << k_size;

	int start_node = 0;

	// GRAPH
	ClusterGraph g;

	// CLUSTERS
	std::vector<node_data> graph(v_size);
	g.graph = graph;

	// CLUSTERS
	std::vector<std::unordered_set<int>> clusters(k_size);
	g.clusters = clusters;

	// BFS_Vi
	std::vector<std::vector<std::pair<int, int>>> bfs_vi_init(v_size);
	g.bfs_vi = bfs_vi_init;

	// l(v, v');
	lvv_data lvv_data_init;
	lvv_data_init.distance = INFINITE;
	std::vector<lvv_data> lvv_init(v_size, lvv_data_init);
	std::vector<std::vector<lvv_data>> lvv(v_size, lvv_init);
	g.lvv = lvv;

	// OPT[ V ][ S ]
	std::vector<uint64_t> uint_init(v_size, INFINITE);
	std::vector<std::vector<uint64_t>> OPT(k_subset_size, uint_init);

	// PREV[ V ][ S ] 
	std::vector<std::pair<uint64_t, uint64_t>> uint_pair_init(v_size, std::make_pair (INFINITE, INFINITE));
	std::vector<std::vector<std::pair<uint64_t, uint64_t>>> PREV(k_subset_size, uint_pair_init);

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

		// L'indice dei cluster va da [0, k-1]
		cluster--;

		g.graph[v].cluster = cluster;
		g.clusters[cluster].insert(v);

		eta[1 << cluster]++;
	}
	
	auto start_without_io = std::chrono::high_resolution_clock::now();

	// Calcolo di tutti i casi base di OPT, ovvero i BFS di Vi
	for (uint64_t i = 1, j = 0; i < k_subset_size; i <<= 1, j++)
	{
		for (int v : g.clusters[j])
		{
			OPT[i][v] = BFS_Vi(g, v);
		}
	}

	// Vettore caratteristico di S
	std::vector<bool> s_flags(k_size);

	// Vettore degli indici di S (ordinato in modo non-decrescente)
	std::vector<int> s_vec;
	
	int number_of_clusters = 0;

	// Per ogni sottoinsieme S dell'insieme dei cluster
	for (uint64_t s_binary = 0; s_binary < k_subset_size; s_binary++)
	{
		int ith_cluster = 0;

		// LOG("S: %d\n", s_binary);

		// Gli OPT di S con cardinalità 1 sono già stati calcolati
		if (number_of_clusters > 1)
		{
			// Per ogni nodo in ogni cluster di S
			for(int r : s_vec)
			{
				for(int v_node : g.clusters[r])
				{
					uint64_t min = INFINITE;
					
					// Vettore caratteristico di S'
					std::vector<bool> s1_flags(s_vec.size());
					
					// Vettore degli indici di S' (ordinato in modo non-decrescente)
					std::vector<int> s1_vec;
					uint64_t s1_subset_size = static_cast<uint64_t>(1) << s1_flags.size();

					/* Inizializzazione delle variabili
					 * Per risolvere il fattore k bisognava gestire S' da S escludendo 
					 * il cluster r del nodo v. E' fondamentale che in S' non entri r 
					 * per il calcolo di OPT.
					 * A questo proposito nelle versioni precedenti veniva creato un vettore
					 * d'appogio e poi rimosso r da esso
					 */
					uint64_t s1_eta = g.clusters[s_vec[0]].size();
					uint64_t s1_binary = static_cast<uint64_t>(1) << s_vec[0];

					// Viene gestito il caso in cui r è il primo cluster di S
					if(s_vec[0] == r){
						s1_flags[1] = 1;
						s1_vec.push_back(s_vec[1]);
						s1_eta = g.clusters[s_vec[1]].size();
						s1_binary = static_cast<uint64_t>(1) << s_vec[1];
					}else{
						s1_flags[0] = 1;
						s1_vec.push_back(s_vec[0]);
					}

					// Per ogni S' in S
					for (uint64_t i = 1 + (s_vec[0] == r); i < s1_subset_size; i++)
					{
						int j = 0;
						uint64_t s_not_s1_binary = s_binary & ~s1_binary;

						// Per ogni nodo in ogni cluster di S'
						for (int cluster : s1_vec)
						{
							for (int v1 : g.clusters[cluster])
							{
								uint64_t tmp = INFINITE;

								// Calcolo di OPT
								if (g.lvv[v_node][v1].distance != INFINITE &&
									OPT[s1_binary][v1] != INFINITE &&
									OPT[s_not_s1_binary][v_node] != INFINITE)
								{
									// l(v, v')η(S') + OPT[v', S'] + OPT[v, S \ S']
									tmp = g.lvv[v_node][v1].distance * s1_eta + OPT[s1_binary][v1] + OPT[s_not_s1_binary][v_node];

									if (tmp < min)
									{
										min = tmp;

										// Ricostruzione della soluzione
										PREV[s_binary][v_node] = std::make_pair(s1_binary, v1);
									}
								}
							}
						}

						/* Gestione del vettore caratteristico di S'
						 * Qui vengono gestiti i casi in cui viene processato il cluster r del nodo
						 * Fondamentalmente se viene incontrato non deve essere inserito e le variabili
						 * di S' non devono essere modificate però il vettore caratteristico deve essere
						 * aggiornato "simulando" che r sia stato valutato quindi i cicli in cui r 
						 * s1_flags[r] è true vengono saltati attraverso il do while
						 * La variabile isDone è utilizzata nel caso in cui r è valutato ma sono già stati
						 * valutati tutti i sotto insiemi
						 */
						bool isDone = false;
						do{
							while (s1_flags[j])
							{
								s1_flags[j] = false;
								
								if(s_vec[j] != r){
									s1_vec.pop_back();
									s1_eta -= g.clusters[s_vec[j]].size();
									s1_binary &= ~(1 << s_vec[j]);
								}

								j++;
							}

							if (j < s1_flags.size())
							{
								s1_flags[j] = true;

								if(s_vec[j] != r){
									s1_vec.push_back(s_vec[j]);
									s1_eta += g.clusters[s_vec[j]].size();
									s1_binary |= 1 << s_vec[j];
								}
							}else{ isDone = true; }
						}while(s_vec[j] == r && !isDone);
					}

					OPT[s_binary][v_node] = min;
				}
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

	// Debug print
	// std::cout <<"\t\t";
	// for (int i = 0; i < v_size; i++)
	// {
	// 	std::cout <<i<<"\t";
	// }
	// std::cout<< std::endl;
	// for (uint64_t i = 0; i < k_subset_size; i++)
	// {
	// 	std::cout <<std::bitset<7>(i)<<" |\t";
	// 	for (int j = 0; j < v_size; j++)
	// 	{
	// 		if (OPT[i][j] != INFINITE)
	// 		{
	// 			std::cout << OPT[i][j]<<"\t";
	// 		}else{
	// 			std::cout <<"I\t";
	// 		}
	// 	}
	// 	std::cout << std::endl;
	// }

	// Print the cost of CLUBFS from start_node
	// std::cout << OPT[k_subset_size - 1][start_node] << std::endl;
	
	auto finish_without_io = std::chrono::high_resolution_clock::now();

	// Output della soluzione
	for(auto link: g.bfs_vi[start_node]){
		std::cout << link.first << " " << link.second << std::endl;
	}
	print_solution(start_node, k_subset_size - 1, PREV, g);

	auto finish_with_io = std::chrono::high_resolution_clock::now();

	auto duration_with_io = std::chrono::duration_cast<std::chrono::microseconds>( finish_with_io - start_with_io ).count();
	auto duration_without_io = std::chrono::duration_cast<std::chrono::microseconds>( finish_without_io - start_without_io ).count();
	std::cout << std::endl << duration_with_io << " & "<< duration_without_io;
}

void print_solution(uint64_t node, uint64_t subset, std::vector<std::vector<std::pair<uint64_t, uint64_t>>> PREV, ClusterGraph g){
	uint64_t s = PREV[subset][node].first;
	uint64_t v = PREV[subset][node].second;
	uint64_t from_node = node;

	while(v!=INFINITE && s!=INFINITE){
	
		std::cout << g.lvv[from_node][v].link.first << " " << g.lvv[from_node][v].link.second << std::endl;
		for(auto link: g.bfs_vi[v]){
			std::cout << link.first << " " << link.second << std::endl;
		}

		uint64_t not_s = subset & ~s;
		
		if(PREV[not_s][from_node].first != INFINITE)
			print_solution(from_node, not_s, PREV, g);

		subset = s;
		from_node = v;
		s = PREV[subset][v].first;
		v = PREV[subset][v].second;
	}
}