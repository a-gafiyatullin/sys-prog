#pragma once

#include <vector>
#include <stdexcept>

template <class T> class Deque {
	std::vector<T> front;
	std::vector<T> back;

    public:
	Deque();

	bool Empty() const;

	size_t Size() const;

	T &operator[](size_t index);

	const T &operator[](size_t index) const;

	T &At(size_t index);

	const T &At(size_t index) const;

	T &Front();

	T &Back();

	const T &Front() const;

	const T &Back() const;

	void PushFront(const T &value);

	void PushBack(const T &value);
};

template <class T> Deque<T>::Deque() : front(0), back(0)
{
}

template <class T> bool Deque<T>::Empty() const
{
	return Size() == 0;
}

template <class T> size_t Deque<T>::Size() const
{
	return (front.size() + back.size());
}

template <class T> T &Deque<T>::operator[](size_t index)
{
	if (index < front.size()) {
		return front[front.size() - index - 1];
	} else {
		return back[index - front.size()];
	}
}

template <class T> const T &Deque<T>::operator[](size_t index) const
{
	if (index < front.size()) {
		return front[front.size() - index - 1];
	} else {
		return back[index - front.size()];
	}
}

template <class T> T &Deque<T>::At(size_t index)
{
	if (index >= Size()) {
		throw std::out_of_range("index is out of range");
	}
	return (*this)[index];
}

template <class T> const T &Deque<T>::At(size_t index) const
{
	if (index >= Size()) {
		throw std::out_of_range("index is out of range");
	}
	return (*this)[index];
}

template <class T> T &Deque<T>::Front()
{
	return At(0);
}

template <class T> T &Deque<T>::Back()
{
	return At(Size() - 1);
}

template <class T> const T &Deque<T>::Front() const
{
	return At(0);
}

template <class T> const T &Deque<T>::Back() const
{
	return At(Size() - 1);
}

template <class T> void Deque<T>::PushFront(const T &value)
{
	front.push_back(value);
}

template <class T> void Deque<T>::PushBack(const T &value)
{
	back.push_back(value);
}
