#pragma once

#include <graphics/material.hpp>
#include <world/transform.hpp>
#include <graphics/mesh.hpp>

#include <optional>
#include <cstdint>
#include <memory>
#include <vector>

struct ModelPrimitive {
    std::shared_ptr<Mesh> Geometry;
    std::optional<uint32_t> MaterialIndex;
};

struct ModelMesh {
    std::vector<ModelPrimitive> Primitives;
};

struct ModelNode {
    Transform LocalTransform;
    std::optional<uint32_t> MeshIndex;
    std::vector<uint32_t> Children;
};

class ModelLoader;
class Model {
    public:
        static std::shared_ptr<Model> FromMesh(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material) {
            auto model = std::make_shared<Model>();
            model->m_materials.push_back(std::move(material));

            model->m_meshes.push_back(ModelMesh{
                .Primitives = {
                    ModelPrimitive {
                        .Geometry = std::move(mesh),
                        .MaterialIndex = 0
                    }
                }
            });

            model->m_nodes.push_back(ModelNode {
                .LocalTransform = {},
                .MeshIndex = 0,
                .Children = {}
            });

            model->m_rootNodes.push_back(0);

            return model;
        }

        const auto& GetMeshes() const { return m_meshes; }
        const auto& GetMaterials() const { return m_materials; }
        const auto& GetNodes() const { return m_nodes; }
        const auto& GetRootNodes() const { return m_rootNodes; }
    private:
        friend class ModelLoader;

        std::vector<ModelMesh> m_meshes;
        std::vector<std::shared_ptr<Material>> m_materials;

        std::vector<ModelNode> m_nodes;
        std::vector<uint32_t> m_rootNodes;
};
