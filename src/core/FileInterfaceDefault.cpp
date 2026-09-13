#include "FileInterfaceDefault.h"

#ifndef UI_NO_FILE_INTERFACE_DEFAULT

namespace ui {

FileInterfaceDefault::~FileInterfaceDefault() {}

FileHandle FileInterfaceDefault::Open(const String& path)
{
	return (FileHandle)fopen(path.c_str(), "rb");
}

void FileInterfaceDefault::Close(FileHandle file)
{
	fclose((FILE*)file);
}

size_t FileInterfaceDefault::Read(void* buffer, size_t size, FileHandle file)
{
	return fread(buffer, 1, size, (FILE*)file);
}

bool FileInterfaceDefault::Seek(FileHandle file, long offset, int origin)
{
	return fseek((FILE*)file, offset, origin) == 0;
}

size_t FileInterfaceDefault::Tell(FileHandle file)
{
	return ftell((FILE*)file);
}

} // namespace ui
#endif /*UI_NO_FILE_INTERFACE_DEFAULT*/
