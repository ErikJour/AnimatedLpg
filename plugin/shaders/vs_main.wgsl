fn projectPerspective(worldPos: vec3f) -> vec4f {
    return u.viewProjMatrix * vec4f(worldPos, 1.0);
}
fn projectFlat(pos: vec3f) -> vec4f {
    return vec4f(pos.x, pos.y, pos.z * 0.5 + 0.5, 1.0);
}
@vertex
fn vs_main(in: VertexInput) -> VertexOutput {
    var out: VertexOutput;
    var position = in.position;

        switch u.materialId {
                case MAT_FLOOR:                   { out.clipPos  = vsFloor(&position);                       }
                default:                          { out.clipPos  = projectPerspective(position);             }
        }
        out.color    = in.color;
        out.worldPos = position;
        out.normal   = in.normal;
        return out;
}

