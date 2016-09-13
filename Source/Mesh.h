#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//For indexing each of vertex attributes
struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

//For storing id and type (e.g. diffuse/specular) of texture
struct Texture{ //TODO: I need to inherrit "vector" as that is what contains "push_back"
	GLuint id;
	string type;
	aiString path; //Store path of the texture to compare with other textures
};

class Mesh {
	public:
		// Mesh Data //
		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		// Functions //
		Mesh(vector<Vertex> vertices, vector<GLuint> indices, vector<Texture> textures) //Constructor
		{
			this->vertices = vertices;
			this->indices = indices;
			this->textures = textures;

			this->setupMesh();
		}
		void Draw(Shader shader)
		{
			GLuint diffuseNr = 1;
			GLuint specularNr = 1;

			for (GLuint i = 0; i < this->textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i); //Activate proper texture units before binding

				//Retrieve Texture number (e.g. N in diffuse_textureN)
				stringstream ss;
				string number;
				string name = this->textures[i].type;
				if (name == "texture_diffuse")
					ss << diffuseNr++; //transfer GLuint to stream
				else if (name == "texture_specular")
					ss << specularNr++; //transfer GLuint to stream
				number = ss.str();

				//Set sampler to the correct texture unit
				//glUniform1f(glGetUniformLocation(shader.Program, ("material." + name + number).c_str()), i);
				glUniform1i(glGetUniformLocation(shader.Program, (name + number).c_str()), i);

				//Bind Texture
				glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
			}
			glUniform1f(glGetUniformLocation(shader.Program, "material.shininess"), 16.0f);

			// Draw Mesh //
			glBindVertexArray(this->VAO);
			glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);

			//Set Everything back to default once configured
			for (GLuint i = 0; i < this->textures.size(); i++)
			{
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

		}




	private:
		// Render Data //
		GLuint VAO, VBO, EBO;

		// Functions //
		void setupMesh()
		{
			//Generate unique ID for buffers for setupMesh
			glGenVertexArrays(1, &this->VAO);
			glGenBuffers(1, &this->VBO);
			glGenBuffers(1, &this->EBO);

			//Bind 
			glBindVertexArray(this->VAO);
			glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);


			glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(GLuint), &this->indices[0], GL_STATIC_DRAW);
			/*
			1. Type of Buffer to copy data into
			2. Size of data in bytes (8 floats x 4 bytes each for struct)
			3. Actual data sending
			4. Specifies how we want the graphics card to manage given data
			*/

			//Vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			/*
			1. Which vertex attribute to configure
			2. Size of vertex attribute
			3. Type of data
			4. Data normalised or not?
			5. Stride - space between consecutive vertex attribute sets.
			6. Offset where position data begins in the buffer.
			*/

			//Vertex Normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),(GLvoid*)offsetof(Vertex, Normal));

			//Vertex Texture Coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),(GLvoid*)offsetof(Vertex, TexCoords));

			glBindVertexArray(0);

		}


};

