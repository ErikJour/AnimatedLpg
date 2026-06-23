//
// Created by Erik Jourgensen on 5/14/26.
//

#pragma once

struct vec2 { float x, y; };

struct CameraState {
    float angleX = 0.0f;
    float posX   = 0.0f;
    float posZ   = 1.5f;

    static constexpr float eyeY        = 0.02f;
};

