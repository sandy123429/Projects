#include "computeGI.hh"

#include "LightMap.hh"

#include <aion/Action.hh>

#include <common/assert.hh>

#include <logic/Level.hh>
#include <logic/GameObject.hh>
#include <logic/Mesh.hh>

#include <glow/data/TextureData.hh>
#include <glow/data/SurfaceData.hh>
#include <glow/common/log.hh>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

void computeGI(Level& level)
{
    ACTION();

    constexpr size_t lightMapSize = 1024;
    LightMap lightMap{lightMapSize, lightMapSize};

    // Construct a representation of the static scene geometry
    {
        ACTION("Construct Scene");

        // Collect all faces of scene objects that should receive global illumination
        // and store them in the list of light map faces

        for (auto const& obj : level.getGameObjects())
        {
            if (obj->getUsesPrecomputedLighting())
            {
                auto& mesh = *obj->getCpuMesh();
                auto const& transform = obj->getTransformation();

                for (auto const& triangle : mesh.getTriangles())
                {
                    LightMapFace f;
                    f.gameObject = obj.get();
                    for (int corner = 0; corner < 3; ++corner)
                    {
                        auto const& p = triangle.vertices[corner].position;
                        f.corners[corner] = glm::vec3{transform * glm::vec4{p, 1.0f}};
                    }
                    lightMap.faces.push_back(std::move(f));
                }
            }
        }
    }

    // Compute layout of light map atlas (UV coordinates for all light map faces)
    {
        ACTION("Create Light Map Atlas");

        // Super stupid atlas generation: Light map is a regular grid of squares,
        // one square per scene triangle.
        //
        // This has several drawbacks:
        // * wastes at least 50% of texture space
        // * strong length distortion
        // * strong angular distortion
        // * discontinuities across triangle borders
        //
        // TODO: Replace this with a more clever segmentation / packing strategy!

        auto gridColumns = static_cast<int>(std::ceil(std::sqrt(lightMap.faces.size())));
        auto cellSize = static_cast<float>(lightMapSize / gridColumns);
        for (size_t i = 0; i < lightMap.faces.size(); ++i)
        {
            auto row = static_cast<int>(i / gridColumns);
            auto col = static_cast<int>(i % gridColumns);
            // Center vertex positions on texels by shifting inward by 0.5
            auto v0 = glm::vec2{col, row} * cellSize + glm::vec2{0.5, 0.5};
            auto v1 = glm::vec2{col + 1, row} * cellSize + glm::vec2{-0.5, 0.5};
            auto v2 = glm::vec2{col + 1, row + 1} * cellSize + glm::vec2{-0.5, -0.5};
            lightMap.faces[i].lightMapUVs[0] = v0 / static_cast<float>(lightMapSize);
            lightMap.faces[i].lightMapUVs[1] = v1 / static_cast<float>(lightMapSize);
            lightMap.faces[i].lightMapUVs[2] = v2 / static_cast<float>(lightMapSize);
        }
    }

    // Compute global illumination and store result in light map texels
    {
        ACTION("Compute Global Illumination");

        // Rasterize faces into light map while computing global illumination for every texel
        for (auto const& f : lightMap.faces)
        {
            // Since we use our (inefficient) non-overlapping box layout for the light map
            // we can just rasterize an axis-aligned rectangle containing the face.
            //
            // TODO: Only compute texels that actually contribute to the appearance of the face!

            auto uvMin = glm::min(f.lightMapUVs[0], glm::min(f.lightMapUVs[1], f.lightMapUVs[2]));
            auto uvMax = glm::max(f.lightMapUVs[0], glm::max(f.lightMapUVs[1], f.lightMapUVs[2]));
            auto texelXMin = static_cast<int>(uvMin.x * lightMapSize);
            auto texelYMin = static_cast<int>(uvMin.y * lightMapSize);
            auto texelXMax = static_cast<int>(uvMax.x * lightMapSize);
            auto texelYMax = static_cast<int>(uvMax.y * lightMapSize);

            for (int y = texelYMin; y <= texelYMax; ++y)
            {
                for (int x = texelXMin; x <= texelXMax; ++x)
                {
                    // TODO: Actually do some global illumination computations here
                    // (or collect results from a previous computation).
                    // For now, we just shade each face based on its normal direction.
                    auto normal = glm::normalize(glm::cross(f.corners[1] - f.corners[0], f.corners[2] - f.corners[0]));
                    float shade = (normal.y + 1.0f) / 2.0f;
                    lightMap.image(x, y) = {shade, shade, shade};
                }
            }
        }
    }

    // Store results
    std::string giDirectory = level.getFileName() + ".data/gi";

    // Save light map texture
    {
        ACTION("Save Light Map Texture");

        // Convert texture data to 24 bit RGB
        std::vector<char> data;
        data.reserve(3 * lightMapSize * lightMapSize);
        for (auto const& texel : lightMap.image.container())
        {
            data.push_back(static_cast<char>(texel.r * 255.0));
            data.push_back(static_cast<char>(texel.g * 255.0));
            data.push_back(static_cast<char>(texel.b * 255.0));
        }

        auto surface = std::make_shared<glow::SurfaceData>();
        surface->setWidth(lightMapSize);
        surface->setHeight(lightMapSize);
        surface->setFormat(GL_RGB);
        surface->setType(GL_UNSIGNED_BYTE);
        surface->setData(std::move(data));

        auto texture = std::make_shared<glow::TextureData>();
        texture->setWidth(lightMapSize);
        texture->setHeight(lightMapSize);
        texture->addSurface(surface);

        texture->saveToFile(giDirectory + "/lightmap.png");
    }

    // Save light map uv coords
    {
        ACTION("Save Light UV Coords");

        for (auto const& obj : level.getGameObjects())
        {
            if (obj->getUsesPrecomputedLighting())
            {
                const auto uvFileName = giDirectory + "/" + obj->getName() + ".uv";
                std::ofstream file{uvFileName};

                for (auto const& f : lightMap.faces)
                {
                    // TODO: more efficient implementation
                    if (f.gameObject == obj.get())
                    {
                        for (int corner = 0; corner < 3; ++corner)
                        {
                            file << f.lightMapUVs[corner].x << " " << f.lightMapUVs[corner].y << std::endl;
                        }
                    }
                }
            }
        }
    }
}
