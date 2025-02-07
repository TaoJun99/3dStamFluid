#version 330 core

uniform sampler3D p; // pressure field
uniform sampler3D w; // velocity
uniform float halfrdx;
uniform int gridSize;
uniform float slice;
uniform sampler3D levelSetTexture;

in vec3 texCoords;
out vec4 fragColor;

bool isSolidOrAirCell(ivec3 cellIndex) {
    return (cellIndex.x <= 0 || cellIndex.x >= gridSize - 1 ||
    cellIndex.y <= 0 || cellIndex.y >= gridSize - 1 ||
    cellIndex.z <= 0 || cellIndex.z >= gridSize - 1);
}


void main() {
//    float phi = texture(levelSetTexture, texCoords).x;
//
//    if (phi > 0.0) {
//        fragColor = texture(w, texCoords);
//        return;
//    }

//    ivec3 texCoordInt = ivec3(texCoords * gridSize);
    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice * gridSize);

    // Current cell is boundary - set velocity = 0
    if (isSolidOrAirCell(texCoordInt)) {
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        float pC = texelFetch(p, texCoordInt, 0).x; // Center

        float pL = texelFetch(p, texCoordInt - ivec3(1, 0, 0), 0).x;  // Left
        float pR = texelFetch(p, texCoordInt + ivec3(1, 0, 0), 0).x;  // Right
        float pD = texelFetch(p, texCoordInt - ivec3(0, 1, 0), 0).x;  // Down
        float pU = texelFetch(p, texCoordInt + ivec3(0, 1, 0), 0).x;  // Up
        float pB = texelFetch(p, texCoordInt - ivec3(0, 0, 1), 0).x;  // Back
        float pF = texelFetch(p, texCoordInt + ivec3(0, 0, 1), 0).x;  // Front

        // Check if neighbouring cells are boundary/solid cells
        if (isSolidOrAirCell(texCoordInt - ivec3(1, 0, 0))) {
            pL = pC;
        }
        if (isSolidOrAirCell(texCoordInt + ivec3(1, 0, 0))) {
            pR = pC;
        }
        if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
            pD = pC;
        }
        if (isSolidOrAirCell(texCoordInt + ivec3(0, 1, 0))) {
            pU = pC;
        }
        if (isSolidOrAirCell(texCoordInt + ivec3(0, 0, 1))) {
            pF = pC;
        }
        if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
            pB = pC;
        }

        fragColor = texelFetch(w, texCoordInt, 0);
        fragColor.xyz = fragColor.xyz - halfrdx * vec3(pR - pL, pU - pD, pF - pB);
    }


}