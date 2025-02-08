#version 330 core

uniform float alpha;
uniform float rBeta;
uniform sampler3D x;
uniform sampler3D b;
uniform int gridSize;
uniform int slice;
uniform sampler3D levelSetTexture;
uniform bool isPressure;

in vec3 texCoords;

out vec4 fragColor;

bool isSolidOrAirCell(ivec3 cellIndex) {
    return (cellIndex.x <= 0 || cellIndex.x >= gridSize - 1 ||
    cellIndex.y <= 0 || cellIndex.y >= gridSize - 1 ||
    cellIndex.z <= 0 || cellIndex.z >= gridSize - 1 ||
    texelFetch(levelSetTexture, cellIndex, 0).x > 0);
}


bool isAirCell(ivec3 cellIndex) {
    return texelFetch(levelSetTexture, cellIndex, 0).x > 0;
}


void main() {
    // Current cell
    float phi = texture(levelSetTexture, texCoords).x;

    if (phi > 0.0) { // Air cell
        if (isPressure) { // Pressure: set to zero
            fragColor = vec4(0.0, 0.0, 0.0, 0.0);
            return;
        } else { // Velocity: remain unchanged
            fragColor = texture(x, texCoords);
            return;
        }

    }

    // 1 Jacobi update iteration
//    ivec3 texCoordInt = ivec3(texCoords * gridSize);  // Tex coordinate to grid cell index
    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice);

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
    if (isSolidOrAirCell(texCoordInt - ivec3(1, 0, 0))) {
        xL = xC;
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(1, 0, 0))) {
        xR = xC;
    }
    if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
        xD = xC;
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(0, 1, 0))) {
        xU = xC;
    }
    if (isSolidOrAirCell(texCoordInt + ivec3(0, 0, 1))) {
        xF = xC;
    }
    if (isSolidOrAirCell(texCoordInt - ivec3(0, 1, 0))) {
        xB = xC;
    }


    if (isPressure) { // Air cell should have zero pressure
        if (isAirCell(texCoordInt - ivec3(1, 0, 0))) {
            xL = vec4(0.0, 0.0, 0.0, 0.0);
        }
        if (isAirCell(texCoordInt + ivec3(1, 0, 0))) {
            xR = vec4(0.0, 0.0, 0.0, 0.0);
        }
        if (isAirCell(texCoordInt - ivec3(0, 1, 0))) {
            xD = vec4(0.0, 0.0, 0.0, 0.0);
        }
        if (isAirCell(texCoordInt + ivec3(0, 1, 0))) {
            xU = vec4(0.0, 0.0, 0.0, 0.0);
        }
        if (isAirCell(texCoordInt + ivec3(0, 0, 1))) {
            xF = vec4(0.0, 0.0, 0.0, 0.0);
        }
        if (isAirCell(texCoordInt - ivec3(0, 1, 0))) {
            xB = vec4(0.0, 0.0, 0.0, 0.0);
        }
    }


    fragColor = (xL + xR + xD + xU + xF + xB + alpha * bC) * rBeta;
}