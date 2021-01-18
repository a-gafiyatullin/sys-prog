#include <string>
#include <set>
#include <vector>
#include "profile.h"
using namespace std;

constexpr int WORDS = 25000;

class Learner {
    private:
	set<string> dict;

    public:
	int Learn(const vector<string> &words)
	{
		int newWords = 0;

		for (const auto &word : words) {
			if (dict.find(word) == dict.end()) {
				++newWords;
				dict.insert(word);
			}
		}
		return newWords;
	}

	vector<string> KnownWords()
	{
		vector<string> words(dict.size());
		copy(dict.begin(), dict.end(), words.begin());
		return words;
	}
};

int main()
{
	Learner learner;
	vector<string> new_words(WORDS);
	for (int i = 0; i < WORDS; i++) {
		new_words[i] = to_string(i);
	}
	vector<string> new_words_2(WORDS / 2);
	for (int i = WORDS / 2; i < WORDS; i++) {
		new_words_2[i - WORDS / 2] = to_string(i);
	}
	LOG_DURATION("Total")

	{
		LOG_DURATION("Learn 1")
		learner.Learn(new_words);
	}
	{
		LOG_DURATION("Learn 2")
		learner.Learn(new_words_2);
	}
	{
		LOG_DURATION("KnownWords")
		cout << learner.KnownWords().size();
	}

	return 0;
}
