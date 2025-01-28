#version 330 core
layout(location = 0) in vec2 aPos;
layout (location = 1) in vec3 aTexCoord;

out vec3 texCoords;

void main() {
    texCoords = aTexCoord;
    gl_Position = vec4(aPos, 0.0, 1.0);
}