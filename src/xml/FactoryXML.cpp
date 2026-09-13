#include <ui/dom/Factory.h>
#include <ui/dom/Context.h>
#include <ui/dom/ElementDocument.h>
#include <ui/base/Log.h>
#include <ui/xml/XMLParser.h>
#include <ui/base/Profiling.h>

namespace ui {

bool Factory::InstanceElementStream(Element* parent, Stream* stream)
{
	XMLParser parser(parent);
	parser.Parse(stream);
	return true;
}

ElementPtr Factory::InstanceDocumentStream(Context* context, Stream* stream, const String& document_base_tag)
{
	UI_ZoneScoped;

	ElementPtr element = Factory::InstanceElement(nullptr, document_base_tag, document_base_tag, XMLAttributes());
	if (!element)
	{
		Log::Message(Log::LT_ERROR, "Failed to instance document, instancer returned nullptr.");
		return nullptr;
	}

	ElementDocument* document = ui_dynamic_cast<ElementDocument*>(element.get());
	if (!document)
	{
		Log::Message(Log::LT_ERROR, "Failed to instance document element. Found type '%s', was expecting derivative of ElementDocument.",
			ui_type_name(*element));
		return nullptr;
	}

	document->context = context;

	XMLParser parser(element.get());
	parser.Parse(stream);

	return element;
}

} // namespace ui
