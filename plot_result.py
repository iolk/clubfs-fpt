import numpy as np
import networkx as nx
import sys
np.random.seed(1)

with open(sys.argv[1],'r') as f:
	in_body = f.read().split('\n')
	in_body.pop()
	f.close()

with open(sys.argv[2],'r') as f:
	res_body = f.read().split('\n')
	res_body.pop()
	f.close()

n = int(in_body[0].split()[0])
m = int(in_body[0].split()[1])
k = int(in_body[0].split()[2])

G = nx.Graph()

for i in range(1, m+1):
	e_from = int(in_body[i].split()[0])
	e_to = int(in_body[i].split()[1])
	G.add_edge(e_from, e_to)

cluster_labels = []
nx.set_node_attributes(G, cluster_labels, 'cluster_label')

clubfs = []
nx.set_edge_attributes(G, clubfs, 'clubfs')

for i, node in enumerate(G.nodes):
	G.nodes[node]['cluster_label'] = int(in_body[1+m+int(node)].split()[1])

for i, link in enumerate(res_body):
	if link == "":
		break
	u = int(link.split()[0])
	v = int(link.split()[1])
	try:
		G.edges[u,v]['clubfs'] = 1
	except:
		print("ERR:",u,v,sys.argv[1],sys.argv[2])

# Gephi rappresentation
nx.write_gexf(G, sys.argv[3])
