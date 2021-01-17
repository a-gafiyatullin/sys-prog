#include "Deque.h"
#include "gtest/gtest.h"

TEST(DequeTest, EmptyTest)
{
	Deque<int> q;
	ASSERT_TRUE(q.Empty());
}

TEST(DequeTest, PushBackTest)
{
	Deque<int> q;
	q.PushBack(1);
	ASSERT_FALSE(q.Empty());
	ASSERT_EQ(q.Front(), 1);
	ASSERT_EQ(q.Back(), 1);
}

TEST(DequeTest, PushFrontTest)
{
	Deque<int> q;
	q.PushFront(1);
	ASSERT_FALSE(q.Empty());
	ASSERT_EQ(q.Front(), 1);
	ASSERT_EQ(q.Back(), 1);
}

TEST(DequeTest, PushBackFrontTest)
{
	Deque<int> q;
	q.PushFront(1);
	q.PushFront(3);
	q.PushBack(2);
	q.PushBack(4);
	ASSERT_FALSE(q.Empty());
	ASSERT_EQ(q.Front(), 3);
	ASSERT_EQ(q.Back(), 4);
}

TEST(DequeTest, AssignTest)
{
	Deque<int> q;
	q.PushFront(1);
	ASSERT_EQ(q.Front(), 1);
	q[0] = 2;
	ASSERT_EQ(q.Back(), 2);
}

TEST(DequeTest, out_of_range)
{
	Deque<int> q;
	try {
		q.At(0) = 1;
		FAIL() << "Expected std::out_of_range";
	} catch (std::out_of_range const &err) {
		EXPECT_EQ(err.what(), std::string("index is out of range"));
	} catch (...) {
		FAIL() << "Expected std::out_of_range";
	}
}

int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
