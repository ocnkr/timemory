// MIT License
//
// Copyright (c) 2020, The Regents of the University of California,
// through Lawrence Berkeley National Laboratory (subject to receipt of any
// required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pybind11/cast.h"
#include "pybind11/chrono.h"
#include "pybind11/embed.h"
#include "pybind11/eval.h"
#include "pybind11/functional.h"
#include "pybind11/iostream.h"
#include "pybind11/numpy.h"
#include "pybind11/pybind11.h"
#include "pybind11/pytypes.h"
#include "pybind11/stl.h"

#include <random>
#include <type_traits>
#include <vector>

#if defined(USE_MPI) || defined(TIMEMORY_USE_MPI)
#    include <mpi.h>
#endif

namespace py = pybind11;

template <typename _Tp>
using vector_t = std::vector<_Tp>;

static std::mt19937 rng;

//--------------------------------------------------------------------------------------//

template <typename _Tp>
inline vector_t<_Tp>
generate(const int64_t& nsize)
{
    std::vector<_Tp> sendbuf(nsize, 0.0);
    std::mt19937     rng;
    rng.seed(54561434UL);
    auto dist = [&]() { return std::generate_canonical<_Tp, 10>(rng); };
    std::generate(sendbuf.begin(), sendbuf.end(), [&]() { return dist(); });
    return sendbuf;
}

//--------------------------------------------------------------------------------------//

template <typename _Tp>
inline vector_t<_Tp>
allreduce(const vector_t<_Tp>& sendbuf)
{
    vector_t<_Tp> recvbuf(sendbuf.size(), 0.0);
#if defined(USE_MPI) || defined(TIMEMORY_USE_MPI)
    auto dtype = (std::is_same<_Tp, float>::value) ? MPI_FLOAT : MPI_DOUBLE;
    MPI_Allreduce(sendbuf.data(), recvbuf.data(), sendbuf.size(), dtype, MPI_SUM,
                  MPI_COMM_WORLD);
#else
    throw std::runtime_error("No MPI support!");
#endif
    return recvbuf;
}

//--------------------------------------------------------------------------------------//

double
run(int nitr, int nsize)
{
    rng.seed(54561434UL);

    printf("[%s] Running MPI algorithm with %i iterations and %i entries...\n", __func__,
           nitr, nsize);

    double dsum = 0.0;
    for(int i = 0; i < nitr; ++i)
    {
        auto dsendbuf = generate<double>(nsize);
        auto drecvbuf = allreduce(dsendbuf);
        dsum += std::accumulate(drecvbuf.begin(), drecvbuf.end(), 0.0);
    }
    return dsum;
}

//--------------------------------------------------------------------------------------//

PYBIND11_MODULE(libex_python_bindings, ex)
{
    ex.def("run", &run, "Run a calculation", py::arg("nitr") = 10,
           py::arg("nsize") = 5000);
}

//--------------------------------------------------------------------------------------//
