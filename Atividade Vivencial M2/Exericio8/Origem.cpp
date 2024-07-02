// Aluno: Diogo Garbinato de Fagundes       Matr�cula: 1189650
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

// Prot�tipo da fun��o de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Prot�tipos das fun��es
int setupShader();
int setupGeometry();

// Estrutura para armazenar um v�rtice
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

// Estrutura para armazenar uma face (tri�ngulo)
struct Face {
    int v[3];    // �ndices dos v�rtices
    int vt[3];   // �ndices das coordenadas de textura
    int vn[3];   // �ndices dos vetores normais
};

bool loadOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<TexCoord>& texCoords, std::vector<Normal>& normals, std::vector<Face>& faces);

// Dimens�es da janela (pode ser alterado em tempo de execu��o)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// C�digo fonte do Vertex Shader (em GLSL): ainda hardcoded
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

// C�digo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

// Vari�veis globais para armazenar as transforma��es
bool rotateX = false, rotateY = false, rotateZ = false;
float transX = 0.0f, transY = 0.0f, transZ = 0.0f;
float scale = 1.0f;


// Fun��o MAIN
int main()
{
    std::vector<Vertex> vertices;
    std::vector<TexCoord> texCoords;
    std::vector<Normal> normals;
    std::vector<Face> faces;
    std::string filename = "../../3D_Models/Cube/cube.obj";

    if (loadOBJ(filename, vertices, texCoords, normals, faces)) {
        std::cout << "Arquivo OBJ carregado com sucesso!" << std::endl;

        std::cout << "V�rtices: " << std::endl;
        for (const auto& vertex : vertices) {
            std::cout << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
        }

        std::cout << "Coordenadas de Textura: " << std::endl;
        for (const auto& texCoord : texCoords) {
            std::cout << texCoord.s << " " << texCoord.t << std::endl;
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
    }
    else {
        std::cerr << "Falha ao carregar o arquivo OBJ." << std::endl;
    }

    // Inicializa��o da GLFW
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Cria��o da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- Diogo!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da fun��o de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros de fun��es da OpenGL
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

    // Define a matriz de proje��o
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Habilitar o teste de profundidade
    glEnable(GL_DEPTH_TEST);

    // Loop da aplica��o - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Verifica se h� eventos de entrada (como pressionar teclas, mover o mouse, etc.)
        glfwPollEvents();

        // Renderiza o frame
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Define a matriz de visualiza��o
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

    // Checando erros de compila��o do Vertex Shader
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

    // Checando erros de compila��o do Fragment Shader
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

    // Deletando os shaders ap�s terem sido linkados
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupGeometry()
{
    // Definindo os v�rtices e cores para o cubo
    GLfloat vertices[] = {
        // Frente
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        // Tr�s
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

    // Gera��o do identificador do VBO
    glGenBuffers(1, &VBO);

    // Faz a conex�o (vincula) do buffer como um buffer de array
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // Envia os dados do array de floats para o buffer da OpenGl
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Gera��o do identificador do VAO (Vertex Array Object)
    glGenVertexArrays(1, &VAO);

    // Vincula (bind) o VAO primeiro, e em seguida conecta e seta o(s) buffer(s) de v�rtices
    // e os ponteiros para os atributos
    glBindVertexArray(VAO);

    // Para cada atributo do v�rtice, criamos um "AttribPointer" (ponteiro para o atributo), indicando:
    // Localiza��o no shader * (a localiza��o dos atributos devem ser correspondentes no layout especificado no vertex shader)
    // N�mero de valores que o atributo tem (por exemplo, 3 coordenadas xyz)
    // Tipo do dado
    // Se est� normalizado (entre zero e um)
    // Tamanho em bytes
    // Deslocamento a partir do byte zero

    // Atributo posi��o (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Observe que isso � permitido, a chamada para glVertexAttribPointer registrou o VBO como o objeto de buffer de v�rtice
    // atualmente vinculado - para que depois possamos desvincular com seguran�a
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Desvincula o VAO (� uma boa pr�tica desvincular qualquer buffer ou array para evitar bugs medonhos)
    glBindVertexArray(0);

    return VAO;
}

// Fun��o de callback de teclado
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

// Fun��o para carregar um arquivo OBJ
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
            // Leitura dos v�rtices
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
    }

    file.close();
    return true;
}
