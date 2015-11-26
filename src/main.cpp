#include <windows.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <iostream>
#include "MeshFileReader.h"
using namespace std;


int main(int argc, char *argv[]) {

	vector<vec3> vertices;

	MeshFileReader::getInstance()->readFile("D:\\Documents de Corentin\\Documents\\ULB\\MA2\\INFO-H-502\\Projet\\data\\baby_original_triangles.ply", vertices);
	
	glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
}