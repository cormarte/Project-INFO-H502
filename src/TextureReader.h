#include <windows.h>

#include <gl\GL.h>

using namespace std;


class TextureReader {

public:

	static TextureReader* getInstance();

	GLubyte* readFile(const char* path, int &imageWidth, int &imageHeight);

private:

	TextureReader();
	~TextureReader();

	static TextureReader* instance;
};
