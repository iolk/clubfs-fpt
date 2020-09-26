import numpy as np
import networkx as nx
from sklearn.cluster import SpectralClustering
import argparse

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
parser.add_argument('-k', action='store', type=int, help='Number of cluster', required=True)

# Clustering options
parser.add_argument('-clu', '--clustering', action='store', nargs=1, type=str, default=['spectral'], choices=['spectral', 'girvan-newman'], help='Clustering algorithm (default: spectral)')

# Graph generator options
parser.add_argument('-gen', '--generation', action='store', nargs=1, type=str, default=['partition'], choices=['partition', 'internet-as'], help='Generation algorithm (default: partition)')

parser.add_argument('-c', '--communities', action='store', nargs='+', type=int, help='Parameter for Partition Graph, list of cardinalities of communities')
parser.add_argument('-p_in', action='store', type=float, help='Parameter for Partition Graph, probability of edges in the community')
parser.add_argument('-p_out', action='store', type=float, help='Parameter for Partition Graph, probability of edges between community')

parser.add_argument('-n', action='store', type=int, help='Parameter for Internet AS Graph, Number of nodes')

# parser.add_argument('-er', '--erdos-renyi', action='store_true', help='Generates a random Erdos Renyi graph')

args = parser.parse_args()

# Argument check
if args.generation[0] == 'partition' and (args.communities is None or args.p_in is None or args.p_out is None):
	parser.error("Partition Graph requires --communities, -p_in and -p_out.")

if args.generation[0] == 'internet-as' and args.n is None:
	parser.error("Internet AS Graph requires -n.")

SEED = 1111 if args.seed is None else args.seed
np.random.seed(SEED)

# Random Internet AS Graph
if args.generation[0] == 'internet-as': 
	G = nx.random_internet_as_graph(args.n)

# Random Partition Graph
if args.generation[0] == 'partition': 
	G = nx.random_partition_graph(args.communities, args.p_in, args.p_out, seed=SEED)

# G = nx.erdos_renyi_graph(100,0.002)

if not nx.is_connected(G):
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

adj_mat = nx.to_numpy_matrix(G)

num_of_clusters = args.k

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
file = open('testcases/gen_'+ args.file_name+'.in','w')

file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

for u, v in G.edges:
	file.write('%d %d\n' % (int(u), int(v)))

for node in G.nodes:
	file.write('%d %d\n' % (int(node), int(G.nodes[node]['cluster_label'])))

file.close()
