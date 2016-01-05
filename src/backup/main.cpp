#define _USE_MATH_DEFINES

#include <windows.h>

#include <chrono>
#include <iostream>
#include <math.h>
#include <thread>

#include <glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <glut.h>

#include "ModelReader.h"
#include "Shaders.h"
#include "TextureReader.h"

using namespace std;


//Window
static int g_window;

//Textures
GLuint textureIDs[2];
const char* texturePaths[2] = {"..//data//uterus_texture.jpg", "..//data//uterus_bump.jpg"};

//Model
vector<vec3> babyNormals;
vector<vec3> babyVertex;

vector<vec3> uterusNormals;
vector<vec3> uterusTangents;
vector<vec2> uterusTextureCoordinates;
vector<vec3> uterusVertex;

vector<vec3> bezierPoints;

//Shaders
GLuint uterusShaderProgram, babyShaderProgram;
GLuint uterusNormalOrientation_GLSL;
GLuint uterusTangent_GLSL;

//Angles
float babyAngles[3];
float uterusAngles[3] = {0, 0, 0};

//Translations
float babyTranslations[3] = {0, 0, 0};
float uterusTranslations[3];

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

	/*Defines vertex of a cylinder of radius 1 and height 2 centered at origin for a given resolution.
	Cylinder axis is along the x-direction w.r.t. OpenGL coordinates system. The vertex enumeration a
	lternates both upper and lower vertex for each value of the angle parameter theta, so that 
	GL_TRIANGLE_STRIP can be applied on consecutive vertex to render the cylinder.
	
	Vertex are defined in a way that y coordinate is always negative for theta belongin to [0, pi],
	so that the first half part of the uterusTextureCoordinates array describes the horizontal lower 
	half of the cylinder with OpenGL coordinates system convention and a camera palced along the z-axis*/


	for (int i = 0; i <= resolution; i++) {

		float theta = (float)i*((2*M_PI)/(float)resolution);

		vec3 normal;
		normal.x = 0.0f;
		normal.y = -sinf(theta);
		normal.z = -cosf(theta);
		uterusNormals.push_back(normal); //Normals are stored only once for both upper and lower vertex

		vec3 tangent = normalize(cross(normal, vec3(-1, 0, 0)));
		uterusTangents.push_back(tangent); //Tangents are perpendicular to normals and to cylinder axis

		vec2 upperTextureCoordinate;
		upperTextureCoordinate.x = (float)i*(1.0f/resolution);
		upperTextureCoordinate.y = 0.0f; //The left part of the texture is mapped on the upper part of the cylinder
		uterusTextureCoordinates.push_back(upperTextureCoordinate);

		vec2 lowerTextureCoordinate;
		lowerTextureCoordinate.x = (float)i*(1.0f/resolution);
		lowerTextureCoordinate.y = 1.0f; //The right part of the texture is mapped on the lower part of the cylinder
		uterusTextureCoordinates.push_back(lowerTextureCoordinate);

		vec3 upperVertex;
		upperVertex.x = 1.0; 
		upperVertex.y = -sinf(theta);
		upperVertex.z = -cosf(theta);
		uterusVertex.push_back(upperVertex);

		vec3 lowerVertex;
		lowerVertex.x = -1.0;
		lowerVertex.y = -sinf(theta);
		lowerVertex.z = -cosf(theta);
		uterusVertex.push_back(lowerVertex);
	}
}

void display() {

	/*Display function*/

	//Color and z-buffer clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH)) / glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

	//Camera positionning
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(cameraPosition[0], cameraPosition[1], cameraPosition[2], 0, 0, 0, 0, 1, 0);
	
	//Translation of both baby and uterus, so that the result is equivalent to a camera translation in the opposite direction 
	glTranslatef(uterusTranslations[0], uterusTranslations[1], uterusTranslations[2]);

	//Rotation of both baby and uterus, so that the result is equivalent to a camera rotation in the opposite direction
	glRotatef(uterusAngles[0], 1, 0, 0);
	glRotatef(uterusAngles[1], 0, 1, 0);
	glRotatef(uterusAngles[2], 0, 0, 1);

	//Push the current state of the matrix
	glPushMatrix();


	//Baby translation
	/*glTranslatef(babyTranslations[0], babyTranslations[1], babyTranslations[2]);

	//Baby rotation
	glRotatef(babyAngles[0], 1, 0, 0);
	glRotatef(babyAngles[1], 0, 1, 0);
	glRotatef(babyAngles[2], 0, 0, 1);

	//Baby drawing
	glUseProgram(babyShaderProgram);

	glBegin(GL_TRIANGLES);

	for (int i = 0; i != babyVertex.size(); i++) {

		vec3 normal = babyNormals.at(i);
		glNormal3f(normal.x, normal.y, normal.z);

		vec3 vertice = babyVertex.at(i);
		glVertex3f(vertice.x, vertice.y, vertice.z);
	}

	glEnd();


	//Pop the last state of the matrix
	glPopMatrix();

	//Uterus axis scaling*/
	glScalef(18.0, 16.0, 16.0);
	glUseProgram(babyShaderProgram);

	/*//Uterus caps drawing
	for (int i = 0; i != 2; i ++) {
	
		glBegin(GL_TRIANGLE_FAN);

		float y = pow(-1, i); //Takes value in [-1,1] to discribe lower and upper cap

		vec3 normal = normalize(vec3(y, 0.0, 0.0)); //All normals of the cap point outwards along the x-axis
		vec3 tangent = normalize(cross(normal, vec3(0.0, 0.0, 1.0))); //All tangents of the caps are perpendicular to both normals and z-axis

		//Cylinder half caps*/
		/*Uterus body is made of a half cylinder and a bezier surface, so does the caps*/
		/*glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]); //Tangent is needed for bump mapping
		glTexCoord2f(0.5, 0.5);  //Central pixel of the texture
		glVertex3f(y, 0.0, 0.0); //Central point of the cap
		
		for (int j = 0; j < uterusVertex.size()/4+2; j += 2) { //Iterate on half vertex of half the array

			vec3 vertex = uterusVertex.at(2 * j + i); //Upper and lower vertex are alternated
			vec2 textureCoordinates;*/
			/*y and z vertex coordinates describe a circle of radius 1, so they can be used to define a circle
			on the texture to be mapped on the vertex. Radius in x direction need to be scaled because texture
			width is greater than its height.*/
			/*textureCoordinates.x = 0.5 + (36.0/(2*M_PI*16)) * 0.5 * vertex.y;
			textureCoordinates.y = 0.5 + 0.5 * vertex.z;

			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinates.s, textureCoordinates.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		//Bezier half capes
		vec3 point1 = vec3(y, 0.0, 1.0); //Bezier points a redifined here because thee caps don't move with the upper surface
		vec3 point2 = vec3(y, 1.3333, 1.0);
		vec3 point3 = vec3(y, 1.3333, -1.0);
		vec3 point4 = vec3(y, 0.0, -1.0);

		for (int j = 0; j != 101; j++) {

			float u = j / 100.0f;
			vec3 vertex = (-pow(u, 3) + 3*pow(u, 2) - 3*u + 1)*point1 + (3*pow(u, 3) - 6*pow(u, 2) + 3*u)*point2 + (-3*pow(u, 3) + 3*pow(u, 2))*point3 + pow(u, 3)*point4;

			vec2 textureCoordinates;*/
			/*y and z vertex coordinates describe a circle of radius 1, so they can be used to define a circle
			on the texture to be mapped on the vertex. Radius in x direction need to be scaled because texture
			width is greater than its height.*/
			/*textureCoordinates.x = 0.5 + (36.0/(2*M_PI*16)) * 0.5 * vertex.y;
			textureCoordinates.y = 0.5 + 0.5 * vertex.z;

			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinates.s, textureCoordinates.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		glEnd();
	}*/

	//Cylinder body drawing

	//Non-Bezier half
	/*Uterus body is made of a half cylinder and a bezier surface*/
	glBegin(GL_TRIANGLE_STRIP);
	
	for (int i = 0; i != uterusVertex.size()/2+1; i++) { //Iterate on half the array

		vec3 normal = uterusNormals.at(i/2);
		vec3 tangent = uterusTangents.at(i/2);
		vec2 textureCoordinate = uterusTextureCoordinates.at(i);
		vec3 vertex = uterusVertex.at(i);
		
		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	//Bezier half

	vector<vec3> points;

	for (int i = 0; i != 21; i++) {

		float u = i / 20.0f;
		vec3 vertex = (-pow(u, 3) + 3*pow(u, 2) - 3*u + 1)*bezierPoints[0] + (3*pow(u, 3) - 6*pow(u, 2) + 3*u)*bezierPoints[1] + (-3*pow(u, 3) + 3*pow(u, 2))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];
		points.push_back(vec3(-1.0, vertex.y, vertex.z));
		points.push_back(vec3(1.0, vertex.y, vertex.z));
	}

	glPointSize(4.0f);

	glBegin(GL_POINTS);

	for (int i = 0; i != points.size(); i++) {

		vec3 pt1 = points.at(i);
		glVertex3f(pt1.x, pt1.y, pt1.z);
	}

	glEnd();

	glUseProgram(uterusShaderProgram);

	glBegin(GL_LINES);

	for (int i = 0; i != points.size()/2-1; i++) {

		vec3 pt1 = points.at(2 * i);
		vec3 pt2 = points.at(2 * i + 2);
		glVertex3f(pt1.x, pt1.y, pt1.z);
		glVertex3f(pt2.x, pt2.y, pt2.z);
	}

	for (int i = 0; i != points.size() / 2 - 1; i++) {

		vec3 pt1 = points.at(2 * i + 1);
		vec3 pt2 = points.at(2 * i + 3);
		glVertex3f(pt1.x, pt1.y, pt1.z);
		glVertex3f(pt2.x, pt2.y, pt2.z);
	}

	for (int i = 0; i != points.size() - 2; i++) {

		vec3 pt1 = points.at(i);
		vec3 pt2 = points.at(i+1);
		glVertex3f(pt1.x, pt1.y, pt1.z);
		glVertex3f(pt2.x, pt2.y, pt2.z);
	}

	glEnd();

	

	//Normals drawing (debug)
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

void glInit() {

	/*OpenGL initialization*/

	//White background
	glClearColor(1.0, 1.0, 1.0, 1.0);

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
		GLubyte* texture = TextureReader::getInstance()->readFile(texturePaths[i], width, height);

		glBindTexture(GL_TEXTURE_2D, textureIDs[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture);

		delete texture;
	}

	//Baby model loading
	ModelReader::getInstance()->readFile("..//data//baby_model.ply", babyVertex, babyNormals);

	//Cylinder creation
	createCylinder(200);

	//Bezier control points
	bezierPoints.push_back(vec3(0.0, 0.0, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, -1.0));
	bezierPoints.push_back(vec3(0.0, 0.0, -1.0));

	//Light definition
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

	uterusNormalOrientation_GLSL = glGetUniformLocation(uterusShaderProgram, "orientation");
	glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);	

	uterusTangent_GLSL = glGetAttribLocation(uterusShaderProgram, "gl_Tangent");

	glActiveTexture(GL_TEXTURE0); //Color texture
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int texture_GLSL = glGetUniformLocation(uterusShaderProgram, "texture");
	glUniform1i(texture_GLSL, 0);

	glActiveTexture(GL_TEXTURE1); //Bump map
	glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	int bumpMap_GLSL = glGetUniformLocation(uterusShaderProgram, "bumpMap");
	glUniform1i(bumpMap_GLSL, 1);

	//Babyshader
	vertexShader = initshaders(GL_VERTEX_SHADER, "BabyShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "BabyShader.fp");
	babyShaderProgram = initprogram(vertexShader, fragmentShader);

	glActiveTexture(GL_TEXTURE0); //Uterus color texture used for reflection
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int environment_GLSL = glGetUniformLocation(babyShaderProgram, "environment");
	glUniform1i(environment_GLSL, 0);
}

void animate(); //Defined at the end of the file

void keyboardFunc(unsigned char key, int x, int y) {

	/*Keyboard function*/

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

		if (cameraPosition[2] > 16.95 && cameraPosition[2] < 30) {

			cameraPosition[2] = 16.95;
			normalOrientation = -1;
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'x':
		cameraPosition[2] *= 1.1;

		if (cameraPosition[2] > 16.95 && cameraPosition[2] < 30) {

			cameraPosition[2] = 30;
			normalOrientation = -1;
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'c':
		fovy -= 1;
		break;

	case 'v':
		fovy += 1;
		break;

	case 'b':
		bezierPoints[0] += vec3(0.0, 0.05, -0.05);
		break;

	case 'n':
		bezierPoints[0] -= vec3(0.0, 0.05, -0.05);
		break;

	case 'm':
		normalOrientation = -normalOrientation;
		glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		break;

	case 'p':
		animate();
		break;
	}

	display();
}

void mouseFunc(int button, int state, int x, int y) {

	/*Mouse function*/

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
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		display();
	}

	else if (button == GLUT_WHEEL_DOWN) {

		cameraPosition[2] *= 1.1;

		if (cameraPosition[2] > 16.85 && cameraPosition[2] < 34) {

			cameraPosition[2] = 34;
			normalOrientation = 1;
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		display();
	}
}

void motionFunc(int x, int y) {

	/*Motion function*/

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

void reshapeFunction(int w, int h) {

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

	glewInit();
	glInit();

	glutDisplayFunc(display);
	glutReshapeFunc(reshapeFunction);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);

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
		{ 15.0, -75.0, 0.0, 30.0, 90.0, 31.5, 0.0, 10.5064, 17.0181, 0.775, 0.225, 1.0, 25.0, 130.0, 200.0, 0.375, -0.15, -0.9, 0.0, 0.0, 0.0, 0.0, 0.0 } 
	
	};

	for (float* p : parameters) {

		for (int i = 0; i != 3; i++) {
		
			uterusAngles[i] = p[i];
			babyAngles[i] = p[3+i];
			babyTranslations[i] = p[6+i];
		}

		bezierPoints[0] = vec3(0.0, p[9], p[10]);

		glUniform1i(uterusNormalOrientation_GLSL, p[11]);

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
	}
}