import networkx as nx
import sys

n_vertex = int(sys.argv[1])
seed_n = int(sys.argv[2])

G = nx.random_internet_as_graph(n_vertex, seed_n)
#G = nx.random_clustered_graph() da vedere

for u, v in G.edges:
	print('%d %d' % (u, v))
