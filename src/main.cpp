#include <windows.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <glut.h>
#include <iostream>
#include "MeshFileReader.h"
using namespace std;
//Madli et Coco

int main(int argc, char *argv[]) {

	vector<vec3> vertices;

	MeshFileReader::getInstance()->readFile("..\\data\\baby_original_triangles.ply", vertices);

	//glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	glutCreateWindow(argv[0]);
	//init();
	//glutDisplayFunc(display);
	//glutReshapeFunc(reshape);
	//glutKeyboardFunc(keyboard);
	glutMainLoop();
	
	

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
	return 0;
}