#include <ui/dom/Element.h>
#include <ui/dom/Context.h>
#include "data/DataModel.h"

namespace ui {

void Element::SetDataModel(DataModel* new_data_model)
{
	UI_ASSERTMSG(!data_model || !new_data_model, "We must either attach a new data model, or detach the old one.");

	if (data_model == new_data_model)
		return;

	// stop descent if a nested data model is encountered
	if (data_model && new_data_model && data_model != new_data_model)
		return;

	if (data_model)
		data_model->OnElementRemove(this);

	data_model = new_data_model;

	if (data_model)
		ElementUtilities::ApplyDataViewsControllers(this);

	for (ElementPtr& child : children)
		child->SetDataModel(new_data_model);
}


bool Element::AttachNamedDataModel(const String& name)
{
	Context* context = GetContext();
	if (!context)
		return false;
	if (DataModel* model = context->GetDataModelPtr(name))
	{
		model->AttachModelRootElement(this);
		SetDataModel(model);
		return true;
	}
	return false;
}

} // namespace ui
