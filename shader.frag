#version 330 core
out vec4 fragColor;
in vec3 texCoords;
//in vec3 gridIndex;
//in vec3 rayDir;
in vec3 fragPos;

uniform sampler3D inputTexture;
uniform vec3 cameraPos;
//uniform int gridSize;

void main() {

//    fragColor = texture(inputTexture, texCoords);
//    vec3 rayPos = cameraPos;
//    vec3 stepSize = rayDir * 0.01; // Adjust step size for quality/performance
//    vec4 accumulatedColor = vec4(0.0);
//
//    for (int i = 0; i < 100; i++) { // Max 100 steps
//        vec3 texCoords = (rayPos + 1.0) * 0.5; // Convert to [0,1] range
//        vec4 sampleColor = texture(inputTexture, texCoords);
//        accumulatedColor += sampleColor * 0.1; // Adjust opacity accumulation
//        rayPos += stepSize;
//    }
//
//    fragColor = accumulatedColor;


    // Compute ray direction (from camera to fragment position)
    vec3 rayDir = normalize(fragPos - cameraPos);
    vec3 rayPos = fragPos; // Start marching from the front face
    vec3 stepSize = rayDir * 0.01;

    vec4 accumulatedColor = vec4(0.0); // Store final color

    // March through the volume
    while(true) {
        // Convert rayPos to texture coordinates (assuming cube is from [-1,1])
        vec3 texCoords = (rayPos + 0.5);

        // Sample the fluid simulation texture
        vec4 sampleColor = texture(inputTexture, texCoords);

        // Accumulate color if sample has fluid
        accumulatedColor += sampleColor * 0.1;

        // Move the ray forward
        rayPos += stepSize;

        // Stop if ray exits the cube (outside [-1,1] range)
        if (any(lessThan(rayPos, vec3(-0.5))) || any(greaterThan(rayPos, vec3(0.5)))) {
            break;
        }
    }

    fragColor = accumulatedColor; // Output final color

}