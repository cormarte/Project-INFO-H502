#include <windows.h>
#include <gl\GL.h>
#include <glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

class ModelReader {

public:

	static ModelReader* getInstance();

	void readFile(char* path, vector<vec3> &vertices, vector<vec3> &normals, vector<vec2> &textureCoordinates);
	/*void readFile(const char* path, vector<vec3> &vertices, vector<vec2> &uvs, vector<vec3> &normals);
	void readFile(const char* path, vector<vec4> &vertices, vector<vec3> &normals, vector<GLushort> &elements);*/

private:

	ModelReader();
	~ModelReader();

	static ModelReader* instance;
};