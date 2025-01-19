
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include "glm/glm.hpp"



struct Vertex
{
	int positionInd;
	int texCoordInd;
	int normalInd;
		Vertex(int positionInd, int texCoordInd, int normalInd):
		positionInd(positionInd), texCoordInd(texCoordInd), normalInd(normalInd) {}
};

class ObjReader
{
	public:
		std::vector<glm::vec3> position;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texCoords;
		std::vector<Vertex> faces;
		std::vector<float> finalData;

		void parseObjFileData(std::string& objFileData)
		{
			std::istringstream objStream(objFileData);
			std::string line;
			while (std::getline(objStream, line))
			{
				std::istringstream lineStream(line);
				std::string type;
				lineStream >> type;
				if (type == "v")
				{
					glm::vec3 position;
					lineStream >> position.x >> position.y >> position.z;
					this->position.push_back(position);
				}
				else if (type == "vt")
				{
					glm::vec2 texCoords;
					lineStream >> texCoords.x >> texCoords.y;
					this->texCoords.push_back(texCoords);
				}
				else if (type == "vn")
				{
					glm::vec3 normals;
					lineStream >> normals.x >> normals.y >> normals.z;
					this->normals.push_back(normals);
				}
				else if (type == "f")
				{
					for (int i = 0; i < 3; i++)
					{
						std::string vertexData;
						lineStream >> vertexData;
						std::replace(vertexData.begin(), vertexData.end(), '/', ' ');

						std::istringstream vertexStream(vertexData);
						int positionInd, texCoordInd, normalInd;
						vertexStream >> positionInd >> texCoordInd >> normalInd;

						this->faces.emplace_back(positionInd, texCoordInd, normalInd);
					}
				}
			}
		}

		void readObj(std::string &objFilePath)
		{
			std::ifstream objFile;
			std::stringstream objFileStream;
			std::string objFileData;
			objFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			try
			{	
				objFile.open(objFilePath,std::ifstream::in);
				objFileStream << objFile.rdbuf();
				objFile.close();
				objFileData = objFileStream.str();
			}
			catch(const std::ifstream::failure e)
			{
				std::cout << "ERROR READING OBJ FILE" << std::endl;
			}
			parseObjFileData(objFileData);
			assembleFinalData();
		}

		void assembleFinalData()
		{
			for (int i = 0; i< faces.size();i++)
			{		
					auto vert = faces[i];
					finalData.push_back(position[vert.positionInd-1].x);
					finalData.push_back(position[vert.positionInd-1].y);
					finalData.push_back(position[vert.positionInd-1].z);

					finalData.push_back(texCoords[vert.texCoordInd-1].x);
					finalData.push_back(texCoords[vert.texCoordInd-1].y);

					finalData.push_back(normals[vert.normalInd-1].x);
					finalData.push_back(normals[vert.normalInd-1].y);
					finalData.push_back(normals[vert.normalInd-1].z);
			}
		}

};





