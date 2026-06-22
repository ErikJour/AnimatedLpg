#pragma once
#include <cstddef>
#include <cstdint>

namespace AnimatedLpg
{
    // Must match common.wgsl
    static constexpr uint32_t MAT_FLOOR = 0;

    struct alignas(16) MyUniforms {
        float    time;            // 0
        float    _pad0[3];        // 4   -> push modelMatrix to 16
        float    modelMatrix[16]; // 16
        float    lightPos[3];     // 80
        float    aspectRatio;     // 92  (fills lightPos's 4th slot)
        float    viewProjMatrix[16]; // 96
        float    projMatrix[16];  // 160
        uint32_t materialId;      // 224
        uint32_t _pad1[3];        // 228 -> round struct to 240
    };
    static_assert(sizeof(MyUniforms) == 240);
    static_assert(offsetof(MyUniforms, modelMatrix)    == 16);
    static_assert(offsetof(MyUniforms, lightPos)       == 80);
    static_assert(offsetof(MyUniforms, viewProjMatrix) == 96);
    static_assert(offsetof(MyUniforms, projMatrix)     == 160);
    static_assert(offsetof(MyUniforms, materialId)     == 224);


    static constexpr float kIdentity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

