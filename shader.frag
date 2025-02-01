#version 330 core
out vec4 FragColor;
in vec3 texCoords;
in vec3 gridIndex;

uniform sampler3D inputTexture;
uniform int gridSize;



void main() {
    int x = int(gridIndex.x);
    int y = int(gridIndex.y) + gridSize * int(gridIndex.z);


    FragColor = texture(inputTexture, texCoords);


}