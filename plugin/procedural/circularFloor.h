//
// Created by Erik Jourgensen on 6/20/26.
//
#pragma once

namespace AnimatedLpg
{
    struct FloorVertex
    {
        float x, y, z;
        float nX, nY, nZ; //vertex normals, used for lighting
        float r, g, b;
    };

    using FloorIndex = uint16_t;

    constexpr float PI = 3.14159265358979323846f;

    class circularFloor
    {
    public:

        static void buildFloor(std::vector<FloorVertex>& vertices,
                                 std::vector<FloorIndex>& indices,
                                        const float radius = 1.0f,
                                        const int segments = 32)
        {
            constexpr float floorHeight = -0.15f;

            vertices.clear();
            indices.clear();
            vertices.reserve(1 + static_cast<size_t>(segments));
            indices.reserve(static_cast<size_t>(segments) * 3);

            vertices.push_back({
                0.0f, floorHeight, 0.0f,
                0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f
            });

            for (int i = 0; i < segments; ++i)
            {
                const float theta = (static_cast<float>(i) / static_cast<float>(segments) * 2.0f * PI);
                const float pointX = std::cos(theta) * radius;
                const float pointZ = std::sin(theta) * radius;

                vertices.push_back({
                pointX, floorHeight, pointZ,
                0.0f, 1.0f, 0.0f,
                1.0f, 1.0f, 1.0f
                });
            }

            for (int i = 0; i < segments; ++i)
            {
                //Create each triangle
                const int nextSegment = (i + 1) % segments;
                indices.push_back(0);
                indices.push_back(static_cast<FloorIndex>(1 + i));
                indices.push_back(static_cast<FloorIndex>(1 + nextSegment));
            }
        }
    };
}



