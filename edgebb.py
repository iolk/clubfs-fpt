import networkx as nx
# import matplotlib.pyplot as plt

# load the graph
# G = nx.karate_club_graph()
G = nx.random_internet_as_graph(1000)

# visualize the graph
# nx.draw(G, with_labels = True)

num_of_clusters = 8

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
		print(sg_count)

	return sg

# find communities in the graph
c = girvan_newman(G.copy())

# find the nodes forming the communities
node_groups = []

for i in c:
	node_groups.append(list(i))

cluster_labels = []
nx.set_node_attributes(G, cluster_labels, 'cluster_label')

for i, clust in enumerate(node_groups):
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