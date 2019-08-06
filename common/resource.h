#pragma once
#include <cstdint>
#include <memory>
#include <utility>
#include <filesystem>
#include <unordered_map>
#include <unordered_set>
#include "fnv_hash.h"

namespace common
{

class BaseResource
{
public:
    BaseResource() = default;
    virtual ~BaseResource() = default;

    BaseResource(const BaseResource&) = delete;
    BaseResource& operator = (const BaseResource&) = delete;
    BaseResource(BaseResource&&) = delete;
    BaseResource& operator = (BaseResource&&) = delete;
};

template<typename T>
class Resource : public BaseResource
{
public:
    using string_set_t = std::unordered_set<std::string>;

    Resource() = default;
    virtual ~Resource() = default;

    static const string_set_t& allowed_extensions()
    {
        return T::allowed_extensions_impl();
    }

protected:
    static const string_set_t& allowed_extensions_impl()
    {
        static string_set_t ss;
        return ss;
    }

private:
    T& underlying()
    {
        return static_cast<T&>(*this);
    }

    const T& underlying() const
    {
        return static_cast<const T&>(*this);
    }
};

class ResourceManager
{
public:
#if INTPTR_MAX == INT32_MAX
    using hasher = common::fnv1a_hash_32;
    using key_t = std::uint32_t;
#elif INTPTR_MAX == INT64_MAX
    using hasher = common::fnv1a_hash_64;
    using key_t = std::uint64_t;
#else
#error "Environment not 32 or 64-bit."
#endif

    ResourceManager() = default;
    virtual ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator = (const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator = (ResourceManager&&) = delete;

    template<typename T>
    void AddResource(const std::string& name, std::unique_ptr<T>&& p)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto key = hasher{}(name);
        resources_.emplace(key, std::move(p));
    }

    template <typename T>
    void RemoveResource(key_t key)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto it = resources_.find(key);
        if(it != resources_.end())
        {
            resources_.erase(it);
        }
    }

    template <typename T>
    void RemoveResource(const std::string& name)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        return RemoveResource<T>(hasher{}(name));
    }

    template <typename T>
    void RemoveResources()
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto it = resources_.begin();
        while(it != resources_.end())
        {
            if(dynamic_cast<T*>(it->second.get()))
            {
                it = resources_.erase(it);
                continue;
            }
            it++;
        }
    }

    void RemoveAllResources() noexcept
    {
        resources_.clear();
    }

    template <typename T>
    const T* GetResource(key_t key) const
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto it = resources_.find(key);
        return (it != resources_.end())? dynamic_cast<T*>(it->second.get()) : nullptr;
    }

    template <typename T>
    T* GetResource(key_t key)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        return const_cast<T*>(std::as_const(*this).GetResource<T>(key));
    }

    template <typename T>
    const T* GetResource(const std::string& name) const
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        return GetResource<T>(hasher{}(name));
    }

    template <typename T>
    T* GetResource(const std::string& name)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        return const_cast<T*>(std::as_const(*this).GetResource<T>(name));
    }

protected:
    std::unordered_map<key_t, std::unique_ptr<BaseResource>> resources_;
};

class DefaultResourceManager final : public ResourceManager
{
public:
    DefaultResourceManager() = default;
    ~DefaultResourceManager() = default;

    DefaultResourceManager(const DefaultResourceManager&) = delete;
    DefaultResourceManager& operator = (const DefaultResourceManager&) = delete;
    DefaultResourceManager(DefaultResourceManager&&) = delete;
    DefaultResourceManager& operator = (DefaultResourceManager&&) = delete;

    using ResourceManager::AddResource;

    template<typename T, typename... Args>
    void AddResource(const std::string& name, Args&&... args)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto p = std::make_unique<T>(std::forward<Args>(args)...);
        ResourceManager::AddResource<T>(name, std::move(p));
    }

    template<typename T, typename... Args>
    void AddResourceFromFile(const std::filesystem::path& filepath, Args&&... args)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        auto p = std::make_unique<T>(filepath, std::forward<Args>(args)...);
        ResourceManager::AddResource<T>(filepath.string(), std::move(p));
    }

    template<typename T, typename... Args>
    void AddResourcesFromDirectory(const std::filesystem::path& dirpath, bool is_recursive, Args&&... args)
    {
        static_assert(std::is_base_of_v<BaseResource, T>, "BaseResource is not base of T.");

        namespace fs = std::filesystem;

        auto exts = Resource<T>::allowed_extensions();
        if(exts.empty())
            return;

        auto func = [&](const fs::path& filepath)
        {
            if(!fs::is_regular_file(filepath))
                return;
            if(exts.find(filepath.extension().string()) == exts.end())
                return;
#if defined(_MSC_VER)
            AddResourceFromFile<T>(filepath.generic_string(), std::forward<Args>(args)...);
#else
            AddResourceFromFile<T>(filepath, std::forward<Args>(args)...);
#endif
        };

        if(is_recursive)
            std::for_each(
                fs::recursive_directory_iterator(dirpath),
                fs::recursive_directory_iterator(),
                func
            );
        else
            std::for_each(
                fs::directory_iterator(dirpath),
                fs::directory_iterator(),
                func
            );
    }
};

}   // namespace common