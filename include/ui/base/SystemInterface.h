#pragma once

#include <ui/base/Header.h>
#include <ui/base/Log.h>
#include <ui/base/Traits.h>
#include <ui/base/Types.h>

namespace ui {

/**
    pp-cpp-ui's system interface provides an interface for time, translation, logging, and other system utilities.

    The default logging implementation outputs to the Windows Debug Console on Windows, and Standard Error on other
    platforms.
 */

class UI_CORE_API SystemInterface : public NonCopyMoveable {
public:
	SystemInterface();
	virtual ~SystemInterface();

	/// Get the number of seconds elapsed since the start of the application.
	/// @return Elapsed time, in seconds.
	virtual double GetElapsedTime();

	/// Translate the input string into the translated string.
	/// @param[out] translated Translated string ready for display.
	/// @param[in] input String as received from XML.
	/// @return Number of translations that occured.
	virtual int TranslateString(String& translated, const String& input);

	/// Joins the path of an RML or RCSS file with the path of a resource specified within the file.
	/// @param[out] translated_path The joined path.
	/// @param[in] document_path The path of the source document (including the file name).
	/// @param[in] path The path of the resource specified in the document.
	virtual void JoinPath(String& translated_path, const String& document_path, const String& path);

	/// Log the specified message.
	/// @param[in] type Type of log message, ERROR, WARNING, etc.
	/// @param[in] message Message to log.
	/// @return True to continue execution, false to break into the debugger.
	virtual bool LogMessage(Log::Type type, const String& message);

	/// Set mouse cursor.
	/// @param[in] cursor_name Cursor name to activate.
	virtual void SetMouseCursor(const String& cursor_name);

	/// Set clipboard text.
	/// @param[in] text Text to apply to clipboard.
	virtual void SetClipboardText(const String& text);

	/// Get clipboard text.
	/// @param[out] text Retrieved text from clipboard.
	virtual void GetClipboardText(String& text);

	/// Activate keyboard (for touchscreen devices).
	/// @param[in] caret_position Position of the caret in absolute window coordinates.
	/// @param[in] line_height Height of the current line being edited.
	virtual void ActivateKeyboard(ui::Vector2f caret_position, float line_height);

	/// Deactivate keyboard (for touchscreen devices).
	virtual void DeactivateKeyboard();
};

/// Sets the interface through which system requests (time, log, clipboard, …) are made.
/// Must outlive ui::Shutdown if set. Optional — a default is installed by Initialise().
UI_CORE_API void SetSystemInterface(SystemInterface* system_interface);
/// Returns the active system interface (may be null before Initialise / after Shutdown).
UI_CORE_API SystemInterface* GetSystemInterface();

} // namespace ui
