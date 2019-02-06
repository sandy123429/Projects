#pragma once

#include "LightMapFace.hh"

#include <common/Grid2D.hh>

#include <glm/glm.hpp>

#include <vector>

struct LightMap
{
    explicit LightMap(std::size_t width, std::size_t height) :
        image(width, height)
    {
    }

    Grid2D<glm::vec3> image;
    std::vector<LightMapFace> faces;
};
