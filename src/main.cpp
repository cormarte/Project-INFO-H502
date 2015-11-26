#include <windows.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <iostream>
#include "MeshFileReader.h"
using namespace std;


int main(int argc, char *argv[]) {

	vector<vec3> vertices;

	MeshFileReader::getInstance()->readFile("D:\\Documents de Corentin\\Documents\\ULB\\MA2\\INFO-H-502\\Projet\\data\\baby_original_triangles.ply", vertices);
	
	//glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glLoadIdentity();
	gluLookAt(0.0, 0.0, 4.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

	glBegin(GL_TRIANGLES);

	for (size_t i = 0; i<vertices.size(); i++)
	{
		glNormal3f(vertices[i].x, vertices[i].y, vertices[i].z);
		glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
	}

	glEnd();

	while (1);
}