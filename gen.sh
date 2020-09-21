#!/bin/bash
GRAPH_NODES=$1
SEED_NUM=1111

python3 gen.py $GRAPH_NODES $SEED_NUM > graph.gen

infomap graph.gen ./ --ftree --clu -s 1111 -N 10

CLUSTER_NUM="$(cat graph.clu | grep "partitioned into 2 levels with" | awk '{print $7}')"

# Node and edges number print
echo "$GRAPH_NODES $(wc -l graph.gen | awk '{ print $1 }') $CLUSTER_NUM" > gen.in
cat graph.gen >> gen.in

# Cluster print
cat graph.clu | grep -v \# | awk '{ print $1,$2 }' >> gen.in