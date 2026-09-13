#include "../include/Shell.h"
#include "../include/PlatformExtensions.h"
#include "../include/ShellFileInterface.h"
#include <ui/dom/Context.h>
#include <ui/core/Core.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/Input.h>
#include <ui/Debugger.h>

static ui::UniquePtr<ShellFileInterface> file_interface;

bool Shell::Initialize()
{
	// Find the path to the 'Samples' directory.
	ui::String root = PlatformExtensions::FindSamplesRoot();
	if (root.empty())
		return false;

	// The shell overrides the default file interface so that absolute paths in RML/RCSS-documents are relative to the 'Samples' directory.
	file_interface = ui::MakeUnique<ShellFileInterface>(root);
	ui::SetFileInterface(file_interface.get());

	return true;
}

void Shell::Shutdown()
{
	file_interface.reset();
}

void Shell::LoadFonts()
{
	const ui::String directory = "assets/";

	struct FontFace {
		const char* filename;
		bool fallback_face;
	};
	FontFace font_faces[] = {
		{"LatoLatin-Regular.ttf", false},
		{"LatoLatin-Italic.ttf", false},
		{"LatoLatin-Bold.ttf", false},
		{"LatoLatin-BoldItalic.ttf", false},
		{"NotoEmoji-Regular.ttf", true},
	};

	for (const FontFace& face : font_faces)
		ui::LoadFontFace(directory + face.filename, face.fallback_face);
}

bool Shell::ProcessKeyDownShortcuts(ui::Context* context, ui::Input::KeyIdentifier key, int key_modifier, float native_dp_ratio, bool priority)
{
	if (!context)
		return true;

	// Result should return true to allow the event to propagate to the next handler.
	bool result = false;

	// This function is intended to be called twice by the backend, before and after submitting the key event to the context. This way we can
	// intercept shortcuts that should take priority over the context, and then handle any shortcuts of lower priority if the context did not
	// intercept it.
	if (priority)
	{
		// Priority shortcuts are handled before submitting the key to the context.

		// Toggle debugger and set dp-ratio using Ctrl +/-/0 keys.
		if (key == ui::Input::KI_F8)
		{
			ui::Debugger::SetVisible(!ui::Debugger::IsVisible());
		}
		else if (key == ui::Input::KI_0 && key_modifier & ui::Input::KM_CTRL)
		{
			context->SetDensityIndependentPixelRatio(native_dp_ratio);
		}
		else if (key == ui::Input::KI_1 && key_modifier & ui::Input::KM_CTRL)
		{
			context->SetDensityIndependentPixelRatio(1.f);
		}
		else if ((key == ui::Input::KI_OEM_MINUS || key == ui::Input::KI_SUBTRACT) && key_modifier & ui::Input::KM_CTRL)
		{
			const float new_dp_ratio = ui::Math::Max(context->GetDensityIndependentPixelRatio() / 1.2f, 0.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio);
		}
		else if ((key == ui::Input::KI_OEM_PLUS || key == ui::Input::KI_ADD) && key_modifier & ui::Input::KM_CTRL)
		{
			const float new_dp_ratio = ui::Math::Min(context->GetDensityIndependentPixelRatio() * 1.2f, 2.5f);
			context->SetDensityIndependentPixelRatio(new_dp_ratio);
		}
		else
		{
			// Propagate the key down event to the context.
			result = true;
		}
	}
	else
	{
		// We arrive here when no priority keys are detected and the key was not consumed by the context. Check for shortcuts of lower priority.
		if (key == ui::Input::KI_R && key_modifier & ui::Input::KM_CTRL)
		{
			for (int i = 0; i < context->GetNumDocuments(); i++)
			{
				ui::ElementDocument* document = context->GetDocument(i);
				const ui::String& src = document->GetSourceURL();
				if (src.size() > 4 && src.substr(src.size() - 4) == ".rml")
				{
					document->ReloadStyleSheet();
				}
			}
		}
		else
		{
			result = true;
		}
	}

	return result;
}
