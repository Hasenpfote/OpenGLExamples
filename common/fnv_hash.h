#pragma once
#include <cstdint>
#include <string>

namespace common
{

struct fnv1_hash_32
{
    template<std::size_t N>
    constexpr std::uint32_t operator()(const char(&s)[N]) const noexcept
    {
        return hash_impl(&s[0], N - 1);
    }

    std::uint32_t operator()(const std::string& s) const noexcept
    {
        return hash_impl(s.c_str(), s.size());
    }

private:
    constexpr std::uint32_t hash_impl(const char* p, size_t size) const noexcept
    {
        auto hash = FNV_offset_basis;
        for(decltype(size) i = 0; i < size; i++)
        {
            hash = (hash * FNV_prime) ^ static_cast<std::uint32_t>(p[i]);
        }
        return hash;
    }

    constexpr static std::uint32_t FNV_offset_basis = 2166136261u;
    constexpr static std::uint32_t FNV_prime = 16777619u;
};

struct fnv1_hash_64
{
    template<std::size_t N>
    constexpr std::uint64_t operator()(const char(&s)[N]) const noexcept
    {
        return hash_impl(&s[0], N - 1);
    }

    std::uint64_t operator()(const std::string& s) const noexcept
    {
        return hash_impl(s.c_str(), s.size());
    }

private:
    constexpr std::uint64_t hash_impl(const char* p, std::size_t size) const noexcept
    {
        auto hash = FNV_offset_basis;
        for(decltype(size) i = 0; i < size; i++)
        {
            hash = (hash * FNV_prime) ^ static_cast<std::uint64_t>(p[i]);
        }
        return hash;
    }

    constexpr static std::uint64_t FNV_offset_basis = 14695981039346656037ull;
    constexpr static std::uint64_t FNV_prime = 1099511628211ull;
};

struct fnv1a_hash_32
{
    template<std::size_t N>
    constexpr std::uint32_t operator()(const char(&s)[N]) const noexcept
    {
        return hash_impl(&s[0], N - 1);
    }

    std::uint32_t operator()(const std::string& s) const noexcept
    {
        return hash_impl(s.c_str(), s.size());
    }

private:
    constexpr std::uint32_t hash_impl(const char* p, std::size_t size) const noexcept
    {
        auto hash = FNV_offset_basis;
        for(decltype(size) i = 0; i < size; i++)
        {
            hash = (hash ^ static_cast<std::uint32_t>(p[i])) * FNV_prime;
        }
        return hash;
    }

    constexpr static std::uint32_t FNV_offset_basis = 2166136261u;
    constexpr static std::uint32_t FNV_prime = 16777619u;
};

struct fnv1a_hash_64
{
    template<std::size_t N>
    constexpr std::uint64_t operator()(const char(&s)[N]) const noexcept
    {
        return hash_impl(&s[0], N - 1);
    }

    std::uint64_t operator()(const std::string& s) const noexcept
    {
        return hash_impl(s.c_str(), s.size());
    }

private:
    constexpr std::uint64_t hash_impl(const char* p, std::size_t size) const noexcept
    {
        auto hash = FNV_offset_basis;
        for(decltype(size) i = 0; i < size; i++)
        {
            hash = (hash ^ static_cast<std::uint64_t>(p[i])) * FNV_prime;
        }
        return hash;
    }
    constexpr static std::uint64_t FNV_offset_basis = 14695981039346656037ull;
    constexpr static std::uint64_t FNV_prime = 1099511628211ull;
};

}   // namespace common