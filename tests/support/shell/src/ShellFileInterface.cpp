#include "../include/ShellFileInterface.h"
#include <stdio.h>

ShellFileInterface::ShellFileInterface(const ui::String& root) : root(root) {}

ShellFileInterface::~ShellFileInterface() {}

ui::FileHandle ShellFileInterface::Open(const ui::String& path)
{
	// Attempt to open the file relative to the application's root.
	FILE* fp = fopen((root + path).c_str(), "rb");
	if (fp != nullptr)
		return (ui::FileHandle)fp;

	// Attempt to open the file relative to the current working directory.
	fp = fopen(path.c_str(), "rb");
	return (ui::FileHandle)fp;
}

void ShellFileInterface::Close(ui::FileHandle file)
{
	fclose((FILE*)file);
}

size_t ShellFileInterface::Read(void* buffer, size_t size, ui::FileHandle file)
{
	return fread(buffer, 1, size, (FILE*)file);
}

bool ShellFileInterface::Seek(ui::FileHandle file, long offset, int origin)
{
	return fseek((FILE*)file, offset, origin) == 0;
}

size_t ShellFileInterface::Tell(ui::FileHandle file)
{
	return ftell((FILE*)file);
}
