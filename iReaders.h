#pragma once
#include <Windows.h>
#include <string>
#include <fstream>

const char* readTextFromFile(const char* filename, DWORD* size);
std::string* GetFileData(char* filename);
void* LoadImageData(const char* filename, bool flip, int* channels, int* x, int* y);
void FreeImageData(void* texture);