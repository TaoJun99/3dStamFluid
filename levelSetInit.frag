#version 330 core

uniform float waterHeight; // [0, 1]

in vec3 texCoords;  // 3D texture coordinates

out vec4 fragColor;


void main() {

    // Store phi value in first value
    fragColor = vec4(texCoords.y - waterHeight, 0.0, 0.0, 0.0);
}