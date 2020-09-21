import networkx as nx
import itertools

G = nx.random_internet_as_graph(500)

num_of_clusters = 10

def girvan_newman(G, most_valuable_edge=None):
    if G.number_of_edges() == 0:
        yield tuple(nx.connected_components(G))
        return

    if most_valuable_edge is None:
        def most_valuable_edge(G):
            betweenness = nx.edge_betweenness_centrality(G)
            return max(betweenness, key=betweenness.get)

    g = G.copy().to_undirected()
    g.remove_edges_from(nx.selfloop_edges(g))
    while g.number_of_edges() > 0:
        yield _without_most_central_edges(g, most_valuable_edge)

def _without_most_central_edges(G, most_valuable_edge):
    original_num_components = nx.number_connected_components(G)
    num_new_components = original_num_components
    while num_new_components <= original_num_components:
        edge = most_valuable_edge(G)
        G.remove_edge(*edge)
        new_components = tuple(nx.connected_components(G))
        num_new_components = len(new_components)
    return new_components

comp = girvan_newman(G)
limited = itertools.takewhile(lambda c: len(c) <= num_of_clusters, comp)
communities = ''
for tmp in limited:
	# print(tuple(sorted(c) for c in communities))
	print(len(tmp))
	if(len(tmp) == num_of_clusters):
		communities = tmp

cluster_labels = []
nx.set_node_attributes(G, cluster_labels, 'cluster_label')

for i, clust in enumerate(communities):
	for j, node in enumerate(clust):
		G.nodes[node]['cluster_label'] = i+1

# Gephi rappresentation
nx.write_gexf(G, 'plot.gexf')

# Algoritm rappresentation
file = open("testcases/generated.in",'w')

file.write('%d %d %d\n' % (len(G.nodes), len(G.edges), num_of_clusters))

for u, v in G.edges:
	file.write('%d %d\n' % (int(u), int(v)))

for node in G.nodes:
	file.write('%d %d\n' % (int(node), int(G.nodes[node]['cluster_label'])))

file.close()