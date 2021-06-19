#include "fft.h"

#include "math.h"

#include <cmath>

namespace fft
{

void fft(Iterator Xbegin, Iterator Xend, ConstIterator xbegin, ConstIterator xend, int s)
{
    double N = xend - xbegin;
    if(N == 1)
        *Xbegin = complex<double>(*xbegin);
    else
    {
        fft(Xbegin, Xbegin + N/2.0, xbegin, xbegin + N/2.0, 2*s);
        fft(Xbegin + N/2.0, Xend, xbegin + s, xbegin + s + N/2.0, 2*s);
        for(double k = 0; k < N/2.0; k++)
        {
            auto& begin_k = *(Xbegin + k);
            auto& begin_k_n_2 = *(Xbegin + k + N/2.0);
            auto p = begin_k;
            auto q = exp(complex<double>(-2*PI*complex<double>(0, 1)*k/N)) * begin_k_n_2;
            begin_k = p + q;
            begin_k_n_2 = p - q;
        }
    }
}

}
