#version 330 core

uniform float alpha;
uniform float rBeta;
uniform sampler3D x;
uniform sampler3D b;
uniform int gridSize;

in vec3 texCoords;

out vec4 fragColor;

void main() {
    // 1 Jacobi update iteration
    ivec3 texCoordInt = ivec3(texCoords * gridSize);  // Tex coordinate to grid cell index

    // Fetch neighboring texels
    vec4 xL = texelFetch(x, texCoordInt - ivec3(1, 0, 0), 0);  // Left
    vec4 xR = texelFetch(x, texCoordInt + ivec3(1, 0, 0), 0);  // Right
    vec4 xD = texelFetch(x, texCoordInt - ivec3(0, 1, 0), 0);  // Down
    vec4 xU = texelFetch(x, texCoordInt + ivec3(0, 1, 0), 0);  // Up
    vec4 xF = texelFetch(x, texCoordInt + ivec3(0, 0, 1), 0);  // Front
    vec4 xB = texelFetch(x, texCoordInt - ivec3(0, 0, 1), 0);  // Back

    vec4 bC = texelFetch(b, texCoordInt, 0);

    fragColor = (xL + xR + xD + xU + xF + xB + alpha * bC) * rBeta;
}