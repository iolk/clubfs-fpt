#ifndef ClusterSet_H
#define ClusterSet_H

#include <iostream>
#include <bitset>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>

class ClusterSet
{
public:
	// Initial Costructor
	ClusterSet(
		int,
		std::shared_ptr<std::vector<int>>,
		std::shared_ptr<std::unordered_set<int>>,
		std::shared_ptr<std::unordered_map<int, int>>,
		std::shared_ptr<std::vector<int>>);

	// Subset Constructor
	ClusterSet(
		std::vector<int>,
		int,
		std::shared_ptr<std::vector<int>>,
		std::shared_ptr<std::unordered_set<int>>,
		std::shared_ptr<std::unordered_map<int, int>>,
		std::shared_ptr<std::vector<int>>);

	void print(void);
	bool canIterate(void);
	ClusterSet nextSubset(void);
	ClusterSet subsetExcludingLast(void);
	std::vector<int> activeSet(void);
	uint64_t nodesNumber(void);
	uint64_t activeUint64_t(void);
	int activeSize(void);

private:
	int k;
	int active_size;
	uint64_t subset_counter;
	bool can_iterate;
	std::vector<int> active_set;
	std::shared_ptr<std::vector<int>> eta;
	std::shared_ptr<std::vector<int>> v_set;
	std::shared_ptr<std::unordered_set<int>> set;
	std::shared_ptr<std::unordered_map<int, int>> s2v_map;
};

#endif
