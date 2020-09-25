import numpy as np
import networkx as nx
from sklearn.cluster import SpectralClustering
import argparse

my_parser = argparse.ArgumentParser(description='Graph generator for clubfs')

# Optional params
my_parser.add_argument('--seed', action='store', type=int)

# Required params
my_parser.add_argument('path', metavar='path', type=str, help='the name of the file')
my_parser.add_argument('-k', action='store', type=int, help='Number of cluster', required=True)

# Clustering options
my_parser.add_argument('-sc', '--spectral-clustering', action='store_true', help='Use spectral clustering, this will used by default')
my_parser.add_argument('-gn', '--girvan-newman',action='store_true', help='Use girvan newman')

# Graph generator options
my_parser.add_argument('-part', '--partition-graph',action='store_true', help='Generates a random partition graph, this will used by default')
my_parser.add_argument('-er', '--erdos-renyi', action='store_true', help='Generates a random Erdos Renyi graph')
my_parser.add_argument('-as', '--internet-as',action='store_true', help='Generates a random Internet AS graph')

args = my_parser.parse_args()
print(vars(args))

SEED = 1111 if args.seed == None else args.seed
np.random.seed(SEED)

G = nx.erdos_renyi_graph(100,0.002)
G = nx.random_internet_as_graph(1000)
G = nx.random_partition_graph([200,100,330,250,90,160,100], 0.3, 0.005, seed=SEED)

if not nx.is_connected(G):
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

adj_mat = nx.to_numpy_matrix(G)

num_of_clusters = args.k

# # SpectralClustering
# sc = SpectralClustering(num_of_clusters, affinity='precomputed', n_init=100, assign_labels='discretize')




# sc.fit(adj_mat)

# labels = sc.labels_

# cluster_labels = []
# nx.set_node_attributes(G, cluster_labels, 'cluster_label')

# for i, node in enumerate(G.nodes):
# 	G.nodes[node]['cluster_label'] = labels[i]+1

# # Gephi rappresentation
# nx.write_gexf(G, 'plot.gexf')

# # Algoritm rappresentation
# file = open("testcases/generated.in",'w')

# file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

# for u, v in G.edges:
# 	file.write('%d %d\n' % (int(u), int(v)))

# for node in G.nodes:
# 	file.write('%d %d\n' % (int(node), int(G.nodes[node]['cluster_label'])))

# file.close()
