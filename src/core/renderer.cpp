#include <core/renderer.hpp>

#include <graphics/premade_meshes/screen_quad.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>
#include <vector>

namespace {
    std::shared_ptr<Texture> CreateBRDFLUT(uint32_t size = 512) {
        auto brdfLUT = std::make_shared<Texture>(size, size, GL_RG16F, GL_RG, GL_FLOAT);

        uint32_t captureFBO = 0, captureRBO = 0;

        glGenFramebuffers(1, &captureFBO);
        glGenRenderbuffers(1, &captureRBO);

        glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
        glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size, size);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUT->GetID(), 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) LogError("BRDF LUT framebuffer is incomplete");

        auto quad = CreateScreenQuad();

        Shader brdfShader("assets/shaders/ibl/brdf.vert", "assets/shaders/ibl/brdf.frag");

        GLint previousViewport[4];
        glGetIntegerv(GL_VIEWPORT, previousViewport);
        glViewport(0, 0, size, size);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        brdfShader.Bind();
        quad->Draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glViewport(
            previousViewport[0],
            previousViewport[1],
            previousViewport[2],
            previousViewport[3]
        );

        glDeleteRenderbuffers(1, &captureRBO);
        glDeleteFramebuffers(1, &captureFBO);

        return brdfLUT;
    }
}

Renderer::Renderer(Window& window) : m_window(window) {
    const auto& props = m_window.GetProperties();
    m_gBuffer = std::make_unique<GBuffer>(props.Width, props.Height);
    m_hdrFramebuffer = std::make_unique<Framebuffer>(props.Width, props.Height);

    m_skyboxMesh = CreateCubemapCube();
    m_screenQuad = CreateScreenQuad();

    m_pbrShader = std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");
    m_unlitShader = std::make_shared<Shader>("assets/shaders/unlit.vert", "assets/shaders/unlit.frag");

    m_skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    m_skyboxShader->SetInt("uEnvironmentMap", 0);

    m_shadowShader = std::make_shared<Shader>("assets/shaders/shadow.vert", "assets/shaders/shadow.frag");
    m_geometryShader = std::make_shared<Shader>("assets/shaders/deferred/geometry.vert", "assets/shaders/deferred/geometry.frag");

    m_postProcessShader = std::make_shared<Shader>("assets/shaders/post_process.vert", "assets/shaders/post_process.frag");
    m_postProcessShader->SetInt("uScene", 0);

    m_deferredLightingShader = std::make_shared<Shader>("assets/shaders/post_process.vert", "assets/shaders/deferred/lighting.frag");
    m_deferredLightingShader->Bind();
    m_deferredLightingShader->SetInt("uGPosition", 0);
    m_deferredLightingShader->SetInt("uGNormalRoughness", 1);
    m_deferredLightingShader->SetInt("uGAlbedoMetallic", 2);
    m_deferredLightingShader->SetInt("uGEmissiveAO", 3);
    m_deferredLightingShader->SetInt("uIrradianceMap", 4);
    m_deferredLightingShader->SetInt("uPrefilterMap", 5);
    m_deferredLightingShader->SetInt("uBRDFLUT", 6);
    m_deferredLightingShader->SetInt("uShadowMap", 7);

    m_shadowMap = std::make_unique<ShadowMap>(1024, 1024);
    m_brdfLUT = CreateBRDFLUT(512);
}

void Renderer::Render(
    const Scene& scene,
    const Camera& camera,
    const Color& clearColor
) {
    const auto& properties = m_window.GetProperties();
    if (properties.Width == 0 || properties.Height == 0) return;

    resizeRenderTargets(properties.Width, properties.Height);

    auto renderItems = buildRenderItems(scene);
    const glm::mat4 lightSpaceMatrix = calculateLightSpaceMatrix(scene.GetSun());

    renderShadowPass(renderItems, lightSpaceMatrix);
    renderGeometryPass(renderItems, camera);
    renderDeferredLightingPass(scene, camera, lightSpaceMatrix, clearColor);
    renderForwardPass(renderItems, camera);
    renderTransparentPass(renderItems, scene, camera, lightSpaceMatrix);
    renderPostProcessPass();
}

void Renderer::Destroy() {
    m_brdfLUT.reset();
    m_shadowMap.reset();

    m_deferredLightingShader.reset();
    m_postProcessShader.reset();
    m_geometryShader.reset();
    m_shadowShader.reset();
    m_skyboxShader.reset();
    m_unlitShader.reset();
    m_pbrShader.reset();

    m_screenQuad.reset();
    m_skyboxMesh.reset();

    m_hdrFramebuffer.reset();
    m_gBuffer.reset();
}

void Renderer::resizeRenderTargets(uint32_t width, uint32_t height) {
    m_hdrFramebuffer->Resize(width, height);
    m_gBuffer->Resize(width, height);
}

glm::mat4 Renderer::calculateLightSpaceMatrix(const DirectionalLight& sun) {
    const glm::vec3 lightDirection = glm::normalize(sun.Direction);
    const glm::vec3 sceneCenter{0.0f};
    const glm::vec3 lightPosition = sceneCenter - lightDirection * 30.0f;

    const glm::mat4 lightView = glm::lookAt(
        lightPosition,
        sceneCenter,
        glm::vec3(0.0f, 1.0f, 0.0f)
    );

    const glm::mat4 lightProjection = glm::ortho(
        -25.0f, 25.0f,
        -25.0f, 25.0f,
         0.1f, 80.0f
    );

    return lightProjection * lightView;
}

void Renderer::renderShadowPass(const std::vector<RenderItem>& items, const glm::mat4& lightSpaceMatrix) {
    m_shadowMap->Bind();

    glViewport(0, 0, m_shadowMap->GetWidth(), m_shadowMap->GetHeight());

    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);

    m_shadowShader->Bind();
    m_shadowShader->SetMat4("uLightSpaceMatrix", lightSpaceMatrix);

    for (const RenderItem& item : items) {
        const Material& material = *item.MaterialData;
        if (material.Alpha == AlphaMode::Blend) continue;

        m_shadowShader->SetMat4("uModel", item.ModelMatrix);
        material.ApplyBase(*m_shadowShader);

        item.Geometry->Draw();
    }
}

void Renderer::renderGeometryPass(const std::vector<RenderItem>& items, const Camera& camera) {
    m_gBuffer->Bind();

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_geometryShader->Bind();
    m_geometryShader->SetMat4("uView", camera.GetView());
    m_geometryShader->SetMat4("uProjection", camera.GetProjection(m_window.GetAspectRatio()));

    for (const RenderItem& item : items) {
        const Material& material = *item.MaterialData;
        if (material.Type != MaterialType::PBR || material.Alpha == AlphaMode::Blend) continue;

        m_geometryShader->SetMat4("uModel", item.ModelMatrix);
        material.ApplyPBR(*m_geometryShader);

        item.Geometry->Draw();
    }
}

void Renderer::renderDeferredLightingPass(
    const Scene& scene,
    const Camera& camera,
    const glm::mat4& lightSpaceMatrix,
    const Color& clearColor
) {
    const auto& properties = m_window.GetProperties();
    const auto& sun = scene.GetSun();
    const auto& environment = scene.GetEnvironment();

    m_hdrFramebuffer->Bind();
    glViewport(0, 0, properties.Width, properties.Height);

    glEnable(GL_DEPTH_TEST);
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, m_gBuffer->GetID());
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_hdrFramebuffer->GetID());

    glBlitFramebuffer(
        0, 0, properties.Width, properties.Height,
        0, 0, properties.Width, properties.Height,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );

    m_hdrFramebuffer->Bind();

    if (environment) {
        glEnable(GL_DEPTH_TEST);

        drawSkybox(*environment->Skybox, camera);
    }

    glDisable(GL_DEPTH_TEST);

    m_gBuffer->GetPosition().Bind(0);
    m_gBuffer->GetNormalRoughness().Bind(1);
    m_gBuffer->GetAlbedoMetallic().Bind(2);
    m_gBuffer->GetEmissiveAO().Bind(3);

    if (environment) {
        environment->Irradiance->Bind(4);
        environment->Prefilter->Bind(5);
    }

    m_brdfLUT->Bind(6);
    m_shadowMap->BindTexture(7);

    m_deferredLightingShader->Bind();
    m_deferredLightingShader->SetVec3("uCameraPosition", camera.GetPosition());
    m_deferredLightingShader->SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
    m_deferredLightingShader->SetFloat("uEnvironmentIntensity", environment ? environment->Intensity : 0.0f);

    sun.Apply(*m_deferredLightingShader);

    m_screenQuad->Draw();
}

void Renderer::renderForwardPass(const std::vector<RenderItem>& items, const Camera& camera) {
    glEnable(GL_DEPTH_TEST);

    m_unlitShader->Bind();
    m_unlitShader->SetMat4("uView", camera.GetView());
    m_unlitShader->SetMat4("uProjection", camera.GetProjection(m_window.GetAspectRatio()));

    for (const RenderItem& item : items) {
        const Material& material = *item.MaterialData;
        if (material.Type != MaterialType::Unlit || material.Alpha == AlphaMode::Blend) continue;

        m_unlitShader->SetMat4("uModel", item.ModelMatrix);
        material.ApplyBase(*m_unlitShader);

        item.Geometry->Draw();
    }
}

void Renderer::renderTransparentPass(
    const std::vector<RenderItem>& items,
    const Scene& scene,
    const Camera& camera,
    const glm::mat4& lightSpaceMatrix
) {
    std::vector<const RenderItem*> transparentItems;

    for (const RenderItem& item : items) {
        if (item.MaterialData->Alpha == AlphaMode::Blend) transparentItems.push_back(&item);
    }

    if (transparentItems.empty()) return;

    const glm::vec3 cameraPosition = camera.GetPosition();
    std::sort(transparentItems.begin(), transparentItems.end(), [&cameraPosition](const RenderItem* a, const RenderItem* b) {
        const glm::vec3 aPos = glm::vec3(a->ModelMatrix[3]);
        const glm::vec3 bPos = glm::vec3(b->ModelMatrix[3]);

        const glm::vec3 dA = aPos - cameraPosition;
        const glm::vec3 dB = bPos - cameraPosition;

        const float distanceA = glm::dot(dA, dA);
        const float distanceB = glm::dot(dB, dB);

        return distanceA > distanceB;
    });

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthMask(GL_FALSE);

    const auto& environment = scene.GetEnvironment();
    const auto& sun = scene.GetSun();

    m_pbrShader->Bind();
    m_pbrShader->SetMat4("uView", camera.GetView());
    m_pbrShader->SetMat4("uProjection", camera.GetProjection(m_window.GetAspectRatio()));
    m_pbrShader->SetVec3("uCameraPosition", camera.GetPosition());
    m_pbrShader->SetMat4("uLightSpaceMatrix", lightSpaceMatrix);
    m_pbrShader->SetFloat("uEnvironmentIntensity", environment ? environment->Intensity : 0.0f);
    m_pbrShader->SetInt("uIrradianceMap", 4);
    m_pbrShader->SetInt("uPrefilterMap", 5);
    m_pbrShader->SetInt("uBRDFLUT", 6);
    m_pbrShader->SetInt("uShadowMap", 7);

    if (environment) {
        environment->Irradiance->Bind(4);
        environment->Prefilter->Bind(5);
    }

    m_brdfLUT->Bind(6);
    m_shadowMap->BindTexture(7);

    sun.Apply(*m_pbrShader);

    m_unlitShader->Bind();
    m_unlitShader->SetMat4("uView", camera.GetView());
    m_unlitShader->SetMat4("uProjection", camera.GetProjection(m_window.GetAspectRatio()));

    for (const RenderItem* item : transparentItems) {
        const Material& material = *item->MaterialData;
        if (material.Type == MaterialType::PBR) {
            m_pbrShader->Bind();
            m_pbrShader->SetMat4("uModel", item->ModelMatrix);

            material.ApplyPBR(*m_pbrShader);
        } else {
            m_unlitShader->Bind();
            m_unlitShader->SetMat4("uModel", item->ModelMatrix);

            material.ApplyBase(*m_unlitShader);
        }

        item->Geometry->Draw();
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

void Renderer::renderPostProcessPass() {
    const auto& properties = m_window.GetProperties();

    Framebuffer::Unbind();

    glViewport(0, 0, properties.Width, properties.Height);
    glDisable(GL_DEPTH_TEST);

    m_postProcessShader->Bind();
    m_postProcessShader->SetFloat("uExposure", m_settings.Exposure);
    m_postProcessShader->SetInt("uToneMapping", static_cast<int>(m_settings.Tonemapper));

    m_hdrFramebuffer->GetColorAttachment().Bind(0);

    m_screenQuad->Draw();
    glEnable(GL_DEPTH_TEST);
}

void Renderer::collectModelNode(
    const Model& model,
    uint32_t nodeIndex,
    const glm::mat4& parentTransform,
    std::vector<RenderItem>& items
) const {
    const ModelNode& node = model.GetNodes()[nodeIndex];
    const glm::mat4 modelMatrix = parentTransform * node.LocalTransform.GetModelMatrix();

    if (node.MeshIndex) {
        const ModelMesh& mesh = model.GetMeshes()[*node.MeshIndex];
        for (const ModelPrimitive& primitive : mesh.Primitives) {
            if (!primitive.Geometry) continue;
            const auto& material = model.GetMaterials()[primitive.MaterialIndex];

            items.push_back({
                .Geometry = primitive.Geometry.get(),
                .MaterialData = material.get(),
                .ModelMatrix = modelMatrix
            });
        }
    }

    for (uint32_t childIndex : node.Children) {
        collectModelNode(
            model,
            childIndex,
            modelMatrix,
            items
        );
    }
}

std::vector<Renderer::RenderItem> Renderer::buildRenderItems(const Scene& scene) const {
    std::vector<RenderItem> items;
    for (const auto& [_, object] : scene.GetGameObjects()) {
        if (!object.GetModel()) continue;

        const auto& model = object.GetModel();
        const glm::mat4 rootTransform = object.GetTransform().GetModelMatrix();

        for (uint32_t rootNode : model->GetRootNodes()) {
            collectModelNode(
                *model,
                rootNode,
                rootTransform,
                items
            );
        }
    }

    return items;
}

void Renderer::drawSkybox(const Cubemap& cubemap, const Camera& camera) {
    glDepthFunc(GL_LEQUAL);

    const GLboolean cullingEnabled = glIsEnabled(GL_CULL_FACE);
    glDisable(GL_CULL_FACE);

    m_skyboxShader->Bind();
    m_skyboxShader->SetMat4("uView", camera.GetView());
    m_skyboxShader->SetMat4("uProjection", camera.GetProjection(m_window.GetAspectRatio()));

    cubemap.Bind(0);
    m_skyboxMesh->Draw();

    if (cullingEnabled) glEnable(GL_CULL_FACE);
    glDepthFunc(GL_LESS);
}
