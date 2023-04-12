#include "bingus.h"
#include <iostream>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using std::string;

#define TEXTURE_PATH "../res/textures/"

Texture::Texture(u32 id, vec2 size)
{
	this->id = id;
	this->size = size;
}

Texture::Texture(const char* filePath, i32 wrapMode, i32 filter)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	//Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

	//Load texture from file
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	string full_path = string(TEXTURE_PATH) + filePath;
	unsigned char* textureData = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);
	size = vec2(width, height);

	//std::cout << "tex size: " << sizeof(textureData) << "\n";

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
		//TODO: Implement proper error return code??
	}

	std::cout << "Successfully made texture : @" << full_path << "\n";
}

