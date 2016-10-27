/*!
* @file range.h
* @brief Simple range class.
* @author Hasenpfote
* @date 2016/08/03
*/
#pragma once
#include <iterator>
#include <type_traits>
#include <cassert>
#include "detail/range_iterator_base.h"

namespace hasenpfote{ namespace range{

template <typename T, bool AllowReverseOrder = true>
class range final
{
    static_assert(
        std::is_integral<T>::value &&
        !std::is_const<T>::value &&
        !std::is_volatile<T>::value &&
        !std::is_same<T, bool>::value,
        "The type T must be a integer type (except for bool type) without cv-qualifier.");

public:
    struct iterator final : public detail::range_iterator_base<T>
    {
    public:
        iterator(T position, T step) : range_iterator_base<T>(position), step(step) {}

        iterator& operator ++ ()
        {
            position += step;
            return *this;
        }

        iterator operator ++ (int)
        {
            iterator temp = *this;
            ++(*this);
            return temp;
        }

    private:
        T step;
    };

public:
    range() = delete;
    ~range() = default;

    range(const range&) = default;
    range& operator = (const range&) = default;
    range(range&&) = default;
    range& operator = (range&&) = default;

    range(const T& first, const T& last)
        : first(first), last(last)
    {
        assert(AllowReverseOrder || (last >= first));
    }

    iterator begin() const
    {
        return iterator(first, AllowReverseOrder? ((last >= first)? +1 : -1) : +1);
    }

    iterator end() const
    {
        return iterator(last, AllowReverseOrder? ((last >= first)? +1 : -1) : +1);
    }

private:
    T first;
    T last;
};

}}