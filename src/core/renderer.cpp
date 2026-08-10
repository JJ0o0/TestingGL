#include <core/renderer.hpp>

#include <graphics/premade_meshes/screen_quad.hpp>

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
    m_hdrFramebuffer = std::make_unique<Framebuffer>(props.Width, props.Height);

    m_skyboxMesh = CreateCubemapCube();
    m_screenQuad = CreateScreenQuad();

    m_pbrShader = std::make_shared<Shader>("assets/shaders/basic.vert", "assets/shaders/basic.frag");

    m_skyboxShader = std::make_shared<Shader>("assets/shaders/skybox.vert", "assets/shaders/skybox.frag");
    m_skyboxShader->SetInt("uEnvironmentMap", 0);

    m_postProcessShader = std::make_shared<Shader>("assets/shaders/post_process.vert", "assets/shaders/post_process.frag");
    m_postProcessShader->SetInt("uScene", 0);

    m_brdfLUT = CreateBRDFLUT(512);
}

void Renderer::Render(
    const Scene& scene,
    const Camera& camera,
    const Color& clearColor
) {
    const auto& properties = m_window.GetProperties();
    if (properties.Width == 0 || properties.Height == 0) return;

    m_hdrFramebuffer->Resize(properties.Width, properties.Height);

    // HDR SCENE PASS
    m_hdrFramebuffer->Bind();

    glEnable(GL_DEPTH_TEST);
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto& environment = scene.GetEnvironment();
    if (environment) {
        environment->Irradiance->Bind(4);
        environment->Prefilter->Bind(5);
    }

    m_brdfLUT->Bind(6);

    const auto& sun = scene.GetSun();
    for (const auto& [_, obj] : scene.GetGameObjects()) {
        drawObject(obj, camera, sun);
    }

    if (environment) drawSkybox(*environment->Skybox, camera);

    // POST PROCESS PASS
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

void Renderer::Destroy() {
    m_brdfLUT.reset();

    m_postProcessShader.reset();
    m_pbrShader.reset();
    m_skyboxShader.reset();

    m_screenQuad.reset();
    m_skyboxMesh.reset();

    m_hdrFramebuffer.reset();
}

void Renderer::drawObject(
    const GameObject& object,
    const Camera& camera,
    const DirectionalLight& sun
) {
    if (!object.GetModel()) return;

    const auto& model = object.GetModel();
    const glm::mat4 rootTransform = object.GetTransform().GetModelMatrix();

    for (uint32_t rootNode : model->GetRootNodes()) {
        drawModelNode(
            *model,
            rootNode,
            rootTransform,
            camera,
            sun
        );
    }
}

void Renderer::drawModelNode(
    const Model& model,
    uint32_t nodeIndex,
    const glm::mat4& parentTransform,
    const Camera& camera,
    const DirectionalLight& sun
) {
    const ModelNode& node = model.GetNodes()[nodeIndex];
    const glm::mat4 modelMatrix = parentTransform * node.LocalTransform.GetModelMatrix();

    if (node.MeshIndex) {
        const ModelMesh& mesh = model.GetMeshes()[*node.MeshIndex];

        for (const ModelPrimitive& primitive : mesh.Primitives) {
            if (!primitive.Geometry) continue;

            const auto& material = model.GetMaterials()[primitive.MaterialIndex];

            m_pbrShader->Bind();
            material->Apply(*m_pbrShader);

            m_pbrShader->SetMat4("uModel", modelMatrix);
            m_pbrShader->SetMat4("uView", camera.GetView());
            m_pbrShader->SetMat4("uProjection",camera.GetProjection(m_window.GetAspectRatio()));
            m_pbrShader->SetVec3("uCameraPosition", camera.GetPosition());
            m_pbrShader->SetInt("uIrradianceMap", 4);
            m_pbrShader->SetInt("uPrefilterMap", 5);
            m_pbrShader->SetInt("uBRDFLUT", 6);

            sun.Apply(*m_pbrShader);

            primitive.Geometry->Draw();
        }
    }

    for (uint32_t childIndex : node.Children) {
        drawModelNode(
            model,
            childIndex,
            modelMatrix,
            camera,
            sun
        );
    }
}

void Renderer::drawSkybox(
    const Cubemap& cubemap,
    const Camera& camera
) {
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
