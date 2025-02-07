#version 330 core

in vec3 texCoords;  // 3D texture coordinates
out vec4 fragColor;  // Output color

uniform sampler3D levelSetTexture;
uniform sampler3D velocityTexture;
uniform vec3 forceApplyPos; // Normalized
uniform vec3 forceDir;      // Direction of the force
uniform float forceRadius;  // Radius of the force application
uniform float forceStrength;
uniform float slice; //normalized

void main() {
//    vec3 texCoord_3d = vec3(texCoords, slice);


    if (texture(levelSetTexture, texCoords).x <= 0) { // Water cell
        //     Calculate distance from position where force is applied
        float distance = length(texCoords - forceApplyPos);

        //     Apply force within the radius
        if (distance < forceRadius) {
            float influence = exp(-distance * distance / (2.0 * forceRadius * forceRadius));
            vec3 currentVelocity = texture(velocityTexture, texCoords).xyz;
            vec3 newVelocity = currentVelocity + influence * forceDir * forceStrength;

            fragColor = vec4(newVelocity, 1.0);
        } else {
            fragColor = texture(velocityTexture, texCoords);
        }
    }  else { // Air cell - velocity set to 0
        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
    }



}
