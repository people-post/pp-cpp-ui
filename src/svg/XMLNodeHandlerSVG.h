#pragma once

#include <ui/Core/XMLParser.h>
#include <ui/SVG/ElementSVG.h>
#include "../core/XMLNodeHandlerDefault.h"

namespace ui {
namespace SVG {
	/**
	    Element Node handler that processes the SVG tag
	 */
	class XMLNodeHandlerSVG : public XMLNodeHandlerDefault {
	public:
		/// Called for element data
		bool ElementData(XMLParser* parser, const String& data, XMLDataType type) override;
	};

} // namespace SVG
} // namespace ui
