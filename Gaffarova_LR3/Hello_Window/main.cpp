#include <C:/OpenGL/glad/include/glad/glad.h>
#include <C:/OpenGL/glflow/glfw3.h>
#include "stb_image.h"

#include <C:\OpenGL\glm\glm\glm.hpp>
#include <C:\OpenGL\glm\glm\gtc\matrix_transform.hpp>
#include <C:\OpenGL\glm\glm\gtc\type_ptr.hpp>


#include "shader.h"
#include "camera.h"
#include "model.h"
#include "LiteMath.h"
#include <random>

#include <iostream>
using namespace LiteMath;
#define GLFW_DLL
#define PI 3.14159265

using namespace glm;

void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode);
void OnMouseMove(GLFWwindow* window, double xpos, double ypos);
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods);
void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset);
void doCameraMovement(Camera& camera, GLfloat deltaTime);
unsigned int loadTexture(const char *path, bool gammaCorrection);
void renderPlane();
void renderQuad();
void renderCube();


GLsizei CreateSphere(float radius, int numberSlices, GLuint& vao)
{
    int i, j;

    int numberParallels = numberSlices;
    int numberVertices = (numberParallels + 1) * (numberSlices + 1);
    int numberIndices = numberParallels * numberSlices * 3;

    float angleStep = (2.0f * 3.14159265358979323846f) / ((float)numberSlices);

    std::vector<float> pos(numberVertices * 4, 0.0f);
    std::vector<float> norm(numberVertices * 4, 0.0f);
    std::vector<float> texcoords(numberVertices * 2, 0.0f);

    std::vector<int> indices(numberIndices, -1);

    for (i = 0; i < numberParallels + 1; i++)
    {
        for (j = 0; j < numberSlices + 1; j++)
        {
            int vertexIndex = (i * (numberSlices + 1) + j) * 4;
            int normalIndex = (i * (numberSlices + 1) + j) * 4;
            int texCoordsIndex = (i * (numberSlices + 1) + j) * 2;

            pos.at(vertexIndex + 0) = radius * sinf(angleStep * (float)i) * sinf(angleStep * (float)j);
            pos.at(vertexIndex + 1) = radius * cosf(angleStep * (float)i);
            pos.at(vertexIndex + 2) = radius * sinf(angleStep * (float)i) * cosf(angleStep * (float)j);
            pos.at(vertexIndex + 3) = 1.0f;

            norm.at(normalIndex + 0) = pos.at(vertexIndex + 0) / radius;
            norm.at(normalIndex + 1) = pos.at(vertexIndex + 1) / radius;
            norm.at(normalIndex + 2) = pos.at(vertexIndex + 2) / radius;
            norm.at(normalIndex + 3) = 1.0f;

            texcoords.at(texCoordsIndex + 0) = (float)j / (float)numberSlices;
            texcoords.at(texCoordsIndex + 1) = (1.0f - (float)i) / (float)(numberParallels - 1);
        }
    }

    int* indexBuf = &indices[0];

    for (i = 0; i < numberParallels; i++)
    {
        for (j = 0; j < numberSlices; j++)
        {
            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);

            *indexBuf++ = i * (numberSlices + 1) + j;
            *indexBuf++ = (i + 1) * (numberSlices + 1) + (j + 1);
            *indexBuf++ = i * (numberSlices + 1) + (j + 1);

            int diff = int(indexBuf - &indices[0]);
            if (diff >= numberIndices)
                break;
        }
        int diff = int(indexBuf - &indices[0]);
        if (diff >= numberIndices)
            break;
    }

    GLuint vboVertices, vboIndices, vboNormals, vboTexCoords;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, pos.size() * sizeof(GLfloat), &pos[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, norm.size() * sizeof(GLfloat), &norm[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &vboTexCoords);
    glBindBuffer(GL_ARRAY_BUFFER, vboTexCoords);
    glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(GLfloat), &texcoords[0], GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCone(GLuint& vao, int numberSlices, float3 center, float height, float radius)
{
    float angleStep = 360 / numberSlices;

    std::vector<float> points = { center.x, center.y, center.z, 1.0f,
                                  center.x, center.y, center.z + height, 1.0f };

    std::vector<float> normals = { 0.0f, 0.0f, 1.0f, 1.0f,
                                   0.0f, 0.0f, 1.0f, 1.0f };
    std::vector<uint32_t> indices;

    for (int i = 0; i < numberSlices; i++) {
        points.push_back(center.x + radius * cos(angleStep * i * PI / 180));  // x
        points.push_back(center.y + radius * sin(angleStep * i * PI / 180));  // y
        points.push_back(center.z);                                       // z
        points.push_back(1.0f);

        normals.push_back(points.at(4 * i + 2) / numberSlices);
        normals.push_back(points.at(4 * i + 3) / numberSlices);
        normals.push_back(points.at(4 * i + 4) / numberSlices);
        normals.push_back(1.0f);

        // основание
        indices.push_back(0);
        indices.push_back(i + 2);
        if ((i + 1) == numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 3);
        }
        // шапка цилиндра
        indices.push_back(1);
        indices.push_back(i + 2);
        if ((i + 1) == numberSlices) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 3);
        }
    }

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateCylinder(GLuint& vao, int numberSlices, float3 center, float height, float radius)
{
    float angleStep = 360 / numberSlices;

    std::vector<float> points = { center.x, center.y, center.z, 1.0f,
                                  center.x, center.y + height, center.z, 1.0f };

    std::vector<float> normals = { 0.0f, 1.0f, 0.0f, 1.0f,
                                   0.0f, 1.0f, 0.0f, 1.0f };

    for (int i = 0; i < numberSlices; i++) {
        // нижняя точка
        points.push_back(center.x + radius * cos(angleStep * i * PI / 180));  // x
        points.push_back(center.y);                                           // y
        points.push_back(center.z + radius * sin(angleStep * i * PI / 180));  // z
        points.push_back(1.0f);

        // верхняя точка
        points.push_back(center.x + radius * cos(angleStep * i * PI / 180));  // x
        points.push_back(center.y + height);                                  // y
        points.push_back(center.z + radius * sin(angleStep * i * PI / 180));  // z
        points.push_back(1.0f);

        // нормали
        normals.push_back(points.at(8 * i + 2) / numberSlices);
        normals.push_back(points.at(8 * i + 3) / numberSlices);
        normals.push_back(points.at(8 * i + 4) / numberSlices);
        normals.push_back(1.0f);
        normals.push_back(points.at(8 * i + 6) / numberSlices);
        normals.push_back(points.at(8 * i + 7) / numberSlices);
        normals.push_back(points.at(8 * i + 8) / numberSlices);
        normals.push_back(1.0f);
    }

    std::vector<uint32_t> indices;

    for (int i = 0; i < numberSlices * 2; i += 2) {
        // нижнее основание
        indices.push_back(0);
        indices.push_back(i + 2);
        if ((i + 2) == numberSlices * 2) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 4);
        }

        // верхнее основание
        indices.push_back(1);
        indices.push_back(i + 3);
        if ((i + 2) == numberSlices * 2) {
            indices.push_back(3);
        }
        else {
            indices.push_back(i + 5);
        }

        indices.push_back(i + 2);
        indices.push_back(i + 3);
        if ((i + 2) == numberSlices * 2) {
            indices.push_back(2);
        }
        else {
            indices.push_back(i + 4);
        }

        indices.push_back(i + 3);
        if ((i + 2) == numberSlices * 2) {
            indices.push_back(2);
            indices.push_back(3);
        }
        else {
            indices.push_back(i + 4);
            indices.push_back(i + 5);
        }

    }

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreatePlane(GLuint& vao, float3 center, float length)
{
    std::vector<float> points = { center.x - length / 2, center.y, center.z - length / 2, 1.0f,
                                 center.x + length / 2, center.y, center.z - length / 2, 1.0f,
                                center.x - length / 2, center.y, center.z + length / 2, 1.0f,
                                center.x + length / 2, center.y, center.z + length / 2, 1.0f };

    std::vector<float> normals = { 0.0f, 1.0f, 0.0f, 1.0f,
                                   0.0f, 1.0f, 0.0f, 1.0f,
                                   0.0f, 1.0f, 0.0f, 1.0f,
                                   0.0f, 1.0f, 0.0f, 1.0f };

    std::vector<uint32_t> indices = { 0u, 1u, 2u,
                                      1u, 2u, 3u };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}

GLsizei CreateParallelepiped(GLuint& vao, float3 center, float lengthx, float lengthy, float lengthz)
{
    std::vector<float> points = { center.x - lengthx / 2, center.y - lengthy / 2, center.z - lengthz / 2, 1.0f,
                                center.x - lengthx / 2, center.y - lengthy / 2, center.z + lengthz / 2, 1.0f,
                                center.x + lengthx / 2, center.y - lengthy / 2, center.z + lengthz / 2, 1.0f,
                                center.x + lengthx / 2, center.y - lengthy / 2, center.z - lengthz / 2, 1.0f,
                                center.x - lengthx / 2, center.y + lengthy / 2, center.z - lengthz / 2, 1.0f,
                                center.x - lengthx / 2, center.y + lengthy / 2, center.z + lengthz / 2, 1.0f,
                                center.x + lengthx / 2, center.y + lengthy / 2, center.z + lengthz / 2, 1.0f,
                                center.x + lengthx / 2, center.y + lengthy / 2, center.z - lengthz / 2, 1.0f };

    std::vector<float> normals = { 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f,
                                 0.0f, 1.0f, 0.0f, 1.0f };

    std::vector<uint32_t> indices = { 0u, 1u, 2u,
                                      0u, 3u, 2u,
                                      3u, 2u, 6u,
                                      3u, 7u, 6u,
                                      0u, 3u, 7u,
                                      0u, 4u, 7u,
                                      1u, 2u, 6u,
                                      1u, 5u, 6u,
                                      4u, 7u, 6u,
                                      4u, 5u, 6u,
                                      0u, 1u, 5u,
                                      0u, 4u, 5u };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vao);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(GLfloat), points.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &vboNormals);
    glBindBuffer(GL_ARRAY_BUFFER, vboNormals);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(GLfloat), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    return indices.size();
}


unsigned int SCR_WIDTH = 1000;
unsigned int SCR_HEIGHT = 800;

static int filling = 0;
static bool keys[1024]; 
static bool g_captureMouse = true;  
static bool g_capturedMouseJustNow = false;

bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

Camera camera(glm::vec3(0.0f, 5.0f, 5.0f));
float lastX = 400;
float lastY = 300;
bool firstMouse = true;

float timer = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}


int main()
{
    // glfw: инициализация и конфигурирование
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 32); 

    glfwWindowHint(GLFW_DECORATED, NULL); // Remove the border and titlebar..   



#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw: создание окна
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH,
        SCR_HEIGHT, "Gaffarova Albina LR-3", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // Регистрируем коллбеки для обработки сообщений от пользователя - клавиатура, мышь..
    glfwSetKeyCallback(window, OnKeyboardPressed);
    glfwSetCursorPosCallback(window, OnMouseMove);
    glfwSetMouseButtonCallback(window, OnMouseButtonClicked);
    glfwSetScrollCallback(window, OnMouseScroll);
   
    // Сообщаем GLFW, чтобы он захватил наш курсор
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: загрузка всех указателей на OpenGL-функции
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Конфигурирование глобального состояния OpenGL
    glEnable(GL_DEPTH_TEST);

    // Компилирование шейдерной программы
    Shader shader("../shader.vs", "../shader.fs");
    Shader skyboxShader("../skybox.vs", "../skybox.fs");
    Shader shaderLight("../shader.vs", "../light.fs");

    // Загрузка текстур
    unsigned int woodTexture = loadTexture("../resources/textures/wood.png",true); 
    unsigned int snowTexture = loadTexture("../resources/textures/snow.jpg", true);
    unsigned int stoneTexture = loadTexture("../resources/textures/stone.jpg", true);
    unsigned int brickwallTexture = loadTexture("../resources/textures/brickwall.jpg", true);
    unsigned int carrotTexture = loadTexture("../resources/textures/carrot.png", true);
    unsigned int birdTexture = loadTexture("../resources/textures/bird.jpg", true);
    unsigned int noseTexture = loadTexture("../resources/textures/nose.jpg", true);
    unsigned int whiteTexture = loadTexture("../resources/textures/white.jpg", true);
    

   
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float cubeVertices[] = {
        // positions          // normals
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
    };
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    // skybox VAO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    // Текстуры для скайбокса
    vector<std::string> faces
    {
       "../posx.jpg", "../negx.jpg", "../posy.jpg", "../negy.jpg", "../posz.jpg", "../negz.jpg"
    };
    unsigned int cubemapTexture = loadCubemap(faces);

    // Источники освещения
    // Координаты
    std::vector<vec3> lightPositions;
    lightPositions.push_back(vec3(3.0f, 2.0f, -7.0f));
    lightPositions.push_back(vec3(-3.0f, 2.0f, -7.0f));
    lightPositions.push_back(vec3(3.0f, 2.0f, -13.0f));
    lightPositions.push_back(vec3(-3.0f, 2.0f, -13.0f));

    // Цвета
    std::vector<vec3> lightColors;
    lightColors.push_back(vec3(1.0f, 1.0f, 0.0f));
    lightColors.push_back(vec3(1.0f, 1.0f, 1.0f));
    lightColors.push_back(vec3(1.0f, 0.0f, 0.0f));
    lightColors.push_back(vec3(0.0f, 0.0f, 1.0f));
    

   // Конфигурация шейдеров
    shader.use();
    shader.setInt("material.diffuse", 0);
    shader.setInt("material.specular", 1);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    GLuint vaoSphereLight;
    GLsizei SphereLightIndices = CreateSphere(0.5f, 20, vaoSphereLight);


    GLuint vaoSphere;
    GLsizei sphereIndices = CreateSphere(1.0f, 64, vaoSphere);

    GLuint vaoCone;
    GLsizei coneIndices = CreateCone(vaoCone, 30, float3(0.0f, 2.25f, -9.55f), 0.4f, 0.1f);

    GLuint vaoCylinder;
    GLsizei cylinderIndices = CreateCylinder(vaoCylinder, 30, float3(0.0f, 2.6f, -10.0f), 0.5f, 0.3f);

    GLuint vaoPlane;
    GLsizei planeIndices = CreatePlane(vaoPlane, float3(0.0f, -0.8f, -10.0f), 10.0f);

    GLuint vaoParallelepiped;
    GLsizei parallelepipedIndices = CreateParallelepiped(vaoParallelepiped, float3(-4.5f, -0.3f, -10.0f), 1.0f, 1.0f, 10.0f);

    GLuint vaoGrass;
    GLsizei grassIndices = CreateCone(vaoGrass, 20, float3(0.0f, 0.0f, 0.0f), 1.0f, 0.1f);

    GLuint vaoBirdBody;
    GLsizei birdBodyIndices = CreateSphere(0.8f, 64, vaoBirdBody);

    GLuint vaoBirdNose;
    GLsizei birdNoseIndices = CreateCone(vaoBirdNose, 30, float3(0.0f, 0.0f, 0.0f), 0.15f, 0.05f);

    GLuint vaoBirdWing;
    GLsizei birdWingIndices = CreateSphere(0.6f, 64, vaoBirdWing);

    std::vector<std::vector<float>> rand_num;
    for (int i = 0; i < 33; i++)
    {
        std::vector<float> rand_n;
        for (int j = 0; j < 29; j++) {
            rand_n.push_back(0.1f * float(rand() % 10 + 1));
        }
        rand_num.push_back(rand_n);
    }


    int step = 0;
    // Цикл рендеринга
    while (!glfwWindowShouldClose(window))
    {
        // Считаем сколько времени прошло за кадр
        GLfloat currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        glfwPollEvents();
        doCameraMovement(camera, deltaTime);

        // Рендер
        glClearColor(0.4f, 0.25f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        
        // be sure to activate shader when setting uniforms/drawing objects
        shader.use();
        shader.setVec3("viewPos", camera.Position);
        shader.setFloat("material.shininess", 64.0f);

        // Отрисовываем сцену 
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 model = glm::mat4(1.0f);

        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("model", model);


        // directional light
        shader.setVec3("dirLight.direction", -5.0f, 0.0f, 0.0f);
        shader.setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
        shader.setVec3("dirLight.color", 1.0f, 1.0f, 0.0f);
        shader.setVec3("dirLight.specular", 1.0f, 1.0f, 1.0f);
       

        glActiveTexture(GL_TEXTURE0);

         //Устанавливаем uniform-переменные освещения
        for (unsigned int i = 0; i < lightPositions.size(); i++)
        {
            shader.setVec3("lights[" + std::to_string(i) + "].Position", lightPositions[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].Color", lightColors[i]);
            shader.setVec3("lights[" + std::to_string(i) + "].ambient", 0.5f, 0.5f, 0.5f);
            shader.setFloat("lights[" + std::to_string(i) + "].constant", 1.0f);
            shader.setFloat("lights[" + std::to_string(i) + "].linear", 0.09);
            shader.setFloat("lights[" + std::to_string(i) + "].quadratic", 0.032);
        }
        shader.setVec3("viewPos", camera.Position);

        

        glBindTexture(GL_TEXTURE_2D, snowTexture);
        glBindVertexArray(vaoSphere);
        {
            mat4 model1 = mat4(1.0f);
            model1 = translate(model1, vec3(0.0f, 0.0f, -10.0f));
            model1 = scale(model1, vec3(1.0f, 1.0f, 1.0f));
            shader.setMat4("model", model1); 
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr);

            mat4 model2 = mat4(1.0f);
            model2 = translate(model2, vec3(0.0f, 1.3f, -10.0f));
            model2 = scale(model2, vec3(0.8f, 0.8f, 0.8f));
            shader.setMat4("model", model2);
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr);

            mat4 model3 = mat4(1.0f);
            model3 = translate(model3, vec3(0.0f, 2.25f, -10.0f));
            model3 = scale(model3, vec3(0.5f, 0.5f, 0.5f));
            shader.setMat4("model", model3); 
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, stoneTexture);
        glBindVertexArray(vaoSphere);
        {
            mat4 model4 = mat4(1.0f);
            model4 = translate(model4, vec3(-0.15f, 2.35f, -9.56f));
            model4 = scale(model4, vec3(0.05f, 0.05f, 0.05f));
            shader.setMat4("model", model4);
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr); 

            mat4 model5 = mat4(1.0f);
            model5 = translate(model5, vec3(0.15f, 2.35f, -9.56f));
            model5 = scale(model5, vec3(0.05f, 0.05f, 0.05f));
            shader.setMat4("model", model5);
            glDrawElements(GL_TRIANGLE_STRIP, sphereIndices, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, carrotTexture);
        glBindVertexArray(vaoCone); 
        {
            glm::mat4 model = mat4(1.0f);
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLE_STRIP, coneIndices, GL_UNSIGNED_INT, nullptr); 

        }
        glBindVertexArray(0); 

        glBindTexture(GL_TEXTURE_2D, woodTexture);
        glBindVertexArray(vaoCylinder); 
        {
            glm::mat4 model = mat4(1.0f);
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLE_STRIP, cylinderIndices, GL_UNSIGNED_INT, nullptr);

        }
        glBindVertexArray(0); 

        glBindTexture(GL_TEXTURE_2D, snowTexture);
        glBindVertexArray(vaoPlane); 
        {
            glm::mat4 model = mat4(1.0f);
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLE_STRIP, planeIndices, GL_UNSIGNED_INT, nullptr); 

        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, brickwallTexture);
        glBindVertexArray(vaoParallelepiped); 
        {
            glm::mat4 model1 = mat4(1.0f);
            shader.setMat4("model", model1);
            glDrawElements(GL_TRIANGLE_STRIP, parallelepipedIndices, GL_UNSIGNED_INT, nullptr); 

            glm::mat4 model2 = mat4(1.0f);
            model2 = translate(model2, vec3(+9.0f, 0.0f, 0.0f));
            shader.setMat4("model", model2);
            glDrawElements(GL_TRIANGLE_STRIP, parallelepipedIndices, GL_UNSIGNED_INT, nullptr); 
        }
        glBindVertexArray(0); 

        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        glBindVertexArray(vaoGrass);
        {
            glm::mat4 model = mat4(1.0f), model_sample = mat4(1.0f);
            model_sample = rotate(translate(model_sample, vec3(-3.5f, -0.8f, -14.0f)), 270 * LiteMath::DEG_TO_RAD, vec3(1.0f, 0.0f, 0.0f));
            model = model_sample;
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLE_STRIP, grassIndices, GL_UNSIGNED_INT, nullptr);

            for (int i = 0; i < 33; i++)
            {
                for (int j = 0; j < 29; j++) {
                    model = translate(model_sample, vec3(+0.25f * float(j), -0.25f * float(i), 0.0f));
                    model = scale(model, vec3(1.0f, 1.0f, rand_num[i][j]));
                    shader.setMat4("model", model);
                    glDrawElements(GL_TRIANGLE_STRIP, grassIndices, GL_UNSIGNED_INT, nullptr);
                }
            }
        }
        glBindVertexArray(0);
        
        glBindTexture(GL_TEXTURE_2D, birdTexture);
        glBindVertexArray(vaoBirdBody); 
        {
            mat4 modelBody = mat4(1.0f);
            modelBody = translate(modelBody, vec3(+4.0f * cos(step * PI / 180), +4.0f, -10.0f + 4.0f * sin(step * PI / 180)));
            modelBody = rotate(modelBody, (360 - step) * LiteMath::DEG_TO_RAD, vec3(0.0f, 1.0f, 0.0f));
            modelBody = scale(modelBody, vec3(0.2f, 0.2f, 0.4f));
            shader.setMat4("model", modelBody);
            glDrawElements(GL_TRIANGLE_STRIP, birdBodyIndices, GL_UNSIGNED_INT, nullptr); 

        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, noseTexture);
        glBindVertexArray(vaoBirdNose);
        {
            mat4 model = mat4(1.0f);
            model = translate(model, vec3(+4.0f * cos((step + 4.5) * PI / 180), +4.0f, -10.0f + 4.0f * sin((step + 4.5) * PI / 180)));
            model = rotate(model, (360 - step) * LiteMath::DEG_TO_RAD, vec3(0.0f, 1.0f, 0.0f));
            shader.setMat4("model", model);
            glDrawElements(GL_TRIANGLE_STRIP, birdNoseIndices, GL_UNSIGNED_INT, nullptr);
        }
        glBindVertexArray(0);

        glBindTexture(GL_TEXTURE_2D, birdTexture);
        glBindVertexArray(vaoBirdWing); 
        {
            glm::mat4 model1 = mat4(1.0f);
            model1 = translate(model1, vec3(+4.16f * cos(step * PI / 180), +4.0f, -10.0f + 4.16f * sin(step * PI / 180)));
            model1 = rotate(model1, (360 - step) * LiteMath::DEG_TO_RAD, vec3(0.0f, 1.0f, 0.0f));
            model1 = rotate(model1, (20 * step % 180) * LiteMath::DEG_TO_RAD, vec3(0.0f, 0.0f, 1.0f));
            model1 = scale(model1, vec3(0.3f, 0.02f, 0.2f));
            shader.setMat4("model", model1); 
            glDrawElements(GL_TRIANGLE_STRIP, birdWingIndices, GL_UNSIGNED_INT, nullptr); 

            glm::mat4 model2 = mat4(1.0f);
            model2 = translate(model2, vec3(+3.84f * cos(step * PI / 180), +4.0f, -10.0f + 3.84f * sin(step * PI / 180)));
            model2 = rotate(model2, (360 - step) * LiteMath::DEG_TO_RAD, vec3(0.0f, 1.0f, 0.0f));
            model2 = rotate(model2, (20 * step % 180) * LiteMath::DEG_TO_RAD, vec3(0.0f, 0.0f, 1.0f));
            model2 = scale(model2, vec3(0.3f, 0.02f, 0.2f));
            shader.setMat4("model", model2);
            glDrawElements(GL_TRIANGLE_STRIP, birdWingIndices, GL_UNSIGNED_INT, nullptr); 
        }
        glBindVertexArray(0); 
        
        glBindTexture(GL_TEXTURE_2D, woodTexture);
        model = mat4(1.0f);
        model = translate(model, glm::vec3(4.6f, 1.0f, -10.0f));
        model = scale(model, glm::vec3(0.5f));
        shader.setMat4("model", model);
        renderCube();

        // Показываем все источники света в виде ярких фигур
        shaderLight.use();
        shaderLight.setMat4("projection", projection);
        shaderLight.setMat4("view", view);
        glBindVertexArray(vaoSphereLight);
        {
            mat4 model = glm::mat4(1.0f);
            model = translate(model, vec3(3.0f, 2.0f, -7.0f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[0]);
            glDrawElements(GL_TRIANGLE_STRIP, SphereLightIndices, GL_UNSIGNED_INT, nullptr);

            model = glm::mat4(1.0f);
            model = translate(model, vec3(-3.0f, 2.0f, -7.0f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[1]);
            glDrawElements(GL_TRIANGLE_STRIP, SphereLightIndices, GL_UNSIGNED_INT, nullptr);

            model = glm::mat4(1.0f);
            model = translate(model, vec3(3.0f, 2.0f, -13.0f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[2]);
            glDrawElements(GL_TRIANGLE_STRIP, SphereLightIndices, GL_UNSIGNED_INT, nullptr);

            model = glm::mat4(1.0f);
            model = translate(model, vec3(-3.0f, 2.0f, -13.0f));
            shaderLight.setMat4("model", model);
            shaderLight.setVec3("lightColor", lightColors[3]);
            glDrawElements(GL_TRIANGLE_STRIP, SphereLightIndices, GL_UNSIGNED_INT, nullptr);

            
        }
        glBindVertexArray(0);


        // Отрисовываем скайбокс последним
        glDepthFunc(GL_LEQUAL); // меняем функцию глубины, чтобы обеспечить прохождение теста глубины, когда значения равны содержимому буфера глубины
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // убираем из матрицы вида секцию, отвечающую за операцию трансляции
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // Куб скайбокса
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // восстанавливаем стандартное значение функции теста глубины

     
        step++;
        // glfw: обмен содержимым front- и back- буферов. Отслеживание событий ввода/вывода (была ли нажата/отпущена кнопка, перемещен курсор мыши и т.п.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &skyboxVAO);
    glfwTerminate();
    return 0;
}

unsigned int vaoPlane;
void renderPlane()
{
    
    std::vector<float> vertices = { 
     // координаты      // цвета            // текстурные координаты
     0.5f, 0.0f, 0.5f,  1.0f, 0.0f, 0.0f,   1.0f, 0.0f,  
     0.5f, 0.0f,-0.5f,  0.0f, 1.0f, 0.0f,   1.0f, 1.0f,   
    -0.5f, 0.0f,-0.5f,  0.0f, 0.0f, 1.0f,   0.0f, 1.0f,   
    -0.5f, 0.0f, 0.5f,  1.0f, 1.0f, 0.0f,   0.0f, 0.0f
    };

    std::vector<float> normals = { 0.0f, 0.0f, -1.0f, 1.0f,
                                   0.0f, 0.0f, -1.0f, 1.0f,
                                   0.0f, 0.0f, -1.0f, 1.0f,
                                   0.0f, 0.0f, -1.0f, 1.0f };

    std::vector<uint32_t> indices = { 0u, 1u, 2u,
                                      0u, 3u, 2u };

    GLuint vboVertices, vboIndices, vboNormals;

    glGenVertexArrays(1, &vaoPlane);
    glGenBuffers(1, &vboIndices);

    glBindVertexArray(vaoPlane);

    glGenBuffers(1, &vboVertices);
    glBindBuffer(GL_ARRAY_BUFFER, vboVertices);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // Координатные атрибуты
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Цветовые атрибуты
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Атрибуты текстурных координат
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndices);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    // Рендер плоскости
    glBindVertexArray(vaoPlane);
    glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}


unsigned int cubeVAO;
unsigned int cubeVBO;
void renderCube()
{
    float vertices[] = {
        // задняя грань
       -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая
        1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // нижняя-правая         
        1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // верхняя-правая
       -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // нижняя-левая
       -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // верхняя-левая

        // передняя грань
       -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // нижняя-левая
        1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // нижняя-правая
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // верхняя-правая
        1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // верхняя-правая
       -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // верхняя-левая
       -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // нижняя-левая

        // грань слева
       -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-правая
       -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // верхняя-левая
       -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-левая
       -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-левая
       -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // нижняя-правая
       -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-правая

        // грань справа
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая
        1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая         
        1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // нижняя-правая
        1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // верхняя-левая
        1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // нижняя-левая     

        // нижняя грань
       -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая
        1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // верхняя-левая
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
        1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // нижняя-левая
       -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // нижняя-правая
       -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // верхняя-правая

        // верхняя грань
       -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // верхняя-левая
        1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая
        1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // верхняя-правая     
        1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // нижняя-правая
       -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // верхняя-левая
       -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // нижняя-левая        
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    // Заполняем буфер
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Связываем вершинные атрибуты
    glBindVertexArray(cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
	
    // Рендер куба
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    
}

// renderQuad() рендерит 1x1 XY-прямоугольник в NDC
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
             // координаты      // текстурные коодинаты
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
		
        // Установка VAO плоскости
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

//функция для обработки нажатий на кнопки клавиатуры
void OnKeyboardPressed(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    switch (key)
    {
    case GLFW_KEY_ESCAPE: //на Esc выходим из программы
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GL_TRUE);
        break;
    case GLFW_KEY_SPACE: 
        if (action == GLFW_PRESS)
        {
            bloom = !bloom;
            bloomKeyPressed = true;
        }
        break;
    case GLFW_KEY_1:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        break;
    case GLFW_KEY_2:
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        break;
    default:
        if (action == GLFW_PRESS)
            keys[key] = true;
        else if (action == GLFW_RELEASE)
            keys[key] = false;
    }
}

//функция для обработки клавиш мыши
void OnMouseButtonClicked(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
        g_captureMouse = !g_captureMouse;


    if (g_captureMouse)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        g_capturedMouseJustNow = true;
    }
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

//функция для обработки перемещения мыши
void OnMouseMove(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = float(xpos);
        lastY = float(ypos);
        firstMouse = false;
    }

    GLfloat xoffset = float(xpos) - lastX;
    GLfloat yoffset = lastY - float(ypos);

    lastX = float(xpos);
    lastY = float(ypos);

    if (g_captureMouse)
        camera.ProcessMouseMovement(xoffset, yoffset);
}


void OnMouseScroll(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(GLfloat(yoffset));
}

void doCameraMovement(Camera& camera, GLfloat deltaTime)
{
    if (keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

// Вспомогательная функция загрузки 2D-текстур из файла
unsigned int loadTexture(char const * path, bool gammaCorrection)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum internalFormat;
        GLenum dataFormat;
        if (nrComponents == 1)
        {
            internalFormat = dataFormat = GL_RED;
        }
        else if (nrComponents == 3)
        {
            internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrComponents == 4)
        {
            internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

