#include <windows.h>
#include <gl\GL.h>
#include <glm.hpp>
#include <vector>

using namespace glm;
using namespace std;

class ModelReader {

public:

	static ModelReader* getInstance();

	void readFile(char* path, vector<vec3> &vertices, vector<vec3> &normals);

private:

	ModelReader();
	~ModelReader();

	static ModelReader* instance;
};