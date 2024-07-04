#pragma once

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"

class Mesh
{
public:
    Mesh() {}
    ~Mesh() {}
    void initialize(GLuint VAO, int nVertices, Shader* shader, glm::vec3 position = glm::vec3(0.0, 0.0, 0.0), glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0), float angle = 0.0, glm::vec3 axis = glm::vec3(0.0, 0.0, 1.0));
    void update();
    void draw();

    glm::vec3 getPosition() const { return position; } // M�todo para obter a posi��o
    void setPosition(glm::vec3 newPos) { position = newPos; } // M�todo para definir a posi��o

protected:
    GLuint VAO; //Identificador do Vertex Array Object - V�rtices e seus atributos
    int nVertices;

    //Informa��es sobre as transforma��es a serem aplicadas no objeto
    glm::vec3 position;
    glm::vec3 scale;
    float angle;
    glm::vec3 axis;

    //Refer�ncia (endere�o) do shader
    Shader* shader;
};

