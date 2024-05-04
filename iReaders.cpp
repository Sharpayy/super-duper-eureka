#include "iReaders.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

const char* readTextFromFile(const char* filename, DWORD* size)
{
	HANDLE file = CreateFileA((LPCSTR)filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (file == INVALID_HANDLE_VALUE)
		return nullptr;

	DWORD file_size, bytes;
	file_size = GetFileSize(file, NULL);

	char* text = new char[file_size];
	ReadFile(file, (LPVOID)text, file_size, &bytes, NULL);

	if (size != NULL)
		*size = bytes;

	CloseHandle(file);
	return text;
}

std::string* GetFileData(char* filename)
{
	std::ifstream stream(filename);
	std::string line;

	std::string* buffer = new std::string;
	while (std::getline(stream, line))
	{
		buffer->append(line);
		buffer->append("\n");
	}
	stream.close();
	return buffer;
}

void* LoadImageData(const char* filename, bool flip, int* channels, int* x, int* y)
{
	return stbi_load(filename, x, y, channels, STBI_rgb_alpha);
}

void FreeImageData(void* texture)
{
	stbi_image_free(texture);
}
