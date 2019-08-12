#pragma once
#include <cassert>
#include <vector>

namespace common
{

template<typename T>
class simple_circular_buffer final
{
public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T*;
    using reference = T &;
    using const_reference = const T &;
    using size_type = std::size_t;
    //using iterator =;
    //using const_iterator =;

    using array_range_t = std::pair<pointer, size_type>;
    using const_array_range_t = std::pair<const_pointer, size_type>;

public:
    simple_circular_buffer() = delete;
    ~simple_circular_buffer() = default;
    simple_circular_buffer(const simple_circular_buffer&) = default;
    simple_circular_buffer(simple_circular_buffer&&) = default;
    simple_circular_buffer& operator=(const simple_circular_buffer&) = default;
    simple_circular_buffer& operator=(simple_circular_buffer&&) = default;

    explicit simple_circular_buffer(size_type capacity = 100)
        : array_(capacity)
    {
        assert(capacity > 1);
        clear();
    }

    reference operator[](size_type index);
    const_reference operator[](size_type index) const;

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    void clear() noexcept
    {
        head_ = 0;
        tail_ = 0;
        contents_size_ = 0;
    }

    void push_front(const_reference item);
    void push_back(const_reference item);

    void pop_front();
    void pop_back();

    size_type head() const noexcept { return head_; }
    size_type tail() const noexcept { return tail_; }

    size_type size() const noexcept { return contents_size_; }

    bool is_empty() const noexcept { return contents_size_ == 0; }
    bool is_full() const noexcept { return contents_size_ == capacity(); }

    size_type capacity() const noexcept { return array_.size(); }

    pointer data() noexcept { return array_.data(); }
    const_pointer data() const noexcept { return array_.data(); }

    array_range_t array_one();
    const_array_range_t array_one() const;

    array_range_t array_two();
    const_array_range_t array_two() const;

    bool is_linearized() const;
    void linearize();

private:
    std::vector<value_type> array_;
    size_type head_;
    size_type tail_;
    size_type contents_size_;
};

template<typename T>
typename simple_circular_buffer<T>::reference
simple_circular_buffer<T>::operator[](size_type index)
{
    return const_cast<reference>(std::as_const(*this).operator[](index));
}

template<typename T>
typename simple_circular_buffer<T>::const_reference
simple_circular_buffer<T>::operator[](size_type index) const
{
    assert(!is_empty() && (index < capacity()));
    constexpr auto max_limit = std::numeric_limits<std::size_t>::max();
    auto diff = max_limit - capacity();
    auto padding = (index > diff) ? diff : static_cast<decltype(diff)>(0);
    auto actual_index = (head_ + index + padding) % capacity();
    return array_[actual_index];
}

template<typename T>
typename simple_circular_buffer<T>::reference
simple_circular_buffer<T>::front()
{
    return const_cast<reference>(std::as_const(*this).front());
}

template<typename T>
typename simple_circular_buffer<T>::const_reference
simple_circular_buffer<T>::front() const
{
    assert(!is_empty());
    return array_[head_];
}

template<typename T>
typename simple_circular_buffer<T>::reference
simple_circular_buffer<T>::back()
{
    return const_cast<reference>(std::as_const(*this).back());
}

template<typename T>
typename simple_circular_buffer<T>::const_reference
simple_circular_buffer<T>::back() const
{
    assert(!is_empty());
    auto index = (tail_ > 0) ? tail_ - 1 : capacity() - 1;
    return array_[tail_];
}

template<typename T>
void simple_circular_buffer<T>::push_front(const_reference item)
{
    const auto cap = capacity();

    head_ = (head_ > 0) ? head_ - 1 : cap - 1;
    array_[head_] = item;

    if(contents_size_ < cap)
    {
        contents_size_++;
    }
    else
    {
        tail_ = (tail_ > 0) ? tail_ - 1 : cap - 1;
    }
}

template<typename T>
void simple_circular_buffer<T>::push_back(const_reference item)
{
    const auto cap = capacity();

    array_[tail_] = item;
    tail_ = (tail_ < (cap - 1)) ? tail_ + 1 : 0;

    if(contents_size_ < cap)
    {
        contents_size_++;
    }
    else
    {
        head_ = (head_ < (cap - 1)) ? head_ + 1 : 0;
    }
}

template<typename T>
void simple_circular_buffer<T>::pop_front()
{
    assert(!is_empty());
    head_ = (head_ < (capacity() - 1)) ? head_ + 1 : 0;
    contents_size_--;
}

template<typename T>
void simple_circular_buffer<T>::pop_back()
{
    assert(!is_empty());
    tail_ = (tail_ > 0) ? tail_ - 1 : capacity() - 1;
    contents_size_--;
}

template<typename T>
typename simple_circular_buffer<T>::array_range_t
simple_circular_buffer<T>::array_one()
{
    assert(!is_empty());
    auto size = (head_ < tail_) ? tail_ - head_ : capacity() - head_;
    return std::make_pair(&array_[head_], size);
}

template<typename T>
typename simple_circular_buffer<T>::const_array_range_t
simple_circular_buffer<T>::array_one() const
{
    assert(!is_empty());
    auto size = (head_ < tail_) ? tail_ - head_ : capacity() - head_;
    return std::make_pair(&array_[head_], size);
}

template<typename T>
typename simple_circular_buffer<T>::array_range_t
simple_circular_buffer<T>::array_two()
{
    assert(!is_empty());
    auto size = (tail_ > head_) ? 0 : tail_;
    return std::make_pair(&array_[0], size);
}

template<typename T>
typename simple_circular_buffer<T>::const_array_range_t
simple_circular_buffer<T>::array_two() const
{
    assert(!is_empty());
    auto size = (tail_ > head_) ? 0 : tail_;
    return std::make_pair(&array_[0], size);
}
#if 0
template<typename T>
bool simple_circular_buffer<T>::is_linearized() const
{
    return head_ == 0;
}

template<typename T>
void simple_circular_buffer<T>::linearize()
{
    if (is_linearized())
        return;

    std::vector<value_type> temp(array_.size());

    auto range1 = array_one();
    for (auto i = static_cast<decltype(range1.second)>(0); i < range1.second; i++)
    {
        temp[i] = std::move(range1.first[i]);
    }
    auto range2 = array_two();
    for (auto i = static_cast<decltype(range2.second)>(0); i < range2.second; i++)
    {
        temp[i] = std::move(range2.first[i]);
    }

    array_ = std::move(temp);
    head_ = 0;
    tail_ = (is_full()) ? 0 : contents_size_;
}
#endif
}   // namespace common