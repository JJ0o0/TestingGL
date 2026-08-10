#pragma once

#include <graphics/vertex.hpp>

#include <cstdint>
#include <vector>

void GenerateTangents(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
