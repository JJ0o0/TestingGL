#include <core/renderer.hpp>

void Renderer::Render(
    const Scene& scene,
    const Camera& camera,
    const Color& clearColor
) {
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
}

void Renderer::Destroy() {
    m_brdfLUT.reset();
    m_pbrShader.reset();
    m_skyboxShader.reset();
    m_skyboxMesh.reset();
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
