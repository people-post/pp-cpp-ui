#pragma once

#include <ui/Core/EventListener.h>
#include <ui/Core/Types.h>

namespace ui {
class Context;
class ElementDocument;
} // namespace ui

enum class SourceType { None, Test, Reference };

class TestViewer {
public:
	TestViewer(ui::Context* context);
	~TestViewer();

	void ShowSource(SourceType type);
	void ShowHelp(bool show);
	bool IsHelpVisible() const;
	bool IsNavigationLocked() const;

	bool LoadTest(const ui::String& directory, const ui::String& filename, int test_index, int number_of_tests, int filtered_test_index,
		int filtered_number_of_tests, int suite_index, int number_of_suites, bool keep_scroll_position = false);

	void SetGoToText(const ui::String& rml);
	ui::Rectanglef GetGoToArea() const;

	void SetAttention(bool active);

private:
	ui::Context* context;

	ui::ElementDocument* document_test = nullptr;
	ui::ElementDocument* document_description = nullptr;
	ui::ElementDocument* document_source = nullptr;
	ui::ElementDocument* document_reference = nullptr;
	ui::ElementDocument* document_help = nullptr;

	ui::String source_test;
	ui::String source_reference;

	ui::String reference_filename;
};
