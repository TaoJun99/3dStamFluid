#version 330 core

uniform sampler3D w; //vector field
uniform float halfrdx;
uniform int gridSize;
uniform float slice;

in vec3 texCoords;
out vec4 fragColor;

void main() {

//    ivec3 texCoordInt = ivec3(texCoords * gridSize);  // Convert normalized to integer coordinates
    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice * gridSize);

    // Fetch neighboring texels
    vec4 wL = texelFetch(w, texCoordInt - ivec3(1, 0, 0), 0);  // Left
    vec4 wR = texelFetch(w, texCoordInt + ivec3(1, 0, 0), 0);  // Right
    vec4 wD = texelFetch(w, texCoordInt - ivec3(0, 1, 0), 0);  // Down
    vec4 wU = texelFetch(w, texCoordInt + ivec3(0, 1, 0), 0);  // Up
    vec4 wF = texelFetch(w, texCoordInt + ivec3(0, 0, 1), 0);  // Front
    vec4 wB = texelFetch(w, texCoordInt - ivec3(0, 0, 1), 0);  // Back

    // Divergence is scalar field
    fragColor = vec4(halfrdx * ((wR.x - wL.x) + (wU.y - wD.y) + (wF.z - wB.z)), 0.0, 0.0, 0.0);
}