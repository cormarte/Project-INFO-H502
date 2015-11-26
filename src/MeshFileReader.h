#include <windows.h>
#include <gl\GL.h>
#include <glm\glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

class MeshFileReader {

public:

	static MeshFileReader* getInstance();

	void readFile(const char* path, vector<vec3> &vertices);
	void readFile(const char* path, vector<vec3> &vertices, vector<vec2> &uvs, vector<vec3> &normals);
	void readFile(const char* path, vector<vec4> &vertices, vector<vec3> &normals, vector<GLushort> &elements);

private:

	MeshFileReader();
	~MeshFileReader();

	static MeshFileReader* instance;
};