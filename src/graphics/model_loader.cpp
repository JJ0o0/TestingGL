#include <graphics/model_loader.hpp>

#include <platform/image_loading.hpp>
#include <graphics/vertex.hpp>
#include <core/logging.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <string_view>
#include <utility>

static const fastgltf::Accessor* GetAccessor(
    const fastgltf::Asset& asset,
    const fastgltf::Primitive& primitive,
    std::string_view attributeName
) {
    const auto* attribute = primitive.findAttribute(attributeName);
    if (attribute == primitive.attributes.end()) return nullptr;

    return &asset.accessors[attribute->accessorIndex];
}

template<typename T, typename F>
static void ReadAttribute(
    const fastgltf::Asset& asset,
    const fastgltf::Primitive& primitive,
    std::string_view name,
    F&& callback
) {
    const auto* accessor = GetAccessor(asset, primitive, name);
    if (!accessor) return;

    fastgltf::iterateAccessorWithIndex<T>(asset, *accessor, std::forward<F>(callback));
}

static ImageData LoadGltfImage(
    const fastgltf::Asset& asset,
    const fastgltf::Image& image
) {
    if (const auto* array = std::get_if<fastgltf::sources::Array>(&image.data)) {
        return LoadImage(
            std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(array->bytes.data()), array->bytes.size()}
        );
    }

    if (const auto* bytes = std::get_if<fastgltf::sources::ByteView>(&image.data)) {
        return LoadImage(
            std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(bytes->bytes.data()), bytes->bytes.size()}
        );
    }

    if (const auto* source = std::get_if<fastgltf::sources::BufferView>(&image.data)) {
        const auto& bufferView = asset.bufferViews[source->bufferViewIndex];
        const auto& buffer = asset.buffers[bufferView.bufferIndex];

        if (const auto* array = std::get_if<fastgltf::sources::Array>(&buffer.data)) {
            const auto* begin = reinterpret_cast<const uint8_t*>(array->bytes.data()) + bufferView.byteOffset;

            return LoadImage(
                std::span<const uint8_t>{
                    begin,
                    bufferView.byteLength
                }
            );
        }

        if (const auto* bytes = std::get_if<fastgltf::sources::ByteView>(&buffer.data)) {
            const auto* begin = reinterpret_cast<const uint8_t*>(bytes->bytes.data()) + bufferView.byteOffset;

            return LoadImage(
                std::span<const uint8_t>{
                    begin,
                    bufferView.byteLength
                }
            );
        }
    }

    LogWarning("Unsupported glTF image data source");
    return {};
}

static std::shared_ptr<Texture> LoadGltfTexture(
    const fastgltf::Asset& asset,
    const fastgltf::TextureInfo& textureInfo,
    TextureFormat format
) {
    const auto& gltfTexture = asset.textures[textureInfo.textureIndex];
    if (!gltfTexture.imageIndex) return nullptr;

    const auto& image = asset.images[*gltfTexture.imageIndex];
    ImageData imageData = LoadGltfImage(asset, image);
    if (imageData.Pixels.empty()) return nullptr;

    return std::make_shared<Texture>(
        imageData,
        format
    );
}

std::shared_ptr<Model> ModelLoader::Load(
    const std::filesystem::path& path,
    std::shared_ptr<Material> fallbackMaterial
) {
    auto file = fastgltf::GltfDataBuffer::FromPath(path);

    if (file.error() != fastgltf::Error::None) {
        LogError("Failed to open model at {}: {}", path.string(), fastgltf::getErrorMessage(file.error()));
        return nullptr;
    }

    fastgltf::Parser parser;

    constexpr auto options = fastgltf::Options::LoadExternalBuffers |
                             fastgltf::Options::LoadExternalImages |
                             fastgltf::Options::GenerateMeshIndices |
                             fastgltf::Options::DecomposeNodeMatrices;

    auto loaded = parser.loadGltf(file.get(), path.parent_path(), options);

    if (loaded.error() != fastgltf::Error::None) {
        LogError("Failed to parse model at {}: {}", path.string(), fastgltf::getErrorMessage(loaded.error()));
        return nullptr;
    }

    const fastgltf::Asset& asset = loaded.get();

    auto model = std::make_shared<Model>();

    // MATERIALS
    for (const auto& gltfMaterial : asset.materials) {
        const auto& pbr = gltfMaterial.pbrData;

        auto material = std::make_shared<Material>();
        material->MaterialShader = fallbackMaterial->MaterialShader;
        material->Tint = Color {
            pbr.baseColorFactor[0],
            pbr.baseColorFactor[1],
            pbr.baseColorFactor[2],
            pbr.baseColorFactor[3]
        };

        material->Diffuse = DefaultResources::WhiteTexture();

        material->Specular = DefaultResources::BlackTexture();
        material->Shininess = 32.0f;

        // BASE
        if (pbr.baseColorTexture) {
            auto texture = LoadGltfTexture(asset, *pbr.baseColorTexture, TextureFormat::SRGBA8);
            if (texture) material->Diffuse = std::move(texture);
        }

        model->m_materials.push_back(std::move(material));
    }

    // MESHES
    for (const auto& gltfMesh : asset.meshes) {
        ModelMesh modelMesh;

        for (const auto& primitive : gltfMesh.primitives) {
            // POSITION
            const auto* positionAccessor = GetAccessor(asset, primitive, "POSITION");
            if (!positionAccessor) continue;

            std::vector<Vertex> vertices(positionAccessor->count);

            if (primitive.type != fastgltf::PrimitiveType::Triangles) {
                LogWarning("Skipping non-triangle primitive");
                continue;
            }

            fastgltf::iterateAccessorWithIndex<glm::vec3>(
                asset, *positionAccessor,
                [&](glm::vec3 position, size_t index) {
                    vertices[index].Position = position;
                }
            );

            // NORMAL
            ReadAttribute<glm::vec3>(
                asset, primitive,
                "NORMAL",
                [&](glm::vec3 value, size_t i) {
                    vertices[i].Normal = value;
                }
            );

            // TEXTURE COORDS
            ReadAttribute<glm::vec2>(
                asset, primitive,
                "TEXCOORD_0",
                [&](glm::vec2 value, size_t i) {
                    vertices[i].TexCoords = value;
                }
            );

            // INDICES
            std::vector<uint32_t> indices;
            if (primitive.indicesAccessor) {
                const auto& accessor = asset.accessors[*primitive.indicesAccessor];
                indices.resize(accessor.count);

                fastgltf::iterateAccessorWithIndex<uint32_t>(
                    asset, accessor,
                    [&](uint32_t value, size_t i) {
                        indices[i] = value;
                    }
                );
            }

            modelMesh.Primitives.push_back(ModelPrimitive{
                .Geometry = std::make_shared<Mesh>(vertices, indices),
                .MaterialIndex = primitive.materialIndex
                    ? std::optional<uint32_t>{static_cast<uint32_t>(*primitive.materialIndex)}
                    : std::nullopt
            });
        }

        model->m_meshes.push_back(std::move(modelMesh));
    }

    // NODES
    model->m_nodes.reserve(asset.nodes.size());

    for (const auto& gltfNode : asset.nodes) {
        ModelNode node;

        const auto& trs = std::get<fastgltf::TRS>(gltfNode.transform);

        // TRANSLATION
        node.LocalTransform.Position = {trs.translation[0], trs.translation[1], trs.translation[2]};

        // ROTATION
        node.LocalTransform.Rotation = glm::quat {
            trs.rotation[3],
            trs.rotation[0],
            trs.rotation[1],
            trs.rotation[2],
        };

        // SCALE
        node.LocalTransform.Scale = {trs.scale[0], trs.scale[1], trs.scale[2]};

        // MESH
        if (gltfNode.meshIndex) {
            node.MeshIndex = static_cast<uint32_t>(*gltfNode.meshIndex);
        }

        // CHILDREN
        node.Children.reserve(gltfNode.children.size());

        for (size_t child : gltfNode.children) {
            node.Children.push_back(static_cast<uint32_t>(child));
        }

        model->m_nodes.push_back(std::move(node));
    }

    // ROOT NODES
    if (!asset.scenes.empty()) {
        size_t sceneIndex = 0;
        if (asset.defaultScene) sceneIndex = *asset.defaultScene;

        const auto& scene = asset.scenes[sceneIndex];
        for (size_t nodeIndex : scene.nodeIndices) {
            model->m_rootNodes.push_back(static_cast<uint32_t>(nodeIndex));
        }
    }

    return model;
}
