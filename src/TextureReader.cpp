#include <windows.h>

#include <gl\GL.h>
#include <FreeImage.h>

#include "TextureReader.h"


TextureReader* TextureReader::instance = 0;

TextureReader::TextureReader() {

}

TextureReader::~TextureReader() {

}

TextureReader* TextureReader::getInstance() {

	if (instance == 0) {

		instance = new TextureReader();
	}

	return instance;
}

GLubyte* TextureReader::readFile(const char* path, int &imageWidth, int &imageHeight) {

	/* Reads an image file, stores image width and height in the so-called variables and returns a GLubyte* pixel values array. */

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path, 0);
	FIBITMAP* image_tmp = FreeImage_Load(format, path);

	FIBITMAP* image = FreeImage_ConvertTo32Bits(image_tmp);
	FreeImage_Unload(image_tmp);

	imageWidth = FreeImage_GetWidth(image);
	imageHeight = FreeImage_GetHeight(image);

	GLubyte* texture = new GLubyte[4*imageWidth*imageHeight];
	char* pixels = (char*)FreeImage_GetBits(image);

	for (int j = 0; j<imageWidth*imageHeight; j++) {

		texture[j*4+0] = pixels[j*4+2];
		texture[j*4+1] = pixels[j*4+1];
		texture[j*4+2] = pixels[j*4+0];
		texture[j*4+3] = pixels[j*4+3];
	}

	return texture;
}