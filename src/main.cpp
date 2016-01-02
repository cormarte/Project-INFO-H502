#define _USE_MATH_DEFINES
#include <windows.h>
#include <chrono>
#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
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
const char* paths[2] = { "..//data//uterus.jpg", "..//data//uterus_bump.jpg" };

//Model
vector<vec3> babyNormals;
vector<vec3> babyVertices;

vector<vec3> cylinderNormals;
vector<vec3> cylinderTangents;
vector<vec2> cylinderTextureCoordinates;
vector<vec3> cylinderVertices;

vector<vec3> bezierPoints;


//Shaders
GLuint uterusShaderProgram, babyShaderProgram;
GLuint normalOrientation_GLSL;
GLuint uterusTangent_GLSL;

//Angles
float babyAngles[3] = {0, 0, 0};
float uterusAngles[3] = {0, 0, 0};

//Translation
//float babyTranslations[3] = {0, 0, -5};
float babyTranslations[3] = {0, 0, 0};

//Zoom
float cameraPosition[3] = {0, 0, 15.0};

//Perspective
float fovy = 75;

//Redering
int normalOrientation = -1;

//Trackball
int xOld = 0;
int yOld = 0;
bool leftButtonPressed = false;
bool rightButtonPressed = false;

void createCylinder(int resolution) {

	for (int i = 0; i <= resolution; i++) {

		float theta = (float)i*((2*M_PI) / (float)resolution);

		vec3 normal;
		normal.x = 0.0f;
		normal.y = -sinf(theta);
		normal.z = -cosf(theta);
		
		cylinderNormals.push_back(normal);
		cylinderNormals.push_back(normal);

		vec3 tangent = normalize(cross(normal, vec3(0, 1, 0)));
		cylinderTangents.push_back(tangent);

		vec2 upperTextureCoordinate;
		upperTextureCoordinate.x = (float)i*(1.0f/resolution);
		upperTextureCoordinate.y = 0.0f;
		cylinderTextureCoordinates.push_back(upperTextureCoordinate);

		vec2 lowerTextureCoordinate;
		lowerTextureCoordinate.x = (float)i*(1.0f/resolution);
		lowerTextureCoordinate.y = 1.0f;
		cylinderTextureCoordinates.push_back(lowerTextureCoordinate);

		vec3 upperVertex;
		upperVertex.x = 1.0;
		upperVertex.y = -sinf(theta);
		upperVertex.z = -cosf(theta);
		cylinderVertices.push_back(upperVertex);

		vec3 lowerVertex;
		lowerVertex.x = -1.0;
		lowerVertex.y = -sinf(theta);
		lowerVertex.z = -cosf(theta);
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
	gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], 0, 0, 0, 0, 1, 0);
	
	//Rotation of both baby and uterus
	glRotatef(uterusAngles[0], 1, 0, 0);
	glRotatef(uterusAngles[1], 0, 1, 0);
	glRotatef(uterusAngles[2], 0, 0, 1);
	glPushMatrix();

	glTranslatef(babyTranslations[0], babyTranslations[1], babyTranslations[2]);
	glRotatef(babyAngles[0], 1, 0, 0);
	glRotatef(babyAngles[1], 0, 1, 0);
	glRotatef(babyAngles[2], 0, 0, 1);

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
	glScalef(18.0, 16.0, 16.0);
		
	glUseProgram(uterusShaderProgram);

	//Cylinder caps drawing
	for (int i = 0; i != 2; i ++) {
	
		glBegin(GL_TRIANGLE_FAN);

		float y = pow(-1, i);

		vec3 normal = normalize(vec3(y, 0.0, 0.0));
		vec3 tangent = normalize(cross(normal, vec3(0.0, 0.0, 1.0)));

		//Non-Bezier half
		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(0.5, 0.5);
		glVertex3f(y, 0.0, 0.0);
		
		for (int j = 0; j < cylinderVertices.size()/4+2; j += 2) {

			vec3 vertex = cylinderVertices.at(2 * j + i);
			vec2 textureCoordinates;
			textureCoordinates.x = 0.5 + (36.0 / (2 * M_PI * 16)) * 0.5 * vertex.y;
			textureCoordinates.y = 0.5 + 0.5 * vertex.z;

			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinates.s, textureCoordinates.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		//Bezier half
		vec3 point1 = vec3(y, 0.0, 1.0);
		vec3 point2 = vec3(y, 1.3333, 1.0);
		vec3 point3 = vec3(y, 1.3333, -1.0);
		vec3 point4 = vec3(y, 0.0, -1.0);

		for (int j = 0; j != 101; j++) {

			float u = j / 100.0f;
			vec3 vertex = (1 - 3 * u + 3 * pow(u, 2) - pow(u, 3))*point1 + (3 * u - 6 * pow(u, 2) + 3 * pow(u, 3))*point2 + (3 * pow(u, 2) - 3 * pow(u, 3))*point3 + pow(u, 3)*point4;

			vec2 textureCoordinates;
			textureCoordinates.x = 0.5 + (36.0 / (2 * M_PI * 16)) * 0.5 * vertex.y;
			textureCoordinates.y = 0.5 + 0.5 * vertex.z;


			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinates.s, textureCoordinates.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		glEnd();
	}

	//Cylinder body drawing
	//Non-Bezier half
	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i = 0; i != cylinderVertices.size()/2+1; i++) {

		vec3 normal = cylinderNormals.at(i);
		vec3 tangent = normalize(cross(normal, vec3(-1, 0, 0)));
		vec2 textureCoordinate = cylinderTextureCoordinates.at(i);
		vec3 vertex = cylinderVertices.at(i);
		
		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Bezier half
	glBegin(GL_TRIANGLE_STRIP);

	for (int i = 0; i != 101; i++) {

		float u = i / 100.0f;
		vec3 tangent = normalize((-3 * pow(u, 2) + 6 * u - 3)*bezierPoints[0] + (9 * pow(u, 2) - 12 * u + 3)*bezierPoints[1] + (-9 * pow(u, 2) + 6 * u)*bezierPoints[2] + (3 * pow(u, 2))*bezierPoints[3]);
		vec3 normal = cross(tangent, vec3(-1, 0, 0));
		vec3 vertex = (1 - 3 * u + 3 * pow(u, 2) - pow(u, 3))*bezierPoints[0] + (3 * u - 6 * pow(u, 2) + 3 * pow(u, 3))*bezierPoints[1] + (3 * pow(u, 2) - 3 * pow(u, 3))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];
		tangent *= -1.0f;

		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(0.5f + u / 2, 1.0f);
		glVertex3f(vertex.x - 1, vertex.y, vertex.z);

		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(0.5f + u / 2, 0.0f);
		glVertex3f(vertex.x + 1, vertex.y, vertex.z);
	}

	glEnd();

	//Normals drawing
	/*glBegin(GL_LINES);
	
	for (int i = 0; i != cylinderVertices.size()/2+1; i++) {

		vec3 v = cylinderVertices[i];
		vec3 n = normalize(cylinderNormals[i]);
		vec3 t = normalize(cross(n, vec3(-1, 0, 0)));
		vec3 pt = v + n;

		glVertex3f(v.x, v.y, v.z);
		glVertex3f(pt.x, pt.y, pt.z);
	}

	for (int i = 0; i != 101; i++) {

		float u = i / 100.0f;
		vec3 v = (1 - 3 * u + 3 * pow(u, 2) - pow(u, 3))*bezierPoints[0] + (3 * u - 6 * pow(u, 2) + 3 * pow(u, 3))*bezierPoints[1] + (3 * pow(u, 2) - 3 * pow(u, 3))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];
		vec3 t = (-3 * pow(u, 2) + 6 * u - 3)*bezierPoints[0] + (9 * pow(u, 2) - 12 * u + 3)*bezierPoints[1] + (-9 * pow(u, 2) + 6 * u)*bezierPoints[2] + (3 * pow(u, 2))*bezierPoints[3];
		vec3 n = cross(t, vec3(-1, 0, 0));
		vec3 pt1 = v + vec3(-1, 0, 0) + n;
		vec3 pt2 = v + vec3(1, 0, 0) + n;

		glVertex3f(v.x - 1, v.y, v.z);
		glVertex3f(pt1.x, pt1.y, pt1.z);

		glVertex3f(v.x + 1, v.y, v.z);
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

	//Z-buffer activation
	glEnable(GL_DEPTH_TEST);

	//Transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	//Projection definition
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH))/glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

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
	ModelReader::getInstance()->readFile("..//data//baby_model.ply", babyVertices, babyNormals);

	//Cylinder creation
	createCylinder(200);

	//Bezier initialization
	bezierPoints.push_back(vec3(0.0, 0.0, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, -1.0));
	bezierPoints.push_back(vec3(0.0, 0.0, -1.0));

	//Light definition
	/*GLfloat lightPosition[4] = {0.0f, 75.0f, 50.0f, 1.0f};
	GLfloat ambient[4] = {0.45f, 0.45f, 0.45f, 1.0f};
	GLfloat diffuse[4] = {0.55f, 0.55f, 0.55f, 1.0f};
	GLfloat specular[4] = {1.0f, 1.0f, 1.0f, 1.0f};*/

	GLfloat lightPosition[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	GLfloat ambient[4] = { 0.35f, 0.35f, 0.35f, 1.0f };
	GLfloat diffuse[4] = { 0.55f, 0.55f, 0.55f, 1.0f };
	GLfloat specular[4] = { 0.9f, 0.9f, 0.9f, 1.0f };

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

	//Uterus Shader
	GLuint vertexShader, fragmentShader;

	vertexShader = initshaders(GL_VERTEX_SHADER, "UterusShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "UterusShader.fp");
	uterusShaderProgram = initprogram(vertexShader, fragmentShader);

	uterusTangent_GLSL = glGetAttribLocation(uterusShaderProgram, "gl_Tangent");

	normalOrientation_GLSL = glGetUniformLocation(uterusShaderProgram, "orientation");
	glUniform1i(normalOrientation_GLSL, normalOrientation);	

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int environment_GLSL = glGetUniformLocation(babyShaderProgram, "environment");
	glUniform1i(environment_GLSL, 0);
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
		babyTranslations[1] += 0.25;
		break;

	case 'i':
		babyTranslations[1] -= 0.25;
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
		cameraPosition[2] /= 1.1;
		cout << "Cam: " << cameraPosition[2] << endl;

		if (cameraPosition[2] > 16.95 && cameraPosition[2] < 30) {

			cameraPosition[2] = 16.95;
			normalOrientation = -1;
			glUniform1i(normalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'x':
		cameraPosition[2] *= 1.1;
		cout << "Cam: " << cameraPosition[2] << endl;

		if (cameraPosition[2] > 16.95 && cameraPosition[2] < 30) {

			cameraPosition[2] = 30;
			normalOrientation = -1;
			glUniform1i(normalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'c':
		fovy -= 1;
		cout << "FOV: " << fovy << endl;
		break;

	case 'v':
		fovy += 1;
		cout << "FOV: " << fovy << endl;
		break;

	case 'b':
		bezierPoints[0] += vec3(0.0, 0.05, -0.05);
		//bezierPoints[1] -= vec3(0.01, 0, 0.01);
		break;

	case 'n':
		bezierPoints[0] -= vec3(0.0, 0.05, -0.05);
		//bezierPoints[1] += vec3(0.01, 0, 0.01);
		break;

	case 'm':
		normalOrientation = -normalOrientation;
		glUniform1i(normalOrientation_GLSL, normalOrientation);
		break;

	case 'p':
		animate();
		break;
	}

	display();
}

void mouse(int button, int state, int x, int y) {

	if (button == GLUT_LEFT_BUTTON) {

		if (state == GLUT_UP) {

			leftButtonPressed = false;
		}

		else {

			leftButtonPressed = true;
			xOld = x;
			yOld = y;
		}
	}

	else if (button == GLUT_RIGHT_BUTTON) {

		if (state == GLUT_UP) {

			rightButtonPressed = false;
		}

		else {
	
			rightButtonPressed = true;
			xOld = x;
			yOld = y;
		}
	}

	else if (button == GLUT_WHEEL_UP) {
	
		cameraPosition[2] /= 1.1;

		if (cameraPosition[2] > 16.85 && cameraPosition[2] < 34) {

			cameraPosition[2] = 16.85;
			normalOrientation = -1;
			glUniform1i(normalOrientation_GLSL, normalOrientation);
		}

		display();
	}

	else if (button == GLUT_WHEEL_DOWN) {

		cameraPosition[2] *= 1.1;

		if (cameraPosition[2] > 16.85 && cameraPosition[2] < 34) {

			cameraPosition[2] = 34;
			normalOrientation = 1;
			glUniform1i(normalOrientation_GLSL, normalOrientation);
		}

		display();
	}
}

void mouseMotion(int x, int y) {

	if (leftButtonPressed || rightButtonPressed) {

		float deltaX = x - xOld;
		float deltaY = y - yOld;


		if (leftButtonPressed) {

			uterusAngles[1] += 0.5*deltaX;
			uterusAngles[0] += 0.5*deltaY;
		}

		else if (rightButtonPressed) {

			babyAngles[1] += 0.5*deltaX;
			babyAngles[2] += 0.5*deltaY;
		}

		xOld = x;
		yOld = y;

		display();
	}
}

void reshape(int w, int h) {

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLfloat)w / (GLfloat)h, 1, 1000);

	display();
}

int main(int argc, char *argv[]) {

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(960, 540);
	glutInitWindowPosition(100, 100);
	g_window = glutCreateWindow("Baby Project");
	init();
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(mouseMotion);
	glutMainLoop();
	return 0;
}

void animate() {

	int frameRate = 60;
	int frameDuration = (1.0 / frameRate) * 1000;

	int babyRotationDirection = 1;

	float parameters[13][24] = { 
		
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 70.0, 125, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,  0.0, 0.0, -0.25 },
		{ 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, 1.0, 70.0, 70.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, -0.25 },
		{ 0.0, -90.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 75.0, 16.85, 900.0, -0.1, 0.0, 0.0, 0.375, 0.0, 0.0, 0.0, 0.0, -0.002 },
		{ 0.0, -180.0, 0.0, 0.0, 0.0, 5.25, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 75.0, 15.0497, 350.0, 0.0, 0.0, 0.0, 0.375, 0.0, 0.0, 0.0, 0.0, 0.0 },
		{ 0.0, -180.0, 0.0, 0.0, 0.0, 15.0, 0.0, 0.0, 0.0, 0.0, 1.0, -1.0, 75.0, 15.0497, 150.0, 0.0, 0.0, 0.0, 0.375, 0.0,  0.0, 0.0, 0.0025, 0.0 },
		{ 30.0, 0.0, 0.0, 0.0, 0.0, -0.75, 0.0, 0.0, 0.0, 0.375, 0.625, -1.0, 70.0, 70.0, 160.0, 0.0, 0.0, 0.0, 0.375, 0.0,  0.0, 0.0, 0.0025, -0.0625 },
		{ 30.0, 0.0, 0.0, 0.0, 0.0, 30.0, 0.0, 0.0, 0.0, 0.775, 0.225, -1.0, 70.0, 60.0, 160.0, 0.0, 0.0, 0.0, 0.375, 0.0, 0.0, 0.0, 0.0, -0.0625 },
		{ 30.0, 0.0, 0.0, 0.0, 0.0, 9.75, 0.0, 0.0, 0.0, 0.775, 0.225, -1.0, 70.0, 50.0, 300.0, 0.0, 0.0, 0.0, 0.375, 0.0, 0.0, 0.0, 0.0, 0.0 },
		{ 30.0, 0.0, 0.0, 0.0, 0.0, 31.5, 0.0, 0.0, 0.0, 0.775, 0.225, -1.0, 70.0, 50.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
		{ 30.0, 0.0, 0.0, 0.0, 0.0, 31.5, 0.0, 0.0, 0.0, 0.775, 0.225, -1.0, 70.0, 50.0, 150.0, 0.0, 0.2, 0.6, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 },
		{ 30.0, 0.0, 0.0, 30.0, 90.0, 31.5, 0.0, 0.0, 0.0, 0.775, 0.225, -1.0, 70.0, 50.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1*cosf(45), 0.1*sinf(45), 0.0, 0.0 },
		{ 15.0, -75.0, 0.0, 30.0, 90.0, 31.5, 0.0, 5.25322, 8.50903, 0.775, 0.225, 1.0, 25.0, 130.0, 100.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.1*cosf(45), 0.1*sinf(45), 0.0, 0.0 },
		{ 15.0, -75.0, 0.0, 30.0, 90.0, 31.5, 0.0, 10.5064, 17.0181, 0.775, 0.225, 1.0, 25.0, 130.0, 200.0, 0.375, -0.15, -0.9, 0.0, 0.0, 0.0, 0.0, 0.0 } };

	for (float* p : parameters) {

		for (int i = 0; i != 3; i++) {
		
			uterusAngles[i] = p[i];
			babyAngles[i] = p[3+i];
			babyTranslations[i] = p[6+i];
		}

		bezierPoints[0] = vec3(0.0, p[9], p[10]);

		glUniform1i(normalOrientation_GLSL, p[11]);

		fovy = p[12];
		cameraPosition[2] = p[13];
	
		for (int i = 0; i != p[14]; i++) {

			auto start = chrono::high_resolution_clock::now();

			uterusAngles[1] += p[15];
			babyAngles[0] += p[16];
			babyAngles[1] += p[17];
			babyAngles[2] += p[18] * babyRotationDirection;

			if (babyAngles[2] > 35 || babyAngles[2] < -10) {

				babyRotationDirection = -babyRotationDirection;
			}

			babyTranslations[0] += p[19];
			babyTranslations[1] += p[20];
			babyTranslations[2] += p[21];

			bezierPoints[0] += vec3(0, p[22], -p[22]);

			cameraPosition[2] += p[23];

			display();

			auto stop = chrono::high_resolution_clock::now();
			this_thread::sleep_for(std::chrono::milliseconds(frameDuration) - chrono::duration_cast<chrono::milliseconds>(stop - start));
		}

		cout << babyTranslations[1] << ", " << babyTranslations[2] << endl;
	}
}