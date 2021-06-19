#pragma once

#include <complex>
#include <vector>

using std::complex;
using std::vector;

using cd = complex<double>;
constexpr double PI = 3.14159265357989;

using Iterator = vector<complex<double>>::iterator;
using ConstIterator = vector<int16_t>::const_iterator;

void fft(Iterator Xbegin, Iterator Xend, ConstIterator xbegin, ConstIterator xend, int s = 1);
