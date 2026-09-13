#pragma once

#include "xml/XMLNodeHandlerDefault.h"

namespace ui {

/**
    XML node handler for processing the select and option tags.
 */

class XMLNodeHandlerSelect : public XMLNodeHandlerDefault {
public:
	XMLNodeHandlerSelect();
	virtual ~XMLNodeHandlerSelect();

	/// Called when a new element start is opened
	Element* ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes) override;
};

} // namespace ui
