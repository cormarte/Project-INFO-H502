#include <glm/glm.hpp>;
#include <vector>;

class MeshFilesReader {

public:

	MeshFilesReader();
	~MeshFilesReader();

private:
	std::vector<glm::vec3> vertices;
};