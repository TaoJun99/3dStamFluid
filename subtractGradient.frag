#version 330 core

uniform sampler3D p; // pressure field
uniform sampler3D w; // velocity
uniform float halfrdx;
uniform int gridSize;


in vec3 texCoords;
out vec4 fragColor;

void main() {
    ivec3 texCoordInt = ivec3(texCoords * gridSize);

    float pL = texelFetch(p, texCoordInt - ivec3(1, 0, 0), 0).x;  // Left
    float pR = texelFetch(p, texCoordInt + ivec3(1, 0, 0), 0).x;  // Right
    float pD = texelFetch(p, texCoordInt - ivec3(0, 1, 0), 0).x;  // Down
    float pU = texelFetch(p, texCoordInt + ivec3(0, 1, 0), 0).x;  // Up
    float pB = texelFetch(p, texCoordInt - ivec3(0, 0, 1), 0).x;  // Back
    float pF = texelFetch(p, texCoordInt + ivec3(0, 0, 1), 0).x;  // Front

    fragColor = texelFetch(w, texCoordInt, 0);
    fragColor.xyz = fragColor.xyz - halfrdx * vec3(pR - pL, pU - pD, pF - pB);
}