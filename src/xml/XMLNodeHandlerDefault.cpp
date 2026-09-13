#include "XMLNodeHandlerDefault.h"
#include <ui/Core/Element.h>
#include <ui/Core/ElementUtilities.h>
#include <ui/Core/Factory.h>
#include <ui/Core/Log.h>
#include <ui/Core/Profiling.h>
#include <ui/Core/XMLParser.h>
#include "XMLParseTools.h"

namespace ui {

XMLNodeHandlerDefault::XMLNodeHandlerDefault() {}

XMLNodeHandlerDefault::~XMLNodeHandlerDefault() {}

Element* XMLNodeHandlerDefault::ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes)
{
	UI_ZoneScopedC(0x556B2F);

	// Determine the parent
	Element* parent = parser->GetParseFrame()->element;

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
