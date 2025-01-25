#version 330 core
out vec4 FragColor;
in vec3 texCoords;
in vec3 gridIndex;

uniform sampler3D inputTexture;
uniform int gridSize;



void main() {
    int x = int(gridIndex.x);
    int y = int(gridIndex.y) + gridSize * int(gridIndex.z);

//    vec4 color = texelFetch(inputTexture, ivec2(x, y), 0);
//    FragColor = color;
    FragColor = texture(inputTexture, texCoords);
//    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}