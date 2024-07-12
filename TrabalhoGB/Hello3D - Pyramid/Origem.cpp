// Aluno: Diogo Garbinato de Fagundes Matrícula: 1189650

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <fstream>
#include <sstream>
#include "json.hpp" // Inclua a biblioteca JSON baixada

using json = nlohmann::json;
using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "stb_image.h"
#include "Shader.h"
#include "Mesh.h"

// Estrutura para armazenar um material
struct Material {
    std::string name;
    float Ns; // Exponent for specular highlight
    float Ka[3]; // Ambient reflectivity
    float Kd[3]; // Diffuse reflectivity
    float Ks[3]; // Specular reflectivity
    float Ke[3]; // Emissive coefficient
    float Ni; // Optical density (index of refraction)
    float d; // Dissolve (transparency)
    int illum; // Illumination model
    std::string textureFile; // File name of the texture
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

// Protótipos das funções
int loadTexture(string path);
int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color = glm::vec3(1.0, 0.0, 1.0));
int loadMTL(string filepath, std::vector<Material>& materials);
json readJSONConfig(const std::string& filepath);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 800;

glm::vec3 cameraPos = glm::vec3(0.0, 0.0, 3.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);

bool firstMouse = true;
float lastX, lastY;
float sensitivity = 0.05;
float pitch = 0.0, yaw = -90.0;
string base_path = "./Objetos/";
std::vector<Material> materials;
Light light; // Apenas uma fonte de luz

// Função MAIN
int main()
{
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
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

    // Ler arquivo de configuração
    json config = readJSONConfig("scene_config.json");

    // Definir a posição e orientação da câmera
    cameraPos = glm::vec3(config["camera"]["position"][0], config["camera"]["position"][1], config["camera"]["position"][2]);
    cameraFront = glm::vec3(config["camera"]["front"][0], config["camera"]["front"][1], config["camera"]["front"][2]);
    cameraUp = glm::vec3(config["camera"]["up"][0], config["camera"]["up"][1], config["camera"]["up"][2]);

    // Definir o frustrum da câmera
    float fov = config["camera"]["frustum"][0];
    float nearPlane = config["camera"]["frustum"][1];
    float farPlane = config["camera"]["frustum"][2];
    glm::mat4 projection = glm::perspective(glm::radians(fov), (float)width / (float)height, nearPlane, farPlane);
    shader.setMat4("projection", glm::value_ptr(projection));

    glEnable(GL_DEPTH_TEST);

    // Carregar e inicializar objetos a partir do arquivo de configuração
    std::vector<Mesh> objects;
    std::vector<GLuint> textures;
    for (const auto& obj : config["objects"]) {
        int nVerts;
        GLuint VAO = loadSimpleOBJ(base_path + obj["file"].get<std::string>(), nVerts);
        glm::vec3 position = glm::vec3(obj["position"][0], obj["position"][1], obj["position"][2]);
        glm::vec3 scale = glm::vec3(obj["scale"][0], obj["scale"][1], obj["scale"][2]);
        float rotation = obj["rotation"];
        glm::vec3 axis = glm::vec3(obj["axis"][0], obj["axis"][1], obj["axis"][2]);

        Mesh object;
        object.initialize(VAO, nVerts, &shader, position, scale, rotation, axis);
        objects.push_back(object);

        std::string textureFile = base_path + materials[objects.size() - 1].textureFile;
        GLuint textureID = loadTexture(textureFile);
        textures.push_back(textureID);
    }

    // Carregar e inicializar a luz a partir do arquivo de configuração
    light.position = glm::vec3(config["light"]["position"][0], config["light"]["position"][1], config["light"]["position"][2]);
    light.color = glm::vec3(config["light"]["color"][0], config["light"]["color"][1], config["light"]["color"][2]);

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
        glfwPollEvents();

        // Obtendo a cor de fundo do comentário
        float red = 12.0f / 255.0f;
        float green = 12.0f / 255.0f;
        float blue = 12.0f / 255.0f;

        glClearColor(red, green, blue, 1.0f); // cor de fundo rgb(18, 18, 18)

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        // Atualizando a posição e orientação da câmera
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("view", glm::value_ptr(view));

        // Atualizando o shader com a posição da câmera
        shader.setVec3("cameraPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Configurar luz
        shader.setVec3("lightPos", light.position.x, light.position.y, light.position.z);
        shader.setVec3("lightColor", light.color.x, light.color.y, light.color.z);

        // Desenhar todos os objetos
        for (size_t i = 0; i < objects.size(); ++i) {
            shader.setFloat("ka", materials[i].Ka[0]);
            shader.setFloat("kd", materials[i].Kd[0]);
            shader.setFloat("ks", materials[i].Ks[0]);
            shader.setFloat("ns", std::max(materials[i].Ns, 0.0f));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[i]);
            shader.setInt("texture1", 0);

            objects[i].update();
            objects[i].draw();
        }

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    for (auto& obj : objects) {
        glDeleteVertexArrays(1, &obj.VAO);
    }
    // Finaliza a execução da GLFW, limpando os recursos alocados por ela
    glfwTerminate();
    return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    float cameraSpeed = 0.05;

    if (key == GLFW_KEY_W)
    {
        cameraPos += cameraFront * cameraSpeed;
    }
    if (key == GLFW_KEY_S)
    {
        cameraPos -= cameraFront * cameraSpeed;
    }
    if (key == GLFW_KEY_A)
    {
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
    if (key == GLFW_KEY_D)
    {
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float offsetx = xpos - lastX;
    float offsety = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    offsetx *= sensitivity;
    offsety *= sensitivity;

    pitch += offsety;
    yaw += offsetx;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

int loadTexture(string path)
{
    GLuint texID;

    // Gera o identificador da textura na memória 
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    // Ajusta os parâmetros de wrapping e filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Carregamento da imagem
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3) // jpg, bmp
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else // png
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}

// Carregar modelo OBJ simples
int loadSimpleOBJ(string filepath, int& nVerts, glm::vec3 color)
{
    vector<glm::vec3> vertices;
    vector<GLuint> indices;
    vector<glm::vec2> texCoords;
    vector<glm::vec3> normals;
    vector<GLfloat> vbuffer;

    ifstream inputFile;
    inputFile.open(filepath.c_str());
    if (inputFile.is_open())
    {
        char line[100];
        string sline;

        while (!inputFile.eof())
        {
            inputFile.getline(line, 100);
            sline = line;

            string word;

            istringstream ssline(line);
            ssline >> word;

            if (word == "v")
            {
                glm::vec3 v;
                ssline >> v.x >> v.y >> v.z;
                vertices.push_back(v);
            }
            if (word == "vt")
            {
                glm::vec2 vt;
                ssline >> vt.s >> vt.t;
                texCoords.push_back(vt);
            }
            if (word == "vn")
            {
                glm::vec3 vn;
                ssline >> vn.x >> vn.y >> vn.z;
                normals.push_back(vn);
            }
            if (word == "f")
            {
                string tokens[3];

                ssline >> tokens[0] >> tokens[1] >> tokens[2];

                for (int i = 0; i < 3; i++)
                {
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
            if (word == "mtllib")
            {
                std::string mtlFilename;
                ssline >> mtlFilename;
                loadMTL(base_path + mtlFilename, materials);
            }
        }
    }
    else
    {
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

int loadMTL(string filepath, std::vector<Material>& materials)
{
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo " << filepath << std::endl;
        return false;
    }

    std::string line;
    Material currentMaterial;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string word;
        ss >> word;

        if (word == "newmtl") {
            if (!currentMaterial.name.empty()) {
                materials.push_back(currentMaterial);
            }
            currentMaterial = Material(); // Reset the current material
            ss >> currentMaterial.name;
        }
        else if (word == "Ns") {
            ss >> currentMaterial.Ns;
        }
        else if (word == "Ka") {
            ss >> currentMaterial.Ka[0] >> currentMaterial.Ka[1] >> currentMaterial.Ka[2];
        }
        else if (word == "Kd") {
            ss >> currentMaterial.Kd[0] >> currentMaterial.Kd[1] >> currentMaterial.Kd[2];
        }
        else if (word == "Ks") {
            ss >> currentMaterial.Ks[0] >> currentMaterial.Ks[1] >> currentMaterial.Ks[2];
        }
        else if (word == "Ke") {
            ss >> currentMaterial.Ke[0] >> currentMaterial.Ke[1] >> currentMaterial.Ke[2];
        }
        else if (word == "Ni") {
            ss >> currentMaterial.Ni;
        }
        else if (word == "d") {
            ss >> currentMaterial.d;
        }
        else if (word == "illum") {
            ss >> currentMaterial.illum;
        }
        else if (word == "map_Kd") {
            ss >> currentMaterial.textureFile;
        }
    }

    materials.push_back(currentMaterial);

    file.close();
    return 0;
}

json readJSONConfig(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo de configuração " << filepath << std::endl;
        return json();
    }

    json config;
    file >> config;
    return config;
}
