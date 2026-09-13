#include <ui/dom/Factory.h>
#include <ui/base/Log.h>
#include "base/ControlledLifetimeResource.h"
#include "data/DataController.h"
#include "data/DataView.h"

namespace ui {

namespace {

struct FactoryDataBindingState {
	UnorderedMap<String, DataViewInstancer*> data_view_instancers;
	UnorderedMap<String, DataControllerInstancer*> data_controller_instancers;
	SmallUnorderedSet<String> structural_data_view_attribute_names;
};

ControlledLifetimeResource<FactoryDataBindingState> data_binding_state;

} // namespace

// Ensure storage exists for the lifetime of Factory::Initialise/Shutdown.
// Called from RegisterDefaultFactories / Factory lifecycle via these methods.


void Factory::RegisterDataViewInstancer(DataViewInstancer* instancer, const String& name, bool is_structural_view)
{
	data_binding_state.InitializeIfEmpty();
	const bool inserted = data_binding_state->data_view_instancers.emplace(name, instancer).second;
	if (!inserted)
	{
		Log::Message(Log::LT_WARNING, "Could not register data view instancer '%s'. The given name is already registered.", name.c_str());
		return;
	}
	if (is_structural_view)
		data_binding_state->structural_data_view_attribute_names.emplace("data-" + name);
}

void Factory::RegisterDataControllerInstancer(DataControllerInstancer* instancer, const String& name)
{
	data_binding_state.InitializeIfEmpty();
	bool inserted = data_binding_state->data_controller_instancers.emplace(name, instancer).second;
	if (!inserted)
		Log::Message(Log::LT_WARNING, "Could not register data controller instancer '%s'. The given name is already registered.", name.c_str());
}

DataViewPtr Factory::InstanceDataView(const String& type_name, Element* element)
{
	if (!data_binding_state)
		return nullptr;
	UI_ASSERT(element);
	const auto it = data_binding_state->data_view_instancers.find(type_name);
	if (it != data_binding_state->data_view_instancers.end())
		return it->second->InstanceView(element);
	return nullptr;
}

DataControllerPtr Factory::InstanceDataController(const String& type_name, Element* element)
{
	if (!data_binding_state)
		return nullptr;
	const auto it = data_binding_state->data_controller_instancers.find(type_name);
	if (it != data_binding_state->data_controller_instancers.end())
		return it->second->InstanceController(element);
	return nullptr;
}

bool Factory::IsStructuralDataView(const String& type_name)
{
	if (!data_binding_state)
		return false;
	const String attribute = "data-" + type_name;
	return data_binding_state->structural_data_view_attribute_names.find(attribute) != data_binding_state->structural_data_view_attribute_names.end();
}

const SmallUnorderedSet<String>& Factory::GetStructuralDataViewAttributeNames()
{
	static const SmallUnorderedSet<String> empty;
	if (!data_binding_state)
		return empty;
	return data_binding_state->structural_data_view_attribute_names;
}

void ShutdownFactoryDataBindings()
{
	if (data_binding_state)
		data_binding_state.Shutdown();
}


} // namespace ui
