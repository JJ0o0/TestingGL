#include <graphics/tangent_generation.hpp>

#include <mikktspace.h>

struct MikkMeshData {
    std::vector<Vertex>* Vertices;
};

void GenerateTangents(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
    if (indices.empty()) return;

    std::vector<Vertex> expandedVertices;
    expandedVertices.reserve(indices.size());

    for (uint32_t index : indices) expandedVertices.push_back(vertices[index]);

    MikkMeshData data {
        .Vertices = &expandedVertices
    };

    SMikkTSpaceInterface interface {};

    interface.m_getNumFaces = [](const SMikkTSpaceContext* context) -> int {
        const auto* data = static_cast<const MikkMeshData*>(context->m_pUserData);
        return static_cast<int>(data->Vertices->size() / 3);
    };

    interface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext*, int) -> int { return 3; };

    interface.m_getPosition = [](
        const SMikkTSpaceContext* context,
        float out[],
        int face,
        int vert
    ) {
        const auto* data = static_cast<const MikkMeshData*>(context->m_pUserData);
        const Vertex& vertex = (*data->Vertices)[face * 3 + vert];

        out[0] = vertex.Position.x;
        out[1] = vertex.Position.y;
        out[2] = vertex.Position.z;
    };

    interface.m_getNormal = [](
        const SMikkTSpaceContext* context,
        float out[],
        int face,
        int vert
    ) {
        const auto* data = static_cast<const MikkMeshData*>(context->m_pUserData);
        const Vertex& vertex = (*data->Vertices)[face * 3 + vert];

        out[0] = vertex.Normal.x;
        out[1] = vertex.Normal.y;
        out[2] = vertex.Normal.z;
    };

    interface.m_getTexCoord = [](
        const SMikkTSpaceContext* context,
        float out[],
        int face,
        int vert
    ) {
        const auto* data = static_cast<const MikkMeshData*>(context->m_pUserData);
        const Vertex& vertex = (*data->Vertices)[face * 3 + vert];

        out[0] = vertex.TexCoords.x;
        out[1] = vertex.TexCoords.y;
    };

    interface.m_setTSpaceBasic = [](
        const SMikkTSpaceContext* context,
        const float tangent[],
        float sign,
        int face,
        int vert
    ) {
        auto* data = static_cast<MikkMeshData*>(context->m_pUserData);
        Vertex& vertex = (*data->Vertices)[face * 3 + vert];

        vertex.Tangent = {
            tangent[0],
            tangent[1],
            tangent[2],
            sign
        };
    };

    SMikkTSpaceContext context {
        .m_pInterface = &interface,
        .m_pUserData = &data
    };

    const bool success = genTangSpaceDefault(&context);
    if (!success) return;

    vertices = std::move(expandedVertices);
    indices.resize(vertices.size());

    for (uint32_t i = 0; i < indices.size(); ++i) indices[i] = i;
}
