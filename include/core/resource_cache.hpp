#pragma once

#include <unordered_map>
#include <utility>
#include <string>
#include <memory>

template<typename T>
class ResourceCache {
    public:
        static std::shared_ptr<T> Get(const std::string& key) {
            const auto it = s_resources.find(key);
            if (it == s_resources.end()) return nullptr;
            return it->second;
        }

        template<typename Factory>
        static std::shared_ptr<T> GetOrCreate(std::string key, Factory&& factory) {
            if (auto resource = Get(key)) return resource;

            auto resource = std::forward<Factory>(factory)();
            if (!resource) return nullptr;

            s_resources.emplace(std::move(key), resource);
            return resource;
        }

        static bool Contains(const std::string& key) { return s_resources.contains(key); }
        static bool Remove(const std::string& key) { return s_resources.erase(key) > 0; }

        static size_t Count() { return s_resources.size(); }

        static void Clear() { s_resources.clear();}
    private:
        static inline std::unordered_map<std::string, std::shared_ptr<T>> s_resources;
};
