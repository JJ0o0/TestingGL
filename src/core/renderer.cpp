#include <core/renderer.hpp>

void Renderer::Render(
    const Scene& scene,
    const Camera& camera,
    const AmbientLight& ambient,
    const DirectionalLight& sun,
    const Color& clearColor
) {
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    for (const auto& [_, obj] : scene.GetGameObjects()) {
        drawObject(obj, camera, ambient, sun);
    }
}

void Renderer::drawObject(
    const GameObject& object,
    const Camera& camera,
    const AmbientLight& ambient,
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
            ambient,
            sun
        );
    }
}

void Renderer::drawModelNode(
    const Model& model,
    uint32_t nodeIndex,
    const glm::mat4& parentTransform,
    const Camera& camera,
    const AmbientLight& ambient,
    const DirectionalLight& sun
) {
    const ModelNode& node = model.GetNodes()[nodeIndex];
    const glm::mat4 modelMatrix = parentTransform * node.LocalTransform.GetModelMatrix();

    if (node.MeshIndex) {
        const ModelMesh& mesh = model.GetMeshes()[*node.MeshIndex];

        for (const ModelPrimitive& primitive : mesh.Primitives) {
            if (!primitive.Geometry) continue;
            if (!primitive.MaterialIndex) continue;

            const auto& material = model.GetMaterials()[*primitive.MaterialIndex];
            if (!material) continue;

            material->Apply();

            auto& shader = material->MaterialShader;
            shader->SetMat4("uModel", modelMatrix);
            shader->SetMat4("uView", camera.GetView());
            shader->SetMat4("uProjection",camera.GetProjection(m_window.GetAspectRatio()));
            shader->SetVec3("uCameraPosition", camera.GetPosition());

            ambient.Apply(*shader);
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
            ambient,
            sun
        );
    }
}
