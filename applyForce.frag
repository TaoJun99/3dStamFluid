#version 330 core

in vec3 texCoords;  // 3D texture coordinates
out vec4 fragColor;  // Output color

uniform sampler3D velocityTexture;
uniform vec3 forceApplyPos; // Normalized
uniform vec3 forceDir;      // Direction of the force
uniform float forceRadius;  // Radius of the force application
uniform float forceStrength;
uniform float slice; //normalized

void main() {
//    vec3 texCoord_3d = vec3(texCoords, slice);

//     Calculate distance from position where force is applied
    float distance = length(texCoords - vec3(0.5, 0.3, 0.5));

//     Apply force within the radius
    if (distance < forceRadius) {
        float influence = exp(-distance * distance / (2.0 * forceRadius * forceRadius));
        vec3 currentVelocity = texture(velocityTexture, texCoords).xyz;
//        vec3 newVelocity = currentVelocity + influence * normalize(texCoords - forceApplyPos) * forceStrength;
        vec3 newVelocity = currentVelocity + influence * vec3(0.0, 1.0, 0.0) * forceStrength;

        fragColor = vec4(newVelocity, 1.0);
    } else {
        fragColor = texture(velocityTexture, texCoords);
    }


}
