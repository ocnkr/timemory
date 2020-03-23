// MIT License
//
// Copyright (c) 2020, The Regents of the University of California,
// through Lawrence Berkeley National Laboratory (subject to receipt of any
// required approvals from the U.S. Dept. of Energy).  All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// storage_initializer of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including without limitation
// the rights to use, storage_initializer, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above storage_initializerright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/**
 * \file timemory/operations/types/storage_initializer.hpp
 * \brief Definition for various functions for storage_initializer in operations
 */

#pragma once

//======================================================================================//
//
#include "timemory/operations/macros.hpp"
//
#include "timemory/operations/types.hpp"
//
#include "timemory/operations/declaration.hpp"
//
//======================================================================================//

namespace tim
{
//
//--------------------------------------------------------------------------------------//
//
///
/// \class storage_initializer
/// \brief This operation class is used for generic storage initalization
///
//
//--------------------------------------------------------------------------------------//
//
template <typename T>
storage_initializer
storage_initializer::get()
{
    if(!trait::runtime_enabled<T>::get())
        return storage_initializer{};

    using storage_type = storage<T>;

    static auto _master = []() {
        auto _instance = storage_type::master_instance();
        consume_parameters(_instance);
        return storage_initializer{};
    }();

    static thread_local auto _worker = []() {
        auto _instance = storage_type::instance();
        consume_parameters(_instance);
        return storage_initializer{};
    }();

    consume_parameters(_master);
    return _worker;
}
//
//--------------------------------------------------------------------------------------//
//
}  // namespace tim
