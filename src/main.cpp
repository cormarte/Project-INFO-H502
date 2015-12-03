#define _USE_MATH_DEFINES
#include <windows.h>
#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>
#include <iostream>
#include <math.h>
#include "ModelReader.h"
#include "TextureReader.h"

using namespace std;

static int g_window;

vector<vec3> babyNormals;
vector<vec2> babyTextureCoordinates;
vector<vec3> babyVertices;

vector<vec3> cylinderNormals;
vector<vec2> cylinderTextureCoordinates;
vector<vec3> cylinderVertices;

GLuint textureID;

int angle[3] = {-90, 0, 0};

void createCylinder(int resolution) {

	for (int i = 0; i <= resolution; i++) {

		float theta = (float)i*((M_PI) / (float)resolution);

		vec3 upperNormal;
		upperNormal.x = cosf(theta+90);
		upperNormal.y = 0.0f;
		upperNormal.z = -sinf(theta);
		cylinderNormals.push_back(upperNormal);

		vec2 upperTextureCoordinate;
		upperTextureCoordinate.x = (float)i*(1.0f/resolution);
		upperTextureCoordinate.y = 1.0f;
		cylinderTextureCoordinates.push_back(upperTextureCoordinate);

		vec2 lowerTextureCoordinate;
		lowerTextureCoordinate.x = (float)i*(1.0f / resolution);
		lowerTextureCoordinate.y = 0.0f;
		cylinderTextureCoordinates.push_back(lowerTextureCoordinate);

		vec3 upperVertex;
		upperVertex.x = cosf(theta);
		upperVertex.y = 1.0f;
		upperVertex.z = -sinf(theta);
		cylinderVertices.push_back(upperVertex);

		vec3 lowerVertex;
		lowerVertex.x = cosf(theta);
		lowerVertex.y = -1.0f;
		lowerVertex.z = -sinf(theta);
		cylinderVertices.push_back(lowerVertex);
	}
}

void display() {

	//Colour and Z-buffer clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Camera positionning
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(-5, 0, 0, 0, 0, 0, 0, 0, 1);
	
	//Axis scaling
	glPushMatrix();
	glScalef(0.4, 0.4, 0.4);
	glRotatef(angle[0], 1, 0, 0);
	glRotatef(angle[1], 0, 1, 0);
	glRotatef(angle[2], 0, 0, 1);
	glTranslatef(0, -0.5, -3.5);

	//Baby drawing
	glBegin(GL_TRIANGLES);
	glBindTexture(GL_TEXTURE_2D, 5);
	glColor3f(0.85, 0.75, 0.75);

	for (int i = 0; i != babyVertices.size(); i++) {

		vec2 textureCoordinate = babyTextureCoordinates.at(i);
		vec3 vertex = babyVertices.at(i);
		//glTexCoord2f(textureCoordinate.x, textureCoordinate.y);  
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Axis scaling
	glPopMatrix();
	glScalef(1.5, 2.0, 1.5);

	//Cylinder drawing
	glBegin(GL_TRIANGLE_STRIP);
	glColor3f(1.0, 1.0, 1.0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//glColor3f(1.0, 0.75, 0.75);

	for (int i = 0; i != cylinderVertices.size(); i++) {

		vec2 textureCoordinate = cylinderTextureCoordinates.at(i);
		vec3 vertex = cylinderVertices.at(i);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y); glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	glFlush();
}

void init()
{
	//Background colour definition
	glClearColor(1.0, 0.85, 0.85, 1.0);

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
	//glBindTexture(GL_TEXTURE_2D, textureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

	//Baby model loading
	ModelReader::getInstance()->readFile("..//data//baby_original_triangles.ply", babyVertices, babyNormals, babyTextureCoordinates);

	//Cylinder creation
	createCylinder(50);
}

void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	
	case '1':
		angle[0] -= 5;
		break;

	case '3':
		angle[0] += 5;
		break;

	case '4':
		angle[1] -= 5;
		break;

	case '6':
		angle[1] += 5;
		break;

	case '7':
		angle[2] -= 5;
		break;

	case '9':
		angle[2] += 5;
		break;
	}

	display();
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