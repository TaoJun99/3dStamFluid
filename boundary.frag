#version 330 core

uniform float scale;
uniform sampler3D texture;
uniform int gridSize;
uniform float slice;

in vec3 texCoords;
in vec3 gridCellIndex;
out vec4 fragColor;

void main() {
//    ivec3 texCoordInt = ivec3(texCoords.xy * gridSize, slice * gridSize);
    ivec3 texCoordInt = ivec3(texCoords * gridSize);

    ivec3 offset = ivec3(0, 0, 0);

    if (texCoordInt.x == 0) {
        if (texCoordInt.y != 0 && texCoordInt.y != gridSize - 1 && texCoordInt.z != 0 && texCoordInt.z != gridSize - 1) {
            offset = ivec3(1, 0, 0);
        }
    } else if (texCoordInt.x == gridSize - 1) {
        if (texCoordInt.y != 0 && texCoordInt.y != gridSize - 1 && texCoordInt.z != 0 && texCoordInt.z != gridSize - 1) {
            offset = ivec3(-1, 0, 0);
        }
    } else if (texCoordInt.y == 0) {
        if (texCoordInt.x != 0 && texCoordInt.x != gridSize - 1 && texCoordInt.z != 0 && texCoordInt.z != gridSize - 1) {
            offset = ivec3(0, 1, 0);
        }
    } else if (texCoordInt.y == gridSize - 1) {
        if (texCoordInt.x != 0 && texCoordInt.x != gridSize - 1 && texCoordInt.z != 0 && texCoordInt.z != gridSize - 1) {
            offset = ivec3(0, -1, 0);
        }
    } else if (texCoordInt.z == 0) {
        if (texCoordInt.x != 0 && texCoordInt.x != gridSize - 1 && texCoordInt.y != 0 && texCoordInt.y != gridSize - 1) {
            offset = ivec3(0, 0, 1);
        }
    } else if (texCoordInt.z == gridSize - 1) {
        if (texCoordInt.x != 0 && texCoordInt.x != gridSize - 1 && texCoordInt.y != 0 && texCoordInt.y != gridSize - 1) {
            offset = ivec3(0, 0, -1);
        }
    } else {
        offset = ivec3(-1, -1, -1);
    }

    if (offset == ivec3(-1, -1, -1)) {
        fragColor = texelFetch(texture, texCoordInt, 0);
    } else if (offset == ivec3(0, 0, 0)) {
        fragColor = vec4(0.0, 0.5, 0.5, 1.0);
    } else {
        fragColor = scale * texelFetch(texture, texCoordInt + offset, 0);
    }


}