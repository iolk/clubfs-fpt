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

def girvan_newman(graph):
	# find number of connected components
	sg = nx.connected_components(graph)
	sg_count = nx.number_connected_components(graph)

	while(sg_count != num_of_clusters):
		graph.remove_edge(edge_to_remove(graph)[0], edge_to_remove(graph)[1])
		sg = nx.connected_components(graph)
		sg_count = nx.number_connected_components(graph)

	return sg

parser = argparse.ArgumentParser(description='Graph generator for clubfs')

# Optional params
parser.add_argument('--seed', action='store', type=int)

# Required params
parser.add_argument('file_name', metavar='file_name', type=str, help='Name of the file')

# Clustering options
parser.add_argument('-clu', '--clustering', action='store', nargs=1, type=str, default=['spectral'], choices=['louvain','spectral', 'girvan-newman'], help='Clustering algorithm (default: spectral)')

# Graph generator options
parser.add_argument('-gen', '--generation', action='store', nargs=1, type=str, default=['lfr-benchmark'], choices=['file','lfr-benchmark','partition', 'internet-as'], help='Generation algorithm (default: partition)')

parser.add_argument('--file-path', action='store', type=str, help='')

parser.add_argument('-t1', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-t2', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-mu', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-ad','--avg_degree', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-mind','--min_degree', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-maxd','--max_degree', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-minc','--min_community', action='store', type=float, help='Parameter for LFR benchmark')
parser.add_argument('-maxc','--max_community', action='store', type=float, help='Parameter for LFR benchmark')


parser.add_argument('-c', '--communities', action='store', nargs='+', type=int, help='Parameter for Partition Graph, list of cardinalities of communities')
parser.add_argument('-p_in', action='store', type=float, help='Parameter for Partition Graph, probability of edges in the community')
parser.add_argument('-p_out', action='store', type=float, help='Parameter for Partition Graph, probability of edges between community')
parser.add_argument('-c_min', action='store', type=float, help='Parameter for Partition Graph, probability of edges in the community')
parser.add_argument('-c_max', action='store', type=float, help='Parameter for Partition Graph, probability of edges between community')

parser.add_argument('-n', action='store', type=int, help='Parameter for Internet AS Graph, Number of nodes')
parser.add_argument('-k', action='store', type=int, help='Number of cluster')

args = parser.parse_args()

# Argument check
# if args.generation[0] == 'lfr-benchmark' and (args.n is None or args.t1 is None or args.t2 is None or args.mu is None or args.min_community is None or (args.avg_degree is None and args.min_degree is None)):
# 	parser.error("Partition Graph requires --communities, -p_in and -p_out.")

if args.generation[0] == 'partition' and (args.p_in is None or args.p_out is None or args.k is None):
	parser.error("Partition Graph requires -p_in and -p_out.")

if args.generation[0] == 'internet-as' and args.n is None:
	parser.error("Internet AS Graph requires -n.")

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
		u = link.split()[0]
		v = link.split()[1]
		# u=int(u) -1 
		# v=int(v) -1
		G.add_edge(u, v)

if not nx.is_connected(G):
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

# spl = nx.average_shortest_path_length(G)
# cost = (2*len(G.edges)) / (len(G.nodes)*(len(G.nodes)-1))
# clu = nx.average_clustering(G)
# print(spl,'&',clu,'&',cost)

num_of_clusters = args.k if args.generation[0] != 'lfr-benchmark' else cluster_number-1


if args.generation[0] != 'lfr-benchmark':
	adj_mat = nx.to_numpy_matrix(G)
		
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
		c = girvan_newman(G.copy())

		communities = []
		for i in c:
			communities.append(list(i))
			
		for i, clust in enumerate(communities):
			for j, node in enumerate(clust):
				G.nodes[node]['cluster_label'] = i+1

# Gephi rappresentation
nx.write_gexf(G, 'gen_'+args.file_name+'_plot.gexf')

# Algoritm rappresentation
file = open('gen_'+ args.file_name+'.in','w')

file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

for u, v in G.edges:
	file.write('%d %d\n' % (int(u), int(v)))

for node in G.nodes:
	file.write('%d %d\n' % (int(node), int(G.nodes[node]['cluster_label'])))

file.close()