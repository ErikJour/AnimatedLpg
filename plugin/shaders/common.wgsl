const MAT_FLOOR:                   u32 = 0u;
struct Uniforms {
    time:           f32,
    modelMatrix:    mat4x4f,
    lightPos:       vec3f,
    aspectRatio:    f32,
    viewProjMatrix: mat4x4f,
    projMatrix:     mat4x4f,
    materialId:     u32
};

@group(0) @binding(0) var<uniform> u: Uniforms;

struct VertexInput {
    @location(0) position: vec3f,
    @location(1) color:    vec3f,
    @location(2) normal:   vec3f,
};

struct VertexOutput {
    @builtin(position) clipPos:  vec4f,
    @location(0)       color:    vec3f,
    @location(1)       worldPos: vec3f,
    @location(2)       normal:   vec3f,
};

