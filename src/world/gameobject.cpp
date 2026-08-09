#include <world/gameobject.hpp>

GameObject::GameObject(const std::string& name)
    : m_uuid(GenerateUUID()),
      m_name(std::move(name)) {}

void GameObject::SetName(std::string name) { m_name = std::move(name); }
void GameObject::SetModel(std::shared_ptr<Model> model) { m_model = std::move(model); }
