#include <core/renderer.hpp>

void Renderer::Render(
    const Scene& scene,
    const Camera& camera,
    const Color& clearColor,
    const Cubemap& environmentMap,
    const Cubemap& irradianceMap,
    const Cubemap& prefilterMap,
    const Texture& brdfLUT
) {
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    irradianceMap.Bind(4);
    prefilterMap.Bind(5);
    brdfLUT.Bind(6);

    const auto& sun = scene.GetSun();
    for (const auto& [_, obj] : scene.GetGameObjects()) {
        drawObject(obj, camera, sun);
    }

    drawSkybox(environmentMap, camera);
}

void Renderer::Destroy() {
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
            material->Apply();

            auto& shader = material->MaterialShader;
            shader->SetMat4("uModel", modelMatrix);
            shader->SetMat4("uView", camera.GetView());
            shader->SetMat4("uProjection",camera.GetProjection(m_window.GetAspectRatio()));
            shader->SetVec3("uCameraPosition", camera.GetPosition());
            shader->SetInt("uIrradianceMap", 4);
            shader->SetInt("uPrefilterMap", 5);
            shader->SetInt("uBRDFLUT", 6);

            sun.Apply(*shader);

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
