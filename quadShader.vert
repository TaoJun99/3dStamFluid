#version 330 core
layout(location = 0) in vec2 aPos; // Between -1 and 1

uniform int slice; // [0, gridSize - 1]
uniform int gridSize;

out vec3 texCoords;
//out vec3 gridCellIndex;

void main() {
    vec2 texCoords_2d = (aPos + 1.0) / 2.0;
    texCoords = vec3(texCoords_2d, (slice + 0.5) / gridSize);
//    gridCellIndex = ivec3(texCoords_2d * 64, slice);
//    texCoords = (aPos + 1.0) / 2.0;
    gl_Position = vec4(aPos, 0.0, 1.0);
}