#include "bingus.h"

#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_RECT_PACK_IMPLEMENTATION
#include <stb_rect_pack.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>

using std::string;
using std::ifstream;
using std::stringstream;

#define TEXTURE_PATH "../res/textures/"
#define SHADER_PATH "../res/shaders/"
#define FONT_PATH "../res/fonts/"

static std::unordered_map<std::string, Texture> textures;
static std::unordered_map<std::string, Shader> shaders;
static std::unordered_map<std::string, Font> fonts;

Texture* LoadTexture(std::string filenameAndPath)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	if (textures.find(filenameAndPath) != textures.end())
	{
		//Return pointer to existing texture
		return &textures[filenameAndPath];
	}

	//Default wrap and filter modes
	Texture texture;

	texture.cachedWrapMode = GL_REPEAT;
	texture.cachedFilterMode = GL_LINEAR_MIPMAP_LINEAR;

	glGenTextures(1, &texture.id);
	glBindTexture(GL_TEXTURE_2D, texture.id);

	//Set parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture.cachedWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture.cachedWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texture.cachedFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texture.cachedFilterMode);

	//Load texture from file
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);

	string full_path = string(TEXTURE_PATH) + filenameAndPath;
	unsigned char* textureData = stbi_load(full_path.c_str(), &width, &height, &nrChannels, 0);
	texture.size = vec2(width, height);

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

	std::cout << "Texture created : @" << full_path << "\n";
	
	textures[filenameAndPath] = texture;
	return &textures[filenameAndPath];
}

void Texture::SetWrapMode(i32 wrapMode)
{
	cachedWrapMode = wrapMode;
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, cachedWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, cachedWrapMode);
}

void Texture::SetFilterMode(i32 filterMode)
{
	cachedFilterMode = filterMode;
	glBindTexture(GL_TEXTURE_2D, id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, cachedFilterMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, cachedFilterMode);
}

static u32 LoadShader(ShaderType type, string filePath)
{
	string s;
	string fullPath = string(SHADER_PATH) + filePath;
	ifstream file(fullPath);

	if (file.fail())
	{
		std::cout << "Failed to make shader! : file not found: " << fullPath << "\n";
		return -1;
	}
	else
	{
		//Read into buffer
		stringstream buffer;
		buffer << file.rdbuf();
		s = buffer.str();
	}

	const char* source = s.c_str();

	u32 id = glCreateShader(type == VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	//Log if we failed
	i32 success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char info[512];
		glGetShaderInfoLog(id, 512, nullptr, info);
		std::cout << "Failed to make shader! : error in file @" << fullPath << "\n" << info << "\n";
		return -1;
	}

	std::cout << "Shader created : @" << fullPath << "\n";
	return id;
}

Shader* LoadShader(std::string vertexFilenameAndPath, std::string fragFilenameAndPath)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	//We add the file addresses together to form the key
	std::string key = vertexFilenameAndPath + fragFilenameAndPath;
	if (shaders.find(key) != shaders.end())
	{
		//Return pointer to existing texture
		return &shaders[key];
	}

	Shader shader;
	shader.uniforms = 0;
	shader.id = glCreateProgram();
	glAttachShader(shader.id, LoadShader(VERTEX, vertexFilenameAndPath));
	glAttachShader(shader.id, LoadShader(FRAGMENT, fragFilenameAndPath));
	glLinkProgram(shader.id);

	GLint success;
	glGetProgramiv(shader.id, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		char info[512];
		glGetProgramInfoLog(shader.id, 512, nullptr, info);
		std::cout << "Failed to make shader program! : linking failed: " << info << "\n";

		return nullptr;
	}

	std::cout << "Shader program created : @" << vertexFilenameAndPath << " + " << fragFilenameAndPath << "\n";

	shaders[key] = shader;
	return &shaders[key];
}

void Shader::EnableUniform(u32 uniformID, const char* name)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	u32 location = glGetUniformLocation(id, name);
	assert(location != -1);
	uniformLocations[uniformID] = location;
	uniforms |= uniformID;
}

void Shader::EnableUniforms(u32 uniformMask)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	uniforms = uniformMask;

	if (HasUniform(SHADER_COLOR)) EnableUniform(SHADER_COLOR, "color");
	if (HasUniform(SHADER_MAIN_TEX)) EnableUniform(SHADER_MAIN_TEX, "main_tex");
	if (HasUniform(SHADER_SPEC_POW)) EnableUniform(SHADER_SPEC_POW, "spec_pow");
}

bool Shader::HasUniform(u32 uniform)
{
	return (uniforms & uniform) == uniform;
}

void Shader::SetUniformInt(u32 uniform, int i)
{
	SetActiveShader(this);
	glUniform1i(uniformLocations[uniform], i);
}

void Shader::SetUniformFloat(u32 uniform, float f)
{
	SetActiveShader(this);
	glUniform1f(uniformLocations[uniform], f);
}

void Shader::SetUniformVec2(u32 uniform, vec2 v2)
{
	SetActiveShader(this);
	glUniform2f(uniformLocations[uniform], v2.x, v2.y);
}

void Shader::SetUniformVec3(u32 uniform, vec3 v3)
{
	SetActiveShader(this);
	glUniform3f(uniformLocations[uniform], v3.x, v3.y, v3.z);
}

void Shader::SetUniformVec4(u32 uniform, vec4 v4)
{
	SetActiveShader(this);
	glUniform4f(uniformLocations[uniform], v4.x, v4.y, v4.z, v4.w);
}

Font* LoadFont(std::string filenameAndPath, u32 pixelHeight)
{
#ifdef TRACY_ENABLE
	ZoneScoped;
#endif

	std::string fontKey = filenameAndPath + "(" + std::to_string(pixelHeight) + "px)";
	auto it = fonts.find(fontKey);
	if (it != fonts.end())
	{
		//Return pointer to existing font
		return &it->second;
	}

	std::string fullPath = std::string(FONT_PATH) + filenameAndPath;

	//Load .ttf file
	//TODO: Currently this crashes if file not found! Pls fix okay thankyou
	long size;
	unsigned char* fontBuffer;
	FILE* fontFile = fopen(fullPath.c_str(), "rb"); //Open file
	fseek(fontFile, 0, SEEK_END); //Seek to end
	size = ftell(fontFile); //Get length
	fseek(fontFile, 0, SEEK_SET); //Seek back to start
	fontBuffer = new unsigned char[(size_t)size]; //Allocate buffer
	fread(fontBuffer, (size_t)size, 1, fontFile); //Read file into buffer
	fclose(fontFile); //Close file

	//Create font
	stbtt_fontinfo fontInfo;
	if (stbtt_InitFont(&fontInfo, fontBuffer, 0) == 0)
	{
		std::cout << "Loading font @" << fullPath << " failed!\n";
		delete[] fontBuffer;
		return nullptr;
	}

	stbtt_pack_context packContext;

	const u32 unicodeCharStart = 32;
	const u32 unicodeCharEnd = 127;
	const u32 unicodeCharRange = unicodeCharEnd - unicodeCharStart;

	const u32 atlasWidth = 1024;
	const u32 atlasHeight = 1024;
	const u32 atlasSize = atlasWidth * atlasHeight;
	stbtt_packedchar packedChars[unicodeCharRange];
	unsigned char* pixelBuffer = new unsigned char[atlasSize];

	if (stbtt_PackBegin(&packContext, pixelBuffer, atlasWidth, atlasHeight, 0, 1, 0) == 0)
	{
		std::cout << "Packing font @" << fullPath << " failed!\n";
		delete[] fontBuffer;
		delete[] pixelBuffer;
		return nullptr;
	}

	stbtt_PackSetOversampling(&packContext, 2, 2);
	stbtt_PackFontRange(&packContext, fontBuffer, 0, (float)pixelHeight, unicodeCharStart, unicodeCharRange, packedChars);
	stbtt_PackEnd(&packContext);

	//Create atlas texture
	u32 textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); //disable byte-alignment restriction
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixelBuffer);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Font font;
	font.lineHeight = pixelHeight;
	font.texture.id = textureID;
	font.texture.size = vec2(atlasWidth, atlasHeight);
	font.texture.cachedWrapMode = GL_CLAMP_TO_EDGE;
	font.texture.cachedFilterMode = GL_LINEAR;

	for (i32 c = unicodeCharStart; c < unicodeCharEnd; c++)
	{
		const stbtt_packedchar& packedChar = packedChars[c - unicodeCharStart];

		//Create character and store in map
		FontCharacter character;
		character.uvMin = vec2((float)packedChar.x0 / (float)atlasWidth, (float)packedChar.y0 / (float)atlasHeight);
		character.uvMax = vec2((float)packedChar.x1 / (float)atlasWidth, (float)packedChar.y1 / (float)atlasHeight);
		character.size = vec2(packedChar.xoff2 - packedChar.xoff, packedChar.yoff2 - packedChar.yoff) / (float)pixelHeight;
		character.bearing = vec2(packedChar.xoff, 1.f - packedChar.yoff) / (float)pixelHeight;
		character.advance = packedChar.xadvance / (float)pixelHeight;

		font.characters.insert(std::pair<i32, FontCharacter>(c, character));
	}

	std::cout << "Successfully loaded font : @" << fullPath << "\n";

	delete[] fontBuffer;
	delete[] pixelBuffer;

	fonts[fontKey] = font;
	return &fonts[fontKey];
}
