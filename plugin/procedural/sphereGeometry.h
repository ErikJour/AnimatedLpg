
#pragma once

//radius = 1
//widthSegments = 32
//heightSegments = 26
//phiStart = 0;
//phiLength = PI * 2
//thetaStart = 0;
//thetaLength = PI

//widthSegments = std::max( 3, std::floor( widthSegments ) );
//heightSegments = std::max( 2, std::floor( heightSegments ) );

//float thetaEnd = std::min( thetaStart + thetaLength, PI );

//int index = 0;

//vector<float> grid = [];

//vec3f vertex = [];
//
// Created by Erik Jourgensen on 6/22/26.
//A sphere geometry based on the Three.js sphere
//

//vec3f normal = [];

//Buffers

//indices [];
//vertices [];
//normals [];
//uvs [];

//for (let iy = 0; iy <= heightSegments; iy++) {
//verticesRow = [];
//v = iy / heightSegments;
//offset = 0;
//if (iy == 0 && thetaStart == 0)
//uOffset = 0.5 / widthSegments;
//else if (iy == heightSegments && thetaEnd == PI) {
//uoffset = -0.5 / widthSegments;
//for (let ix = 0; ix <= widthSegments; ix++) {
//u = ix / widthSegments;

//vertex
//vertex.x = -radius * cos( phiStart + u * phiLength ) * sin (thetaStart + v * thetaLength );
//vertex.y = radius * cos(thetaStart + v * thetaLength);
//vertex.z = radius * sin (phiStart + u * phiLength) * sin (thetaStart  + v * thetaLength);
//vertices.push9vertex.x, vertex.y, vertex.z);

//normal
//normal.copy( vertex ).normalize();
//normals.push( normal.x, normal.y, normal.z );

//uv

//uvs.push( u + offset, 1 - v);
//verticesRow.push ( index++ );
//grid.push(verticesRow);

//indices

//for ( let iy = 0; iy < heightSegments; iy++ ) {
//for (let ix = 0; ix < widthSegments; ix++) {
//a = grid[ iy ] [ix + 1];
//b = grid[ iy ][ ix ];
//c = grid[ iy + 1][ ix ];
//d = grid [ iy + 1][ ix + 1];
//if (iy !== 0 ||thetaStart > 0) indices.push (a, b, d);
//if (iy !== heightSegments - 1 || thetaEnd < PI) indices.push (b, c, d);

//build the geometry





