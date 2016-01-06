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
#include <type_ptr.hpp>

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
vector<vec3> babyVertices;

vector<vec3> uterusNormals;
vector<vec3> uterusTangents;
vector<vec2> uterusTextureCoordinates;
vector<vec3> uterusVertices;

vector<vec3> bezierPoints;

//Shaders
GLuint uterusShaderProgram, babyShaderProgram;
GLuint uterusNormalOrientation_GLSL;
GLuint uterusTangent_GLSL;
GLuint babyReflection_GLSL;

//Lighting
vector<vec4> lightPositions;
int normalOrientation;
float babyReflection;

//Camera
float cameraAngles[3];
float cameraTranslations[3];

//Baby
float babyAngles[3];
float babyTranslations[3];

//Perspective
float fovy;

//Bezier opening
float t;

//Trackball
int xOld = 0;
int yOld = 0;
bool leftButtonPressed = false;
bool rightButtonPressed = false;


void varInit() {

	/* Variables (re)-initialization */

	normalOrientation = -1;
	glUseProgram(uterusShaderProgram);
	glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);

	babyReflection = 0.35;
	glUseProgram(babyShaderProgram);
	glUniform1f(babyReflection_GLSL, babyReflection);

	for (int i=0; i != 3; i++) {
	
		cameraAngles[i] = 0;
		cameraTranslations[i] = 0;

		babyAngles[i] = 0;
		babyTranslations[i] = 0;
	}

	fovy = 75;

	t = 0.0;
	bezierPoints[0] = vec3(0.0, 0.0, 1.0)*(1.0f - t) + vec3(0.0, 0.85, 0.0)*t;
}


void createCylinder(int resolution) {

	/* Defines the vertices of a cylinder of radius 1 and height 2 centered at the origin for a given resolution.
	Cylinder axis is along the x-direction in OpenGL coordinates system. The vertices enumeration alternates 
	upper and lower vertices for each value of the angle parameter theta, so that GL_TRIANGLE_STRIP can be applied 
	on consecutive vertices to render the cylinder.
	
	Vertices are defined in a way that y coordinate is always negative for theta belongin to [0, pi],
	so that the first half part of the uterusTextureCoordinates array describes the horizontal lower 
	half of the cylinder with OpenGL coordinates system convention and a camera placed along the z-axis */


	for (int i = 0; i <= resolution; i++) {

		float theta = (float)i*((2*M_PI)/(float)resolution);

		vec3 normal;
		normal.x = 0.0f;
		normal.y = -sinf(theta);
		normal.z = -cosf(theta);

		//Normals are stored only once for both upper and lower vertices
		uterusNormals.push_back(normal);

		//Tangents are perpendicular to normals and to cylinder axis
		vec3 tangent = normalize(cross(normal, vec3(-1, 0, 0)));
		uterusTangents.push_back(tangent); 

		//The left part of the texture is mapped on the upper part of the cylinder
		vec2 upperTextureCoordinate;
		upperTextureCoordinate.s = (float)i*(1.0f/resolution);
		upperTextureCoordinate.t = 0.0f; 
		uterusTextureCoordinates.push_back(upperTextureCoordinate);

		//The right part of the texture is mapped on the lower part of the cylinder
		vec2 lowerTextureCoordinate;
		lowerTextureCoordinate.s = (float)i*(1.0f/resolution);
		lowerTextureCoordinate.t = 1.0f;
		uterusTextureCoordinates.push_back(lowerTextureCoordinate);

		vec3 upperVertex;
		upperVertex.x = 1.0; 
		upperVertex.y = -sinf(theta);
		upperVertex.z = -cosf(theta);
		uterusVertices.push_back(upperVertex);

		vec3 lowerVertex;
		lowerVertex.x = -1.0;
		lowerVertex.y = -sinf(theta);
		lowerVertex.z = -cosf(theta);
		uterusVertices.push_back(lowerVertex);
	}
}


void glInit() {

	/* OpenGL initialization */

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
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH)) / glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

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
	ModelReader::getInstance()->readFile("..//data//baby_model.ply", babyVertices, babyNormals);

	//Cylinder creation
	createCylinder(200);

	//Bezier control points
	bezierPoints.push_back(vec3(0.0, 0.0, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, 1.0));
	bezierPoints.push_back(vec3(0.0, 1.3333, -1.0));
	bezierPoints.push_back(vec3(0.0, 0.0, -1.0));

	//Light definition
	lightPositions.push_back(vec4(10.0f, 0.0f, 10.0f, 1.0f));
	lightPositions.push_back(vec4(0.0f, 10.0f, 0.0f, 1.0f));
	lightPositions.push_back(vec4(-10.0f, 0.0f, -10.0f, 1.0f));

	GLfloat ambient[4] = { 0.35f, 0.35f, 0.35f, 1.0f };
	GLfloat diffuse[4] = { 0.55f, 0.55f, 0.55f, 1.0f };
	GLfloat specular[4] = { 0.9f, 0.9f, 0.9f, 1.0f };

	for (int i = 0; i != lightPositions.size(); i++) {

		glLightfv(GL_LIGHT0+i, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0+i, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0+i, GL_SPECULAR, specular);
	}


	//***************//
	// Uterus Shader //
	//***************//

	GLuint vertexShader, fragmentShader;

	vertexShader = initshaders(GL_VERTEX_SHADER, "UterusShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "UterusShader.fp");
	uterusShaderProgram = initprogram(vertexShader, fragmentShader);

	glUseProgram(uterusShaderProgram);

	uterusNormalOrientation_GLSL = glGetUniformLocation(uterusShaderProgram, "orientation");
	glUniform1i(uterusNormalOrientation_GLSL, -1);

	uterusTangent_GLSL = glGetAttribLocation(uterusShaderProgram, "glTangent");

	//Color texture
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int texture_GLSL = glGetUniformLocation(uterusShaderProgram, "texture");
	glUniform1i(texture_GLSL, 0);

	//Bump map
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, textureIDs[1]);
	int bumpMap_GLSL = glGetUniformLocation(uterusShaderProgram, "bumpMap");
	glUniform1i(bumpMap_GLSL, 1);


	//*************//
	// Baby Shader //
	//*************//

	vertexShader = initshaders(GL_VERTEX_SHADER, "BabyShader.vp");
	fragmentShader = initshaders(GL_FRAGMENT_SHADER, "BabyShader.fp");
	babyShaderProgram = initprogram(vertexShader, fragmentShader);

	glUseProgram(babyShaderProgram);

	babyReflection_GLSL = glGetUniformLocation(babyShaderProgram, "reflection");
	glUniform1f(babyReflection_GLSL, 0.35);

	//Uterus color texture used for reflection
	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, textureIDs[0]);
	int environment_GLSL = glGetUniformLocation(babyShaderProgram, "environment");
	glUniform1i(environment_GLSL, 0);
}


void display() {

	/* Display function */

	//Color and z-buffer clearing
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (1.0*glutGet(GLUT_WINDOW_WIDTH)) / glutGet(GLUT_WINDOW_HEIGHT), 1, 1000);

	//Camera positionning
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 15, 0, 0, 0, 0, 1, 0);
	
	//Camera translations and zoom
	glTranslatef(cameraTranslations[0], cameraTranslations[1], cameraTranslations[2]);

	//Camera rotations
	glRotatef(cameraAngles[0], 1, 0, 0);
	glRotatef(cameraAngles[1], 0, 1, 0);
	glRotatef(cameraAngles[2], 0, 0, 1);

	//Pushes the current state of the matrix
	glPushMatrix();

	//Defines lights position according to the current state of the ModelView matrix, meaning that lights are
	//fixed in world space and the camera is rotating around the scene. Thus the following baby rotations in
	//baby model space won't affect light positionning in view space.
	glUseProgram(babyShaderProgram);

	for (int i = 0; i != lightPositions.size(); i++) {

		//OpenGL automatically transposes the light position in view space w.r.t. the current ModelView matrix
		glLightfv(GL_LIGHT0+i, GL_POSITION, (GLfloat*)&lightPositions[i][0]);
	}
	
	//Baby translations
	glTranslatef(babyTranslations[0], babyTranslations[1], babyTranslations[2]);

	//Baby rotations
	glRotatef(babyAngles[0], 1, 0, 0);
	glRotatef(babyAngles[1], 0, 1, 0);
	glRotatef(babyAngles[2], 0, 0, 1);


	//**************//
	// Baby drawing //
	//**************//
	
	glBegin(GL_TRIANGLES);

	for (int i = 0; i != babyVertices.size(); i++) {

		vec3 normal = babyNormals.at(i);
		glNormal3f(normal.x, normal.y, normal.z);

		vec3 vertex = babyVertices.at(i);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();


	//****************//
	// Uterus drawing //
	//****************//

	//Pop the last state of the matrix
	glPopMatrix();

	//Uterus axis scaling
	glScalef(18.0, 16.0, 16.0);
		
	//Defines lights position according to the current state of the ModelView matrix, meaning that lights are
	//fixed in world space and the camera is rotating around the scene.
	glUseProgram(uterusShaderProgram);

	for (int i = 0; i != lightPositions.size(); i++) {

		//OpenGL automatically transposes the light position in view space w.r.t. the current ModelView matrix
		glLightfv(GL_LIGHT0+i, GL_POSITION, (GLfloat*)&lightPositions[i][0]);
	}

	
	/////////////////////////
	// Uterus caps drawing //
	/////////////////////////

	for (int i = 0; i != 2; i ++) {
	
		glBegin(GL_TRIANGLE_FAN);

		//Takes value in [-1,1] to describe lower and upper cap
		float x = pow(-1, i); 

		//All normals of the cap point outwards along the x-axis
		vec3 normal = normalize(vec3(x, 0.0, 0.0));

		//All tangents of the caps are perpendicular to both normals and z-axis
		vec3 tangent = normalize(cross(normal, vec3(0.0, 0.0, -x*normalOrientation)));


		// Cylinder half caps //
		
		//Uterus body is made of a half cylinder and a bezier surface, so does the caps borders
		glNormal3f(normal.x, normal.y, normal.z);

		//Tangent is needed for bump mapping
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);

		//Central pixel of the texture is mapped on central point of the cap
		glTexCoord2f(0.5, 0.5);  
		glVertex3f(x, 0.0, 0.0);
		
		//Iterate on half of the vertices of half the array
		for (int j = 0; j < uterusVertices.size()/4+2; j += 2) { 

			//Upper and lower vertices are alternated
			vec3 vertex = uterusVertices.at(2*j+i); 
			vec2 textureCoordinate;

			//y and z vertices coordinates describe a circle of radius 1, so they can be used to define a circle
			//on the texture to be mapped on the vertex. Radius in s direction need to be scaled because texture
			//width is greater than its height.
			textureCoordinate.s = 0.5 + (36.0/(2*M_PI*16)) * 0.5 * vertex.y;
			textureCoordinate.t = 0.5 + 0.5 * vertex.z;

			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinate.s, textureCoordinate.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		// Bezier half caps //

		//Bezier points are redifined here because the caps don't move with the upper surface
		vec3 point1 = vec3(x, 0.0, 1.0); 
		vec3 point2 = vec3(x, 1.3333, 1.0);
		vec3 point3 = vec3(x, 1.3333, -1.0);
		vec3 point4 = vec3(x, 0.0, -1.0);

		for (int j = 0; j != 101; j++) {

			float u = j / 100.0f;
			vec3 vertex = (-pow(u, 3) + 3*pow(u, 2) - 3*u + 1)*point1 + (3*pow(u, 3) - 6*pow(u, 2) + 3*u)*point2 + (-3*pow(u, 3) + 3*pow(u, 2))*point3 + pow(u, 3)*point4;

			vec2 textureCoordinate;

			//y and z vertices coordinates describe a circle of radius 1, so they can be used to define a circle
			//on the texture to be mapped on the vertex. Radius in s direction need to be scaled because texture
			//width is greater than its height.*/
			textureCoordinate.s = 0.5 + (36.0/(2*M_PI*16)) * 0.5 * vertex.y;
			textureCoordinate.t = 0.5 + 0.5 * vertex.z;

			glNormal3f(normal.x, normal.y, normal.z);
			glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
			glTexCoord2f(textureCoordinate.s, textureCoordinate.t);
			glVertex3f(vertex.x, vertex.y, vertex.z);
		}

		glEnd();
	}

	/////////////////////////
	// Uterus body drawing //
	/////////////////////////

	// Non-Bezier half //

	//Uterus body is made of a half cylinder and a bezier surface
	glBegin(GL_TRIANGLE_STRIP);
	
	//Iterate on half the array
	for (int i = 0; i != uterusVertices.size()/2+1; i++) { 

		vec3 normal = uterusNormals.at(i/2);
		vec3 tangent = uterusTangents.at(i/2);
		vec2 textureCoordinate = uterusTextureCoordinates.at(i);
		vec3 vertex = uterusVertices.at(i);
		
		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(textureCoordinate.x, textureCoordinate.y);
		glVertex3f(vertex.x, vertex.y, vertex.z);
	}

	glEnd();

	// Bezier half //

	glBegin(GL_TRIANGLE_STRIP);

	for (int i = 0; i != 101; i++) {

		float u = i / 100.0f;

		//Tangent is the curve derivative
		vec3 tangent = normalize((-3*pow(u, 2) + 6*u - 3)*bezierPoints[0] + (9*pow(u, 2) - 12*u + 3)*bezierPoints[1] + (-9*pow(u, 2) + 6*u)*bezierPoints[2] + (3*pow(u, 2))*bezierPoints[3]);
		
		//Normal is perpendicular to both tangent and cylinder axis
		vec3 normal = cross(tangent, vec3(-1, 0, 0)); 
		
		//Bezier curve equation w.r.t. u
		vec3 vertex = (-pow(u, 3) + 3*pow(u, 2) - 3*u + 1)*bezierPoints[0] + (3*pow(u, 3) - 6*pow(u, 2) + 3*u)*bezierPoints[1] + (-3*pow(u, 3) + 3*pow(u, 2))*bezierPoints[2] + pow(u, 3)*bezierPoints[3];
		
		//Invert tangents to point in the same direction as the cylinder ones
		tangent *= -1.0f;

		glNormal3f(normal.x, normal.y, normal.z);
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);

		//Mapping begins at 0.5, where half cylinder mapping ended
		glTexCoord2f(0.5f + u/2, 1.0f); 
		glVertex3f(vertex.x-1, vertex.y, vertex.z);

		glNormal3f(normal.x, normal.y, normal.z);

		//Mapping begins at 0.5, where half cylinder mapping ended
		glVertexAttrib3fv(uterusTangent_GLSL, &tangent[0]);
		glTexCoord2f(0.5f + u/2, 0.0f);
		glVertex3f(vertex.x + 1, vertex.y, vertex.z);
	}

	glEnd();


	//*********************************//
	// Normals drawing (debug purpose) //
	//*********************************//

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


//Defined at the end of the file
void animate();


void keyboardFunc(unsigned char key, int x, int y) {

	/* Keyboard function */

	switch (key) {


	// Animation // 

	case 'a':
		varInit();
		animate();
		break;

	case 'A':
		varInit();
		animate();
		break;


	// Baby rotations //

	case 'x':
		babyAngles[0] += 5;
		break;

	case 'X':
		babyAngles[0] -= 5;
		break;

	case 'y':
		babyAngles[1] += 5;
		break;

	case 'Y':
		babyAngles[1] -= 5;
		break;

	case 'z':
		babyAngles[2] += 5;
		break;

	case 'Z':
		babyAngles[2] -= 5;
		break;


	// Camera translations //

	case 'l':
		cameraTranslations[0] += 2;
		break;

	case 'L':
		cameraTranslations[0] += 2;
		break;

	case 'r':
		cameraTranslations[0] -= 2;
		break;

	case 'R':
		cameraTranslations[0] -= 2;
		break;

	case 'u':
		cameraTranslations[1] -= 2;
		break;

	case 'U':
		cameraTranslations[1] -= 2;
		break;

	case 'd':
		cameraTranslations[1] += 2;
		break;

	case 'D':
		cameraTranslations[1] += 2;
		break;


	// Zoom //

	case 'i':

		cameraTranslations[2] += 0.1*(15 - cameraTranslations[2]);

		// Jumps from inside to outside the uterus, ensuring that the camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -1.85;

			//Inverse lighting
			normalOrientation = -1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'I':

		cameraTranslations[2] += 0.1*(15 - cameraTranslations[2]);

		// Jumps from inside to outside the uterus, ensuring that camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -1.85;

			//Inverse lighting
			normalOrientation = -1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'o':

		cameraTranslations[2] -= 0.1*(15 - cameraTranslations[2]);

		// Jumps from inside to outside the uterus, ensuring that the camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -19;

			//Inverse lighting
			normalOrientation = 1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;

	case 'O':

		cameraTranslations[2] -= 0.1*(15 - cameraTranslations[2]);

		// Jumps from inside to outside the uterus, ensuring that the camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -19;

			//Inverse lighting
			normalOrientation = 1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		break;


	//Camera rotations //

	case '1':
		cameraAngles[0] -= 5;
		break;

	case '3':
		cameraAngles[0] += 5;
		break;

	case '4':
		cameraAngles[1] -= 5;
		break;

	case '6':
		cameraAngles[1] += 5;
		break;

	case '7':
		cameraAngles[2] -= 5;
		break;

	case '9':
		cameraAngles[2] += 5;
		break;


	// FoV //

	case 'p':
		fovy -= 1;
		break;

	case 'P':
		fovy += 1;
		break;


	// Cylinder opening //

	case 'b':
		
		t += 0.01;

		if (t > 1.0) {

			t = 1.0;
		}

		//Linear intepolation of Bezier point P_0
		bezierPoints[0] = vec3(0.0, 0.0, 1.0)*(1.0f-t) + vec3(0.0, 0.85, 0.0)*t;
		break;

	case 'B':
		t -= 0.01;

		if (t < 0.0) {

			t = 0.0;
		}

		//Linear intepolation of Bezier point P_0
		bezierPoints[0] = vec3(0.0, 0.0, 1.0)*(1.0f - t) + vec3(0.0, 0.85, 0.0)*t;
		break;


	// Lighting inversion //

	case 'n':
		normalOrientation = -normalOrientation;
		glUseProgram(uterusShaderProgram);
		glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		break;

	case 'N':
		normalOrientation = -normalOrientation;
		glUseProgram(uterusShaderProgram);
		glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		break;


	// Variables reinitialization //

	case '0':
		varInit();
		break;
	}

	display();
}


void mouseFunc(int button, int state, int x, int y) {

	/* Mouse function */

	if (button == GLUT_LEFT_BUTTON) {

		if (state == GLUT_UP) {

			leftButtonPressed = false;
		}

		else {

			leftButtonPressed = true;

			//x and y position are stored for camera trackball
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

			//x and y position are kept for camera trackball
			xOld = x;
			yOld = y;
		}
	}

	else if (button == GLUT_WHEEL_UP) {
	
		cameraTranslations[2] += 0.1*(15 - cameraTranslations[2]);
	
		// Jumps from inside to outside the uterus, ensuring that the camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -1.85;

			//Inverse lighting
			normalOrientation = -1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		display();
	}

	else if (button == GLUT_WHEEL_DOWN) {

		cameraTranslations[2] -= 0.1*(15 - cameraTranslations[2]);

		// Jumps from inside to outside the uterus, ensuring that the camera doesn't pass through the uterus border
		if (cameraTranslations[2] < -1.85 && cameraTranslations[2] > -19) {

			cameraTranslations[2] = -19;

			//Inverse lighting
			normalOrientation = 1;
			glUseProgram(uterusShaderProgram);
			glUniform1i(uterusNormalOrientation_GLSL, normalOrientation);
		}

		display();
	}
}


void motionFunc(int x, int y) {

	/* Motion function for camera trackball */

	if (leftButtonPressed || rightButtonPressed) {

		float deltaX = x - xOld;
		float deltaY = y - yOld;

		if (leftButtonPressed) {

			cameraAngles[1] += 0.5*deltaX;
			cameraAngles[0] += 0.5*deltaY;
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

	/* Reshape function for new window size w x h */

	glViewport(0, 0, (GLsizei)w, (GLsizei)h); 

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fovy, (GLfloat)w / (GLfloat)h, 1, 1000);

	display();
}


int main(int argc, char *argv[]) {

	/* Main function */

	//Glut initialization and window setup
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(960, 540);
	glutInitWindowPosition(100, 100);
	g_window = glutCreateWindow("Baby Project");


	//Glew and OpenGL initialization
	glewInit();
	glInit();

	//Variables initialization
	varInit();

	//Glut callback functions
	glutDisplayFunc(display);
	glutReshapeFunc(reshapeFunction);
	glutKeyboardFunc(keyboardFunc);
	glutMouseFunc(mouseFunc);
	glutMotionFunc(motionFunc);

	//Glut main loop
	glutMainLoop();
	
	return 0;
}


void animate() {

	/* Animation function
	
	  The complete animation is divided into scenes of a given number of frames. A scene x begins with some 
	  rendering variables initialization whose values are stored in parameters[x][0] to parameters[x][5], 
	  allowing for example to change view point between scenes. Then, for each of the parameters[x][6] frames 
	  of the scene, some rendering variables are incremented by a constant value, stored in parameters[x][7] 
	  to parameters[x][14] and the display function is called. The complete rendering time of the frame is
	  measured and the thread finally sleeps for ((1/framerate)-rendering time) s to ensure the frame rate */

	int frameRate = 80;
	int frameDuration = (1.0 / frameRate) * 1000000000;

	//Allows baby to swing forwards and backwards
	int babyRotationDirection = 1;

	float parameters[13][15] = { 

        //   0	    1      2    3      4      5       6     7       8      9   10     11            12            13      14
		{  0.0,   0.0,   1.0, 0.35, 70.0, -110.0,  100.0,  0.0,    0.0,   0.0, 0.0,   0.0,          0.0,          0.0,    0.25   },
		{  0.0,   0.0,   1.0, 0.35, 70.0,  -55.0,  100.0,  0.0,    0.0,   0.0, 0.0,   0.0,          0.0,          0.0,    0.25   },
		{  0.0,  -90.0, -1.0, 0.35, 75.0,   -1.85, 900.0, -0.1,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0,    0.002  },
		{  0.0, -180.0, -1.0, 0.35, 75.0,   -0.05, 350.0,  0.0,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0,    0.0    },
		{  0.0, -180.0, -1.0, 0.35, 75.0,   -0.05, 150.0,  0.0,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0032, 0.0    },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -55.0,  160.0,  0.0,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0032, 0.0625 },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -45.0,  160.0,  0.0,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0,    0.0625 },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -35.0,  300.0,  0.0,    0.0,   0.0, 0.375, 0.0,          0.0,          0.0,    0.0    },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -35.0,   20.0,  0.0,    0.0,   0.0, 0.0,   0.0,          0.0,          0.0,    0.0    },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -35.0,  150.0,  0.0,    0.2,   0.6, 0.0,   0.0,          0.0,          0.0,    0.0    },
		{ 30.0,    0.0, -1.0, 0.35, 70.0,  -35.0,  100.0,  0.0,    0.0,   0.0, 0.0,   0.1*cosf(45), 0.1*sinf(45), 0.0,    0.0    },
		{ 15.0,  -75.0, -1.0, 0.20, 25.0, -115.0,  100.0,  0.0,    0.0,   0.0, 0.0,   0.1*cosf(45), 0.1*sinf(45), 0.0,    0.0    },
		{ 15.0,  -75.0, -1.0, 0.20, 25.0, -115.0,  200.0,  0.375, -0.15, -0.9, 0.0,   0.0,          0.0,          0.0,    0.0    }
	
	};

	for (float* p : parameters) {
		
		//Allows view point change
		cameraAngles[0] = p[0];
		cameraAngles[1] = p[1];
		
		//Normal orientation modification to switch lighting from inside to outside the uterus
		glUseProgram(uterusShaderProgram);
		glUniform1i(uterusNormalOrientation_GLSL, (int)p[2]);

		//Texture reflection is attenuated when the baby is out of the uterus
		glUseProgram(babyShaderProgram);
		glUniform1f(babyReflection_GLSL, p[3]);

		//Field of view correction to avoid/create artistic distortion
		fovy = p[4];

		//Setting initial scene zoom
		cameraTranslations[2] = p[5];
		
		for (int i = 0; i != p[6]; i++) {

			//Tick
			auto start = chrono::high_resolution_clock::now();
			
			//Camera rotations
			cameraAngles[1] += p[7];

			//Baby rotations
			babyAngles[0] += p[8];
			babyAngles[1] += p[9];
			babyAngles[2] += p[10] * babyRotationDirection;

			//Baby swings backwards and forwards between -10 and 35 degrees
			if (babyAngles[2] > 35 || babyAngles[2] < -10) {

				babyRotationDirection = -babyRotationDirection; 
			}

			//Baby translations
			babyTranslations[1] += p[11];
			babyTranslations[2] += p[12];

			//Bezier point P_0 linear interpolation for cylinder opening
			t += p[13];
			bezierPoints[0] = vec3(0.0, 0.0, 1.0)*(1.0f - t) + vec3(0.0, 0.85, 0.0)*t; 

			//Zoom in/out
			cameraTranslations[2] += p[14];

			display();

			//Tock
			auto stop = chrono::high_resolution_clock::now();

			//Sleep for ((1/framerate)-rendering time) s to ensure the frame rate
			this_thread::sleep_for(std::chrono::nanoseconds(frameDuration - std::chrono::duration<int, std::nano>(stop - start).count()));
		}
	}
}