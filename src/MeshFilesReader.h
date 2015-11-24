#include <glm/glm.hpp>;
#include <vector>;

class MeshFilesReader {

public:

	MeshFilesReader();
	~MeshFilesReader();

	void read(const char* path);
	void getVertices();
	void getUVs();
	void getNormals();

private:

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec2> uvs;
	std::vector<glm::vec3> normals;

};