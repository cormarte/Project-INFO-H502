#include <windows.h>

#include <vector>

#include <gl\GL.h>
#include <glm.hpp>


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