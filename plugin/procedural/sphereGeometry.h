#pragma once
#include <vector>

#include "geometryMath.h"

namespace AnimatedLpg
{

    struct SphereVertex
    {
        float x, y, z;
        float nX, nY, nZ;
        float r, g, b;
    };

    using SphereIndex = uint16_t;

    constexpr float PI = 3.14159265358979323846f;
    constexpr float EPSILON = 0.00001f;
    class SphereGeometry
    {
    public:
        static void buildFloor(std::vector<SphereVertex>& vertices,
                                 std::vector<SphereIndex>& indices,
                                    const float radius = 1.0f,
                                    int widthSegments = 32,
                                    int heightSegments = 26,
                                    const float phiStart = 0.0f,
                                    const float phiLength = PI * 2,
                                    const float thetaStart = 0.0f,
                                    const float thetaLength = PI)
        {
            widthSegments = std::max( 3, static_cast<int>(std::floor( widthSegments ) ) );
            heightSegments = std::max( 2, static_cast<int>(std::floor( heightSegments ) ) );
            const float thetaEnd = std::min( thetaStart + thetaLength, PI );

            int index = 0;
            const std::vector<std::vector<uint32_t>> grid = {};

            SphereVertex vertex = {};
            vec3 normal = {};

            std::vector<float> normals = {};
            std::vector<float> uvs = {};

            //generate vertices, normals, and uvs
            for (int iy = 0; iy <= static_cast<int>(heightSegments); iy++)
            {
                std::vector<int> verticesRow = {};

                int v = iy / static_cast<int>(heightSegments);
                float offset = 0.0f;

                if ( iy == 0 && thetaStart == 0.0f)
                {
                    offset = 0.5f / static_cast<float>(widthSegments);
                }

                else if (iy == heightSegments && std::abs(thetaEnd - PI) < EPSILON)
                {
                    offset = -0.5f / static_cast<float>(widthSegments);
                }
            }

            for (int ix = 0; ix <= widthSegments; ix++)
            {
                const int u = ix / widthSegments;

                //vertex
                vertex.x = -radius * std::cos( phiStart + u * phiLength) * std::sin (thetaStart + v * thetaLength);
                vertex.y = radius * std::cos( thetaStart + v * thetaLength );
                vertex.z = radius * std::sin( phiStart + u * phiLength ) * std::sin( thetaStart + v * thetaLength );

                vertices.push_back({
               vertex.x, vertex.y, vertex.z,
               0.0f, 1.0f, 0.0f,
               1.0f, 1.0f, 1.0f
               });
            }

            for (int iy = 0; iy < heightSegments; iy++)
            {
                for (int ix = 0; ix < widthSegments; ix++)
                {
                    const auto a = grid[ iy ][ ix + 1 ];
                    const auto b = grid[ iy ][ ix ];
                    const auto c = grid[ iy + 1 ][ ix ];
                    const auto d = grid[ iy + 1 ][ ix + 1 ];

                    if ( iy != 0 || thetaStart > 0)  indices.push_back( a, b, d );
                    if (iy != heightSegments - 1 || thetaEnd < PI) indices.push_back( b, c, d );
                }
            }








        }
    };
}





