#version 330 core

uniform sampler3D dyeTexture;
uniform vec3 addDyePos;
uniform float dyeRadius;
uniform vec3 dyeColor;
uniform bool addDye;
uniform float slice;

in vec3 texCoords;  // 3D texture coordinates

out vec4 fragColor;

void main() {
    // Current color of fragment
    vec4 currentColor = texture(dyeTexture, texCoords);

    // Distance of fragment from position where dye is dropped
    float dist = distance(texCoords, addDyePos);

    // Fragment within dyeRadius
    if (addDye && dist < dyeRadius) {
        float colorIntensity = 1.0 - dist / dyeRadius;
        vec3 addedDye = dyeColor * colorIntensity;
        currentColor.rgb += addedDye;

    }

    fragColor = currentColor;
}