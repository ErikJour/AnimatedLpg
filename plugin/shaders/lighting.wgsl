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

fn sliderGlow(worldPos: vec3f) -> vec3f {
    let cream = vec3f(0.92, 0.86, 0.72);
    var sum   = 0.0;
    return cream * sum * 0.5;
}

fn skylightBeam(worldPos: vec3f, normal: vec3f) -> vec3f {
    var pal = array<vec3f, 7>(
        vec3f(0.76, 0.33, 0.17),  // terra cotta
        vec3f(0.55, 0.62, 0.38),  // sage green
        vec3f(0.85, 0.68, 0.18),  // warm gold
        vec3f(0.92, 0.87, 0.72),  // cream
        vec3f(0.24, 0.48, 0.47),  // oxidized teal
        vec3f(0.35, 0.22, 0.14),  // walnut
        vec3f(0.72, 0.42, 0.12),  // burnt amber
    );
    let t         = u.time * 0.08;           // full palette cycle every ~88 s
    let i         = i32(floor(t)) % 7;
    let beamColor = mix(pal[i], pal[(i + 1) % 7], smoothstep(0.0, 1.0, fract(t)));

    // Beam is centred on the nearest room's oculus (directly above room centre)
    let roomXZ    = nearestRoomLightPos(worldPos).xz;
    let xzDist    = length(worldPos.xz - roomXZ);
    let falloff   = 1.0 - smoothstep(0.0, 0.50, xzDist);  // ~0.5-unit radius pool

    // Floor receives more color than vertical walls (upward-facing bias)
    let uplift    = 0.35 + 0.65 * max(0.0, normal.y);

    return beamColor * falloff * uplift * 0.20;
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
    let ambient    = vec3f(0.035, 0.018, 0.03);
    return ambient + diffuse * lampColor + sliderGlow(worldPos) + skylightBeam(worldPos, normal);
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

//    var offsetA     = vec3f(wave, wave, 0.1);
    let toLight     = u.lightPos  - worldPos;
    let dist        = length(toLight);
    let lightDir    = toLight / dist;
    let attenuation = 1.0 / (1.0 + 12.0 * dist * dist);
    let diffuse     = max(dot(normal, lightDir), 0.0) * attenuation;
    let lampColor   = vec3f(1.0, 0.92, 0.80);
    let ambient     = vec3f(0.15, 0.018, 0.03);
    return ambient + diffuse * lampColor + sliderGlow(worldPos) + skylightBeam(worldPos, normal);
}

