#pragma once

#include <graphics/model.hpp>
#include <world/transform.hpp>
#include <utils/uuid.hpp>

#include <utility>
#include <memory>
#include <string>

class GameObject {
    public:
        GameObject(const std::string& name = "GameObject")
            : m_uuid(GenerateUUID()), m_name(std::move(name)) {}

        const uuids::uuid& GetUUID() const { return m_uuid; }

        void SetName(std::string name) { m_name = std::move(name); }
        const std::string& GetName() const { return m_name; }

        Transform& GetTransform() { return m_transform; }
        const Transform& GetTransform() const { return m_transform; }

        void SetModel(std::shared_ptr<Model> model) { m_model = std::move(model); }
        const std::shared_ptr<Model>& GetModel() const { return m_model; }
    private:
        uuids::uuid m_uuid;
        std::string m_name;

        Transform m_transform{};
        std::shared_ptr<Model> m_model;
};
