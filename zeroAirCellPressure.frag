#version 330 core

in vec3 texCoords;
out vec4 fragColor;

uniform sampler3D pressureTexture;
uniform sampler3D levelSetTexture;
uniform int slice;
uniform gridSize;

void main() {
    float phi = texture(levelSetTexture, texCoords).x;

    if (phi > 0) { // Air Cell
        fragColor = vec4(0.0, 0.0, 0.0, 0.0); // Set pressure to zero
    } else {
        fragColor = texture(pressureTexture, texCoords);
    }
}
