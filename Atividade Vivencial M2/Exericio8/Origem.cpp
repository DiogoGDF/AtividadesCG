// Aluno: Diogo Garbinato de Fagundes       Matrícula: 1189650
#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Mesh and Shader classes
#include "Mesh.h"
#include "Shader.h"

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Estrutura para armazenar um vértice
struct Vertex {
    float x, y, z;
};

// Estrutura para armazenar coordenadas de textura
struct TexCoord {
    float s, t;
};

// Estrutura para armazenar um vetor normal
struct Normal {
    float x, y, z;
};

// Estrutura para armazenar uma face (triângulo)
struct Face {
    int v[3];    // Índices dos vértices
    int vt[3];   // Índices das coordenadas de textura
    int vn[3];   // Índices dos vetores normais
};

bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<TexCoord>& texCoords, std::vector<Normal>& normals, std::vector<Face>& faces);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 800;

Mesh setupMesh(const std::vector<Vertex>& vertices, const std::vector<Face>& faces, Shader* shader) {
    std::vector<GLfloat> vertexData;
    std::vector<GLfloat> colorData;

    // Cores para as faces do cubo
    std::vector<glm::vec3> faceColors = {
        {1.0f, 0.0f, 0.0f}, // Vermelho
        {0.0f, 1.0f, 0.0f}, // Verde
        {0.0f, 0.0f, 1.0f}, // Azul
        {1.0f, 1.0f, 0.0f}, // Amarelo
        {0.0f, 1.0f, 1.0f}, // Ciano
        {1.0f, 0.0f, 1.0f}  // Magenta
    };

    int faceIndex = 0;
    for (const auto& face : faces) {
        glm::vec3 color = faceColors[faceIndex % faceColors.size()];
        for (int i = 0; i < 3; ++i) {
            const Vertex& vertex = vertices[face.v[i] - 1];
            vertexData.push_back(vertex.x);
            vertexData.push_back(vertex.y);
            vertexData.push_back(vertex.z);

            colorData.push_back(color.r);
            colorData.push_back(color.g);
            colorData.push_back(color.b);
        }
        faceIndex++;
    }

    GLuint VAO, VBO[2];
    glGenVertexArrays(1, &VAO);
    glGenBuffers(2, VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(GLfloat), vertexData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, colorData.size() * sizeof(GLfloat), colorData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    Mesh mesh;
    mesh.initialize(VAO, vertexData.size() / 3, shader);
    return mesh;
}

// Variáveis globais para armazenar as transformações
bool rotateX = false, rotateY = false, rotateZ = false;
float transX = 0.0f, transY = 0.0f, transZ = 0.0f;
float scale = 1.0f;
glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f); // Inicialmente, o eixo Z é o padrão

// Função MAIN
int main()
{
    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::string filename = "../../3D_Models/Cube/cube.obj";

    if (!loadOBJ(filename, vertices, texCoords, normals, faces)) {
        std::cerr << "Falha ao carregar o arquivo OBJ." << std::endl;
        return -1;
    }

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Diogo!", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Shader shader("./shaders.vs", "./shaders.fs");
    Mesh cubeMesh = setupMesh(vertices, faces, &shader);

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.Use();
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        shader.setMat4("view", glm::value_ptr(view));

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        shader.setMat4("projection", glm::value_ptr(projection));

        // Atualiza as transformações
        cubeMesh.position = glm::vec3(transX, transY, transZ);
        cubeMesh.scale = glm::vec3(scale, scale, scale);
        if (rotateX)
            cubeMesh.axis = glm::vec3(1.0f, 0.0f, 0.0f);
        if (rotateY)
            cubeMesh.axis = glm::vec3(0.0f, 1.0f, 0.0f);
        if (rotateZ)
            cubeMesh.axis = glm::vec3(0.0f, 0.0f, 1.0f);
        if (rotateX || rotateY || rotateZ)
            cubeMesh.angle += 0.2f; // Adiciona uma rotação contínua apenas se uma das teclas estiver pressionada

        cubeMesh.update();
        cubeMesh.draw();

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return -1;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        rotateX = !rotateX;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        rotateY = !rotateY;
        rotateX = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT)) {
        rotateZ = !rotateZ;
        rotateX = false;
        rotateY = false;
    }

    // Controles para mover o cubo nos eixos x, y e z
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transY += 0.01f;

    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transY -= 0.01f;

    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transX -= 0.01f;

    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transX += 0.01f;

    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transZ += 0.01f;

    if (key == GLFW_KEY_J && (action == GLFW_PRESS || action == GLFW_REPEAT))
        transZ -= 0.01f;

    // Controles para escalar o cubo
    if (key == GLFW_KEY_LEFT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT))
        scale -= 0.1f;

    if (key == GLFW_KEY_RIGHT_BRACKET && (action == GLFW_PRESS || action == GLFW_REPEAT))
        scale += 0.1f;
}

// Função para carregar um arquivo OBJ
bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<TexCoord>& texCoords, std::vector<Normal>& normals, std::vector<Face>& faces) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo " << filename << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string word;
        ss >> word;

        if (word == "v") {
            // Leitura dos vértices
            Vertex vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (word == "vt") {
            // Leitura das coordenadas de textura
            TexCoord texCoord;
            ss >> texCoord.s >> texCoord.t;
            texCoords.push_back(texCoord);
        }
        else if (word == "vn") {
            // Leitura dos vetores normais
            Normal normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (word == "f") {
            // Leitura das faces
            std::string tokens[3];
            ss >> tokens[0] >> tokens[1] >> tokens[2];

            Face face;
            for (int i = 0; i < 3; i++) {
                int pos = tokens[i].find("/");
                std::string token = tokens[i].substr(0, pos);
                face.v[i] = std::stoi(token);

                tokens[i] = tokens[i].substr(pos + 1);
                pos = tokens[i].find("/");
                token = tokens[i].substr(0, pos);
                face.vt[i] = std::stoi(token);

                tokens[i] = tokens[i].substr(pos + 1);
                face.vn[i] = std::stoi(tokens[i]);
            }
            faces.push_back(face);
        }
    }

    file.close();
    return true;
}
