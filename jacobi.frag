#version 330 core

uniform float alpha;
uniform float rBeta;
uniform sampler3D x;
uniform sampler3D b;
uniform int gridSize;
uniform float slice;

in vec3 texCoords;

out vec4 fragColor;

bool isSolidCell(ivec3 cellIndex) {
    if (cellIndex.x <= 0 || cellIndex.x >= gridSize - 1 ||
    cellIndex.y <= 0 || cellIndex.y >= gridSize - 1 ||
    cellIndex.z <= 0 || cellIndex.z >= gridSize - 1) {
        return true;
    } else {
        return false;
    }
}


void main() {
    // 1 Jacobi update iteration
//    ivec3 texCoordInt = ivec3(texCoords * gridSize);  // Tex coordinate to grid cell index
    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice * gridSize);

    vec4 xC = texelFetch(x, texCoordInt, 0); // Center
    // Fetch neighboring texels
    vec4 xL = texelFetch(x, texCoordInt - ivec3(1, 0, 0), 0);  // Left
    vec4 xR = texelFetch(x, texCoordInt + ivec3(1, 0, 0), 0);  // Right
    vec4 xD = texelFetch(x, texCoordInt - ivec3(0, 1, 0), 0);  // Down
    vec4 xU = texelFetch(x, texCoordInt + ivec3(0, 1, 0), 0);  // Up
    vec4 xF = texelFetch(x, texCoordInt + ivec3(0, 0, 1), 0);  // Front
    vec4 xB = texelFetch(x, texCoordInt - ivec3(0, 0, 1), 0);  // Back

    vec4 bC = texelFetch(b, texCoordInt, 0);

    // Check if neighbouring cells are boundary/solid cells
    if (isSolidCell(texCoordInt - ivec3(1, 0, 0))) {
        xL = xC;
    }
    if (isSolidCell(texCoordInt + ivec3(1, 0, 0))) {
        xR = xC;
    }
    if (isSolidCell(texCoordInt - ivec3(0, 1, 0))) {
        xD = xC;
    }
    if (isSolidCell(texCoordInt + ivec3(0, 1, 0))) {
        xU = xC;
    }
    if (isSolidCell(texCoordInt + ivec3(0, 0, 1))) {
        xF = xC;
    }
    if (isSolidCell(texCoordInt - ivec3(0, 1, 0))) {
        xB = xC;
    }

    fragColor = (xL + xR + xD + xU + xF + xB + alpha * bC) * rBeta;
}