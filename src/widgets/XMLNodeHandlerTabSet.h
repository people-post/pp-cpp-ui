#pragma once

#include <ui/xml/XMLNodeHandler.h>
namespace ui {

/**
    XML node handler for processing the tabset tags.
 */

class XMLNodeHandlerTabSet : public XMLNodeHandler {
public:
	XMLNodeHandlerTabSet();
	virtual ~XMLNodeHandlerTabSet();

	/// Called when a new element start is opened
	Element* ElementStart(XMLParser* parser, const String& name, const XMLAttributes& attributes) override;
	/// Called when an element is closed
	bool ElementEnd(XMLParser* parser, const String& name) override;
	/// Called for element data
	bool ElementData(XMLParser* parser, const String& data, XMLDataType type) override;
};

} // namespace ui
