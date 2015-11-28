#include <windows.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <glut.h>
#include <iostream>
#include "MeshFileReader.h"
using namespace std;

static int g_window;

vector<vec3> normals;
vector<vec2> textureCoordinates;
vector<vec3> vertices;

int lookAt[9] = {0, 0, 4, -1, 0, 4, 0, -1, 2};
int increment = 1;

void display() {

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	gluLookAt(lookAt[0], lookAt[1], lookAt[2], lookAt[3], lookAt[4], lookAt[5], lookAt[6], lookAt[7], lookAt[8]);
	
	glBegin(GL_TRIANGLES);

	for (int i = 0; i != vertices.size(); i++) {

		glColor3f(0.9, 0.6, 0.6);
		vec3 vertex = vertices.at(i);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();
	glFlush();
}

void init()
{
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glShadeModel(GL_FLAT);

	MeshFileReader::getInstance()->readFile("..//data//baby_original_triangles.ply", vertices, normals, textureCoordinates);
}

void keyboard(unsigned char key, int x, int y)
{

	if (key == '0') {
		
		increment = -increment;
	}

	else if (key >= '1' && key <= '9') {
		
		lookAt[(int)(key - 49)] += increment;
		display();
	}
}

void reshape(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0 * (GLfloat)h / (GLfloat)w,
		4.0 * (GLfloat)h / (GLfloat)w, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(5, 0, 0, 0, 0, 0, 0, 0, 1);
}

int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(640, 480);
	glutInitWindowPosition(100, 100);
	g_window = glutCreateWindow("Baby Project");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}