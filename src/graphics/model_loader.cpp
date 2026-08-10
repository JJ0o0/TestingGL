#include <graphics/model_loader.hpp>
#include <graphics/tangent_generation.hpp>

#include <platform/image_loading.hpp>
#include <platform/default_resources.hpp>
#include <graphics/vertex.hpp>
#include <core/logging.hpp>

#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include <string_view>
#include <utility>
#include <string>
#include <format>

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

static ImageData LoadGltfTextureImage(const fastgltf::Asset& asset, const fastgltf::TextureInfo& textureInfo) {
    const auto& gltfTexture = asset.textures[textureInfo.textureIndex];
    if (!gltfTexture.imageIndex) return {};

    return LoadGltfImage(asset, asset.images[*gltfTexture.imageIndex]);
}

static std::shared_ptr<Texture> LoadGltfTexture(
    const fastgltf::Asset& asset,
    const fastgltf::TextureInfo& textureInfo,
    TextureFormat format,
    const std::filesystem::path& modelPath
) {
    std::string key = std::format(
        "{}#texture:{}:{}",
        modelPath.string(), textureInfo.textureIndex, static_cast<int>(format)
    );

    return ResourceManager::GetOrCreate<Texture>(key, [&]() -> std::shared_ptr<Texture> {
        ImageData imageData = LoadGltfTextureImage(asset, textureInfo);
        if (imageData.Pixels.empty()) return nullptr;

        return std::make_shared<Texture>(imageData, format);
    });
}

static std::shared_ptr<Texture> LoadGltfARMTexture(
    const fastgltf::Asset& asset,
    const std::optional<fastgltf::OcclusionTextureInfo>& aoInfo,
    const std::optional<fastgltf::TextureInfo>& mrInfo,
    const std::filesystem::path& modelPath
) {
    const bool hasMR = mrInfo.has_value();
    const bool hasAO = aoInfo.has_value();
    const bool packedArm = hasMR && hasAO && mrInfo->textureIndex == aoInfo->textureIndex;

    if (packedArm) return LoadGltfTexture(asset, *mrInfo, TextureFormat::RGBA8, modelPath);
    if (!hasMR && !hasAO) return nullptr;

    std::string key = std::format(
        "{}#ARM:{}:{}",
        modelPath.string(),
        hasAO ? std::to_string(aoInfo->textureIndex) : "none",
        hasMR ? std::to_string(mrInfo->textureIndex) : "none"
    );

    return ResourceManager::GetOrCreate<Texture>(key, [&]() -> std::shared_ptr<Texture> {
        ImageData aoImage;
        ImageData mrImage;

        if (hasAO) aoImage = LoadGltfTextureImage(asset, *aoInfo);
        if (hasMR) mrImage = LoadGltfTextureImage(asset, *mrInfo);

        const bool aoHasPixels = !aoImage.Pixels.empty();
        const bool mrHasPixels = !mrImage.Pixels.empty();

        if (hasAO && !aoHasPixels) LogWarning("Failed to load AO texture for '{}'", modelPath.string());
        if (hasMR && !mrHasPixels) LogWarning("Failed to load metallic-roughness texture for '{}'", modelPath.string());
        if (!aoHasPixels && !mrHasPixels) return nullptr;

        const bool isDifferentSize = aoImage.Width != mrImage.Width || aoImage.Height != mrImage.Height;
        if (aoHasPixels && mrHasPixels && isDifferentSize) {
            LogWarning(
                "Cannot pack ARM textures with different sizes: {}x{} and {}x{}",
                aoImage.Width, aoImage.Height,
                mrImage.Width, mrImage.Height
            );

            return nullptr;
        }

        uint32_t width, height;
        if (mrHasPixels) {
            width = mrImage.Width;
            height = mrImage.Height;
        } else {
            width = aoImage.Width;
            height = aoImage.Height;
        }

        ImageData armImage;
        armImage.Width = width;
        armImage.Height = height;

        const size_t pixelCount = static_cast<size_t>(width) * static_cast<size_t>(height);
        armImage.Pixels.resize(pixelCount * 4);

        for (size_t i = 0; i < pixelCount; ++i) {
            const size_t p = i * 4;

            uint8_t ao = 255;
            uint8_t roughness = 255;
            uint8_t metallic = 255;

            if (aoHasPixels) ao = aoImage.Pixels[p + 0];

            if (mrHasPixels) {
                roughness = mrImage.Pixels[p + 1];
                metallic = mrImage.Pixels[p + 2];
            }

            armImage.Pixels[p + 0] = ao;
            armImage.Pixels[p + 1] = roughness;
            armImage.Pixels[p + 2] = metallic;
            armImage.Pixels[p + 3] = 255;
        }

        return std::make_shared<Texture>(armImage, TextureFormat::RGBA8);
    });
}

static std::shared_ptr<Material> CreateFallbackMaterial() {
    auto material = std::make_shared<Material>();

    material->BaseColor = Color{1.0f};
    material->EmissiveColor = Color{0.0f, 0.0f, 0.0f};

    material->Metallic = 0.0f;
    material->Roughness = 1.0f;
    material->NormalScale = 1.0f;
    material->EmissiveStrength = 1.0f;
    material->OcclusionStrength = 1.0f;

    material->BaseColorTexture = DefaultResources::WhiteTexture();
    material->ARMTexture = DefaultResources::WhiteTexture();
    material->NormalTexture = DefaultResources::FlatNormalTexture();
    material->EmissiveTexture = DefaultResources::WhiteTexture();

    return material;
}

std::shared_ptr<Model> ModelLoader::Load(const std::filesystem::path& path) {
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
        material->BaseColor = Color {
            pbr.baseColorFactor[0],
            pbr.baseColorFactor[1],
            pbr.baseColorFactor[2],
            pbr.baseColorFactor[3]
        };

        material->Metallic = pbr.metallicFactor;
        material->Roughness = pbr.roughnessFactor;

        // BASE COLOR TEXTURE
        if (pbr.baseColorTexture) {
            auto texture = LoadGltfTexture(asset, *pbr.baseColorTexture, TextureFormat::SRGBA8, path);
            if (texture) material->BaseColorTexture = std::move(texture);
        }

        // ARM TEXTURE
        material->ARMTexture = LoadGltfARMTexture(asset, gltfMaterial.occlusionTexture, pbr.metallicRoughnessTexture, path);
        if (gltfMaterial.occlusionTexture) material->OcclusionStrength = gltfMaterial.occlusionTexture->strength;

        // NORMAL TEXTURE
        if (gltfMaterial.normalTexture) {
            const auto& normalInfo = *gltfMaterial.normalTexture;

            auto texture = LoadGltfTexture(asset, normalInfo, TextureFormat::RGBA8, path);
            if (texture) material->NormalTexture = std::move(texture);

            material->NormalScale = normalInfo.scale;
        }

        // EMISSIVE
        material->EmissiveColor = Color {
            gltfMaterial.emissiveFactor[0],
            gltfMaterial.emissiveFactor[1],
            gltfMaterial.emissiveFactor[2],
            1.0f
        };

        material->EmissiveStrength = gltfMaterial.emissiveStrength;

        if (gltfMaterial.emissiveTexture) {
            auto texture = LoadGltfTexture(asset, *gltfMaterial.emissiveTexture, TextureFormat::SRGBA8, path);
            if (texture) material->EmissiveTexture = std::move(texture);
        }

        model->m_materials.push_back(std::move(material));
    }

    const uint32_t fallbackMaterialIndex = static_cast<uint32_t>(model->m_materials.size());
    model->m_materials.push_back(CreateFallbackMaterial());

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

            // TANGENT
            const bool hasTangents = GetAccessor(asset, primitive, "TANGENT") != nullptr;
            if (hasTangents) {
                ReadAttribute<glm::vec4>(
                    asset, primitive,
                    "TANGENT",
                    [&](glm::vec4 value, size_t i) {
                        vertices[i].Tangent = value;
                    }
                );
            }

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

            const bool hasUVs = GetAccessor(asset, primitive, "TEXCOORD_0") != nullptr;
            const bool hasNormals = GetAccessor(asset, primitive, "NORMAL") != nullptr;
            if (!hasTangents && hasUVs && hasNormals) {
                LogInfo("Generating tangents for mesh without TANGENT attribute");
                GenerateTangents(vertices, indices);
            }

            const uint32_t materialIndex =
                primitive.materialIndex
                    ? static_cast<uint32_t>(*primitive.materialIndex)
                    : fallbackMaterialIndex;

            modelMesh.Primitives.push_back(ModelPrimitive{
                .Geometry = std::make_shared<Mesh>(vertices, indices),
                .MaterialIndex = materialIndex
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
