#include "test_runner.h"
#include <sstream>
#include <string>
using namespace std;

class Logger {
    public:
	explicit Logger(ostream &output_stream) : os(output_stream)
	{
	}

	void SetLogLine(bool value)
	{
		log_line = value;
	}
	void SetLogFile(bool value)
	{
		log_file = value;
	}

	bool getLogLine() const
	{
		return log_line;
	}
	bool getLogFile() const
	{
		return log_file;
	}

	void Log(const string &message)
	{
		os << message << endl;
	}

    private:
	ostream &os;
	bool log_line = false;
	bool log_file = false;
};

#define LOG(logger, message)                                                   \
	{                                                                      \
		ostringstream os;                                              \
		if (logger.getLogFile()) {                                     \
			os << __FILE__;                                        \
			if (logger.getLogLine()) {                             \
				os << ":";                                     \
			} else {                                               \
				os << " ";                                     \
			}                                                      \
		}                                                              \
		if (logger.getLogLine()) {                                     \
			if (!logger.getLogFile()) {                            \
				os << "Line ";                                 \
			}                                                      \
			os << __LINE__ << " ";                                 \
		}                                                              \
		os << message;                                                 \
		logger.Log(os.str());                                          \
	}

void TestLog()
{
#line 1 "logger.cpp"

	ostringstream logs;
	Logger l(logs);
	LOG(l, "hello");

	l.SetLogFile(true);
	LOG(l, "hello");

	l.SetLogLine(true);
	LOG(l, "hello");

	l.SetLogFile(false);
	LOG(l, "hello");

	string expected = "hello\n";
	expected += "logger.cpp hello\n";
	expected += "logger.cpp:10 hello\n";
	expected += "Line 13 hello\n";
	ASSERT_EQUAL(logs.str(), expected);
}

int main()
{
	TestRunner tr;
	RUN_TEST(tr, TestLog);
}
