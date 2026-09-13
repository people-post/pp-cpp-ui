#include "TestViewer.h"
#include "../Common/TestsShell.h"
#include "TestConfig.h"
#include "XmlNodeHandlers.h"
#include <ui/dom/Context.h>
#include <ui/core/Core.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/core/FileInterface.h>
#include <ui/core/SystemInterface.h>
#include <ui/base/Types.h>
#include <ui/xml/XMLParser.h>
#include <Shell.h>

using namespace ui;

static SharedPtr<XMLNodeHandlerMeta> meta_handler;
static SharedPtr<XMLNodeHandlerLink> link_handler;

static void InitializeXmlNodeHandlers()
{
	meta_handler = MakeShared<XMLNodeHandlerMeta>();
	ui::XMLParser::RegisterNodeHandler("meta", meta_handler);

	link_handler = MakeShared<XMLNodeHandlerLink>();
	ui::XMLParser::RegisterNodeHandler("link", link_handler);
}

class EventListenerLinks : public ui::EventListener {
public:
	void ProcessEvent(ui::Event& event) override
	{
		ui::Element* element = event.GetCurrentElement();
		ui::String href = element->GetAttribute<ui::String>("href", "");

		if (href.empty() || !hover_text)
			return;

		if (event == ui::EventId::Click)
		{
			if (ui::SystemInterface* system_interface = ui::GetSystemInterface())
			{
				system_interface->SetClipboardText(href);
				hover_text->SetInnerRML("Copied to clipboard");
				hover_text->SetClass("confirmation", true);
			}
		}
		else if (event == ui::EventId::Mouseover)
		{
			hover_text->SetInnerRML(ui::StringUtilities::EncodeRml(href));
		}
		else if (event == ui::EventId::Mouseout)
		{
			hover_text->SetInnerRML("");
			hover_text->SetClass("confirmation", false);
		}
	}

	void SetHoverTextElement(Element* element) { hover_text = element; }

private:
	Element* hover_text = nullptr;
};
static EventListenerLinks event_listener_links;

TestViewer::TestViewer(ui::Context* context) : context(context)
{
	InitializeXmlNodeHandlers();

	const String local_data_path_prefix = "/../Tests/Data/";

	document_description = context->LoadDocument(local_data_path_prefix + "description.rml");
	UI_ASSERT(document_description);
	event_listener_links.SetHoverTextElement(document_description->GetElementById("hovertext"));
	document_description->Show();

	document_source = context->LoadDocument(local_data_path_prefix + "view_source.rml");
	UI_ASSERT(document_source);
	document_help = context->LoadDocument(local_data_path_prefix + "visual_tests_help.rml");
	UI_ASSERT(document_help);
	if (Element* element = document_help->GetElementById("test_directories"))
	{
		String rml;
		const StringList dirs = GetTestInputDirectories();
		for (const String& dir : dirs)
			rml += "<value>" + ui::StringUtilities::EncodeRml(dir) + "</value>";
		element->SetInnerRML(rml);
	}
	if (Element* element = document_help->GetElementById("compare_input"))
	{
		element->SetInnerRML(ui::StringUtilities::EncodeRml(GetCompareInputDirectory()));
	}
	if (Element* element = document_help->GetElementById("capture_output"))
	{
		element->SetInnerRML(ui::StringUtilities::EncodeRml(GetCaptureOutputDirectory()));
	}
}

TestViewer::~TestViewer()
{
	event_listener_links.SetHoverTextElement(nullptr);

	for (ElementDocument* doc : {document_test, document_description, document_source, document_reference, document_help})
	{
		if (doc)
			doc->Close();
	}
}

static ui::String LoadFile(const String& file_path)
{
	String result;
	ui::GetFileInterface()->LoadFile(file_path, result);
	return result;
}

void TestViewer::ShowSource(SourceType type)
{
	if (type == SourceType::None)
	{
		document_source->Hide();
	}
	else
	{
		const String& source_string = (type == SourceType::Test ? source_test : source_reference);

		if (source_string.empty())
		{
			document_source->Hide();
		}
		else
		{
			const String rml_source = StringUtilities::EncodeRml(source_string);

			Element* element = document_source->GetElementById("code");
			UI_ASSERT(element);
			element->SetInnerRML(rml_source);

			document_source->Show(ModalFlag::None, FocusFlag::None);
		}
	}
}

void TestViewer::ShowHelp(bool show)
{
	if (show)
		document_help->Show();
	else
		document_help->Hide();
}

bool TestViewer::IsHelpVisible() const
{
	return document_help->IsVisible();
}

bool TestViewer::IsNavigationLocked() const
{
	if (Element* element = context->GetFocusElement())
	{
		if (document_test && element->GetOwnerDocument() == document_test)
		{
			if (document_test->HasAttribute("lock-navigation"))
				return true;
		}
	}
	return false;
}

bool TestViewer::LoadTest(const ui::String& directory, const ui::String& filename, int test_index, int number_of_tests, int filtered_test_index,
	int filtered_number_of_tests, int suite_index, int number_of_suites, bool keep_scroll_position)
{
	float scroll_position = 0.f;
	if (document_test)
	{
		scroll_position = document_test->GetScrollTop();
		document_test->Close();
		document_test = nullptr;
	}
	if (document_reference)
	{
		document_reference->Close();
		document_reference = nullptr;
	}

	reference_filename.clear();
	source_test.clear();
	source_reference.clear();

	meta_handler->ClearMetaList();
	link_handler->ClearLinkList();

	const ui::String test_path = directory + '/' + filename;
	ui::String reference_path;

	// Load test document, and reference document if it exists.
	{
		source_test = LoadFile(test_path);
		if (source_test.empty())
			return false;

		document_test = context->LoadDocumentFromMemory(source_test, ui::StringUtilities::Replace(test_path, ':', '|'));
		if (!document_test)
			return false;

		document_test->Show(ModalFlag::None, FocusFlag::None);
		if (keep_scroll_position)
			document_test->SetScrollTop(scroll_position);

		for (const LinkItem& item : link_handler->GetLinkList())
		{
			if (item.rel == "match")
			{
				reference_filename = item.href;
				break;
			}
		}

		reference_path = directory + '/' + reference_filename;

		if (!reference_filename.empty())
		{
			source_reference = LoadFile(reference_path);

			if (!source_reference.empty())
			{
				document_reference = context->LoadDocumentFromMemory(source_reference, ui::StringUtilities::Replace(reference_path, ':', '|'));
				if (document_reference)
				{
					document_reference->SetProperty(PropertyId::Left, Property(510.f, Unit::DP));
					document_reference->Show(ModalFlag::None, FocusFlag::None);
				}
			}
		}
	}

	// Description Header
	{
		Element* description_header = document_description->GetElementById("header");
		UI_ASSERT(description_header);

		description_header->SetInnerRML(
			CreateString("Test suite %d of %d<br/>Test %d of %d<br/>", suite_index + 1, number_of_suites, test_index + 1, number_of_tests));
	}

	// Description Filter
	{
		Element* description_filter_text = document_description->GetElementById("filter_text");
		UI_ASSERT(description_filter_text);
		if (filtered_number_of_tests == 0)
			description_filter_text->SetInnerRML("No matches");
		else if (filtered_number_of_tests < number_of_tests && filtered_test_index >= 0)
			description_filter_text->SetInnerRML(CreateString("Filtered %d of %d", filtered_test_index + 1, filtered_number_of_tests));
		else if (filtered_number_of_tests < number_of_tests && filtered_test_index < 0)
			description_filter_text->SetInnerRML(CreateString("Filtered X of %d", filtered_number_of_tests));
		else
			description_filter_text->SetInnerRML("");
	}

	// Description Content
	{
		String rml_description =
			ui::CreateString("<h1>%s</h1><p><a href=\"%s\">%s</a>", document_test->GetTitle().c_str(), test_path.c_str(), filename.c_str());

		if (!reference_filename.empty())
		{
			if (document_reference)
				rml_description += "<br/><a href=\"" + reference_path + "\">" + reference_filename + "</a>";
			else
				rml_description += "<br/>(missing)&nbsp;" + reference_filename + "";
		}
		rml_description += "</p>";

		const LinkList& link_list = link_handler->GetLinkList();
		if (!link_list.empty())
		{
			rml_description += "<p class=\"links\">";
			for (const LinkItem& item : link_list)
			{
				if (item.rel == "match")
					continue;

				rml_description += "<a href=\"" + item.href + "\">" + item.rel + "</a> ";
			}
			rml_description += "</p>";
		}

		for (const MetaItem& item : meta_handler->GetMetaList())
		{
			rml_description += "<h3>" + item.name + "</h3>";
			rml_description += "<p>" + item.content + "</p>";
		}

		Element* description_content = document_description->GetElementById("content");
		UI_ASSERT(description_content);
		description_content->SetInnerRML(rml_description);

		// Add link hover and click handler.
		ui::ElementList link_elements;
		description_content->GetElementsByTagName(link_elements, "a");

		for (ui::Element* element : link_elements)
		{
			element->AddEventListener(ui::EventId::Click, &event_listener_links);
			element->AddEventListener(ui::EventId::Mouseover, &event_listener_links);
			element->AddEventListener(ui::EventId::Mouseout, &event_listener_links);
		}
	}

	return true;
}

void TestViewer::SetGoToText(const ui::String& rml)
{
	Element* description_goto = document_description->GetElementById("goto");
	UI_ASSERT(description_goto);
	description_goto->SetInnerRML(rml);
}

ui::Rectanglef TestViewer::GetGoToArea() const
{
	Element* description_goto = document_description->GetElementById("goto");
	UI_ASSERT(description_goto);
	return ui::Rectanglef::FromPositionSize(description_goto->GetAbsoluteOffset(), description_goto->GetBox().GetSize());
}

void TestViewer::SetAttention(bool active)
{
	if (active)
		document_description->SetProperty(ui::PropertyId::BackgroundColor, ui::Property(ui::Colourb(100, 100, 30), ui::Unit::COLOUR));
	else
		document_description->RemoveProperty(ui::PropertyId::BackgroundColor);
}
