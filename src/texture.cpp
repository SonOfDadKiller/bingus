#include "bingus.h"
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::string;

static const char* texture_path = "../res/textures/";

u32 LoadTexture(const char* file_path, i32 wrapMode)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	//Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	//Load texture from file
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	string full_path = string(texture_path) + file_path;
	unsigned char* textureData = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);

	//Detect format
	unsigned int imageFormat = 0;

	switch (nrChannels)
	{
	case 2: imageFormat = GL_RG; break;
	case 3: imageFormat = GL_RGB; break;
	case 4: imageFormat = GL_RGBA; break;
	}


	if (textureData)
	{
		//Bind image to texture
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, textureData);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Image load failed! : @" << full_path << "\n";
		stbi_image_free(textureData);
		//TODO: Implement proper error return code
		return 0;
	}

	std::cout << "Successfully made texture : @" << full_path << "\n";

	return textureID;
}
