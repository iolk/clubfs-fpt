#include "ClusterSet.h"

ClusterSet::ClusterSet(
	int k,
	std::shared_ptr<std::vector<int>> v_set,
	std::shared_ptr<std::unordered_set<int>> set,
	std::shared_ptr<std::unordered_map<int, int>> s2v_map,
	std::shared_ptr<std::vector<int>> eta)
{
	this->k = k;
	this->v_set = v_set;
	this->set = set;
	this->eta = eta;
	this->s2v_map = s2v_map;
	this->can_iterate = true;
	this->active_size = k;
	this->subset_counter = 1;

	for (int i : *set)
		this->active_set.push_back(i);
}

ClusterSet::ClusterSet(
	std::vector<int> active_set,
	int k,
	std::shared_ptr<std::vector<int>> v_set,
	std::shared_ptr<std::unordered_set<int>> set,
	std::shared_ptr<std::unordered_map<int, int>> s2v_map,
	std::shared_ptr<std::vector<int>> eta)
{
	this->k = k;
	this->v_set = v_set;
	this->set = set;
	this->eta = eta;
	this->s2v_map = s2v_map;
	this->can_iterate = true;
	this->active_set = active_set;
	this->active_size = this->active_set.size();
	this->subset_counter = 1;
}

bool ClusterSet::canIterate()
{
	return can_iterate;
}

std::vector<int> ClusterSet::activeSet()
{
	return active_set;
}

uint64_t ClusterSet::nodesNumber()
{
	uint64_t it = this->activeUint64_t();
	if ((*eta)[it] != 0)
		return (*eta)[it];

	uint64_t curr_eta = 0;
	for (int c : active_set)
		curr_eta += (*eta)[1 << ((*s2v_map)[c])];

	(*eta)[it] = curr_eta;
	return (*eta)[it];
}

uint64_t ClusterSet::activeUint64_t()
{
	uint64_t x = 0;

	for (int c : active_set)
		x |= 1 << ((*s2v_map)[c]);

	return x;
}

int ClusterSet::activeSize() { return active_set.size(); }

ClusterSet ClusterSet::subsetExcludingLast()
{
	std::vector<int> subset_vec;

	uint64_t i = ~(subset_counter - 1);
	for (int j = 0; j < active_size; i >>= 1, j++)
	{
		if (i & 1)
		{
			subset_vec.push_back(active_set.at(j));
		}
	}

	ClusterSet cs(subset_vec, k, v_set, set, s2v_map, eta);
	return cs;
}

ClusterSet ClusterSet::nextSubset()
{
	std::vector<int> subset_vec;

	uint64_t i = subset_counter;
	for (int j = 0; j < active_size; i >>= 1, j++)
	{
		if (i & 1)
		{
			subset_vec.push_back(active_set.at(j));
		}
	}

	ClusterSet cs(subset_vec, k, v_set, set, s2v_map, eta);

	subset_counter++;
	uint64_t cmp = (1 << active_size);
	if (subset_counter >= cmp)
		can_iterate = false;

	return cs;
}

void ClusterSet::print()
{
	std::cout << "{ ";
	for (int clu : active_set)
		std::cout << clu << ", ";

	std::cout << "}" << std::endl;
}
