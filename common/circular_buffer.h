#pragma once
#include <cassert>
#include <vector>

namespace common
{

template<typename T>
class simple_circular_buffer
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef ptrdiff_t difference_type;

public:
    explicit simple_circular_buffer(size_type capacity = 100)
        : array_(capacity), head_(0), tail_(0), contents_size_(0)
    {
    }

    reference front()
    {
        return array_[head_];
    }

    reference back()
    {
        return const_cast<reference>(std::as_const(*this).back());
    }

    const_reference front() const
    {
        return array_[head_];
    }

    const_reference back() const
    {
        auto index = (tail_ > 0)? tail_ - 1 : capacity() - 1;
        return array_[tail_];
    }

    void clear() noexcept
    {
        head_ = 0;
        tail_ = 0;
        contents_size_ = 0;
    }

    void push_back(const value_type& item)
    {
        array_[tail_] = item;

        const auto cap = capacity();

        tail_++;
        if(tail_ == cap)
            tail_ = 0;

        if(contents_size_ < cap)
        {
            contents_size_++;
        }
        else
        {
            head_++;
            if (head_ == cap)
                head_ = 0;
        }
    }

    void pop_front()
    {
        assert(contents_size_ > 0);
        head_++;
        if(head_ == capacity())
            head_ = 0;
        contents_size_--;
    }

    size_type head() const noexcept
    {
        return head_;
    }

    size_type tail() const noexcept
    {
        return tail_;
    }

    size_type size() const noexcept
    {
        return contents_size_;
    }

    bool is_empty() const noexcept
    {
        return contents_size_ == 0;
    }

    bool is_full() const noexcept
    {
        return contents_size_ == capacity();
    }

    size_type capacity() const noexcept
    {
        return array_.size();
    }

    auto data() const noexcept
    {
        return array_.data();
    }

private:
    std::vector<value_type> array_;
    size_type head_;
    size_type tail_;
    size_type contents_size_;
};

}   // namespace common