import numpy as np
import networkx as nx
from sklearn.cluster import SpectralClustering
from sklearn.cluster import DBSCAN
np.random.seed(1)

G = nx.read_edgelist('../testcases/datasets/as-data.txt')

#G = nx.random_internet_as_graph(5000) troppi cluster
# G = nx.erdos_renyi_graph(100,0.002) #onesto
#G = nx.connected_watts_strogatz_graph(200, 2, 0.4) nhe

# num_communities = 20
# size_of_communities = 30
# G = nx.planted_partition_graph(num_communities, size_of_communities, 0.6, 0.01,seed=42) #Ottima view

# n_nodes = 250
# tau1 = 3
# tau2 = 1.5
# mu = 0.1
# G = nx.LFR_benchmark_graph(n_nodes, tau1, tau2, mu, average_degree=5, min_community=20, seed=10)

if not nx.is_connected(G):
	largest_cc = max(nx.connected_components(G), key=len)
	G = G.subgraph(largest_cc).copy()

adj_mat = nx.to_numpy_matrix(G)

# SpectralClustering
sc = SpectralClustering(30, affinity='precomputed', n_init=100, assign_labels='discretize')

# DBSCAN
# sc = DBSCAN(eps=3, min_samples=2)

sc.fit(adj_mat)
labels = sc.labels_

# Number of clusters in labels, ignoring noise if present.
n_clusters_ = len(set(labels)) - (1 if -1 in labels else 0)
n_noise_ = list(labels).count(-1)

print('Number of clusters: %d' % n_clusters_)
print('Number of noise points: %d' % n_noise_)

cluster_labels = []
nx.set_node_attributes(G, cluster_labels, 'cluster_label')

for i, node in enumerate(G.nodes):
	G.nodes[node]['cluster_label'] = labels[i]

nx.write_gexf(G, 'plot.gexf')
