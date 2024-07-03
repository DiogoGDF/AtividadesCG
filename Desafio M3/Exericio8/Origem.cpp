// Aluno: Diogo Garbinato de Fagundes       Matrícula: 1189650
#include <iostream>
#include <string>
#include <assert.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Estrutura para armazenar um vértice
struct Vertex {
    float x, y, z; // Coordenadas do vértice
    float s, t; // Coordenadas de textura
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

// Estrutura para armazenar um material
struct Material {
    std::string name;
    float Ns = 0.0f; // Exponent for specular highlight
    float Ka[3] = { 0.0f, 0.0f, 0.0f }; // Ambient reflectivity
    float Kd[3] = { 0.0f, 0.0f, 0.0f }; // Diffuse reflectivity
    float Ks[3] = { 0.0f, 0.0f, 0.0f }; // Specular reflectivity
    float Ke[3] = { 0.0f, 0.0f, 0.0f }; // Emissive coefficient
    float Ni = 0.0f; // Optical density (index of refraction)
    float d = 0.0f; // Dissolve (transparency)
    int illum = 0; // Illumination model
    std::string textureFile; // File name of the texture
};


bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<Normal>& normals, std::vector<Face>& faces, std::unordered_map<std::string, Material>& materials);
bool loadMTL(const std::string& filename, std::unordered_map<std::string, Material>& materials);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

// Variáveis globais para armazenar as transformações
bool rotateX = false, rotateY = false, rotateZ = false;
float transX = 0.0f, transY = 0.0f, transZ = 0.0f;
float scale = 1.0f;


// Função MAIN
int main()
{
    std::vector<Vertex> vertices;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::unordered_map<std::string, Material> materials;

    std::string objFilename = "cube.obj";
    if (loadOBJ(objFilename, vertices, normals, faces, materials)) {
        std::cout << "Arquivo OBJ carregado com sucesso!" << std::endl;

        std::cout << "Vértices: " << std::endl;
        for (const auto& vertex : vertices) {
            std::cout << vertex.x << " " << vertex.y << " " << vertex.z << " "
                << vertex.s << " " << vertex.t << std::endl;
        }

        std::cout << "Vetores Normais: " << std::endl;
        for (const auto& normal : normals) {
            std::cout << normal.x << " " << normal.y << " " << normal.z << std::endl;
        }

        std::cout << "Faces: " << std::endl;
        for (const auto& face : faces) {
            std::cout << face.v[0] << "/" << face.vt[0] << "/" << face.vn[0] << " "
                << face.v[1] << "/" << face.vt[1] << "/" << face.vn[1] << " "
                << face.v[2] << "/" << face.vt[2] << "/" << face.vn[2] << std::endl;
        }

        std::cout << "Materiais: " << std::endl;
        std::cout << "Número de materiais: " << materials.size() << std::endl;
            for (const auto& pair : materials) {
            const auto& material = pair.second;
            std::cout << "Material: " << material.name << std::endl;
            std::cout << " Ns: " << material.Ns << std::endl;
            std::cout << " Ka: " << material.Ka[0] << " " << material.Ka[1] << " " << material.Ka[2] << std::endl;
            std::cout << " Kd: " << material.Kd[0] << " " << material.Kd[1] << " " << material.Kd[2] << std::endl;
            std::cout << " Ks: " << material.Ks[0] << " " << material.Ks[1] << " " << material.Ks[2] << std::endl;
            std::cout << " Ke: " << material.Ke[0] << " " << material.Ke[1] << " " << material.Ke[2] << std::endl;
            std::cout << " Ni: " << material.Ni << std::endl;
            std::cout << " d: " << material.d << std::endl;
            std::cout << " illum: " << material.illum << std::endl;
            std::cout << " Texture: " << material.textureFile << std::endl;
        }
    }
    else {
        std::cerr << "Falha ao carregar o arquivo OBJ." << std::endl;
    }

    // Inicialização da GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Diogo!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros de funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Compilando e ativando o shader
    int shaderProgram = setupShader();
    glUseProgram(shaderProgram);

    // Definindo a geometria
    int VAO = setupGeometry();

    // Define a matriz de projeção
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Habilitar o teste de profundidade
    glEnable(GL_DEPTH_TEST);

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Verifica se há eventos de entrada (como pressionar teclas, mover o mouse, etc.)
        glfwPollEvents();

        // Renderiza o frame
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Define a matriz de visualização
        glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f));
        GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Define a matriz de modelo
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(transX, transY, transZ));
        model = glm::scale(model, glm::vec3(scale, scale, scale));
        if (rotateX)
            model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        if (rotateY)
            model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        if (rotateZ)
            model = glm::rotate(model, (GLfloat)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // Renderiza o cubo
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);

        // Troca os buffers e desenha
        glfwSwapBuffers(window);
    }

    // Libera os recursos alocados pela GLFW
    glfwTerminate();
    return 0;
}

int setupShader()
{
    // Compilando o Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Checando erros de compilação do Vertex Shader
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Compilando o Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Checando erros de compilação do Fragment Shader
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
    }

    // Linkando os shaders em um programa
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Checando erros de linkagem
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
    }

    // Deletando os shaders após terem sido linkados
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupGeometry()
{
    // Definindo os vértices e cores para o cubo
    GLfloat vertices[] = {
        // Frente
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        // Trás
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

        // Esquerda
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

        // Direita
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

        // Embaixo
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

        // Em cima
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f
    };

    GLuint VBO, VAO;

    // Geração do identificador do VBO
    glGenBuffers(1, &VBO);

    // Faz a conexão (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Geração do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida conecta e seta o(s) buffer(s) de vértices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    // Para cada atributo do vértice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    // Localização no shader * (a localização dos atributos devem ser correspondentes no layout especificado no vertex shader)
    // Número de valores que o atributo tem (por exemplo, 3 coordenadas xyz)
    // Tipo do dado
    // Se está normalizado (entre zero e um)
    // Tamanho em bytes
    // Deslocamento a partir do byte zero

    // Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Observe que isso é permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de vértice
    // atualmente vinculado - para que depois possamos desvincular com segurança
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (é uma boa prática desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
        rotateX = !rotateX;

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
        rotateY = !rotateY;

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
        rotateZ = !rotateZ;

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
bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<Normal>& normals, std::vector<Face>& faces, std::unordered_map<std::string, Material>& materials) {
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
            Vertex vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        }
        else if (word == "vt") {
            float s, t;
            ss >> s >> t;
            if (!vertices.empty()) {
                vertices.back().s = s;
                vertices.back().t = t;
            }
        }
        else if (word == "vn") {
            Normal normal;
            ss >> normal.x >> normal.y >> normal.z;
            normals.push_back(normal);
        }
        else if (word == "f") {
            std::string tokens[3];
            ss >> tokens[0] >> tokens[1] >> tokens[2];

            Face face;
            for (int i = 0; i < 3; i++) {
                int pos = tokens[i].find("/");
                std::string token = tokens[i].substr(0, pos);
                face.v[i] = std::stoi(token) - 1;

                tokens[i] = tokens[i].substr(pos + 1);
                pos = tokens[i].find("/");
                token = tokens[i].substr(0, pos);
                face.vt[i] = std::stoi(token) - 1;

                tokens[i] = tokens[i].substr(pos + 1);
                face.vn[i] = std::stoi(tokens[i]) - 1;
            }
            faces.push_back(face);
        }
        else if (word == "mtllib") {
            std::string mtlFilename;
            ss >> mtlFilename;
            std::cout << "Carregando MTL: " << mtlFilename << std::endl;
            loadMTL(mtlFilename, materials);
        }
        else if (word == "usemtl") {
            std::string materialName;
            ss >> materialName;
            std::cout << "Usando material: " << materialName << std::endl;
        }
    }

    file.close();
    return true;
}

// Função para carregar um arquivo MTL
bool loadMTL(const std::string& filename, std::unordered_map<std::string, Material>& materials) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo " << filename << std::endl;
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
                materials[currentMaterial.name] = currentMaterial;
                std::cout << "Material armazenado: " << currentMaterial.name << std::endl;
            }
            ss >> currentMaterial.name;
            currentMaterial = Material(); // Reset the current material
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

    materials[currentMaterial.name] = currentMaterial;
    std::cout << "Material armazenado: " << currentMaterial.name << std::endl;

    file.close();
    return true;
}
