import numpy as np
import networkx as nx
from cdlib import algorithms
from sklearn.cluster import SpectralClustering
import argparse
import random
import collections
import matplotlib.pyplot as plt

def edge_to_remove(graph):
	G_dict = nx.edge_betweenness_centrality(graph)
	edge = ()

	# extract the edge with highest edge betweenness centrality score
	for key, value in sorted(G_dict.items(), key=lambda item: item[1], reverse = True):
		edge = key
		break

	return edge

def girvan_newman(graph, num_of_clusters):
	# find number of connected components
	sg = nx.connected_components(graph)
	sg_count = nx.number_connected_components(graph)

	prev_count = sg_count
	while(sg_count != num_of_clusters):
		graph.remove_edge(edge_to_remove(graph)[0], edge_to_remove(graph)[1])
		print("Edge removed")

		sg = nx.connected_components(graph)
		sg_count = nx.number_connected_components(graph)

		if(prev_count != sg_count):
			print("Suddivisi %d cluster" % sg_count)
			prev_count = sg_count

	return sg

parser = argparse.ArgumentParser(description='Generatore di istanze di CLUBFS')

# Optional params
parser.add_argument('--seed', action='store', type=int)

# Required params
parser.add_argument('file_name', metavar='file_name', type=str, help='Nome del file di output')

# Clustering options
parser.add_argument('-clu', '--clustering', action='store', nargs=1, type=str, default=['spectral'], choices=['spectral', 'girvan-newman'], help='Algoritmo di clustering (default: spectral)')

# Graph generator options
parser.add_argument('-gen', '--generation', action='store', nargs=1, type=str, default=['lfr-benchmark'], choices=['file','lfr-benchmark','partition', 'internet-as'], help='Algoritmo di generazione (default: partition)')

parser.add_argument('--file-path', action='store', type=str, help='Path del dataset')

parser.add_argument('-t1', action='store', type=float, help='t1 per LFR benchmark')
parser.add_argument('-t2', action='store', type=float, help='t2 Per LFR benchmark')
parser.add_argument('-mu', action='store', type=float, help='mu Per LFR benchmark')
parser.add_argument('-ad','--avg_degree', action='store', type=float, help='Degree medio per LFR benchmark, non inserire se specificato --min_degree')
parser.add_argument('-mind','--min_degree', action='store', type=float, help='Degree minimo per LFR benchmark, non inserire se specificato --avg_degree')
parser.add_argument('-maxd','--max_degree', action='store', type=float, help='Degree massimo per LFR benchmark')
parser.add_argument('-minc','--min_community', action='store', type=float, help='Numero minimo di nodi per comunità per LFR benchmark')
parser.add_argument('-maxc','--max_community', action='store', type=float, help='Numero massimo di nodi per comunità per LFR benchmark')


parser.add_argument('-c', '--communities', action='store', nargs='+', type=int, help='Lista delle cardinalità delle comunità, scelte a random se non specificato')
parser.add_argument('-p_in', action='store', type=float, help='Probabilità degli archi all\'interno della comunità')
parser.add_argument('-p_out', action='store', type=float, help='Probabilità degli archi tra le comunità')
parser.add_argument('-c_min', action='store', type=float, help='Numero minimo di nodi per comunità, in caso -c non presente')
parser.add_argument('-c_max', action='store', type=float, help='Numero massimo di nodi per comunità, in caso -c non presente')

parser.add_argument('-n', action='store', type=int, help='Numero di nodi')
parser.add_argument('-k', action='store', type=int, help='Numero di cluster')

args = parser.parse_args()

if args.seed is None: 
	np.random.seed()
	random.seed()
else:
	np.random.seed(args.seed)
	random.seed(args.seed)

cluster_number = 1

# LFR Benchmark Graph
if args.generation[0] == 'lfr-benchmark': 
	G = nx.LFR_benchmark_graph(
		args.n, 
		args.t1, 
		args.t2 , 
		args.mu, 
		min_degree=args.min_degree, 
		max_degree=args.max_degree, 
		average_degree=args.avg_degree,
		min_community=args.min_community, 
		max_community=args.max_community, 
		seed=args.seed
	)

	H = G.__class__()
	H.add_nodes_from(G)
	H.add_edges_from(G.edges)

	cluster_labels = None
	nx.set_node_attributes(H, cluster_labels, 'cluster_label')

	for (node, nodedata) in G.nodes.items():
		if H.nodes[node]['cluster_label'] is None:
			H.nodes[node]['cluster_label'] = cluster_number
			# print(nodedata)

			for i in nodedata['community']:
				H.nodes[i]['cluster_label'] = cluster_number

			cluster_number+=1
	
	G = H.copy()


# Random Internet AS Graph
if args.generation[0] == 'internet-as': 
	G = nx.random_internet_as_graph(args.n*5)
	for node in range(0,args.n*5):
		if G.nodes[node]['type'] == 'C':
			G.remove_node(node)

# Random Partition Graph
if args.generation[0] == 'partition': 
	communities = [0]*args.k if args.communities is None else args.communities
	if args.communities is None: 
		for i in range(0,args.k):
			communities[i] = random.randint(args.c_min, args.c_max)
	
	G = nx.random_partition_graph(communities, args.p_in, args.p_out, seed=args.seed)

if args.generation[0] == 'file': 
	G = nx.Graph()

	with open(args.file_path,'r') as f:
		res_body = f.read().split('\n')
		res_body.pop()
		f.close()

	for i, link in enumerate(res_body):
		u = int(link.split()[0])
		v = int(link.split()[1])
		u = u - 1 
		v = v - 1
		G.add_edge(u, v)

if not nx.is_connected(G):
	print("Grafo disconnesso, verrà scelta la componente connessa più grande")
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

	H = G.__class__()
	remap_nodes = dict()
	for i,node in enumerate(G.nodes):
		remap_nodes[node] = i

	for u,v in sorted(G.edges):
		if u != v:
			H.add_edge(int(remap_nodes[u]), int(remap_nodes[v]))
	
	G = H

spl = nx.average_shortest_path_length(G)
cost = (2*len(G.edges)) / (len(G.nodes)*(len(G.nodes)-1))
clu = nx.average_clustering(G)
print(spl,'&',clu,'&',cost)

num_of_clusters = args.k if args.generation[0] != 'lfr-benchmark' else cluster_number-1


if args.generation[0] != 'lfr-benchmark':
	adj_mat = nx.to_numpy_matrix(G)
		
	# Louvain
	if args.clustering[0] == 'louvain':
		im = algorithms.louvain(G)

		cluster_labels = []
		nx.set_node_attributes(G, cluster_labels, 'cluster_label')
		num_of_clusters = 0
		for (node , comm) in im.to_node_community_map().items():
			G.nodes[node]['cluster_label'] = comm[0]+1
			if num_of_clusters < comm[0]+1:
				num_of_clusters = comm[0]+1
		num_of_clusters += 1

	# SpectralClustering
	if args.clustering[0] == 'spectral':
		sc = SpectralClustering(num_of_clusters, affinity='precomputed', n_init=100, assign_labels='discretize')

		sc.fit(adj_mat)
		labels = sc.labels_

		cluster_labels = []
		nx.set_node_attributes(G, cluster_labels, 'cluster_label')

		for i, node in enumerate(G.nodes):
			G.nodes[node]['cluster_label'] = labels[i]+1

	# Girvan-Newman
	if args.clustering[0] == 'girvan-newman':
		c = girvan_newman(G.copy(),args.k)

		communities = []
		for i in c:
			communities.append(list(i))
			
		for i, clust in enumerate(communities):
			for j, node in enumerate(clust):
				G.nodes[node]['cluster_label'] = i+1

# Gephi rappresentation
nx.write_gexf(G, args.file_name+'.gexf')

# Algoritm rappresentation
file = open(args.file_name+'.in','w')

file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

for u,v in sorted(G.edges):
	file.write('%d %d\n' % (int(u), int(v)))

for node in sorted(G.nodes):
	file.write('%d %d\n' % (int(node), int(G.nodes[node]['cluster_label'])))

file.close()