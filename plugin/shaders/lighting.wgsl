fn nearestRoomLightPos(worldPos: vec3f) -> vec3f {
    let hy: f32 = 0.35;
    let c0 = vec2f(  0.000,  0.0000 );
    let c1 = vec2f(  1.900,  0.0000 );
    let c2 = vec2f(  0.950,  1.6454 );
    let c3 = vec2f( -0.950,  1.6454 );
    let c4 = vec2f( -1.900,  0.0000 );
    let c5 = vec2f( -0.950, -1.6454 );
    let c6 = vec2f(  0.950, -1.6454 );
    var best = c0;
    var bd   = distance(worldPos.xz, c0);
    let d1 = distance(worldPos.xz, c1); if (d1 < bd) { bd = d1; best = c1; }
    let d2 = distance(worldPos.xz, c2); if (d2 < bd) { bd = d2; best = c2; }
    let d3 = distance(worldPos.xz, c3); if (d3 < bd) { bd = d3; best = c3; }
    let d4 = distance(worldPos.xz, c4); if (d4 < bd) { bd = d4; best = c4; }
    let d5 = distance(worldPos.xz, c5); if (d5 < bd) { bd = d5; best = c5; }
    let d6 = distance(worldPos.xz, c6); if (d6 < bd) {           best = c6; }
    return vec3f(best.x, hy, best.y);
}

fn roomPointLight(worldPos: vec3f, normal: vec3f) -> vec3f {
    let wave       = sin(u.time);
    let lightPos   = nearestRoomLightPos(worldPos);
    let toLight    = lightPos - worldPos;
    let dist       = length(toLight);
    let lightDir   = toLight / dist;
    let atten      = 1.0 / (1.0 + 12.0 * dist * dist);
    let diffuse    = max(dot(normal, lightDir), 0.0) * atten;
    let lampColor  = vec3f(1.0, 0.92, 0.80);
    let ambient    = vec3f(0.035, 0.18, 0.3);
    return ambient + diffuse * lampColor;
}

fn computeNormal(worldPos: vec3f) -> vec3f {
    let dp_dx  = dpdx(worldPos);
    let dp_dy  = dpdy(worldPos);
    var normal = normalize(cross(dp_dx, dp_dy));
    if dot(normal, u.lightPos - worldPos) < 0.0 { normal = -normal; }
    return normal;
}

fn pointLight(worldPos: vec3f, normal: vec3f) -> vec3f {
    let wave = sin(u.time);

    let toLight     = u.lightPos  - worldPos;
    let dist        = length(toLight);
    let lightDir    = toLight / dist;
    let attenuation = 1.0 / (1.0 + 12.0 * dist * dist);
    let diffuse     = max(dot(normal, lightDir), 0.0) * attenuation;
    let lampColor   = vec3f(1.0, 0.92, 0.80);
    let ambient     = vec3f(0.5, 0.8, 0.3);
    return ambient + diffuse * lampColor;
}

