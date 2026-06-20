#pragma once
#include <cstddef>
#include <cstdint>

namespace AnimatedLPG
{
    // Must match common.wgsl
    static constexpr uint32_t MAT_FLOOR = 0;

    struct MyUniforms {
        float    time;
    };

    static_assert(sizeof(MyUniforms) % 4 == 0);

    static constexpr float kIdentity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
}

