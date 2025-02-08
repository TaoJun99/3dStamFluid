#version 330 core

in vec3 texCoords;  // 3D texture coordinates
out vec4 fragColor;  // Output color

uniform sampler3D levelSetTexture;
uniform sampler3D velocityTexture;


void main() {
    //    vec3 texCoord_3d = vec3(texCoords, slice);

    if (texture(levelSetTexture, texCoords).x <= 0) { // Water cell
        float strength = 3.0;
        fragColor = texture(velocityTexture, texCoords) + strength * vec4(0.0, -1.0, 0.0, 0.0);
    }  else { // Air cell - velocity set to 0
//        fragColor = vec4(0.0, 0.0, 0.0, 0.0);
        fragColor = texture(velocityTexture, texCoords);
    }


}