#version 330 core

uniform sampler3D dyeTexture;
uniform sampler3D velocityTexture;
uniform sampler3D advectedTexture;
uniform float timestep;
uniform float rdx;
uniform bool isAdvectDye;

in vec3 texCoords;

out vec4 fragColor;

void main() {
    vec3 newX = texCoords - timestep * rdx * texture(velocityTexture, texCoords).xyz;
    newX = clamp(newX, vec3(0.0), vec3(1.0));

    vec4 advectedValue = texture(advectedTexture, newX);
//    if (isAdvectDye) {
//        advectedValue = texture(dyeTexture, newX);
//    } else {
//        advectedValue = texture(velocityTexture, newX);
//    }



    fragColor = advectedValue;


}