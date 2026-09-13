#include <ui/dom/ElementUtilities.h>
#include <ui/dom/Element.h>
#include <ui/dom/Factory.h>
#include "data/DataController.h"
#include "data/DataModel.h"
#include "data/DataView.h"

namespace ui {

bool ElementUtilities::ApplyDataViewsControllers(Element* element)
{
	UI_ASSERT(element);

	// If we have an active data model, check the attributes for any data bindings
	DataModel* data_model = element->GetDataModel();
	if (!data_model)
		return false;

	struct ViewControllerInitializer {
		String type;
		String modifier;
		String expression;
		DataViewPtr view;
		DataControllerPtr controller;
		explicit operator bool() const { return view || controller; }
	};

	// Since data views and controllers may modify the element's attributes during initialization, we
	// need to iterate over all the attributes _before_ initializing any views or controllers. We store
	// the information needed to initialize them in the following container.
	Vector<ViewControllerInitializer> initializer_list;

	for (auto& attribute : element->GetAttributes())
	{
		// Data views and controllers are declared by the following element attribute:
		//     data-[type]-[modifier]="[expression]"

		constexpr size_t data_str_length = sizeof("data-") - 1;

		const String& name = attribute.first;

		if (name.size() > data_str_length && name[0] == 'd' && name[1] == 'a' && name[2] == 't' && name[3] == 'a' && name[4] == '-')
		{
			const size_t type_end = name.find('-', data_str_length);
			const size_t type_size = (type_end == String::npos ? String::npos : type_end - data_str_length);
			const String type_name = name.substr(data_str_length, type_size);

			ViewControllerInitializer initializer;

			const size_t modifier_offset = data_str_length + type_name.size() + 1;
			if (modifier_offset < name.size())
				initializer.modifier = name.substr(modifier_offset);

			if (DataViewPtr view = Factory::InstanceDataView(type_name, element))
				initializer.view = std::move(view);

			if (DataControllerPtr controller = Factory::InstanceDataController(type_name, element))
				initializer.controller = std::move(controller);

			if (initializer)
			{
				initializer.type = type_name;
				initializer.expression = attribute.second.Get<String>();

				if (Factory::IsStructuralDataView(type_name))
				{
					// Structural data views should cancel all other non-structural data views and controllers.
					// Clear all other views, and only initialize this one.
					// E.g. in elements with a 'data-for' attribute, the data views should be constructed on the
					// generated children elements and not on the current element generating the 'for' view.
					initializer_list.clear();
					initializer_list.push_back(std::move(initializer));
					break;
				}
				else
				{
					initializer_list.push_back(std::move(initializer));
				}
			}
		}
	}

	bool result = false;

	// Now, we can safely initialize the data views and controllers, even modifying the element's attributes when desired.
	for (ViewControllerInitializer& initializer : initializer_list)
	{
		DataViewPtr& view = initializer.view;
		DataControllerPtr& controller = initializer.controller;

		if (view)
		{
			if (view->Initialize(*data_model, element, initializer.expression, initializer.modifier))
			{
				data_model->AddView(std::move(view));
				result = true;
			}
			else
				Log::Message(Log::LT_WARNING, "Could not add data-%s view to element: %s", initializer.type.c_str(), element->GetAddress().c_str());
		}

		if (controller)
		{
			if (controller->Initialize(*data_model, element, initializer.expression, initializer.modifier))
			{
				data_model->AddController(std::move(controller));
				result = true;
			}
			else
				Log::Message(Log::LT_WARNING, "Could not add data-%s controller to element: %s", initializer.type.c_str(),
					element->GetAddress().c_str());
		}
	}

	return result;
}

} // namespace ui
