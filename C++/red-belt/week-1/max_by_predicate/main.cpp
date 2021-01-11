#include <iostream>
#include <vector>

template <typename ForwardIterator, typename UnaryPredicate>
ForwardIterator max_element_if(ForwardIterator first, ForwardIterator last,
			       UnaryPredicate pred)
{
	ForwardIterator result = first;
	bool found = false;

	for (auto iter = first; iter != last; iter++) {
		if ((*result < *iter || !found) && pred(*iter)) {
			result = iter;
			found = true;
		}
	}

	return (found ? result : last);
}

int main()
{
	std::vector<int> v{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::cout << *(max_element_if(v.begin(), v.end(), [](const int &val) {
		return val % 2 == 0;
	})) << std::endl;
	std::cout << *(max_element_if(v.begin(), v.end(), [](const int &val) {
		return val == 1;
	})) << std::endl;

	std::vector<int> v2{ 3, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
	std::cout << *(max_element_if(v.begin(), v.end(), [](const int &val) {
		return val == 2;
	})) << std::endl;

	return 0;
}
