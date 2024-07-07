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
	//glm::vec3 getPosition() { return position; }
	//void setPosition(glm::vec3 position) { this->position = position; }
	//void setScale(glm::vec3 scale) { this->scale = scale; }
	//float getAngle() { return angle; }
	//void setAngle(float angle) { this->angle = angle; }
	glm::vec3 position;
	glm::vec3 scale;
	float angle;
	glm::vec3 axis;

protected:
	GLuint VAO; //Identificador do Vertex Array Object - Vértices e seus atributos
	int nVertices;

	//Informações sobre as transformações a serem aplicadas no objeto

	//Referência (endereço) do shader
	Shader* shader;

};

