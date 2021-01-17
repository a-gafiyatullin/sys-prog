#pragma once

#include <vector>

template <class T> class Table {
	size_t rows = 0;
	size_t cols = 0;

	std::vector<std::vector<T> > table;

    public:
	Table(const size_t &rows, const size_t &cols);

	std::vector<T> &operator[](const size_t &index);

	const std::vector<T> &operator[](const size_t &index) const;

	void Resize(const size_t &rows, const size_t &cols);

	std::pair<size_t, size_t> Size() const;
};

template <class T> Table<T>::Table(const size_t &n_rows, const size_t &n_cols)
{
	Resize(n_rows, n_cols);
}

template <class T> std::vector<T> &Table<T>::operator[](const size_t &index)
{
	return table[index];
}

template <class T>
const std::vector<T> &Table<T>::operator[](const size_t &index) const
{
	return table[index];
}

template <class T>
void Table<T>::Resize(const size_t &n_rows, const size_t &n_cols)
{
	if (n_rows == 0 || n_cols == 0) {
		rows = 0;
		cols = 0;
	} else {
		rows = n_rows;
		cols = n_cols;
	}

	table.resize(n_rows);
	for (auto &row : table) {
		row.resize(n_cols);
	}
}

template <class T> std::pair<size_t, size_t> Table<T>::Size() const
{
	return std::pair<size_t, size_t>(rows, cols);
}
