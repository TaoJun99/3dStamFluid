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

GLuint dyeTexture;
GLuint VAO, VBO, EBO;
GLuint shaderProgram;

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
void generateCubeVertices(float size, std::vector<float>& vertices) {
    float halfSize = size / 2.0f;

    vertices = {
            // Positions for each vertex of the cube
            // Front face
            -halfSize, -halfSize,  halfSize, // 0
            halfSize, -halfSize,  halfSize, // 1
            halfSize,  halfSize,  halfSize, // 2
            -halfSize,  halfSize,  halfSize, // 3

            // Back face
            -halfSize, -halfSize, -halfSize, // 4
            halfSize, -halfSize, -halfSize, // 5
            halfSize,  halfSize, -halfSize, // 6
            -halfSize,  halfSize, -halfSize, // 7

            // Left face
            -halfSize, -halfSize, -halfSize, // 8
            -halfSize, -halfSize,  halfSize, // 9
            -halfSize,  halfSize,  halfSize, // 10
            -halfSize,  halfSize, -halfSize, // 11

            // Right face
            halfSize, -halfSize, -halfSize, // 12
            halfSize, -halfSize,  halfSize, // 13
            halfSize,  halfSize,  halfSize, // 14
            halfSize,  halfSize, -halfSize, // 15

            // Bottom face
            -halfSize, -halfSize, -halfSize, // 16
            halfSize, -halfSize, -halfSize, // 17
            halfSize, -halfSize,  halfSize, // 18
            -halfSize, -halfSize,  halfSize, // 19

            // Top face
            -halfSize,  halfSize, -halfSize, // 20
            halfSize,  halfSize, -halfSize, // 21
            halfSize,  halfSize,  halfSize, // 22
            -halfSize,  halfSize,  halfSize  // 23
    };
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

    float cubeSize = 1.0f;
    std::vector<float> cubeVertices;
    generateCubeVertices(cubeSize, cubeVertices);

    // Indices for drawing the cube with EBO
    std::vector<unsigned int> cubeIndices = {
            0, 1, 2, 2, 3, 0,       // Front face
            4, 5, 6, 6, 7, 4,       // Back face
            8, 9, 10, 10, 11, 8,    // Left face
            12, 13, 14, 14, 15, 12, // Right face
            16, 17, 18, 18, 19, 16, // Bottom face
            20, 21, 22, 22, 23, 20  // Top face
    };

    // Create VAO, VBO, EBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Max 3D Texture Size: " << maxTextureSize << std::endl;

    // Create textures
    GLfloat zeroData[GRID_SIZE * GRID_SIZE * GRID_SIZE * 4] = {0.0f};

    // Modify specific elements (optional)
    for (int k = 0; k < GRID_SIZE; ++k) {
        for (int j = 0; j < GRID_SIZE; ++j) {
            for (int i = 0; i < GRID_SIZE; ++i) {
                int index = k * GRID_SIZE * GRID_SIZE + j * GRID_SIZE + i;
                // You can modify the values here if needed
                zeroData[index * 4 + 0] = 1.0f; // Set R to 1.0f, for example
                zeroData[index * 4 + 1] = 0.0f; // G component
                zeroData[index * 4 + 2] = 0.0f; // B component
                zeroData[index * 4 + 3] = 1.0f; // A component
            }
        }
    }

//    glActiveTexture(GL_TEXTURE0);
//    glGenTextures(1, &dyeTexture);
//    glBindTexture(GL_TEXTURE_2D, dyeTexture);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE * GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &dyeTexture);
    glBindTexture(GL_TEXTURE_3D, dyeTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, GRID_SIZE, GRID_SIZE, GRID_SIZE, 0, GL_RGBA, GL_FLOAT, zeroData);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);



// Set up transformations
    glm::mat4 model = glm::mat4(1.0f); // Identity matrix
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)); // Camera back by 5 units
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f); // Perspective

// Use the shader program
    glUseProgram(shaderProgram);

    GLenum err;
    while ((err = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error: " << err << std::endl;
    }

    // Uniform variables
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    GLuint inputTextureLoc = glGetUniformLocation(shaderProgram, "inputTexture");
    GLuint fluidSizeLoc = glGetUniformLocation(shaderProgram, "fluidSize");
    GLuint gridSizeLoc = glGetUniformLocation(shaderProgram, "gridSize");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(inputTextureLoc, 0);
    glUniform1f(fluidSizeLoc, cubeSize);
    glUniform1i(gridSizeLoc, GRID_SIZE);



    // Render loop
    while (!glfwWindowShouldClose(window)) {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float angle = static_cast<float>(glfwGetTime()); // Use time for rotation
        model = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Bind the VAO
        glBindVertexArray(VAO);

        // Draw the cube
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(cubeIndices.size()), GL_UNSIGNED_INT, 0);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteTextures(1, &dyeTexture);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}
