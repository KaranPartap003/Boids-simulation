#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm\gtc\random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
int PARTICLE_COUNT = 128000;


GLuint position_buffer, velocity_buffer,position_tex, velocity_tex, attractorBuffer, VAO;
Shader shader, compute;

GLint numOfWorkgroups = PARTICLE_COUNT / 128;

int numOfAttractors = 64;

const float aspRatio = 1366 / 768;
const float toRadians = 3.14159265f / 180.0f;

std::vector<int> attractor_masses(numOfAttractors);

void ComputeShaderPass(double deltaTime)
{
    compute.UseShader();

    //setting deltatime
    GLuint loc = glGetUniformLocation(compute.GetID(), "dt");
    glUniform1f(loc, deltaTime);
    // Bind position and velocity buffers to image units
    glBindImageTexture(0, position_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, velocity_tex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    // Update the buffer containing the attractor positions and masses
    glBindBuffer(GL_UNIFORM_BUFFER, attractorBuffer);
    glm::vec4* attractors =(glm::vec4*)glMapBufferRange(GL_UNIFORM_BUFFER,
                                                        0,
                                                        64 * sizeof(glm::vec4),
                                                        GL_MAP_WRITE_BIT |
                                                        GL_MAP_INVALIDATE_BUFFER_BIT);
    for (int i = 0; i < 64; i++)
    {
        attractors[i] = glm::vec4(sinf(deltaTime * (float)(i + 4) * 7.5f * 20.0f) * 50.0f,
                        cosf(deltaTime * (float)(i + 7) * 3.9f * 20.0f) * 50.0f,
                        sinf(deltaTime * (float)(i + 3) * 5.3f * 20.0f) *
                        cosf(deltaTime * (float)(i + 5) * 9.1f) * 100.0f,
                        attractor_masses[i]);
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Dispatch compute shader
    glDispatchCompute(numOfWorkgroups, 1, 1);

    // Ensure compute shader finishes execution
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void RenderPass(double deltaTime)
{
    shader.UseShader();
    //setup mvp matrix
    glm::mat4 mvp = glm::mat4(1.0f);
    mvp = glm::perspective(45.0f * toRadians, aspRatio, 0.1f, 1000.0f);
    mvp = glm::translate(mvp, glm::vec3(0.0f, 0.0f, -5.0f));
    int loc = glGetUniformLocation(shader.GetID(), "mvp");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));
    // Bind VAO and issue draw commands
    glBindVertexArray(VAO);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glDrawArrays(GL_POINTS, 0, PARTICLE_COUNT);
    glBindVertexArray(0);
}

void InitialiseBuffers()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    //POSITIONS BUFFER
    glGenBuffers(1, &position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, position_buffer);

    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
    //glMapBufferRange provides a pointer to the location allocated by glBufferData to be filled
    glm::vec4* positions = (glm::vec4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    //fill the positions vector with randomised values
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        // Generate random values for each component using glm::linearRand()
        float x = glm::linearRand(-5.0f, 5.0f);
        float y = glm::linearRand(-5.0f, 5.0f);
        float z = glm::linearRand(-5.0f, 5.0f);
        float w = glm::linearRand(0.0f, 1.0f);
        positions[i] = glm::vec4(x, y, z, w);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //bind buffer as texture
    glBindBuffer(GL_TEXTURE_BUFFER, position_buffer);
    //generate texture
    glGenTextures(1, &position_tex);
    glBindTexture(GL_TEXTURE_BUFFER, position_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);


    //INITIALISING VELOCITIES
    glGenBuffers(1, &velocity_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, velocity_buffer);
    glBufferData(GL_TEXTURE_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
    glm::vec4* velocities = (glm::vec4*)glMapBufferRange(GL_TEXTURE_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    //fill the velocity vector with randomised values
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        // Generate random values for each component using glm::linearRand()
        float x = glm::linearRand(-0.1f, 0.1f);
        float y = glm::linearRand(-0.1f, 0.1f);
        float z = glm::linearRand(-0.1f, 0.1f);
        velocities[i] = glm::vec4(x, y, z, 0);
    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);
    glGenTextures(1, &velocity_tex);
    glBindTexture(GL_TEXTURE_BUFFER, velocity_tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocity_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);


    //INITIALISING ATTRACTORS
    glGenBuffers(1, &attractorBuffer);
    glBindBuffer(GL_UNIFORM_BUFFER, attractorBuffer);
    glBufferData(GL_UNIFORM_BUFFER, numOfAttractors * sizeof(glm::vec4), NULL, GL_DYNAMIC_DRAW);
    glm::vec4* attractors = (glm::vec4*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, numOfAttractors * sizeof(glm::vec4),
                                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
  
    for (int i = 0; i < numOfAttractors; ++i)
    {
        attractor_masses[i] = glm::linearRand(0.5f, 1.0f);
        attractors[i] = glm::vec4(0.0f, 0.0f, 0.0f, attractor_masses[i]);
    }
    glUnmapBuffer(GL_UNIFORM_BUFFER);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

int main(void)
{
    /* Initialize the library */
    if (!glfwInit()) {
        std::cout << "GLFW initialisation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // Core Profile
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // Allow Forward Compatbility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    GLFWwindow* window = glfwCreateWindow(1366, 768, "Hello World", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Error: Window creation failed\n");
        glfwTerminate();
        return -1;
    }
    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    // Allow modern extension features
    glewExperimental = GL_TRUE;

    GLenum error = glewInit();
    if (error != GLEW_OK)
    {
        printf("Error: %s", glewGetErrorString(error));
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    // Setup Viewport size
    glViewport(0, 0, 1366, 768);
    
    shader.CreateFromFile("shaders/shader.vert", "shaders/shader.frag");
    compute.CreateFromFile("shaders/particles.comp");

    InitialiseBuffers();

    compute.LinkShader();
    shader.LinkShader();

    double previousTime = glfwGetTime();
    double deltaTime;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        ComputeShaderPass(deltaTime);
        RenderPass(deltaTime);

        // Unbind shader program
        glUseProgram(0);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    compute.ClearShader();
    shader.ClearShader();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

