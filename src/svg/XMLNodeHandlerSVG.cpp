#include "XMLNodeHandlerSVG.h"

namespace ui {
namespace SVG {
	bool XMLNodeHandlerSVG::ElementData(XMLParser* parser, const String& data, XMLDataType /*type*/)
	{
		auto* element = ui_dynamic_cast<ElementSVG*>(parser->GetParseFrame()->element);
		UI_ASSERT(element);
		if (element)
			element->SetInnerRML(data);
		return true;
	}
} // namespace SVG
} // namespace ui
