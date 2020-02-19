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
//

/** \file timemory/variadic/auto_list.hpp
 * \headerfile timemory/variadic/auto_list.hpp "timemory/variadic/auto_list.hpp"
 * Automatic starting and stopping of components. Accept unlimited number of
 * parameters. The constructor starts the components, the destructor stops the
 * components
 *
 * Usage with macros (recommended):
 *    \param TIMEMORY_AUTO_LIST()
 *    \param TIMEMORY_BASIC_AUTO_LIST()
 *    \param auto t = TIMEMORY_AUTO_LIST_OBJ()
 *    \param auto t = TIMEMORY_BASIC_AUTO_LIST_OBJ()
 */

#pragma once

#include <cstdint>
#include <string>

#include "timemory/mpl/filters.hpp"
#include "timemory/runtime/initialize.hpp"
#include "timemory/utility/macros.hpp"
#include "timemory/utility/utility.hpp"
#include "timemory/variadic/component_list.hpp"
#include "timemory/variadic/macros.hpp"
#include "timemory/variadic/types.hpp"

namespace tim
{
//--------------------------------------------------------------------------------------//

template <typename... Types>
class auto_list
{
public:
    using this_type           = auto_list<Types...>;
    using base_type           = component_list<Types...>;
    using component_type      = typename base_type::component_type;
    using data_type           = typename component_type::data_type;
    using type_tuple          = typename component_type::type_tuple;
    using data_value_type     = typename component_type::data_value_type;
    using data_label_type     = typename component_type::data_label_type;
    using sample_type         = typename component_type::sample_type;
    using type                = convert_t<typename component_type::type, auto_list<>>;
    using initializer_type    = std::function<void(this_type&)>;
    using string_t            = std::string;
    using captured_location_t = typename component_type::captured_location_t;

    // used by component hybrid and gotcha
    static constexpr bool is_component_list   = false;
    static constexpr bool is_component_tuple  = false;
    static constexpr bool is_component_hybrid = false;
    static constexpr bool is_component_type   = false;
    static constexpr bool is_auto_list        = true;
    static constexpr bool is_auto_tuple       = false;
    static constexpr bool is_auto_hybrid      = false;
    static constexpr bool is_auto_type        = true;
    static constexpr bool is_component        = false;
    static constexpr bool has_gotcha_v        = component_type::has_gotcha_v;
    static constexpr bool has_user_bundle_v   = component_type::has_user_bundle_v;

public:
    //----------------------------------------------------------------------------------//
    //
    static void init_storage() { component_type::init_storage(); }

    //----------------------------------------------------------------------------------//
    //
    static auto& get_initializer()
    {
        static auto env_enum = enumerate_components(
            tim::delimit(tim::get_env<string_t>("TIMEMORY_AUTO_LIST_INIT", "")));

        static initializer_type _instance = [=](this_type& cl) {
            ::tim::initialize(cl, env_enum);
        };
        return _instance;
    }

public:
    template <typename _Func = initializer_type>
    explicit auto_list(const string_t&, bool flat = settings::flat_profile(),
                       bool report_at_exit = settings::destructor_report(),
                       const _Func&        = get_initializer());

    template <typename _Func = initializer_type>
    explicit auto_list(const captured_location_t&, bool flat = settings::flat_profile(),
                       bool report_at_exit = settings::destructor_report(),
                       const _Func&        = get_initializer());

    template <typename _Func = initializer_type>
    explicit auto_list(size_t, bool flat = settings::flat_profile(),
                       bool report_at_exit = settings::destructor_report(),
                       const _Func&        = get_initializer());

    explicit auto_list(component_type& tmp, bool flat = settings::flat_profile(),
                       bool report_at_exit = settings::destructor_report());
    ~auto_list();

    // copy and move
    inline auto_list(const this_type&) = default;
    inline auto_list(this_type&&)      = default;
    inline this_type& operator=(const this_type&) = default;
    inline this_type& operator=(this_type&&) = default;

    static constexpr std::size_t size() { return component_type::size(); }

public:
    // public member functions
    inline component_type&       get_component() { return m_temporary_object; }
    inline const component_type& get_component() const { return m_temporary_object; }

    inline operator component_type&() { return m_temporary_object; }
    inline operator const component_type&() const { return m_temporary_object; }

    // partial interface to underlying component_list
    inline void measure()
    {
        if(m_enabled)
            m_temporary_object.measure();
    }
    inline void sample()
    {
        if(m_enabled)
            m_temporary_object.sample();
    }
    inline void start()
    {
        if(m_enabled)
            m_temporary_object.start();
    }
    inline void stop()
    {
        if(m_enabled)
            m_temporary_object.stop();
    }
    inline void push()
    {
        if(m_enabled)
            m_temporary_object.push();
    }
    inline void pop()
    {
        if(m_enabled)
            m_temporary_object.pop();
    }
    template <typename... _Args>
    inline void mark_begin(_Args&&... _args)
    {
        if(m_enabled)
            m_temporary_object.mark_begin(std::forward<_Args>(_args)...);
    }
    template <typename... _Args>
    inline void mark_end(_Args&&... _args)
    {
        if(m_enabled)
            m_temporary_object.mark_end(std::forward<_Args>(_args)...);
    }
    template <typename... _Args>
    inline void store(_Args&&... _args)
    {
        if(m_enabled)
            m_temporary_object.store(std::forward<_Args>(_args)...);
    }
    template <typename... _Args>
    inline void audit(_Args&&... _args)
    {
        if(m_enabled)
            m_temporary_object.audit(std::forward<_Args>(_args)...);
    }

    inline data_value_type get() const { return m_temporary_object.get(); }

    inline data_label_type get_labeled() const
    {
        return m_temporary_object.get_labeled();
    }

    inline bool enabled() const { return m_enabled; }
    inline void report_at_exit(bool val) { m_report_at_exit = val; }
    inline bool report_at_exit() const { return m_report_at_exit; }

    inline bool             store() const { return m_temporary_object.store(); }
    inline data_type&       data() { return m_temporary_object.data(); }
    inline const data_type& data() const { return m_temporary_object.data(); }
    inline int64_t          laps() const { return m_temporary_object.laps(); }
    inline string_t         key() const { return m_temporary_object.key(); }
    inline uint64_t         hash() const { return m_temporary_object.hash(); }
    inline void rekey(const string_t& _key) { m_temporary_object.rekey(_key); }

public:
    template <typename _Tp>
    decltype(auto) get()
    {
        return m_temporary_object.template get<_Tp>();
    }

    template <typename _Tp>
    decltype(auto) get() const
    {
        return m_temporary_object.template get<_Tp>();
    }

    inline void get(void*& ptr, size_t _hash) { m_temporary_object.get(ptr, _hash); }

    template <typename _Tp, typename... _Args>
    void init(_Args&&... _args)
    {
        m_temporary_object.template init<_Tp>(std::forward<_Args>(_args)...);
    }

    template <typename... T>
    void initialize()
    {
        m_temporary_object.template initialize<T...>();
    }

public:
    friend std::ostream& operator<<(std::ostream& os, const this_type& obj)
    {
        os << obj.m_temporary_object;
        return os;
    }

private:
    bool            m_enabled        = true;
    bool            m_report_at_exit = false;
    component_type  m_temporary_object;
    component_type* m_reference_object = nullptr;
};

//======================================================================================//

template <typename... Types>
template <typename _Func>
auto_list<Types...>::auto_list(const string_t& key, bool flat, bool report_at_exit,
                               const _Func& _func)
: m_enabled(settings::enabled())
, m_report_at_exit(report_at_exit)
, m_temporary_object(m_enabled ? component_type(key, m_enabled, flat) : component_type{})
, m_reference_object(nullptr)
{
    if(m_enabled)
    {
        _func(*this);
        m_temporary_object.start();
    }
}

//--------------------------------------------------------------------------------------//

template <typename... Types>
template <typename _Func>
auto_list<Types...>::auto_list(const captured_location_t& loc, bool flat,
                               bool report_at_exit, const _Func& _func)
: m_enabled(settings::enabled())
, m_report_at_exit(report_at_exit)
, m_temporary_object(m_enabled ? component_type(loc, m_enabled, flat) : component_type{})
, m_reference_object(nullptr)
{
    if(m_enabled)
    {
        _func(*this);
        m_temporary_object.start();
    }
}

//--------------------------------------------------------------------------------------//

template <typename... Types>
template <typename _Func>
auto_list<Types...>::auto_list(size_t _hash, bool flat, bool report_at_exit,
                               const _Func& _func)
: m_enabled(settings::enabled())
, m_report_at_exit(report_at_exit)
, m_temporary_object(m_enabled ? component_type(_hash, m_enabled, flat)
                               : component_type{})
, m_reference_object(nullptr)
{
    if(m_enabled)
    {
        _func(*this);
        m_temporary_object.start();
    }
}

//--------------------------------------------------------------------------------------//

template <typename... Types>
auto_list<Types...>::auto_list(component_type& tmp, bool flat, bool report_at_exit)
: m_enabled(true)
, m_report_at_exit(report_at_exit)
, m_temporary_object(tmp.clone(true, flat))
, m_reference_object(&tmp)
{
    if(m_enabled)
    {
        m_temporary_object.start();
    }
}

//--------------------------------------------------------------------------------------//

template <typename... Types>
auto_list<Types...>::~auto_list()
{
    if(m_enabled)
    {
        // stop the timer
        m_temporary_object.stop();

        // report timer at exit
        if(m_report_at_exit)
        {
            std::stringstream ss;
            ss << m_temporary_object;
            if(ss.str().length() > 0)
                std::cout << ss.str() << std::endl;
        }

        if(m_reference_object)
        {
            *m_reference_object += m_temporary_object;
        }
    }
}

//======================================================================================//

template <typename... _Types,
          typename _Ret = typename auto_list<_Types...>::data_value_type>
_Ret
get(const auto_list<_Types...>& _obj)
{
    return (_obj.enabled()) ? get(_obj.get_component()) : _Ret{};
}

//--------------------------------------------------------------------------------------//

template <typename... _Types,
          typename _Ret = typename auto_list<_Types...>::data_label_type>
_Ret
get_labeled(const auto_list<_Types...>& _obj)
{
    return (_obj.enabled()) ? get_labeled(_obj.get_component()) : _Ret{};
}

//======================================================================================//

}  // namespace tim

//======================================================================================//

//--------------------------------------------------------------------------------------//
// variadic versions

#define TIMEMORY_VARIADIC_BLANK_AUTO_LIST(tag, ...)                                      \
    using _TIM_TYPEDEF(__LINE__) = ::tim::auto_list<__VA_ARGS__>;                        \
    TIMEMORY_BLANK_AUTO_LIST(_TIM_TYPEDEF(__LINE__), tag);

#define TIMEMORY_VARIADIC_BASIC_AUTO_LIST(tag, ...)                                      \
    using _TIM_TYPEDEF(__LINE__) = ::tim::auto_list<__VA_ARGS__>;                        \
    TIMEMORY_BASIC_AUTO_LIST(_TIM_TYPEDEF(__LINE__), tag);

#define TIMEMORY_VARIADIC_AUTO_LIST(tag, ...)                                            \
    using _TIM_TYPEDEF(__LINE__) = ::tim::auto_list<__VA_ARGS__>;                        \
    TIMEMORY_AUTO_LIST(_TIM_TYPEDEF(__LINE__), tag);

//======================================================================================//
//
//      std::get operator
//
namespace std
{
//--------------------------------------------------------------------------------------//

template <std::size_t N, typename... Types>
auto
get(tim::auto_list<Types...>& obj) -> decltype(get<N>(obj.data()))
{
    return get<N>(obj.data());
}

//--------------------------------------------------------------------------------------//

template <std::size_t N, typename... Types>
auto
get(const tim::auto_list<Types...>& obj) -> decltype(get<N>(obj.data()))
{
    return get<N>(obj.data());
}

//--------------------------------------------------------------------------------------//

template <std::size_t N, typename... Types>
auto
get(tim::auto_list<Types...>&& obj)
    -> decltype(get<N>(std::forward<tim::auto_list<Types...>>(obj).data()))
{
    using obj_type = tim::auto_list<Types...>;
    return get<N>(std::forward<obj_type>(obj).data());
}

//--------------------------------------------------------------------------------------//

template <typename... Types>
TSTAG(struct)
tuple_size<::tim::auto_list<Types...>>
{
public:
    using value_type                  = size_t;
    using type                        = typename ::tim::auto_list<Types...>::type_tuple;
    static constexpr value_type value = tuple_size<type>::value;
};

//======================================================================================//

}  // namespace std

//======================================================================================//
