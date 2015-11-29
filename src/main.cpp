#include <windows.h>
#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>
#include <iostream>
#include "ModelReader.h"
#include "TextureReader.h"

using namespace std;

static int g_window;

vector<vec3> normals;
vector<vec2> textureCoordinates;
vector<vec3> vertices;

GLuint textureID;

/*int lookAt[9] = {-5, -3, 6, 3, 2, 3, 5, -3, 10};
int increment = 1;*/
int angle[3] = {0, 0, 0};

void display() {

	//Colour and Z-buffer clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Camera positionning
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-5, 0, 0, 0, 0, 0, 0, 0, 1);
	//gluLookAt(lookAt[0], lookAt[1], lookAt[2], lookAt[3], lookAt[4], lookAt[5], lookAt[6], lookAt[7], lookAt[8]);
	//cout << lookAt[0] << ", " << lookAt[1] << ", " << lookAt[2] << ", " << lookAt[3] << ", " << lookAt[4] << ", " << lookAt[5] << ", " << lookAt[6] << ", " << lookAt[7] << ", " << lookAt[8] << endl;
	
	//Axis scaling
	glScalef(0.5, 0.5, 0.5);
	glRotatef(angle[0], 1, 0, 0);
	glRotatef(angle[1], 0, 1, 0);
	glRotatef(angle[2], 0, 0, 1);
	glTranslatef(0, -0.5, -3.5);

	//Baby drawing
	glBegin(GL_TRIANGLES);

	//glColor3f(0.9, 0.6, 0.6);

	for (int i = 0; i != vertices.size(); i++) {

		vec2 textureCoordinate = textureCoordinates.at(i);
		vec3 vertex = vertices.at(i);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y);  glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Cylinder drawing
	/*glBindTexture(GL_TEXTURE_2D, textureID);
	glBegin(GL_TRIANGLES);
	glEnd();*/

	glFlush();
}

void init()
{
	//Background colour definition
	glClearColor(0.98, 0.88, 0.88, 1.0);

	//Shading mode definition
	glShadeModel(GL_SMOOTH);

	//Projection definition
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (1.0*glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

	//Z-buffer activation
	glEnable(GL_DEPTH_TEST);

	//Texture loading
	int imageWidth, imageHeight;
	GLubyte* texture = TextureReader::getInstance()->readFile("..//data//baby_texture.jpg", imageWidth, imageHeight);

	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

	//Baby model loading
	ModelReader::getInstance()->readFile("..//data//baby_original_triangles.ply", vertices, normals, textureCoordinates);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	
	case '2':
		angle[1] -= 5;
		break;

	case '8':
		angle[1] += 5;
		break;

	case '4':
		angle[2] -= 5;
		break;

	case '6':
		angle[2] += 5;
		break;
	}

	display();

	/*if (key == '0') {
		
		increment = -increment;
	}

	else if (key >= '1' && key <= '9') {
		
		lookAt[(int)(key - 49)] += increment;
		display();
	}*/
}

void reshape(int w, int h) {

	/*glViewport(0, 0, (GLsizei)w, (GLsizei)h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0 * (GLfloat)h / (GLfloat)w,
		4.0 * (GLfloat)h / (GLfloat)w, -10.0, 10.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(5, 0, 0, 0, 0, 0, 0, 0, 1);*/
}

int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize(500, 500);
	glutInitWindowPosition(100, 100);
	g_window = glutCreateWindow("Baby Project");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}