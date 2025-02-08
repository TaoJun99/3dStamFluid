#version 330 core

uniform sampler3D w; //vector field (velocity)
uniform float halfrdx;
uniform int gridSize;
uniform float slice;
uniform sampler3D levelSetTexture;

in vec3 texCoords;
out vec4 fragColor;


bool isSolidOrAirCell(ivec3 cellIndex) {
    return (cellIndex.x <= 0 || cellIndex.x >= gridSize - 1 ||
    cellIndex.y <= 0 || cellIndex.y >= gridSize - 1 ||
    cellIndex.z <= 0 || cellIndex.z >= gridSize - 1 ||
    texelFetch(levelSetTexture, cellIndex, 0).x > 0);
}


void main() {
    float phi = texture(levelSetTexture, texCoords).x;

    if (phi > 0.0) {
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
        return;
    }


    ivec3 texCoordInt = ivec3(texCoords * gridSize);  // Convert normalized to integer coordinates
//    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice * gridSize);

    // Fetch neighboring texels
    vec4 wL = texelFetch(w, texCoordInt - ivec3(1, 0, 0), 0);  // Left
    vec4 wR = texelFetch(w, texCoordInt + ivec3(1, 0, 0), 0);  // Right
    vec4 wD = texelFetch(w, texCoordInt - ivec3(0, 1, 0), 0);  // Down
    vec4 wU = texelFetch(w, texCoordInt + ivec3(0, 1, 0), 0);  // Up
    vec4 wF = texelFetch(w, texCoordInt + ivec3(0, 0, 1), 0);  // Front
    vec4 wB = texelFetch(w, texCoordInt - ivec3(0, 0, 1), 0);  // Back

    // Check if neighbouring cells are boundary/solid cells
    if (isSolidOrAirCell(texCoordInt - ivec3(1, 0, 0))) {
        wL = vec4(0.0, 0.0, 0.0, 0.0);
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(1, 0, 0))) {
        wR = vec4(0.0, 0.0, 0.0, 0.0);
    }
    if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
        wD = vec4(0.0, 0.0, 0.0, 0.0);
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(0, 1, 0))) {
        wU = vec4(0.0, 0.0, 0.0, 0.0);
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(0, 0, 1))) {
        wF = vec4(0.0, 0.0, 0.0, 0.0);
    }
    if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
        wB = vec4(0.0, 0.0, 0.0, 0.0);
    }


    // Divergence is scalar field
    fragColor = vec4(halfrdx * ((wR.x - wL.x) + (wU.y - wD.y) + (wF.z - wB.z)), 0.0, 0.0, 0.0);
}