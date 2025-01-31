#version 330 core
layout(location = 0) in vec2 aPos; // Between -1 and 1

uniform float slice;

out vec3 texCoords;

void main() {
    vec2 texCoords_2d = (aPos + 1.0) / 2.0;
//    texCoords = texCoords_2d;
//    texCoords = vec3(texCoords_2d.x,  slice, texCoords_2d.y);
    texCoords = vec3(texCoords_2d, slice);
    gl_Position = vec4(aPos, 0.0, 1.0);
//    texCoords = gl_Position;
}