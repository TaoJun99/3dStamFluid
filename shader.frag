#version 330 core
out vec4 fragColor;

in vec3 fragPos;

uniform sampler3D inputTexture;
uniform vec3 cameraPos;

void main() {
    vec3 rayDir = normalize(fragPos - cameraPos);
    vec3 rayPos = fragPos;
    vec3 stepSize = rayDir * 0.01;

    vec4 accumulatedColor = vec4(0.0);

    for (int i = 0; i < 256; i++) {
        vec3 texCoords = (rayPos + 0.5);
        float phi = texture(inputTexture, texCoords).x;

        if (phi <= 0.0) {  // Inside water
            vec4 waterColor = vec4(0.2, 0.4, 1.0, 1.0);// Blue water color
            accumulatedColor += waterColor * 0.02;

        }

        rayPos += stepSize;

        // Early termination if the ray exits the volume
        if (any(lessThan(rayPos, vec3(-0.5))) || any(greaterThan(rayPos, vec3(0.5)))) {
            break;
        }
    }

    fragColor = accumulatedColor;
}