#include <iostream>
#include <vector>
#include <algorithm>

int main()
{
	int N = 0, T;

	std::cin >> N;
	std::vector<std::pair<int, int> > plan;
	for (int i = 0; i < N; i++) {
		std::cin >> T;
		plan.emplace_back(i, T);
	}
	std::sort(plan.begin(), plan.end(),
		  [](const std::pair<int, int> &a,
		     const std::pair<int, int> &b) {
			  return a.second < b.second;
		  });
	for (auto val : plan) {
		std::cout << val.first << " ";
	}
	return 0;
}
