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
	this->subset_counter = (1 << this->active_size) - 1;

	for (int i : *set)
		this->active_set.push_back(i);

	for (int i = 0; i < this->active_size; i++)
	{
		this->active_s2v_map[this->active_set[i]] = i;
	}

	std::cout << std::endl
			  << "CLUSTER CREATION" << std::endl;
	for (int i = 0; i < this->active_size; i++)
	{
		std::cout << i << " " << this->active_set[i] << std::endl;
	}
	for (auto ads : this->active_s2v_map)
		std::cout << ads.first << " " << ads.second << std::endl;
	std::cout
		<< "CLUSTER CREATION END" << std::endl
		<< std::endl;
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
	this->subset_counter = (1 << this->active_size) - 1;

	for (int i = 0; i < this->active_size; i++)
	{
		this->active_s2v_map[this->active_set[i]] = i;
	}

	std::cout << std::endl
			  << "CLUSTER CREATION" << std::endl;
	for (int i = 0; i < this->active_size; i++)
	{
		std::cout << i << " " << this->active_set[i] << std::endl;
	}
	for (auto ads : this->active_s2v_map)
		std::cout << ads.first << " " << ads.second << std::endl;
	std::cout
		<< "CLUSTER CREATION END" << std::endl
		<< std::endl;
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

	uint64_t i = ~(subset_counter + 1);
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

ClusterSet ClusterSet::nextSubset(int s_cluster)
{
	std::vector<int> subset_vec;

	std::cout << "sc: " << s_cluster << " v[sc]:" << active_set[active_s2v_map[s_cluster]] << " smap: " << active_s2v_map[s_cluster] << std::endl;

	while ((subset_counter & (1 << active_s2v_map[s_cluster])) != 0)
	{
		subset_counter--;
	}
	std::bitset<5> asd3(subset_counter);
	std::cout << "counter: " << asd3 << std::endl;

	uint64_t i = subset_counter;
	for (int j = 0; j < active_size; i >>= 1, j++)
	{
		std::bitset<5> asd4(i);
		std::cout << "curr: " << j << " " << active_set.at(j) << " " << asd4 << " " << ((i & 1) ? "vero" : "falso") << std::endl;
		if (i & 1)
		{
			subset_vec.push_back(active_set.at(j));
		}
	}
	std::cout << "actset: ";
	for (int v : subset_vec)
		std::cout << v << " ";
	std::cout << std::endl
			  << std::endl;

	ClusterSet cs(subset_vec, k, v_set, set, s2v_map, eta);

	if (subset_counter == 0)
		can_iterate = false;
	if (subset_counter != 0)
		subset_counter--;

	return cs;
}

void ClusterSet::print()
{
	std::cout << "{ ";
	for (int clu : active_set)
		std::cout << clu << ", ";

	std::cout << "}"; //<< std::endl;
}
