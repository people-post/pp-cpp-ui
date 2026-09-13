// Element — DOM tree / query / RML (definitions only).
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementInstancer.h>
#include <ui/dom/Factory.h>
#include <ui/dom/ElementUtilities.h>
#include <ui/dom/Context.h>
#include <ui/base/Log.h>
#include <ui/dom/ElementStyle.h>
#include "StyleSheetNode.h"
#include "StyleSheetParser.h"
#include "dom/PluginRegistry.h"

namespace ui {

static constexpr int ChildNotifyLevels = 2;

static Element* QuerySelectorMatchRecursive(const StyleSheetNodeListRaw& nodes, Element* element, Element* scope)
{
	const int num_children = element->GetNumChildren();

	for (int i = 0; i < num_children; i++)
	{
		Element* child = element->GetChild(i);
		if (child->GetTagName() == "#text")
			continue;

		for (const StyleSheetNode* node : nodes)
		{
			if (node->IsApplicable(child, scope))
				return child;
		}

		Element* matching_element = QuerySelectorMatchRecursive(nodes, child, scope);
		if (matching_element)
			return matching_element;
	}

	return nullptr;
}

static void QuerySelectorAllMatchRecursive(ElementList& matching_elements, const StyleSheetNodeListRaw& nodes, Element* element, Element* scope)
{
	const int num_children = element->GetNumChildren();

	for (int i = 0; i < num_children; i++)
	{
		Element* child = element->GetChild(i);
		if (child->GetTagName() == "#text")
			continue;

		for (const StyleSheetNode* node : nodes)
		{
			if (node->IsApplicable(child, scope))
			{
				matching_elements.push_back(child);
				break;
			}
		}

		QuerySelectorAllMatchRecursive(matching_elements, nodes, child, scope);
	}
}

ElementPtr Element::Clone() const
{
	ElementPtr clone;

	if (instancer)
	{
		clone = instancer->InstanceElement(nullptr, GetTagName(), attributes);
		if (clone)
			clone->SetInstancer(instancer);
	}
	else
		clone = Factory::InstanceElement(nullptr, GetTagName(), GetTagName(), attributes);

	if (clone)
	{
		// Copy over the attributes. The 'style' and 'class' attributes are skipped because inline styles and class names are copied manually below.
		// This is necessary in case any properties or classes have been set manually, in which case the 'style' and 'class' attributes are out of
		// sync with the used style and active classes.
		ElementAttributes clone_attributes = attributes;
		clone_attributes.erase("style");
		clone_attributes.erase("class");
		clone->SetAttributes(clone_attributes);

		for (auto& id_property : Style().GetLocalStyleProperties())
			clone->SetProperty(id_property.first, id_property.second);

		clone->Style().SetClassNames(Style().GetClassNames());

		String inner_rml;
		GetInnerRML(inner_rml);

		clone->SetInnerRML(inner_rml);
	}

	return clone;
}
String Element::GetAddress(bool include_pseudo_classes, bool include_parents) const
{
	// Add the tag name onto the address.
	String address(tag);

	// Add the ID if we have one.
	if (!id.empty())
	{
		address += "#";
		address += id;
	}

	String classes = Style().GetClassNames();
	if (!classes.empty())
	{
		classes = StringUtilities::Replace(classes, ' ', '.');
		address += ".";
		address += classes;
	}

	if (include_pseudo_classes)
	{
		const PseudoClassMap& pseudo_classes = Style().GetActivePseudoClasses();
		for (auto& pseudo_class : pseudo_classes)
		{
			address += ":";
			address += pseudo_class.first;
		}
	}

	if (include_parents && parent)
	{
		address += " < ";
		return address + parent->GetAddress(include_pseudo_classes, true);
	}
	else
		return address;
}
const String& Element::GetTagName() const
{
	return tag;
}
const String& Element::GetId() const
{
	return id;
}
void Element::SetId(const String& _id)
{
	SetAttribute("id", _id);
}
ElementDocument* Element::GetOwnerDocument() const
{
#ifdef UI_DEBUG
	if (parent && !owner_document)
	{
		// Since we have a parent but no owner_document, then we must be a 'loose' element -- that is, constructed
		// outside of a document and not attached to a child of any element in the hierarchy of a document.
		// This check ensures that we didn't just forget to set the owner document.
		UI_ASSERT(!parent->GetOwnerDocument());
	}
#endif

	return owner_document;
}
Element* Element::GetParentNode() const
{
	return parent;
}
Element* Element::Closest(const String& selectors) const
{
	StyleSheetNode root_node;
	StyleSheetNodeListRaw leaf_nodes = StyleSheetParser::ConstructNodes(root_node, selectors);

	if (leaf_nodes.empty())
	{
		Log::Message(Log::LT_WARNING, "Query selector '%s' is empty. In element %s", selectors.c_str(), GetAddress().c_str());
		return nullptr;
	}

	Element* parent = GetParentNode();

	while (parent)
	{
		for (const StyleSheetNode* node : leaf_nodes)
		{
			if (node->IsApplicable(parent, this))
			{
				return parent;
			}
		}

		parent = parent->GetParentNode();
	}

	return nullptr;
}
Element* Element::GetNextSibling() const
{
	if (parent == nullptr)
		return nullptr;

	for (size_t i = 0; i < parent->children.size() - (parent->num_non_dom_children + 1); i++)
	{
		if (parent->children[i].get() == this)
			return parent->children[i + 1].get();
	}

	return nullptr;
}
Element* Element::GetPreviousSibling() const
{
	if (parent == nullptr)
		return nullptr;

	for (size_t i = 1; i < parent->children.size() - parent->num_non_dom_children; i++)
	{
		if (parent->children[i].get() == this)
			return parent->children[i - 1].get();
	}

	return nullptr;
}
Element* Element::GetFirstChild() const
{
	if (GetNumChildren() > 0)
		return children[0].get();

	return nullptr;
}
Element* Element::GetLastChild() const
{
	if (GetNumChildren() > 0)
		return (children.end() - (num_non_dom_children + 1))->get();

	return nullptr;
}
Element* Element::GetChild(int index) const
{
	if (index < 0 || index >= (int)children.size())
		return nullptr;

	return children[index].get();
}
int Element::GetNumChildren(bool include_non_dom_elements) const
{
	return (int)children.size() - (include_non_dom_elements ? 0 : num_non_dom_children);
}
void Element::GetInnerRML(String& content) const
{
	for (int i = 0; i < GetNumChildren(); i++)
	{
		children[i]->GetRML(content);
	}
}
String Element::GetInnerRML() const
{
	String result;
	GetInnerRML(result);
	return result;
}
void Element::SetInnerRML(const String& rml)
{
	UI_ZoneScopedC(0x6495ED);

	// Remove all DOM children.
	while ((int)children.size() > num_non_dom_children)
		RemoveChild(children.front().get());

	if (!rml.empty())
		Factory::InstanceElementText(this, rml);
}
Element* Element::AppendChild(ElementPtr child, bool dom_element)
{
	UI_ASSERT(child);
	Element* child_ptr = child.get();
	if (dom_element)
		children.insert(children.end() - num_non_dom_children, std::move(child));
	else
	{
		children.push_back(std::move(child));
		num_non_dom_children++;
	}
	// Set parent just after inserting into children. This allows us to eg. get our previous sibling in SetParent.
	child_ptr->SetParent(this);

	Element* ancestor = child_ptr;
	for (int i = 0; i <= ChildNotifyLevels && ancestor; i++, ancestor = ancestor->GetParentNode())
		ancestor->OnChildAdd(child_ptr);

	DirtyStackingContext();

	// Not only does the element definition of the newly inserted element need to be dirtied, but also our own definition and implicitly all of our
	// children's. This ensures correct styles being applied in the presence of tree-structural selectors such as ':first-child'.
	DirtyDefinition(DirtyNodes::Self);

	if (dom_element)
		DirtyLayout();

	return child_ptr;
}
Element* Element::InsertBefore(ElementPtr child, Element* adjacent_element)
{
	UI_ASSERT(child);
	// Find the position in the list of children of the adjacent element. If
	// it's nullptr or we can't find it, then we insert it at the end of the dom
	// children, as a dom element.
	size_t child_index = 0;
	bool found_child = false;
	if (adjacent_element)
	{
		for (child_index = 0; child_index < children.size(); child_index++)
		{
			if (children[child_index].get() == adjacent_element)
			{
				found_child = true;
				break;
			}
		}
	}

	Element* child_ptr = nullptr;

	if (found_child)
	{
		child_ptr = child.get();

		if ((int)child_index >= GetNumChildren())
			num_non_dom_children++;
		else
			DirtyLayout();

		children.insert(children.begin() + child_index, std::move(child));
		child_ptr->SetParent(this);

		Element* ancestor = child_ptr;
		for (int i = 0; i <= ChildNotifyLevels && ancestor; i++, ancestor = ancestor->GetParentNode())
			ancestor->OnChildAdd(child_ptr);

		DirtyStackingContext();
		DirtyDefinition(DirtyNodes::Self);
	}
	else
	{
		child_ptr = AppendChild(std::move(child));
	}

	return child_ptr;
}
ElementPtr Element::ReplaceChild(ElementPtr inserted_element, Element* replaced_element)
{
	UI_ASSERT(inserted_element);
	auto insertion_point = children.begin();
	while (insertion_point != children.end() && insertion_point->get() != replaced_element)
	{
		++insertion_point;
	}

	Element* inserted_element_ptr = inserted_element.get();

	if (insertion_point == children.end())
	{
		AppendChild(std::move(inserted_element));
		return nullptr;
	}

	children.insert(insertion_point, std::move(inserted_element));
	inserted_element_ptr->SetParent(this);

	ElementPtr result = RemoveChild(replaced_element);

	Element* ancestor = inserted_element_ptr;
	for (int i = 0; i <= ChildNotifyLevels && ancestor; i++, ancestor = ancestor->GetParentNode())
		ancestor->OnChildAdd(inserted_element_ptr);

	return result;
}
ElementPtr Element::RemoveChild(Element* child)
{
	size_t child_index = 0;

	for (auto itr = children.begin(); itr != children.end(); ++itr)
	{
		// Add the element to the delete list
		if (itr->get() == child)
		{
			Element* ancestor = child;
			for (int i = 0; i <= ChildNotifyLevels && ancestor; i++, ancestor = ancestor->GetParentNode())
				ancestor->OnChildRemove(child);

			if (child_index >= children.size() - num_non_dom_children)
				num_non_dom_children--;

			ElementPtr detached_child = std::move(*itr);
			children.erase(itr);

			// Remove the child element as the focused child of this element.
			if (child == focus)
			{
				focus = nullptr;

				// If this child (or a descendant of this child) is the context's currently
				// focused element, set the focus to us instead.
				if (Context* context = GetContext())
				{
					Element* focus_element = context->GetFocusElement();
					while (focus_element)
					{
						if (focus_element == child)
						{
							Focus();
							break;
						}

						focus_element = focus_element->GetParentNode();
					}
				}
			}

			detached_child->SetParent(nullptr);

			DirtyLayout();
			DirtyStackingContext();
			DirtyDefinition(DirtyNodes::Self);

			return detached_child;
		}

		child_index++;
	}

	return nullptr;
}
bool Element::HasChildNodes() const
{
	return (int)children.size() > num_non_dom_children;
}
Element* Element::GetElementById(const String& id)
{
	// Check for special-case tokens.
	if (id == "#self")
		return this;
	else if (id == "#document")
		return GetOwnerDocument();
	else if (id == "#parent")
		return this->parent;
	else
	{
		Element* search_root = GetOwnerDocument();
		if (search_root == nullptr)
			search_root = this;
		return ElementUtilities::GetElementById(search_root, id);
	}
}
void Element::GetElementsByTagName(ElementList& elements, const String& tag)
{
	return ElementUtilities::GetElementsByTagName(elements, this, tag);
}
void Element::GetElementsByClassName(ElementList& elements, const String& class_name)
{
	return ElementUtilities::GetElementsByClassName(elements, this, class_name);
}
Element* Element::QuerySelector(const String& selectors)
{
	StyleSheetNode root_node;
	StyleSheetNodeListRaw leaf_nodes = StyleSheetParser::ConstructNodes(root_node, selectors);

	if (leaf_nodes.empty())
	{
		Log::Message(Log::LT_WARNING, "Query selector '%s' is empty. In element %s", selectors.c_str(), GetAddress().c_str());
		return nullptr;
	}

	return QuerySelectorMatchRecursive(leaf_nodes, this, this);
}
void Element::QuerySelectorAll(ElementList& elements, const String& selectors)
{
	StyleSheetNode root_node;
	StyleSheetNodeListRaw leaf_nodes = StyleSheetParser::ConstructNodes(root_node, selectors);

	if (leaf_nodes.empty())
	{
		Log::Message(Log::LT_WARNING, "Query selector '%s' is empty. In element %s", selectors.c_str(), GetAddress().c_str());
		return;
	}

	QuerySelectorAllMatchRecursive(elements, leaf_nodes, this, this);
}
bool Element::Matches(const String& selectors)
{
	StyleSheetNode root_node;
	StyleSheetNodeListRaw leaf_nodes = StyleSheetParser::ConstructNodes(root_node, selectors);

	if (leaf_nodes.empty())
	{
		Log::Message(Log::LT_WARNING, "Query selector '%s' is empty. In element %s", selectors.c_str(), GetAddress().c_str());
		return false;
	}

	for (const StyleSheetNode* node : leaf_nodes)
	{
		if (node->IsApplicable(this, this))
		{
			return true;
		}
	}

	return false;
}
bool Element::Contains(Element* element) const
{
	while (element)
	{
		if (element == this)
			return true;
		element = element->GetParentNode();
	}
	return false;
}
void Element::OnChildAdd(Element* /*child*/) {}
void Element::OnChildRemove(Element* /*child*/) {}
void Element::SetOwnerDocument(ElementDocument* document)
{
	if (owner_document && !document)
	{
		// We are detaching from the document and thereby also the context.
		if (Context* context = owner_document->GetContext())
			context->OnElementDetach(this);
	}

	// If this element is a document, then never change owner_document.
	if (owner_document != this && owner_document != document)
	{
		owner_document = document;
		for (ElementPtr& child : children)
			child->SetOwnerDocument(document);
	}
}
void Element::SetParent(Element* _parent)
{
	// Assumes we are already detached from the hierarchy or we are detaching now.
	UI_ASSERT(!parent || !_parent);

	parent = _parent;

	if (parent)
	{
		// We need to update our definition and make sure we inherit the properties of our new parent.
		DirtyDefinition(DirtyNodes::Self);
		Style().DirtyInheritedProperties();
	}

	// The transform state may require recalculation.
	if (transform_state || (parent && parent->transform_state))
		DirtyTransformState(true, true);

	SetOwnerDocument(parent ? parent->GetOwnerDocument() : nullptr);

	if (!parent)
	{
		if (data_model)
			SetDataModel(nullptr);
	}
	else
	{
		auto it = attributes.find("data-model");
		if (it == attributes.end())
		{
			SetDataModel(parent->data_model);
		}
		else if (Context* context = GetContext())
		{
			String name = it->second.Get<String>();
			if (!AttachNamedDataModel(name))
				Log::Message(Log::LT_ERROR, "Could not locate data model '%s' in element %s.", name.c_str(), GetAddress().c_str());
		}
	}
}

} // namespace ui
