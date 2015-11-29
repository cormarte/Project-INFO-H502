#define _CRT_SECURE_NO_DEPRECATE
#include "ModelReader.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;
using namespace glm;

ModelReader* ModelReader::instance = 0;

ModelReader::ModelReader() {

}

ModelReader::~ModelReader() {

}

ModelReader* ModelReader::getInstance() {

	if (instance == 0) {

		instance = new ModelReader();
	}

	return instance;
}

void ModelReader::readFile(char* path, vector<vec3> &vertices, vector<vec3> &normals, vector<vec2> &textureCoordinates) {

	int numberOfVertices;
	int numberOfFaces;
	vector<vec3> vertices_tmp;
	vector<vec3> normals_tmp;
	vector<vec2> textureCoordinates_tmp;

	FILE* file = fopen(path, "r");

	if (!file) {

		printf("Cannot open the file !\n");
		return;
	}

	char buffer[512];
	int result = fscanf(file, "%s", buffer);

	while (strcmp(buffer, "vertex") != 0 && result != EOF) {

		result = fscanf(file, "%s", buffer);
	}

	result = fscanf(file, "%i", &numberOfVertices);

	while (strcmp(buffer, "face") != 0 && result != EOF) {

		result = fscanf(file, "%s", buffer);
	}

	result = fscanf(file, "%i", &numberOfFaces);

	while (strcmp(buffer, "end_header") != 0 && result != EOF) {

		result = fscanf(file, "%s", buffer);
	}

	for (int i = 0; i < numberOfVertices; i++) {
	
		vec3 vertex;
		vec3 normal;
		vec2 textureCoordinate;

		fscanf(file, "%f %f %f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z, &textureCoordinate.x, &textureCoordinate.y);

		vertices_tmp.push_back(vertex);
		normals_tmp.push_back(normal);
		textureCoordinates_tmp.push_back(textureCoordinate);
	}

	for (int i = 0; i < numberOfFaces; i++) {
	
		int x, a, b, c;
		fscanf(file, "%i %i %i %i\n", &x, &a, &b, &c);
		vertices.push_back(vertices_tmp[a]);
		vertices.push_back(vertices_tmp[b]);
		vertices.push_back(vertices_tmp[c]);
		normals.push_back(normals_tmp[a]);
		normals.push_back(normals_tmp[b]);
		normals.push_back(normals_tmp[c]);
		textureCoordinates.push_back(textureCoordinates_tmp[a]);
		textureCoordinates.push_back(textureCoordinates_tmp[b]);
		textureCoordinates.push_back(textureCoordinates_tmp[c]);
	}
	

	/*ifstream file(path, ifstream::in);

	if (!file) {

		cout << "Cannot open " << path << "..." << endl;
		return;
	}

	string line = "";

	while (line.find("element vertex") == string::npos) {

		getline(file, line);
	}

	istringstream stream(line.substr(14));
	stream >> numberOfVertices;

	while (line.find("element face") == string::npos) {

		getline(file, line);
	}
	istringstream stream2(line.substr(13));
	stream2 >> numberOfFaces;

	cout << numberOfVertices << endl;
	cout << numberOfFaces << endl;

	while (1);*/
}

/*void ModelReader::readFile(const char* path, vector<vec3> &vertices, vector<vec2> &uvs, vector<vec3> &normals) {

	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<vec3> temp_vertices;
	vector<vec2> temp_uvs;
	vector<vec3> temp_normals;

	FILE* file = fopen(path, "r");

	if (file == NULL) {

		printf("Impossible to open the file !\n");
		return;
	}

	while (1) {

		char lineHeader[128];
		int res = fscanf(file, "%s", lineHeader);

		if (res == EOF) {

			break;
		}

		if (strcmp(lineHeader, "v") == 0) {
			
			vec3 vertex;
			fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}

		else if (strcmp(lineHeader, "vt") == 0) {

			vec2 uv;
			fscanf(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}

		else if (strcmp(lineHeader, "vn") == 0) {

			vec3 normal;
			fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}

		else if (strcmp(lineHeader, "f") == 0) {

			string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf(file, "%d/%d %d/%d %d/%d", &vertexIndex[0], &uvIndex[0], &vertexIndex[1], &uvIndex[1], &vertexIndex[2], &uvIndex[2]);
			//int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

			if (matches != 6) {

				printf("File can't be read by our simple parser : ( Try exporting with other options\n");
				return;
			}

			vertexIndices.push_back(vertexIndex[0]);
			vertexIndices.push_back(vertexIndex[1]);
			vertexIndices.push_back(vertexIndex[2]);
			uvIndices.push_back(uvIndex[0]);
			uvIndices.push_back(uvIndex[1]);
			uvIndices.push_back(uvIndex[2]);
			//normalIndices.push_back(normalIndex[0]);
			//normalIndices.push_back(normalIndex[1]);
			//normalIndices.push_back(normalIndex[2]);
		}

		for (unsigned int i = 0; i < vertexIndices.size(); i++) {
		
			unsigned int vertexIndex = vertexIndices[i];
			vec3 vertex = temp_vertices[vertexIndex-1];
			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < uvIndices.size(); i++) {

			unsigned int uvIndex = uvIndices[i];
			vec2 uv = temp_uvs[uvIndex - 1];
			uvs.push_back(uv);
		}
	}
}

void ModelReader::readFile(const char* path, vector<vec4>& vertices, vector<vec3>& normals, vector<GLushort>& elements) {

	ifstream file(path, ifstream::in);

	if (!file) {

		cout << "Cannot open " << path << endl;
		exit(1);
	}

	string line;

	while (getline(file, line)) {

		if (line.substr(0, 2) == "v ") {

			istringstream stream(line.substr(2));
			vec4 vertice; 
			stream >> vertice.x; stream >> vertice.y; stream >> vertice.z; 
			vertice.w = 1.0f;
			vertices.push_back(vertice);
		}

		else if (line.substr(0, 2) == "f ") {

			istringstream stream(line.substr(2));
			GLushort a, b, c;
			stream >> a; stream >> b; stream >> c;
			a--; b--; c--;
			elements.push_back(a); elements.push_back(b); elements.push_back(c);
		}
	}
	
	normals.resize(vertices.size(), vec3(0.0, 0.0, 0.0));

	for (int i = 0; i < elements.size(); i += 3) {

		GLushort a = elements[i];
		GLushort b = elements[i+1];
		GLushort c = elements[i+2];
		vec3 normal = glm::normalize(glm::cross(vec3(vertices[b]) - vec3(vertices[a]), vec3(vertices[c]) - vec3(vertices[a])));

		normals[a] = normals[b] = normals[c] = normal;

		cout << a << ", " << b << ", " << c << endl;
	}

}*/
