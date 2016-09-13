#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"

GLuint TextureFromFile(const char* path, string directory);

class Model
{
public:

	// Functions //
	//Constructor - expects a filepath to a 3D model
	Model(GLchar* path)
	{
		this->loadModel(path);
	}
	void Draw(Shader shader) //Draws model
	{
		for (GLuint i = 0; i < this->meshes.size(); i++)
			this->meshes[i].Draw(shader);
	}

private:
	// Model Data //
	vector<Mesh> meshes;
	string directory;
	vector<Texture> textures_loaded;

	// Functions //
	void loadModel(string path)
	{
		//Loads Model
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

		//Error Handling
		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
			//Check if scene and root node != null & check flags to see if returned data is incomplete
		{
			//Error Report
			cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
			return;
		}
		//Retrieve the directory path of the filepath
		this->directory = path.substr(0, path.find_last_of('/'));

		//Process Assimp root node recursively (recursive processNode Function)
		//(Each node possibly contains a set of children to process)
		this->processNode(scene->mRootNode, scene);


	}


	// Recursive function to process nodes until all are processed //

	/*
	Each node contains a set of mesh indices where each index points to a specific mesh in the scene object
	So
	1. Retrieve mesh indices
	2. Retrieve specific mesh
	3. Process each mesh
	4. Repeat for all nodes and their children

	*/
	void processNode(aiNode* node, const aiScene* scene)
	{
		//Process all meshes of the nodes
		for (GLuint i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			//Check each of node's mesh indices, retrieve corresponding mesh by indexing mMeshes array
			this->meshes.push_back(this->processMesh(mesh, scene));
			//returned mesh passed to processMesh function (which returns a Mesh object to store in Mesh list)
		}

		//Same for each node's children
		for (GLuint i = 0; i < node->mNumChildren; i++)
		{
			this->processNode(node->mChildren[i], scene);
		}
	}


	// processMesh Function //
	/*
	1. Retrieve all vertex data
	2. Retrieve mesh's indices
	3. Retrieve relevant material data

	Proecssed data is stored in one of 3 vectors. Mesh is created from those and returned.
	*/
	Mesh processMesh(aiMesh* mesh, const aiScene* scene)
	{

		vector<Vertex> vertices;
		vector<GLuint> indices;
		vector<Texture> textures;

		for (GLuint i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex; //"vertex" of data type "Vertex" (in Mesh.h)
			glm::vec3 vector; //placeholder for transferring Assimp data

							  // Positions //
			vector.x = mesh->mVertices[i].x;
			vector.y = mesh->mVertices[i].y;
			vector.z = mesh->mVertices[i].z;
			vertex.Position = vector;


			// Normals //
			vector.x = mesh->mNormals[i].x;
			vector.y = mesh->mNormals[i].y;
			vector.z = mesh->mNormals[i].z;
			vertex.Normal = vector;


			// Tex Coords //
			//Assimp allows 8 different Tex Coords per Vertex. Only using first set.

			if (mesh->mTextureCoords[0]) //Does mesh contain Tex Coords?
			{
				glm::vec2 vec;
				vec.x = mesh->mTextureCoords[0][i].x; //[0] for first set
				vec.y = mesh->mTextureCoords[0][i].y;
				vertex.TexCoords = vec;
			}
			else
				vertex.TexCoords = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
			//Vertex struct filled
		}

		// Indices //

		for (GLuint i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			//Retrieve all indices of the face and store them in the indices vector
			for (GLuint j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}
		/*
		Each mesh has an array of faces where each face represents a single primitive.
		A face contains indices that define which vertices to draw in what order for each primitive.
		This section sorts through to store all face indices in the indices vector.
		*/

		// Materials //

		if (mesh->mMaterialIndex >= 0)
		{
			//Retrieve aiMaterial object from mMaterials in scene
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			/*
			We assume a convention for sampler names in the shaders. Each diffuse texture should be named
			as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
			Same applies to other texture as the following list summarizes:
			Diffuse: texture_diffuseN
			Specular: texture_specularN
			Normal: texture_normalN
			*/


			//Load mesh's diffuse textures
			vector<Texture> diffuseMaps = this->loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

			//Load mesh's specular textures
			vector<Texture> specularMaps = this->loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
			textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

		}
		return Mesh(vertices, indices, textures);
	}

	// Load Material Textures Function //

	/*
	Iterates through the texture locations of given texture type
	Retrieves texture's file location
	Loads & generates texture & stores info in a Vertex struct
	*/
	vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName)
	{
		vector<Texture> textures;
		for (GLuint i = 0; i < mat->GetTextureCount(type); i++)
			//Check amount of textures stored in material using GetTextureCount
		{
			aiString str;
			mat->GetTexture(type, i, &str);
			//Retrieve texture file's location and store results in aiString


			GLboolean skip = false;
			/*
			If textures have already been loaded (comparing load paths)
			then skip loading process
			*/
			for (GLuint j = 0; j < textures_loaded.size(); j++)
			{
				if (textures_loaded[j].path == str)
				{
					textures.push_back(textures_loaded[j]);
					skip = true;
					break;
				}

			}
			if (!skip)
			{
				//If textures haven't been loaded already, then load it
				Texture texture;
				texture.id = TextureFromFile(str.C_Str(), this->directory);
				//TextureFromFile loads a texture (using SOIL) and returns its ID
				texture.type = typeName;
				texture.path = str;
				textures.push_back(texture);
				this->textures_loaded.push_back(texture); //add to loaded textures
			}

		}
		return textures;

	}


	GLint TextureFromFile(const char* path, string directory)
	{
		//Generate texture ID and load texture data 
		string filename = string(path);
		filename = directory + '/' + filename;
		GLuint textureID;
		glGenTextures(1, &textureID);
		int width, height;
		unsigned char* image = SOIL_load_image(filename.c_str(), &width, &height, 0, SOIL_LOAD_RGB);
		// Assign texture to ID
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		
		glGenerateMipmap(GL_TEXTURE_2D);

		// Parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		SOIL_free_image_data(image);
		return textureID;
	}
};