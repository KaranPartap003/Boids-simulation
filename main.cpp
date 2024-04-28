#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm\gtc\random.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Window.h"
int PARTICLE_COUNT = 19200;

//-----------UI VARIABLES-------------//
GLfloat viewRadius, avoidRadius, cohesionWeight, 
        alignmentWeight, separationWeight, boundary;

glm::vec3 Maxbndry = { 1.0f, 1.0f ,1.0f};
glm::vec3 Minbndry = {-1.0f,-1.0f,-1.0f };

GLuint dt, n, Maxlimit, Minlimit,
       view, avoid, cohesion, align, seperation, fov;

GLuint position_buffer, velocity_buffer, color_buffer, 
       currPos, currVel, currCol, 
       prevPos, prevVel, prevCol,VAO;

Shader shader, compute;

GLint numOfWorkgroups = PARTICLE_COUNT / 128;

int numOfAttractors = 64;

Window mainWindow;
Camera camera;

const float aspRatio = 1366 / 768;
const float toRadians = 3.14159265f / 180.0f;

void setUniforms(GLuint ID, double deltaTime)
{
    //setting deltatime
    dt = glGetUniformLocation(ID, "dt");
    glUniform1f(dt, deltaTime);
    n = glGetUniformLocation(ID, "particles");
    glUniform1i(n, PARTICLE_COUNT);
    Maxlimit = glGetUniformLocation(ID, "MaxB");
    glUniform3f(Maxlimit, Maxbndry.x, Maxbndry.y, Maxbndry.z);
    Minlimit = glGetUniformLocation(ID, "MinB");
    glUniform3f(Minlimit, Minbndry.x, Minbndry.y, Minbndry.z);
    fov = glGetUniformLocation(ID, "FOV");
    glUniform1f(fov, glm::cos(100 * toRadians));
    view = glGetUniformLocation(ID, "viewRadius");
    glUniform1f(view, 0.5);
    avoid = glGetUniformLocation(ID, "avoidRadius");
    glUniform1f(avoid, 0.005);
    cohesion = glGetUniformLocation(ID, "cohesionWeight");
    glUniform1f(cohesion, 0.7);
    align = glGetUniformLocation(ID, "alignmentWeight");
    glUniform1f(align, 0.4);
    seperation= glGetUniformLocation(ID, "separationWeight");
    glUniform1f(seperation, 1.5);
}

void ComputeShaderPass(double deltaTime)
{
    compute.UseShader();

    setUniforms(compute.GetID(), deltaTime);

    // Bind curr position, color and velocity buffers to image units
    glBindImageTexture(0, currPos, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(1, currVel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(2, currCol, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    // prev buffers
    glBindImageTexture(3, prevPos, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(4, prevVel, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glBindImageTexture(5, prevCol, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
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
    mvp *= glm::perspective(glm::radians(60.0f), (GLfloat)mainWindow.GetBufferWidth() / mainWindow.GetBufferHeight(), 0.1f, 100.0f);
    mvp *= camera.calculateViewMatrix();
    int m = glGetUniformLocation(shader.GetID(), "mvp");
    glUniformMatrix4fv(m, 1, GL_FALSE, glm::value_ptr(mvp));
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
        float x = glm::linearRand(-1.0f, 1.0f);
        float y = glm::linearRand(-1.0f, 1.0f);
        float z = glm::linearRand(-1.0f, 1.0f);
        positions[i] = glm::vec4(x, y, z, 1.0f);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    //setup position attribute
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(0);
    // color buffer :
    glGenBuffers(1, &color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, color_buffer);

    glBufferData(GL_ARRAY_BUFFER, PARTICLE_COUNT * sizeof(glm::vec4), NULL, GL_DYNAMIC_COPY);
    glm::vec4* colors = (glm::vec4*)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLE_COUNT * sizeof(glm::vec4),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    //fill the colors vector with randomised values
    for (int i = 0; i < PARTICLE_COUNT; ++i)
    {
        // Generate random values for each component using glm::linearRand()
        float r = glm::linearRand(0.0f, 1.0f);
        float g = glm::linearRand(0.0f, 1.0f);
        float b = glm::linearRand(0.0f, 1.0f);
        colors[i] = glm::vec4(r, g, b, 1.0f);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);
    //setup color attribute : 
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), nullptr);
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //bind posiiton buffer as texture
    glBindBuffer(GL_TEXTURE_BUFFER, position_buffer);
    //generate currPos
    glGenTextures(1, &currPos);
    glBindTexture(GL_TEXTURE_BUFFER, currPos);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    //generate prevPos
    glGenTextures(1, &prevPos);
    glBindTexture(GL_TEXTURE_BUFFER, prevPos);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, position_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    //bind color buffer as texture
    glBindBuffer(GL_TEXTURE_BUFFER, color_buffer);
    //generate currCol
    glGenTextures(1, &currCol);
    glBindTexture(GL_TEXTURE_BUFFER, currCol);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, color_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    //prev Color
    glGenTextures(1, &prevCol);
    glBindTexture(GL_TEXTURE_BUFFER, prevCol);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, color_buffer);
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
        float x = glm::linearRand(-0.04f,0.04f);
        float y = glm::linearRand(-0.04f,0.04f);
        float z = glm::linearRand(-0.04f,0.04f);
        velocities[i] = glm::vec4(x, y, z, 0);
    }
    glUnmapBuffer(GL_TEXTURE_BUFFER);
    //curr Velocity
    glGenTextures(1, &currVel);
    glBindTexture(GL_TEXTURE_BUFFER, currVel);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocity_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);
    //prev Velocity
    glGenTextures(1, &prevVel);
    glBindTexture(GL_TEXTURE_BUFFER, prevVel);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, velocity_buffer);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    glBindBuffer(0, GL_TEXTURE_BUFFER);
}

int main()
{
    mainWindow = Window(1366, 768);
    mainWindow.initialise();

    camera = Camera(glm::vec3(0.0f, 0.0f, 3.5f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f, 5.0f, 0.1f);

    shader.CreateFromFile("shaders/shader.vert", "shaders/shader.frag");
    compute.CreateFromFile("shaders/particles.comp");

    InitialiseBuffers();

    compute.LinkShader();
    shader.LinkShader();

    double previousTime = glfwGetTime();
    double deltaTime;

    /* Loop until the user closes the window */
    while (!mainWindow.getShouldClose())
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - previousTime;
        previousTime = currentTime;
        /* Poll for and process events */
        glfwPollEvents();

        camera.keyControl(mainWindow.getKeys(), deltaTime);
        camera.mouseControl(mainWindow.getXChange(), mainWindow.getYChange(), deltaTime);

        /* Render here */
        glClearColor(0.0f, 0.0f, 0.0f, 0.5);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ComputeShaderPass(deltaTime);
        RenderPass(deltaTime);
        std::swap(prevCol, currCol);
        std::swap(prevVel, currVel);
        std::swap(prevPos, currPos);

        // Unbind shader program
        glUseProgram(0);
        /* Swap front and back buffers */
        mainWindow.swapBuffer();
    }
    compute.ClearShader();
    shader.ClearShader();
    glfwTerminate();
    return 0;
}
