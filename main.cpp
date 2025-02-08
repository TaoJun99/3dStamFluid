#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const int GRID_SIZE = 64;

GLuint levelSetTexture;
GLuint velocityTexture;
GLuint pressureTexture;
GLuint jacobiTexture1;
GLuint jacobiTexture2;
GLuint outputTexture;
GLuint VAO, VBO, EBO;
GLuint quadVAO, quadVBO, quadEBO;
GLuint framebuffer;
GLuint shaderProgram;
GLuint addDyeShaderProgram;
GLuint applyForceShaderProgram;
GLuint advectShaderProgram;
GLuint jacobiShaderProgram;
GLuint divergenceShaderProgram;
GLuint gradientSubtractShaderProgram;
GLuint boundaryShaderProgram;
GLuint levelSetInitShaderProgram;
GLuint zeroAirCellPressureShaderProgram;

float cubeSize = 1.0f;

float viewportWidth;
float viewportHeight;

glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;

float timeStep = 0.01;

// Fullscreen Quad Vertices
float quadVertices[] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        1.0f,  1.0f,
        -1.0f,  1.0f
};

unsigned int quadIndices[] = {
        0, 1, 2,
        2, 3, 0
};

// Function to read shader source from file
std::string readShaderSource(const std::string& filePath) {
    std::ifstream file(filePath);
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to compile shader
GLuint compileShader(const std::string& source, GLenum shaderType) {
    GLuint shader = glCreateShader(shaderType);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    // Check for compilation errors
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }

    return shader;
}

// Function to create shader program
GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexSource = readShaderSource(vertexPath);
    std::string fragmentSource = readShaderSource(fragmentPath);

    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for linking errors
    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }

    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Function to generate cube vertices based on a size variable
void generateCubeVertices(std::vector<float>& vertices, float size) {
    float halfSize = size / 2.0f;

    vertices = {
            // Front face
            -halfSize, -halfSize,  halfSize,  0.0f, 0.0f, 1.0f, // 0
            halfSize, -halfSize,  halfSize,  1.0f, 0.0f, 1.0f, // 1
            halfSize,  halfSize,  halfSize,  1.0f, 1.0f, 1.0f, // 2
            -halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 1.0f, // 3

            // Back face
            -halfSize, -halfSize, -halfSize,  0.0f, 0.0f, 0.0f, // 4
            halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 0.0f, // 5
            halfSize,  halfSize, -halfSize,  1.0f, 1.0f, 0.0f, // 6
            -halfSize,  halfSize, -halfSize,  0.0f, 1.0f, 0.0f, // 7

            // Left face
            -halfSize, -halfSize, -halfSize,  0.0f, 0.0f, 0.0f, // 8
            -halfSize, -halfSize,  halfSize,  0.0f, 0.0f, 1.0f, // 9
            -halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 1.0f, // 10
            -halfSize,  halfSize, -halfSize,  0.0f, 1.0f, 0.0f, // 11

            // Right face
            halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 0.0f, // 12
            halfSize, -halfSize,  halfSize,  1.0f, 0.0f, 1.0f, // 13
            halfSize,  halfSize,  halfSize,  1.0f, 1.0f, 1.0f, // 14
            halfSize,  halfSize, -halfSize,  1.0f, 1.0f, 0.0f, // 15

            // Bottom face
            -halfSize, -halfSize, -halfSize,  0.0f, 0.0f, 0.0f, // 16
            halfSize, -halfSize, -halfSize,  1.0f, 0.0f, 0.0f, // 17
            halfSize, -halfSize,  halfSize,  1.0f, 0.0f, 1.0f, // 18
            -halfSize, -halfSize,  halfSize,  0.0f, 0.0f, 1.0f, // 19

            // Top face
            -halfSize,  halfSize, -halfSize,  0.0f, 1.0f, 0.0f, // 20
            halfSize,  halfSize, -halfSize,  1.0f, 1.0f, 0.0f, // 21
            halfSize,  halfSize,  halfSize,  1.0f, 1.0f, 1.0f, // 22
            -halfSize,  halfSize,  halfSize,  0.0f, 1.0f, 1.0f  // 23
    };
}

void levelSetInit() {
    glUseProgram(levelSetInitShaderProgram);

    GLuint waterHeightLoc = glGetUniformLocation(levelSetInitShaderProgram, "waterHeight");
    GLuint sliceLoc = glGetUniformLocation(levelSetInitShaderProgram, "slice");

    glUniform1f(waterHeightLoc, 0.6);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, levelSetTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }


    glViewport(0, 0, viewportWidth, viewportHeight);
    // Unbind the framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void getMouseNDC(GLFWwindow* window, glm::vec2& mouseNDC) {
    // Window coordinates
    double mouseX, mouseY;
    glfwGetCursorPos(window, &mouseX, &mouseY);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    mouseNDC.x = (2.0f * static_cast<float>(mouseX) / windowWidth) - 1.0f;
    mouseNDC.y = 1.0f - (2.0f * static_cast<float>(mouseY) / windowHeight);
}

glm::vec3 computeForcePosition(const glm::vec2& mouseNDC) {
    glm::mat4 invVP = glm::inverse(projection * view);

    // Create a ray in NDC space (near and far plane points)
    glm::vec4 nearPoint = invVP * glm::vec4(mouseNDC, -1.0f, 1.0f);
    glm::vec4 farPoint = invVP * glm::vec4(mouseNDC, 1.0f, 1.0f);

    // Divide by w to convert to world coordinates
    nearPoint /= nearPoint.w;
    farPoint /= farPoint.w;

    glm::vec3 rayOrigin = glm::vec3(nearPoint);
    glm::vec3 rayDirection = glm::normalize(glm::vec3(farPoint) - rayOrigin);

    // Check if ray is parallel to the top face (y = halfSize)
    if (glm::abs(rayDirection.y) < 1e-6f) {
        return glm::vec3(-1, -1, -1); // No intersection
    }

    float halfSize = cubeSize / 2;
    // Compute t for the intersection with the top plane (y = halfSize)
    float t = (halfSize - rayOrigin.y) / rayDirection.y;

    // If t < 0, intersection is behind the camera
    if (t < 0) {
        return glm::vec3(-1, -1, -1);
    }

    // Compute the intersection point
    glm::vec3 intersection = rayOrigin + t * rayDirection;

    // Check if the intersection point is within the bounds of the top face
    if (intersection.x < -halfSize || intersection.x > halfSize ||
        intersection.z < -halfSize || intersection.z > halfSize) {
        return glm::vec3(-1, -1, -1); // Outside the top face bounds
    }

    // If intersect: intersection point -halfSize <= x,z <= halfSize, y = halfSize
    // Map intersection point to normalized grid space (0 to 1 range)
    glm::vec3 normalizedPoint = (intersection + glm::vec3(halfSize)) / (2.0f * halfSize);
    std::cout <<  "Intersection : "
              << intersection.x << ", "
              << intersection.y << ", "
              << intersection.z << std::endl;
    return normalizedPoint;
}


void applyForce(GLFWwindow* window) {
//    glm::vec2 mouseNDC;
//    getMouseNDC(window, mouseNDC);

    // World space
//    glm::vec3 forcePos = computeForcePosition(mouseNDC);
    glm::vec3 forcePos = glm::vec3(0.5, 0.5, 0.5);


    // No intersection with top face
    if (forcePos == glm::vec3(-1, -1, -1)) {
        return;
    }

    glm::vec3 forceDir = glm::vec3(0.0, 1.0, 0.0);
//    glm::vec3 forceDir = -forcePos; // Point towards origin (center of cube)
    float forceRadius = 0.4f; // Normalized
    float forceStrength = 30.0f; // Example strength

//    std::cout << "Force Position: "
//                  << forcePos.x << ", "
//                  << forcePos.y << ", "
//                  << forcePos.z << std::endl;

    // Use the applyForceShaderProgram
    glUseProgram(applyForceShaderProgram);

    // Uniform variables
    GLuint forceApplyPosLoc = glGetUniformLocation(applyForceShaderProgram, "forceApplyPos");
    GLuint forceDirLoc = glGetUniformLocation(applyForceShaderProgram, "forceDir");
    GLuint forceRadiusLoc = glGetUniformLocation(applyForceShaderProgram, "forceRadius");
    GLuint forceStrengthLoc = glGetUniformLocation(applyForceShaderProgram, "forceStrength");
    GLuint velocityTextureLoc = glGetUniformLocation(applyForceShaderProgram, "velocityTexture");
    GLuint sliceLoc = glGetUniformLocation(applyForceShaderProgram, "slice");
    GLuint levelSetTextureLoc = glGetUniformLocation(applyForceShaderProgram, "levelSetTexture");

    glUniform3fv(forceApplyPosLoc, 1, glm::value_ptr(forcePos));
    glUniform3fv(forceDirLoc, 1, glm::value_ptr(forceDir));
    glUniform1f(forceRadiusLoc, forceRadius);
    glUniform1f(forceStrengthLoc, forceStrength);
    glUniform1i(velocityTextureLoc, 1);
    glUniform1i(levelSetTextureLoc, 0);

    // Bind the 3D texture as the framebuffer target
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, velocityTexture);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    for (int slice = 0; slice < GRID_SIZE; ++slice) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // Bind each slice of the 3D texture
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, velocityTexture, 0, slice);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, velocityTexture, 0, slice);
        // Check framebuffer status
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

//        unsigned char* pixels = new unsigned char[GRID_SIZE * GRID_SIZE * 3];  // 3 bytes for RGB
//
//        glReadPixels(0, 0, GRID_SIZE, GRID_SIZE, GL_RGB, GL_UNSIGNED_BYTE, pixels);
//
//        std::cout << "First pixel (RGB): "
//                  << (int)pixels[0] << ", "
//                  << (int)pixels[1] << ", "
//                  << (int)pixels[2] << std::endl;
//
//        delete[] pixels;
//
//        std::cout << "Apply slice " << std::endl;
    }


    glViewport(0, 0, viewportWidth, viewportHeight);
    // Unbind the framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void copyTexture(GLuint srcTexture, GLuint dstTexture) {
    GLuint srcFBO, dstFBO;
    glGenFramebuffers(1, &srcFBO);
    glGenFramebuffers(1, &dstFBO);

    for (int z = 0; z < GRID_SIZE; ++z) {
        // Bind source framebuffer
        glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, srcTexture, 0, z);

        // Bind destination framebuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, dstTexture, 0, z);

        // Copy the layer using blit
        glBlitFramebuffer(
                0, 0, GRID_SIZE, GRID_SIZE, // Source rectangle
                0, 0, GRID_SIZE, GRID_SIZE, // Destination rectangle
                GL_COLOR_BUFFER_BIT, GL_NEAREST
        );
    }

    // Cleanup
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &srcFBO);
    glDeleteFramebuffers(1, &dstFBO);
}


void addDye(GLFWwindow *window, bool click) {
    if (!click) {
        return;
    }

    glm::vec2 mouseNDC;
    getMouseNDC(window, mouseNDC);

    // World space
    glm::vec3 applyDyePos = computeForcePosition(mouseNDC);

    // No intersection with top face
    if (applyDyePos == glm::vec3(-1, -1, -1)) {
        std::cout << "No intersection " << std::endl;
        return;
    }

    std::cout << "Intersection " << std::endl;
    std::cout << "Force Position: "
              << applyDyePos.x << ", "
              << applyDyePos.y << ", "
              << applyDyePos.z << std::endl;


    glUseProgram(addDyeShaderProgram);

    GLuint dyeTextureLoc = glGetUniformLocation(addDyeShaderProgram, "levelSetTexture");
    GLuint addDyePosLoc = glGetUniformLocation(addDyeShaderProgram, "addDyePos");
    GLuint dyeRadiusLoc = glGetUniformLocation(addDyeShaderProgram, "dyeRadius");
    GLuint dyeColorLoc = glGetUniformLocation(addDyeShaderProgram, "dyeColor");
    GLuint addDyeLoc = glGetUniformLocation(addDyeShaderProgram, "addDye");
    GLuint sliceLoc = glGetUniformLocation(addDyeShaderProgram, "slice");

    glUniform1i(dyeTextureLoc, 0);
    glUniform3fv(addDyePosLoc, 1, glm::value_ptr(applyDyePos));
    glUniform1f(dyeRadiusLoc, 0.3);
    GLfloat dyeColor[3] = { 1.0f, 0.0f, 0.0f };
    glUniform3fv(dyeColorLoc, 1, dyeColor);
    glUniform1i(addDyeLoc, click);

    // Bind the 3D texture as the framebuffer target
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_3D, outputTexture);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);


//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, outputTexture, 0, slice);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    copyTexture(outputTexture, levelSetTexture);


    glViewport(0, 0, viewportWidth, viewportHeight);
    // Unbind the framebuffer and texture
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void applyBoundaryConditions(GLuint texture, bool isPressure) {
    return;
    glUseProgram(boundaryShaderProgram);

    GLuint scaleLoc = glGetUniformLocation(boundaryShaderProgram, "scale");
    GLuint textureLoc = glGetUniformLocation(boundaryShaderProgram, "texture");
    GLuint gridSizeLoc = glGetUniformLocation(boundaryShaderProgram, "gridSize");
    GLuint sliceLoc = glGetUniformLocation(boundaryShaderProgram, "slice");

    if (isPressure) {
        glUniform1f(scaleLoc, 1.0);
    } else {
        glUniform1f(scaleLoc, -1.0);
    }

    if (texture == levelSetTexture) {
        glUniform1i(textureLoc, 0);
    } else if (texture == velocityTexture) {
        glUniform1i(textureLoc, 1);
    } else if (texture == pressureTexture) {
        glUniform1i(textureLoc, 2);
    }
//    glUniform1i(textureLoc, texture);
    glUniform1i(gridSizeLoc, GRID_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, GRID_SIZE, GRID_SIZE);


    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, texture, 0, slice);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0, slice);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
            break;
        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }



//    glViewport(0, 0, viewportWidth, viewportHeight);
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}


void advect(GLuint texture) {
    glUseProgram(advectShaderProgram);

    // Bind the velocity and dye textures
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, levelSetTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_3D, velocityTexture);

    // Set the uniform variables
    GLuint timestepLoc = glGetUniformLocation(advectShaderProgram, "timestep");
    GLuint rdxLoc = glGetUniformLocation(advectShaderProgram, "rdx");
    GLuint velocityTextureLoc = glGetUniformLocation(advectShaderProgram, "velocityTexture");
    GLuint advectedTextureLoc = glGetUniformLocation(advectShaderProgram, "advectedTexture");
    GLuint sliceLoc = glGetUniformLocation(advectShaderProgram, "slice");

    glUniform1f(timestepLoc, timeStep);
    glUniform1f(rdxLoc, 1.0 / GRID_SIZE);
    glUniform1i(velocityTextureLoc, 1);


    if (texture == levelSetTexture) {
        glUniform1i(advectedTextureLoc, 0);
    } else if (texture == velocityTexture) {
        glUniform1i(advectedTextureLoc, 1);
    }

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, outputTexture, 0, slice);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }



        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    copyTexture(outputTexture, texture);


//    applyBoundaryConditions(texture, false);



    glViewport(0, 0, viewportWidth, viewportHeight);

    // Unbind the framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void jacobi(GLuint texture, GLuint xLoc, GLuint sliceLoc) {

    if (texture == velocityTexture) {
        glUniform1i(xLoc, 1);
    } else if (texture == pressureTexture) {
        glUniform1i(xLoc, 2);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    // 1st iteration: write to jacobiTexture1
    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, jacobiTexture1, 0, slice);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, jacobiTexture1, 0, slice);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
            break;
        }



        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }

//    if (outputTexture == velocityTexture) {
//        applyBoundaryConditions(jacobiTexture1, false);
//    } else if (outputTexture == pressureTexture) {
//        applyBoundaryConditions(jacobiTexture1, true);
//    }

    int NO_OF_ITERATIONS = 10;
    GLuint currTexture; //texture to write to
    for (int i = 0; i < NO_OF_ITERATIONS; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        for (int slice = 0; slice < GRID_SIZE; slice++) {
            float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
            glUniform1f(sliceLoc, sliceDepth);

            // Alternate between two textures to read & write
            if (i % 2 == 0) { // Multiple of 2 - input: jacobiTexture1, output: jacobiTexture2
                currTexture = jacobiTexture2;
                // Bind output texture to framebuffer
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, currTexture, 0, slice);
//                glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, currTexture, 0, slice);
                // Input texture
                glUniform1i(xLoc, 3);
            } else {// input: jacobiTexture2, output: jacobiTexture1
                currTexture = jacobiTexture1;
                // Bind output texture to framebuffer
                glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, currTexture, 0, slice);
//                glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, currTexture, 0, slice);
                // Input texture
                glUniform1i(xLoc, 4);
            }

//            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//                std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//                break;
//            }


            // Render a full-screen quad to update the texture slice
            glBindVertexArray(quadVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }



//        if (outputTexture == velocityTexture) {
//            applyBoundaryConditions(currTexture, false);
//        } else if (outputTexture == pressureTexture) {
//            applyBoundaryConditions(currTexture, true);
//        }

    }


    // Copy final texture (jacobiTexture1: odd, jacobiTexture2: even) to texture
    copyTexture(jacobiTexture1, texture);

    glViewport(0, 0, viewportWidth, viewportHeight);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void diffuse(GLuint texture) {
    glUseProgram(jacobiShaderProgram);

    // Uniform variables
    GLuint alphaLoc = glGetUniformLocation(jacobiShaderProgram, "alpha");
    GLuint rBetaLoc = glGetUniformLocation(jacobiShaderProgram, "rBeta");
    GLuint xLoc = glGetUniformLocation(jacobiShaderProgram, "x");
    GLuint bLoc = glGetUniformLocation(jacobiShaderProgram, "b");
    GLuint gridSizeLoc = glGetUniformLocation(jacobiShaderProgram, "gridSize");
    GLuint sliceLoc = glGetUniformLocation(jacobiShaderProgram, "slice");
    GLuint levelSetTextureLoc = glGetUniformLocation(jacobiShaderProgram, "levelSetTexture");
    GLuint isPressureLoc = glGetUniformLocation(jacobiShaderProgram, "isPressure");

    float dx = 1.0 / GRID_SIZE;
    float nu = 0.0002;
    float alpha = (dx * dx) / (nu * timeStep);

    glUniform1f(alphaLoc, alpha);
    glUniform1f(rBetaLoc, 1.0f / (6.0f + alpha));
    glUniform1i(gridSizeLoc, GRID_SIZE);
    glUniform1i(levelSetTextureLoc, 0);
    glUniform1i(isPressureLoc, 0);

    if (texture == levelSetTexture) {
        glUniform1i(bLoc, 0);
//        jacobi(levelSetTexture, xLoc, sliceLoc);
    } else if (texture == velocityTexture) {
        glUniform1i(bLoc, 1);
        jacobi(velocityTexture, xLoc, sliceLoc);
    }

}

void divergence(GLuint divergenceTexture) {
    glUseProgram(divergenceShaderProgram);

    GLuint wLoc = glGetUniformLocation(divergenceShaderProgram, "w");
    GLuint halfrdxLoc = glGetUniformLocation(divergenceShaderProgram, "halfrdx");
    GLuint gridSizeLoc = glGetUniformLocation(divergenceShaderProgram, "gridSize");
    GLuint sliceLoc = glGetUniformLocation(divergenceShaderProgram, "slice");

    glUniform1i(wLoc, 1);
    glUniform1f(halfrdxLoc, 1.0 / (2.0  * GRID_SIZE));
    glUniform1i(gridSizeLoc, GRID_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

//        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, divergenceTexture, 0, slice);
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, divergenceTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    glViewport(0, 0, viewportWidth, viewportHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void subtractGradient() {
    glUseProgram(gradientSubtractShaderProgram);

    GLuint pLoc = glGetUniformLocation(gradientSubtractShaderProgram, "p");
    GLuint wLoc = glGetUniformLocation(gradientSubtractShaderProgram, "w");
    GLuint halfrdxLoc = glGetUniformLocation(gradientSubtractShaderProgram, "halfrdx");
    GLuint gridSizeLoc = glGetUniformLocation(gradientSubtractShaderProgram, "gridSize");
    GLuint sliceLoc = glGetUniformLocation(gradientSubtractShaderProgram, "slice");
    GLuint levelSetTextureLoc = glGetUniformLocation(gradientSubtractShaderProgram, "levelSetTexture");

    glUniform1i(pLoc, 2);
    glUniform1i(wLoc, 1);
    glUniform1f(halfrdxLoc, 1.0 / (2.0  * GRID_SIZE));
    glUniform1i(gridSizeLoc, GRID_SIZE);
    glUniform1i(levelSetTextureLoc, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, outputTexture, 0, slice);
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, outputTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

    }

    copyTexture(outputTexture, velocityTexture);

//    applyBoundaryConditions(velocityTexture, false);

    glViewport(0, 0, viewportWidth, viewportHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void zeroAirCellPressure() {
    glUseProgram(zeroAirCellPressureShaderProgram);

    GLuint pressureTextureLoc = glGetUniformLocation(zeroAirCellPressureShaderProgram, "pressureTexture");
    GLuint levelSetTextureLoc = glGetUniformLocation(zeroAirCellPressureShaderProgram, "levelSetTexture");
    GLuint sliceLoc = glGetUniformLocation(zeroAirCellPressureShaderProgram, "slice");

    glUniform1i(pressureTextureLoc, 2);
    glUniform1i(levelSetTextureLoc, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    glViewport(0, 0, GRID_SIZE, GRID_SIZE);

    for (int slice = 0; slice < GRID_SIZE; slice++) {
        float sliceDepth = (float) (slice + 0.5f) / GRID_SIZE;
        glUniform1f(sliceLoc, sliceDepth);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, pressureTexture, 0, slice);
//        glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_3D, outputTexture, 0, slice);
//        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
//            std::cerr << "Framebuffer is not complete for slice " << slice << std::endl;
//            break;
//        }

        // Render a full-screen quad to update the texture slice
        glBindVertexArray(quadVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glViewport(0, 0, viewportWidth, viewportHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
}

void project() {
    // Divergence of intermediate velocity field w
    GLuint divergenceTexture;
    glActiveTexture(GL_TEXTURE6);
    glGenTextures(1, &divergenceTexture);
    glBindTexture(GL_TEXTURE_3D, divergenceTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    divergence(divergenceTexture);

    glUseProgram(jacobiShaderProgram);

    GLuint alphaLoc = glGetUniformLocation(jacobiShaderProgram, "alpha");
    GLuint rBetaLoc = glGetUniformLocation(jacobiShaderProgram, "rBeta");
    GLuint xLoc = glGetUniformLocation(jacobiShaderProgram, "x");
    GLuint bLoc = glGetUniformLocation(jacobiShaderProgram, "b");
    GLuint gridSizeLoc = glGetUniformLocation(jacobiShaderProgram, "gridSize");
    GLuint sliceLoc = glGetUniformLocation(jacobiShaderProgram, "slice");
    GLuint levelSetTextureLoc = glGetUniformLocation(jacobiShaderProgram, "levelSetTexture");
    GLuint isPressureLoc = glGetUniformLocation(jacobiShaderProgram, "isPressure");

    float dx = GRID_SIZE;
    float alpha = -(dx * dx);
    float rBeta = 1.0 / 6.0;

    glUniform1f(alphaLoc, alpha);
    glUniform1f(rBetaLoc, rBeta);
    glUniform1i(bLoc, 6); // divergence of w
    glUniform1i(gridSizeLoc, GRID_SIZE);
    glUniform1i(levelSetTextureLoc, 0);
    glUniform1i(isPressureLoc, 1);

//    zeroAirCellPressure();
    // Solve for pressure field
    jacobi(pressureTexture, xLoc, sliceLoc);

    subtractGradient();

    glDeleteTextures(1, &divergenceTexture);

}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // OpenGL version and core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);  // Required for macOS

    GLFWwindow* window = glfwCreateWindow(800, 800, "Fluid Simulation", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewInit();

    shaderProgram = createShaderProgram("../shader.vert", "../shader.frag");
    applyForceShaderProgram = createShaderProgram("../quadShader.vert", "../applyForce.frag");
    addDyeShaderProgram = createShaderProgram("../quadShader.vert", "../addDye.frag");
    advectShaderProgram = createShaderProgram("../quadShader.vert", "../advect.frag");
    jacobiShaderProgram = createShaderProgram("../quadShader.vert", "../jacobi.frag");
    divergenceShaderProgram = createShaderProgram("../quadShader.vert", "../divergence.frag");
    gradientSubtractShaderProgram = createShaderProgram("../quadShader.vert", "../subtractGradient.frag");
    boundaryShaderProgram = createShaderProgram("../quadShader.vert", "../boundary.frag");
    levelSetInitShaderProgram = createShaderProgram("../quadShader.vert", "../levelSetInit.frag");
    zeroAirCellPressureShaderProgram = createShaderProgram("../quadShader.vert", "../zeroAirCellPressure.frag");


    std::vector<float> cubeVertices;
    generateCubeVertices(cubeVertices, 1.0f);

    // Indices for drawing the cube with EBO
    std::vector<unsigned int> cubeIndices = {
            0, 1, 2, 2, 3, 0,       // Front face
            4, 5, 6, 6, 7, 4,       // Back face
            8, 9, 10, 10, 11, 8,    // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Bottom face
            20, 21, 22, 22, 23, 20  // Top face
    };

    std::vector<unsigned int> waterIndices = {
            0, 1, 2, 2, 3, 0,       // Front face
            4, 5, 6, 6, 7, 4,       // Back face
            8, 9, 10, 10, 11, 8,    // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Bottom face
            20, 21, 22, 22, 23, 20  // Top face
    };

    // Create VAO, VBO, EBO
    // Cube vertices
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);           // Position
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Texture coordinate
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Full screen quad for texture update
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

//    glEnable(GL_DEPTH_TEST);

    glGenFramebuffers(1, &framebuffer);


    // Create textures
    std::vector<GLfloat> zeroData(GRID_SIZE * GRID_SIZE * GRID_SIZE * 4, 0.0f);

    std::vector<GLfloat> colorData(GRID_SIZE * GRID_SIZE * GRID_SIZE * 4, 0.0f);

    // Modify specific elements (optional)
    for (int k = 0; k < GRID_SIZE; ++k) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            for (int i = 0; i < GRID_SIZE; ++i) {
                int index = k * GRID_SIZE * GRID_SIZE + j * GRID_SIZE + i;
                // You can modify the values here if needed
                colorData[index * 4 + 0] = 0.0f; // Set R to 1.0f, for example
                colorData[index * 4 + 1] = 0.5f; // G component
                colorData[index * 4 + 2] = 0.5f; // B component
                colorData[index * 4 + 3] = 1.0f; // A component
            }
        }
    }


    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &levelSetTexture);
    glBindTexture(GL_TEXTURE_3D, levelSetTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, colorData.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE1);
    glGenTextures(1, &velocityTexture);
    glBindTexture(GL_TEXTURE_3D, velocityTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE2);
    glGenTextures(1, &pressureTexture);
    glBindTexture(GL_TEXTURE_3D, pressureTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE3);
    glGenTextures(1, &jacobiTexture1);
    glBindTexture(GL_TEXTURE_3D, jacobiTexture1);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE4);
    glGenTextures(1, &jacobiTexture2);
    glBindTexture(GL_TEXTURE_3D, jacobiTexture2);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE5);
    glGenTextures(1, &outputTexture);
    glBindTexture(GL_TEXTURE_3D, outputTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    viewportWidth = viewport[2];
    viewportHeight = viewport[3];

    levelSetInit();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
//            addDye(window, true);
//        } else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
//            addDye(window, false);
//        }

        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            applyForce(window);
        }

        advect(levelSetTexture);
        advect(velocityTexture);

        diffuse(velocityTexture);

         project();


// Use the shader program
        glUseProgram(shaderProgram);

        model = glm::mat4(1.0f); // Identity matrix

        // Camera position (slightly above and behind the cube)
        glm::vec3 cameraPosition = glm::vec3(-1.0f, 1.0f, -2.0f);
        glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

        // Create the view matrix using glm::lookAt
        view = glm::lookAt(cameraPosition, cameraTarget, up);
        projection = projection = glm::perspective(glm::radians(45.0f), (float)viewportWidth / (float)viewportHeight, 0.1f, 100.0f);
        glViewport(0, 0, viewportWidth, viewportHeight);

        // Uniform variables
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        GLuint inputTextureLoc = glGetUniformLocation(shaderProgram, "inputTexture");
        GLuint fluidSizeLoc = glGetUniformLocation(shaderProgram, "fluidSize");
//        GLuint gridSizeLoc = glGetUniformLocation(shaderProgram, "gridSize");
        GLuint cameraPosLoc = glGetUniformLocation(shaderProgram, "cameraPos");
        GLuint levelSetTextureLoc = glGetUniformLocation(shaderProgram, "levelSetTexture");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniform1i(inputTextureLoc, 0);
        glUniform1i(levelSetTextureLoc, 0);
        glUniform1f(fluidSizeLoc, cubeSize);
//        glUniform1i(gridSizeLoc, GRID_SIZE);
        glUniform3fv(cameraPosLoc, 1, glm::value_ptr(cameraPosition));

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//        glEnable(GL_CULL_FACE);
//        glCullFace(GL_BACK);


        // Bind the VAO
        glBindVertexArray(VAO);

        // Draw the cube
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeIndices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        glDisable(GL_BLEND);
//        glDisable(GL_CULL_FACE);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &levelSetTexture);
    glDeleteTextures(1, &velocityTexture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
