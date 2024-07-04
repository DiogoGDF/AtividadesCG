#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Mesh.h"
#include "Camera.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void addControlPoint(glm::vec3 point);
void saveControlPointsToFile(const string& filename);
void loadControlPointsFromFile(const string& filename);
GLuint generateControlPointsBuffer(const vector<glm::vec3>& controlPoints);

// Protótipos das funções
int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color = glm::vec3(1.0, 0.0, 1.0));

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

bool rotateX = false, rotateY = false, rotateZ = false;
vector<glm::vec3> controlPoints;
int currentControlPointIndex = 0;
float moveSpeed = 0.01f;

// Definição das variáveis globais
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, 0.0f);
bool firstMouse = true;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;

// Função para adicionar ponto de controle
void addControlPoint(glm::vec3 point) {
    controlPoints.push_back(point);
}

// Função para salvar pontos de controle em um arquivo
void saveControlPointsToFile(const string& filename) {
    ofstream outFile(filename);
    for (const auto& point : controlPoints) {
        outFile << point.x << " " << point.y << " " << point.z << endl;
    }
}

// Função para carregar pontos de controle de um arquivo
void loadControlPointsFromFile(const string& filename) {
    ifstream inFile(filename);
    controlPoints.clear();
    glm::vec3 point;
    while (inFile >> point.x >> point.y >> point.z) {
        controlPoints.push_back(point);
    }
}

// Função para gerar um VAO para os pontos de controle
GLuint generateControlPointsBuffer(const vector<glm::vec3>& controlPoints) {
    GLuint VBO, VAO;

    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);

    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, controlPoints.size() * sizeof(GLfloat) * 3, controlPoints.data(), GL_STATIC_DRAW);

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos 
    glBindVertexArray(VAO);

    // Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice 
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

int main() {
    // Inicialização da GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetCursorPos(window, WIDTH / 2, HEIGHT / 2);

    // Desabilita o desenho do cursor 
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    Shader shader("Phong.vs", "Phong.fs");
    glUseProgram(shader.ID);

    // Matriz de projeção perspectiva - definindo o volume de visualização (frustum)
    glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()), (float)width / (float)height, 0.1f, 100.0f);
    shader.setMat4("projection", glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    int nVerts;
    GLuint VAO = loadSimpleOBJ("../../3D_Models/Naves/Destroyer05.obj", nVerts, glm::vec3(1.0, 1.0, 1.0));

    Mesh destroyer;
    destroyer.initialize(VAO, nVerts, &shader, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.02, 0.02, 0.02));

    // Definindo as propriedades do material da superfície
    shader.setFloat("ka", 0.2);
    shader.setFloat("kd", 0.5);
    shader.setFloat("ks", 0.5);
    shader.setFloat("q", 10.0);

    // Definindo a fonte de luz pontual
    shader.setVec3("lightPos", -2.0, 10.0, 2.0);
    shader.setVec3("lightColor", 1.0, 1.0, 0.0);

    // Inicializando os pontos de controle
    loadControlPointsFromFile("control_points.txt");

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window)) {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Limpa o buffer de cor
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); // cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        // Atualizando a posição do objeto de acordo com os pontos de controle
        if (!controlPoints.empty()) {
            glm::vec3 currentPos = destroyer.getPosition();
            glm::vec3 targetPos = controlPoints[currentControlPointIndex];
            glm::vec3 direction = glm::normalize(targetPos - currentPos);
            destroyer.setPosition(currentPos + direction * moveSpeed);

            if (glm::distance(destroyer.getPosition(), targetPos) < moveSpeed) {
                currentControlPointIndex = (currentControlPointIndex + 1) % controlPoints.size();
            }
        }

        // Atualizando a matriz de view
        glm::mat4 view = camera.GetViewMatrix();
        shader.setMat4("view", glm::value_ptr(view));

        // Atualizando o shader com a posição da câmera
        shader.setVec3("cameraPos", camera.GetPosition().x, camera.GetPosition().y, camera.GetPosition().z);

        // Chamada de desenho - drawcall
        destroyer.update();
        destroyer.draw();

        // Desenha pontos de controle
        GLuint controlPointsVAO = generateControlPointsBuffer(controlPoints);
        glBindVertexArray(controlPointsVAO);
        shader.setVec4("finalColor", 1.0, 0.0, 0.0, 1.0); // Cor vermelha para pontos de controle
        glDrawArrays(GL_POINTS, 0, controlPoints.size());
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &VAO);
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        glm::vec3 newPoint = camera.GetPosition() + camera.GetFront() * 2.0f;
        addControlPoint(newPoint);
        saveControlPointsToFile("control_points.txt");
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float offsetx = xpos - lastX;
    float offsety = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(offsetx, offsety);
}

// Carregar modelo OBJ simples
int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color) {
    vector <glm::vec3> vertices;
    vector <GLuint> indices;
    vector <glm::vec2> texCoords;
    vector <glm::vec3> normals;
    vector <GLfloat> vbuffer;

    ifstream inputFile;
    inputFile.open(filepath.c_str());
    if (inputFile.is_open()) {
        char line[100];
        string sline;

        while (!inputFile.eof()) {
            inputFile.getline(line, 100);
            sline = line;

            string word;

            istringstream ssline(line);
            ssline >> word;

            if (word == "v") {
                glm::vec3 v;
                ssline >> v.x >> v.y >> v.z;
                vertices.push_back(v);
            }
            if (word == "vt") {
                glm::vec2 vt;
                ssline >> vt.s >> vt.t;
                texCoords.push_back(vt);
            }
            if (word == "vn") {
                glm::vec3 vn;
                ssline >> vn.x >> vn.y >> vn.z;
                normals.push_back(vn);
            }
            if (word == "f") {
                string tokens[3];

                ssline >> tokens[0] >> tokens[1] >> tokens[2];

                for (int i = 0; i < 3; i++) {
                    int pos = tokens[i].find("/");
                    string token = tokens[i].substr(0, pos);
                    int index = atoi(token.c_str()) - 1;
                    indices.push_back(index);

                    vbuffer.push_back(vertices[index].x);
                    vbuffer.push_back(vertices[index].y);
                    vbuffer.push_back(vertices[index].z);
                    vbuffer.push_back(color.r);
                    vbuffer.push_back(color.g);
                    vbuffer.push_back(color.b);

                    tokens[i] = tokens[i].substr(pos + 1);
                    pos = tokens[i].find("/");
                    token = tokens[i].substr(0, pos);
                    index = atoi(token.c_str()) - 1;

                    vbuffer.push_back(texCoords[index].s);
                    vbuffer.push_back(texCoords[index].t);

                    tokens[i] = tokens[i].substr(pos + 1);
                    index = atoi(tokens[i].c_str()) - 1;

                    vbuffer.push_back(normals[index].x);
                    vbuffer.push_back(normals[index].y);
                    vbuffer.push_back(normals[index].z);
                }
            }
        }
    }
    else {
        cout << "Problema ao encontrar o arquivo " << filepath << endl;
    }
    inputFile.close();

    GLuint VBO, VAO;

    nVerts = vbuffer.size() / 11;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vbuffer.size() * sizeof(GLfloat), vbuffer.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

