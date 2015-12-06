#define _USE_MATH_DEFINES
#include <windows.h>
#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>
#include <iostream>
#include <math.h>
#include "ModelReader.h"
#include "Shaders.h"
#include "TextureReader.h"

#include <FreeImage.h>

using namespace std;

static int g_window;

//Textures
GLuint textureIDs[2];
const char* paths[2] = { "..//data//uterus_texture.jpg", "..//data//uterus_bump.jpg" };

//Model
vector<vec3> babyNormals;
vector<vec2> babyTextureCoordinates;
vector<vec3> babyVertices;

vector<vec3> cylinderNormals;
vector<vec2> cylinderTextureCoordinates;
vector<vec3> cylinderVertices;

vector<vec3> bezierPoints;

//Shaders
GLuint uterusShaderProgram, babyShaderProgram;

//Lightning
const float lightPosition[3] = {0, 10, 10};
GLuint uterusLightPosition_GLSL, babyLightPosition_GLSL;

const float ambient = 0.1;
float uterusAmbient_GLSL, babyAmbient_GLSL;

//Angles
int babyAngles[3] = {0, 0, 0};
int uterusAngles[3] = {0, 0, 0};

//Zoom
float zoomFactor = 1;

void createCylinder(int resolution) {

	for (int i = 0; i <= resolution; i++) {

		float theta = (float)i*((M_PI) / (float)resolution);

		vec3 normal;
		normal.x = cosf(theta);
		normal.y = 0.0f;
		normal.z = -sinf(theta);
		cylinderNormals.push_back(normal);
		cylinderNormals.push_back(normal);

		vec2 upperTextureCoordinate;
		upperTextureCoordinate.x = (float)i*(0.5f/resolution);
		upperTextureCoordinate.y = 1.0f;
		cylinderTextureCoordinates.push_back(upperTextureCoordinate);

		vec2 lowerTextureCoordinate;
		lowerTextureCoordinate.x = (float)i*(0.5f/resolution);
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
	gluLookAt(-40*zoomFactor, 0, 0, 0, 0, 0, 0, 0, 1);
	
	//Axis scaling
	glRotatef(uterusAngles[0], 1, 0, 0);
	glRotatef(uterusAngles[1], 0, 1, 0);
	glRotatef(uterusAngles[2], 0, 0, 1);
	glPushMatrix();
	glScalef(2.5, 2.5, 2.5);
	glRotatef(babyAngles[0], 1, 0, 0);
	glRotatef(babyAngles[1], 0, 1, 0);
	glRotatef(babyAngles[2], 0, 0, 1);
	glTranslatef(0.0, 0.0, -5.0);

	//Baby drawing
	glUseProgram(babyShaderProgram);
	glBegin(GL_TRIANGLES);

	for (int i = 0; i != babyVertices.size(); i++) {

		vec3 normal = babyNormals.at(i);
		glNormal3f(normal.x, normal.y, normal.z);

		vec3 vertex = babyVertices.at(i);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Axis scaling
	glPopMatrix();
	glScalef(14.0, 14.0, 14.0);
	
	//Half cylinder drawing
	glUseProgram(uterusShaderProgram);
	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i = 0; i != cylinderVertices.size(); i++) {

		vec3 normal = cylinderNormals.at(i);
		glNormal3f(normal.x, normal.y, normal.z);

		vec2 textureCoordinate = cylinderTextureCoordinates.at(i);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y);

		vec3 vertex = cylinderVertices.at(i);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Bezier drawing
	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i = 0; i != 101; i++) {

		float u = i / 100.0f;
		vec3 point = (1 - 3 * u + 3 * pow(u, 2) - pow(u, 3))*bezierPoints[0] + (3 * u - 6 * pow(u, 2) + 3 * pow(u, 3))*bezierPoints[1] + (3 * pow(u, 2) - 3 * pow(u, 3))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];	
		vec3 tangent = (-3 * pow(u, 2) + 6 * u - 3)*bezierPoints[0] + (9 * pow(u, 2) - 12 * u + 3)*bezierPoints[1] + (-9 * pow(u, 2) + 6 * u)*bezierPoints[2] + (3 * pow(u, 2))*bezierPoints[3];
		vec3 normal = cross(tangent, vec3(0, 1, 0));
		
		glVertex3f(point.x, point.y - 1, point.z);
		glNormal3f(normal.x, normal.y, normal.z);
		glTexCoord2f(0.5f + u / 2, 1.0f);

		glVertex3f(point.x, point.y + 1, point.z);
		glNormal3f(normal.x, normal.y, normal.z);
		glTexCoord2f(0.5f + u / 2, 0.0f);
	}

	glEnd();

	//Normals drawing
	/*glBegin(GL_LINES);

	for (int i = 0; i != cylinderVertices.size(); i++) {

		vec3 v = cylinderVertices[i];
		vec3 n = normalize(cylinderNormals[i]);
		vec3 pt = v + n;

		glVertex3f(v.x, v.y, v.z);
		glVertex3f(pt.x, pt.y, pt.z);
	}

	for (int i = 0; i != 101; i++) {

		float u = i / 100.0f;
		vec3 v = (1 - 3 * u + 3 * pow(u, 2) - pow(u, 3))*bezierPoints[0] + (3 * u - 6 * pow(u, 2) + 3 * pow(u, 3))*bezierPoints[1] + (3 * pow(u, 2) - 3 * pow(u, 3))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];
		vec3 t = (-3 * pow(u, 2) + 6 * u - 3)*bezierPoints[0] + (9 * pow(u, 2) - 12 * u + 3)*bezierPoints[1] + (-9 * pow(u, 2) + 6 * u)*bezierPoints[2] + (3 * pow(u, 2))*bezierPoints[3];
		vec3 n = cross(t, vec3(0, 1, 0));
		vec3 pt1 = v + vec3(0, -1, 0) + normalize(n);
		vec3 pt2 = v + vec3(0, 1, 0) + normalize(n);

		glVertex3f(v.x, v.y - 1, v.z);
		glVertex3f(pt1.x, pt1.y, pt1.z);

		glVertex3f(v.x, v.y + 1, v.z);
		glVertex3f(pt2.x, pt2.y, pt2.z);
	}

	glEnd();*/
	
	glFlush();
}

void init()
{
	//Glew initialization
	glewInit();

	//Background colour definition
	glClearColor(1.0, 1.0, 1.0, 1.0);

	//Shading mode definition
	glShadeModel(GL_SMOOTH);

	//Projection definition
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(70, (1.0*glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

	//Z-buffer activation
	glEnable(GL_DEPTH_TEST);

	//Texture loading
	glEnable(GL_TEXTURE_2D);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(2, textureIDs);
	
	for (int i = 0; i != 2; i++) {

		int width, height;
		GLubyte* texture = TextureReader::getInstance()->readFile(paths[i], width, height);

		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

		delete texture;
	}

	//Baby model loading
	ModelReader::getInstance()->readFile("..//data//baby_model.ply", babyVertices, babyNormals, babyTextureCoordinates);

	//Cylinder creation
	createCylinder(50);

	//Bezier initialization
	bezierPoints.push_back(vec3(-1.0, 0.0, 0.0));
	bezierPoints.push_back(vec3(-1.1, 0.0, 1.3333));
	bezierPoints.push_back(vec3(0.9, 0.0, 1.3333));
	bezierPoints.push_back(vec3(1.0, 0.0, 0.0));

	//Uterus Shader
	GLuint vertexShader, fragmentShader;

	vertexShader = initshaders(GL_VERTEX_SHADER, "UterusShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "UterusShader.fp");
	uterusShaderProgram = initprogram(vertexShader, fragmentShader);

	uterusAmbient_GLSL = glGetUniformLocation(uterusShaderProgram, "ambient");
	glUniform1f(uterusAmbient_GLSL, ambient);

	uterusLightPosition_GLSL = glGetUniformLocation(uterusShaderProgram, "lightPosition");
	glUniform3fv(uterusLightPosition_GLSL, 1, &lightPosition[0]);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int texture_GLSL = glGetUniformLocation(uterusShaderProgram, "texture");
	glUniform1i(texture_GLSL, 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	int bumpMap_GLSL = glGetUniformLocation(uterusShaderProgram, "bumpMap");
	glUniform1i(bumpMap_GLSL, 1);

	//Babyshader
	vertexShader = initshaders(GL_VERTEX_SHADER, "BabyShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "BabyShader.fp");
	babyShaderProgram = initprogram(vertexShader, fragmentShader);

	babyAmbient_GLSL = glGetUniformLocation(babyShaderProgram, "ambient");
	glUniform1f(babyAmbient_GLSL, ambient);

	babyLightPosition_GLSL = glGetUniformLocation(babyShaderProgram, "lightPosition");
	glUniform3fv(babyLightPosition_GLSL, 1, &lightPosition[0]);
}

void keyboard(unsigned char key, int x, int y) {

	switch (key) {

	case 'a':
		babyAngles[0] -= 5;
		break;

	case 'z':
		babyAngles[0] += 5;
		break;

	case 'e':
		babyAngles[1] -= 5;
		break;

	case 'r':
		babyAngles[1] += 5;
		break;

	case 't':
		babyAngles[2] -= 5;
		break;

	case 'y':
		babyAngles[2] += 5;
		break;

	case 'q':
		uterusAngles[0] -= 5;
		break;

	case 's':
		uterusAngles[0] += 5;
		break;

	case 'd':
		uterusAngles[1] -= 5;
		break;

	case 'f':
		uterusAngles[1] += 5;
		break;

	case 'g':
		uterusAngles[2] -= 5;
		break;

	case 'h':
		uterusAngles[2] += 5;
		break;

	case 'w':
		zoomFactor /= 1.1;
		break;

	case 'x':
		zoomFactor *= 1.1;
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