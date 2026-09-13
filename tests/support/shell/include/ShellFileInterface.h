#pragma once

#include <ui/Core/FileInterface.h>
#include <ui/Core/Types.h>

/**
    pp-cpp-ui file interface for the shell examples.
 */

class ShellFileInterface : public ui::FileInterface {
public:
	ShellFileInterface(const ui::String& root);
	virtual ~ShellFileInterface();

	/// Opens a file.
	ui::FileHandle Open(const ui::String& path) override;

	/// Closes a previously opened file.
	void Close(ui::FileHandle file) override;

	/// Reads data from a previously opened file.
	size_t Read(void* buffer, size_t size, ui::FileHandle file) override;

	/// Seeks to a point in a previously opened file.
	bool Seek(ui::FileHandle file, long offset, int origin) override;

	/// Returns the current position of the file pointer.
	size_t Tell(ui::FileHandle file) override;

private:
	ui::String root;
};
