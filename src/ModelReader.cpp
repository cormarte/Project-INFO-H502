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

void ModelReader::readFile(char* path, vector<vec3> &vertex, vector<vec3> &normals) {

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

		fscanf(file, "%f %f %f %f %f %f\n", &vertex.x, &vertex.y, &vertex.z, &normal.x, &normal.y, &normal.z);

		vertices_tmp.push_back(vertex);
		normals_tmp.push_back(normal);
	}

	for (int i = 0; i < numberOfFaces; i++) {
	
		int x, a, b, c;
		fscanf(file, "%i %i %i %i\n", &x, &a, &b, &c);
		vertex.push_back(vertices_tmp[a]);
		vertex.push_back(vertices_tmp[b]);
		vertex.push_back(vertices_tmp[c]);
		normals.push_back(normals_tmp[a]);
		normals.push_back(normals_tmp[b]);
		normals.push_back(normals_tmp[c]);
	}
}