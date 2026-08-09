#pragma once

#include <graphics/model.hpp>
#include <world/transform.hpp>
#include <utils/uuid.hpp>

#include <memory>
#include <string>

class GameObject {
    public:
        GameObject(const std::string& name = "GameObject");

        const UUID& GetUUID() const { return m_uuid; }

        void SetName(std::string name);
        const std::string& GetName() const { return m_name; }

        Transform& GetTransform() { return m_transform; }
        const Transform& GetTransform() const { return m_transform; }

        void SetModel(std::shared_ptr<Model> model);
        const std::shared_ptr<Model>& GetModel() const { return m_model; }
    private:
        UUID m_uuid;
        std::string m_name;

        Transform m_transform{};
        std::shared_ptr<Model> m_model;
};
