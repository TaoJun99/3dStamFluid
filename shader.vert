#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aTexCoord;  // 3D texture coordinate

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float fluidSize;
uniform int gridSize;
uniform vec3 cameraPos;

out vec3 texCoords;
//out vec3 gridIndex;
//out vec3 rayDir;
out vec3 fragPos;

void main() {
    // 3D grid cell index
//    gridIndex = (aPos + fluidSize / 2) / fluidSize * (gridSize - 1);

//    texCoords = aTexCoord;
//    gl_Position = projection * view * model * vec4(aPos, 1.0);

    texCoords = aTexCoord;

    // Transform vertex position into world space
    vec4 worldPos = model * vec4(aPos, 1.0);

    // Transform into clip space
    gl_Position = projection * view * worldPos;

    // Compute ray direction from camera to vertex
//    rayDir = normalize(worldPos.xyz - cameraPos);
    fragPos = worldPos.xyz;


}