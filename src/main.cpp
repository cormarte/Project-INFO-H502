#define _USE_MATH_DEFINES
#include <windows.h>
#include <chrono>
#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>
#include <iostream>
#include <math.h>
#include "ModelReader.h"
#include "Shaders.h"
#include "TextureReader.h"
#include <thread>

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
float babyAngles[3] = {0, 0, 0};
float uterusAngles[3] = {0, 0, 0};

//Translation
float babyTranslation[3] = {0, 0, 0};

//Zoom
float zoomFactor = 1.25;

//Perspective
int fovy = 70;

//Animation
bool animation = false;
float normalOrientation = 1.0;

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

	//Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH)) / glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

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
	glTranslatef(0.0, 0.0, -5.0+babyTranslation[2]);

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
	glScalef(16.0, 18.0, 16.0);
	
	//Half cylinder drawing
	glUseProgram(uterusShaderProgram);
	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i = 0; i != cylinderVertices.size(); i++) {

		vec3 normal = cylinderNormals.at(i)*normalOrientation;
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
		vec3 normal = cross(tangent, vec3(0, 1, 0))*normalOrientation;
		
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
	//glClearColor(0.2902, 0.0314, 0.0353, 1.0);

	//Shading mode definition
	glShadeModel(GL_SMOOTH);

	//Projection definition
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

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
	bezierPoints.push_back(vec3(-1.0, 0.0, 1.3333));
	bezierPoints.push_back(vec3(1.0, 0.0, 1.3333));
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

void animate();

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

	case 'u':
		babyTranslation[2] += 0.25;
		break;

	case 'i':
		babyTranslation[2] -= 0.25;
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

	case 'c':
		fovy -= 1;
		break;

	case 'v':
		fovy += 1;
		break;

	case 'b':
		bezierPoints[0] += vec3(0.05, 0, 0.05);
		//bezierPoints[1] -= vec3(0.01, 0, 0.01);
		break;

	case 'n':
		bezierPoints[0] -= vec3(0.05, 0, 0.05);
		//bezierPoints[1] += vec3(0.01, 0, 0.01);
		break;

	case 'p':
		animate();
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

void animate() {

	animation = true;
	int ms = (1.0 / 30) * 1000;

	for (int i = 0; i != 3; i++) {

		babyAngles[i] = 0;
		uterusAngles[i] = 0;
		babyTranslation[i] = 0;
	}

	bezierPoints[0] = vec3(-1.0, 0.0, 0.0);
	bezierPoints[1] = vec3(-1.0, 0.0, 1.3333);
	bezierPoints[2] = vec3(1.0, 0.0, 1.3333);
	bezierPoints[3] = vec3(1.0, 0.0, 0.0);

	zoomFactor = 2.5;
	fovy = 70;

	display();

	for (int i = 0; i != 100; i++) {

		auto start = chrono::high_resolution_clock::now();

		zoomFactor -= 0.005;
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	zoomFactor = 1.5;

	for (int i = 0; i != 100; i++) {

		auto start = chrono::high_resolution_clock::now();

		zoomFactor -= 0.005;
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	uterusAngles[2] = -90;
	display();

	zoomFactor = 0.65;
	normalOrientation = -1;
	int babyRotationDirection = 1;

	for (int i = 0; i != 350; i++) {

		auto start = chrono::high_resolution_clock::now();

		zoomFactor -= 0.0005;
		fovy += 0.0025;
		uterusAngles[2] -= 0.1;
		babyAngles[0] += 1.1*babyRotationDirection;

		if (babyAngles[0] > 35 || babyAngles[0] < -35) {

			babyRotationDirection = -babyRotationDirection;
		}
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	for (int i = 0; i != 275; i++) {

		auto start = chrono::high_resolution_clock::now();

		zoomFactor -= 0.00025;
		fovy += 0.01;
		uterusAngles[2] -= 0.2;
		babyAngles[0] += 1.1*babyRotationDirection;

		if (babyAngles[0] > 35 || babyAngles[0] < -35) {

			babyRotationDirection = -babyRotationDirection;
		}
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	for (int i = 0; i != 150; i++) {

		auto start = chrono::high_resolution_clock::now();

		babyAngles[0] += 1.1*babyRotationDirection;

		if (babyAngles[0] > 35 || babyAngles[0] < -35) {

			babyRotationDirection = -babyRotationDirection;
		}
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	for (int i = 0; i != 150; i++) {

		auto start = chrono::high_resolution_clock::now();

		bezierPoints[0] += vec3(0.0025, 0, 0.0025);
		babyAngles[0] += 1.1*babyRotationDirection;

		if (babyAngles[0] > 35 || babyAngles[0] < -35) {

			babyRotationDirection = -babyRotationDirection;
		}
		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}

	uterusAngles[1] = -30;
	uterusAngles[2] = 0;
	zoomFactor = 1.5;
	fovy = 70;

	for (int i = 0; i != 160; i++) {

		auto start = chrono::high_resolution_clock::now();

		zoomFactor -= 0.0025;
		bezierPoints[0] += vec3(0.0025, 0, 0.0025);
		babyAngles[0] += 1.1*babyRotationDirection;

		if (babyAngles[0] > 35 || babyAngles[0] < -35) {

			babyRotationDirection = -babyRotationDirection;
		}

		display();

		auto stop = chrono::high_resolution_clock::now();
		this_thread::sleep_for(std::chrono::milliseconds(ms) - chrono::duration_cast<chrono::milliseconds>(stop - start));
	}
}