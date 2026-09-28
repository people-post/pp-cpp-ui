#include "XMLNodeHandlerDefault.h"
#include <ui/dom/Element.h>
#include <ui/dom/ElementUtilities.h>
#include <ui/dom/Factory.h>
#include <ui/base/Log.h>
#include <ui/base/Profiling.h>
#include <ui/xml/XMLParser.h>
#include "XMLParseTools.h"

namespace ui {

// Limits how deeply nested a document's elements may be. Layout, style cascading, and element destruction
// all recurse over the DOM tree, so an unbounded document depth (e.g. tens of thousands of nested <div>s)
// can overflow the stack well before any of those later stages get a chance to reject it.
static constexpr size_t MAX_DOCUMENT_DEPTH = 512;

XMLNodeHandlerDefault::XMLNodeHandlerDefault() {}

XMLNodeHandlerDefault::~XMLNodeHandlerDefault() {}

Element* XMLNodeHandlerDefault::ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes)
{
	UI_ZoneScopedC(0x556B2F);

	// Determine the parent
	Element* parent = parser->GetParseFrame()->element;

	if (parser->GetStackDepth() >= MAX_DOCUMENT_DEPTH)
	{
		Log::Message(Log::LT_WARNING, "Element '%s' exceeds the maximum document depth (%zu) and was discarded.", name.c_str(),
			MAX_DOCUMENT_DEPTH);
		return nullptr;
	}

	// Attempt to instance the element with the instancer
	ElementPtr element = Factory::InstanceElement(parent, name, name, attributes);
	if (!element)
	{
		Log::Message(Log::LT_ERROR, "Failed to create element for tag %s, instancer returned nullptr.", name.c_str());
		return nullptr;
	}

	// Move and append the element to the parent
	Element* result = parent->AppendChild(std::move(element));

	return result;
}

bool XMLNodeHandlerDefault::ElementEnd(XMLParser* /*parser*/, const String& /*name*/)
{
	return true;
}

bool XMLNodeHandlerDefault::ElementData(XMLParser* parser, const String& data, XMLDataType type)
{
	UI_ZoneScopedC(0x006400);

	// Determine the parent
	Element* parent = parser->GetParseFrame()->element;
	UI_ASSERT(parent);

	if (type == XMLDataType::InnerXML)
	{
		// Structural data views use the raw inner xml contents of the node, store them as an attribute to be processed by the data view.
		parent->SetAttribute("rmlui-inner-rml", data);
		return true;
	}

	// Parse the text into the element
	return Factory::InstanceElementText(parent, data);
}

} // namespace ui
