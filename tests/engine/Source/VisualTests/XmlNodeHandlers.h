#pragma once

#include <ui/dom/EventListener.h>
#include <ui/base/Types.h>
#include <ui/xml/XMLNodeHandler.h>
struct MetaItem {
	ui::String name;
	ui::String content;
};
using MetaList = ui::Vector<MetaItem>;

struct LinkItem {
	ui::String rel;
	ui::String href;
};
using LinkList = ui::Vector<LinkItem>;

class XMLNodeHandlerMeta : public ui::XMLNodeHandler {
public:
	XMLNodeHandlerMeta();
	~XMLNodeHandlerMeta();

	/// Called when a new element start is opened
	ui::Element* ElementStart(ui::XMLParser* parser, const ui::String& name, const ui::XMLAttributes& attributes) override;
	/// Called when an element is closed
	bool ElementEnd(ui::XMLParser* parser, const ui::String& name) override;
	/// Called for element data
	bool ElementData(ui::XMLParser* parser, const ui::String& data, ui::XMLDataType type) override;

	const MetaList& GetMetaList() const { return meta_list; }
	void ClearMetaList() { meta_list.clear(); }

private:
	MetaList meta_list;
};

class XMLNodeHandlerLink : public ui::XMLNodeHandler {
public:
	XMLNodeHandlerLink();
	~XMLNodeHandlerLink();

	/// Called when a new element start is opened
	ui::Element* ElementStart(ui::XMLParser* parser, const ui::String& name, const ui::XMLAttributes& attributes) override;
	/// Called when an element is closed
	bool ElementEnd(ui::XMLParser* parser, const ui::String& name) override;
	/// Called for element data
	bool ElementData(ui::XMLParser* parser, const ui::String& data, ui::XMLDataType type) override;

	const LinkList& GetLinkList() const { return link_list; }
	void ClearLinkList() { link_list.clear(); }

private:
	LinkList link_list;
	ui::XMLNodeHandler* node_handler_head;
};
