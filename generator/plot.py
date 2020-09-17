import numpy as np
import networkx as nx
from sklearn.cluster import SpectralClustering
from sklearn.cluster import DBSCAN
np.random.seed(1)

# G = nx.read_edgelist('../testcases/datasets/as-data.txt')
#G = nx.random_internet_as_graph(5000) troppi cluster
# G = nx.erdos_renyi_graph(100,0.002) #onesto
#G = nx.connected_watts_strogatz_graph(200, 2, 0.4)
# num_communities = 20
# size_of_communities = 30
# G = nx.planted_partition_graph(num_communities, size_of_communities, 0.6, 0.01,seed=42)

num_of_clusters = 7

G = nx.random_partition_graph([4,8,10,25,30,60,100], 0.6, 0.02,seed=42)

if not nx.is_connected(G):
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

adj_mat = nx.to_numpy_matrix(G)

# SpectralClustering
sc = SpectralClustering(num_of_clusters, affinity='precomputed', n_init=100, assign_labels='discretize')
sc.fit(adj_mat)

labels = sc.labels_

cluster_labels = []
nx.set_node_attributes(G, cluster_labels, 'cluster_label')

for i, node in enumerate(G.nodes):
	G.nodes[node]['cluster_label'] = labels[i]

# Gephi rappresentation
nx.write_gexf(G, 'generator/plot.gexf')

# Algoritm rappresentation
file = open("testcases/generated.in",'w')

file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

for u, v in G.edges:
	file.write('%d %d\n' % (u, v))

for node in G.nodes:
	file.write('%d %d\n' % (node, G.nodes[node]['cluster_label']))

file.close() 