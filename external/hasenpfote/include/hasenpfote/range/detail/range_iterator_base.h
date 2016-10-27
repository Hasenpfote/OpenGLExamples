#pragma once
#include <iterator>
#include <type_traits>

namespace hasenpfote{ namespace range{

namespace detail{

template <typename T>
struct range_iterator_base : public std::iterator<std::input_iterator_tag, T>
{
    static_assert(std::is_integral<T>::value, "T must be a integer type.");
    static_assert(!std::is_same<T, bool>::value, "bool type is not supported.");

public:
    range_iterator_base() = default;
    ~range_iterator_base() = default;

    range_iterator_base(const range_iterator_base&) = default;
    range_iterator_base& operator = (const range_iterator_base&) = default;
    range_iterator_base(range_iterator_base&&) = default;
    range_iterator_base& operator = (range_iterator_base&&) = default;

    explicit range_iterator_base(T position)
        : position(position)
    {
    }

    range_iterator_base& operator ++ ()
    {
        ++position;
        return *this;
    }

    range_iterator_base operator ++ (int)
    {
        range_iterator_base temp = *this;
        ++(*this);
        return temp;
    }

    bool operator == (const range_iterator_base& rhs) const
    {
        return position == rhs.position;
    }

    bool operator != (const range_iterator_base& rhs) const
    {
        return !(*this == rhs);
    }

    reference operator * ()
    {
        return position;
    }

    const reference operator * () const
    {
        return position;
    }

protected:
    T position;
};

}

}}