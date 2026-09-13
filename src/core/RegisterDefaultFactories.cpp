#include "RegisterDefaultFactories.h"

#include <ui/dom/ElementInstancer.h>
#include <ui/dom/Factory.h>
#include <ui/widgets/ElementForm.h>
#include <ui/widgets/ElementFormControlInput.h>
#include <ui/widgets/ElementFormControlSelect.h>
#include <ui/widgets/ElementFormControlTextArea.h>
#include <ui/widgets/ElementProgress.h>
#include <ui/widgets/ElementTabSet.h>
#include <ui/xml/XMLParser.h>

#include "base/ControlledLifetimeResource.h"
#include "data/DataController.h"
#include "data/DataControllerDefault.h"
#include "data/DataViewDefault.h"
#include "widgets/ElementImage.h"
#include "widgets/ElementLabel.h"
#include "widgets/ElementSelectableText.h"
#include "widgets/ElementTextSelection.h"
#include "widgets/XMLNodeHandlerSelect.h"
#include "widgets/XMLNodeHandlerTabSet.h"
#include "widgets/XMLNodeHandlerTextArea.h"
#include "xml/XMLNodeHandlerBody.h"
#include "xml/XMLNodeHandlerDefault.h"
#include "xml/XMLNodeHandlerHead.h"
#include "xml/XMLNodeHandlerTemplate.h"

namespace ui {
namespace RegisterDefaultFactories {

namespace {

/// "*" instancer with selectable="text" support (widgets-owned).
class ElementInstancerStar final : public ElementInstancer {
public:
	ElementPtr InstanceElement(Element* parent, const String& tag, const XMLAttributes& attributes) override
	{
		auto selectable_it = attributes.find("selectable");
		if (selectable_it != attributes.end() && selectable_it->second.Get<String>() == "text")
			return ElementPtr(new ElementSelectableText(tag));
		return fallback.InstanceElement(parent, tag, attributes);
	}

	void ReleaseElement(Element* element) override
	{
		if (ui_dynamic_cast<ElementSelectableText*>(element))
			delete element;
		else
			fallback.ReleaseElement(element);
	}

private:
	ElementInstancerElement fallback;
};

struct DefaultExtendedInstancers {
	ElementInstancerStar element_star;

	ElementInstancerGeneric<ElementImage> element_img;
	ElementInstancerGeneric<ElementForm> form;
	ElementInstancerGeneric<ElementFormControlInput> input;
	ElementInstancerGeneric<ElementFormControlSelect> select;
	ElementInstancerGeneric<ElementLabel> element_label;
	ElementInstancerGeneric<ElementFormControlTextArea> textarea;
	ElementInstancerGeneric<ElementTextSelection> selection;
	ElementInstancerGeneric<ElementTabSet> tabset;
	ElementInstancerGeneric<ElementProgress> progress;

	DataViewInstancerDefault<DataViewAttribute> data_view_attribute;
	DataViewInstancerDefault<DataViewAttributeIf> data_view_attribute_if;
	DataViewInstancerDefault<DataViewClass> data_view_class;
	DataViewInstancerDefault<DataViewIf> data_view_if;
	DataViewInstancerDefault<DataViewVisible> data_view_visible;
	DataViewInstancerDefault<DataViewRml> data_view_rml;
	DataViewInstancerDefault<DataViewStyle> data_view_style;
	DataViewInstancerDefault<DataViewText> data_view_text;
	DataViewInstancerDefault<DataViewValue> data_view_value;
	DataViewInstancerDefault<DataViewChecked> data_view_checked;
	DataViewInstancerDefault<DataViewAlias> data_view_alias;
	DataViewInstancerDefault<DataViewFor> structural_data_view_for;

	DataControllerInstancerDefault<DataControllerEvent> data_controller_event;
	DataControllerInstancerDefault<DataControllerValue> data_controller_value;
};

ControlledLifetimeResource<DefaultExtendedInstancers> extended_instancers;

} // namespace

void Initialise()
{
	extended_instancers.Initialize();
	DefaultExtendedInstancers* d = extended_instancers.operator->();

	Factory::RegisterElementInstancer("*", &d->element_star);

	Factory::RegisterElementInstancer("img", &d->element_img);
	Factory::RegisterElementInstancer("form", &d->form);
	Factory::RegisterElementInstancer("input", &d->input);
	Factory::RegisterElementInstancer("select", &d->select);
	Factory::RegisterElementInstancer("label", &d->element_label);
	Factory::RegisterElementInstancer("textarea", &d->textarea);
	Factory::RegisterElementInstancer("#selection", &d->selection);
	Factory::RegisterElementInstancer("tabset", &d->tabset);
	Factory::RegisterElementInstancer("progress", &d->progress);
	Factory::RegisterElementInstancer("progressbar", &d->progress);

	// clang-format off
	Factory::RegisterDataViewInstancer(&d->data_view_attribute,      "attr",    false);
	Factory::RegisterDataViewInstancer(&d->data_view_attribute_if,   "attrif",  false);
	Factory::RegisterDataViewInstancer(&d->data_view_class,          "class",   false);
	Factory::RegisterDataViewInstancer(&d->data_view_if,             "if",      false);
	Factory::RegisterDataViewInstancer(&d->data_view_visible,        "visible", false);
	Factory::RegisterDataViewInstancer(&d->data_view_rml,            "rml",     false);
	Factory::RegisterDataViewInstancer(&d->data_view_style,          "style",   false);
	Factory::RegisterDataViewInstancer(&d->data_view_text,           "text",    false);
	Factory::RegisterDataViewInstancer(&d->data_view_value,          "value",   false);
	Factory::RegisterDataViewInstancer(&d->data_view_checked,        "checked", false);
	Factory::RegisterDataViewInstancer(&d->data_view_alias,          "alias",   false);
	Factory::RegisterDataViewInstancer(&d->structural_data_view_for, "for",     true );
	// clang-format on

	Factory::RegisterDataControllerInstancer(&d->data_controller_value, "checked");
	Factory::RegisterDataControllerInstancer(&d->data_controller_event, "event");
	Factory::RegisterDataControllerInstancer(&d->data_controller_value, "value");

	XMLParser::RegisterPersistentCDATATag("script");
	XMLParser::RegisterPersistentCDATATag("style");

	XMLParser::RegisterNodeHandler("", MakeShared<XMLNodeHandlerDefault>());
	XMLParser::RegisterNodeHandler("body", MakeShared<XMLNodeHandlerBody>());
	XMLParser::RegisterNodeHandler("head", MakeShared<XMLNodeHandlerHead>());
	XMLParser::RegisterNodeHandler("template", MakeShared<XMLNodeHandlerTemplate>());
	XMLParser::RegisterNodeHandler("tabset", MakeShared<XMLNodeHandlerTabSet>());
	XMLParser::RegisterNodeHandler("textarea", MakeShared<XMLNodeHandlerTextArea>());
	XMLParser::RegisterNodeHandler("select", MakeShared<XMLNodeHandlerSelect>());
}

void Shutdown()
{
	XMLParser::ReleaseHandlers();
	extended_instancers.Shutdown();
}

} // namespace RegisterDefaultFactories
} // namespace ui
