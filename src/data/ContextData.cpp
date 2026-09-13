#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/data/DataModelHandle.h>
#include "data/DataModel.h"

namespace ui {

void DataModelDeleter::operator()(DataModel* model) const
{
	delete model;
}


void Context::UpdateDataModels(bool clear_dirty_variables)
{
	for (auto& data_model : data_models)
		data_model.second->Update(clear_dirty_variables);
}


DataModelConstructor Context::CreateDataModel(const String& name, DataTypeRegister* data_type_register)
{
	if (!data_type_register)
	{
		if (!default_data_type_register)
			default_data_type_register = MakeUnique<DataTypeRegister>();
		data_type_register = default_data_type_register.get();
	}

	auto result = data_models.emplace(name, std::unique_ptr<DataModel, DataModelDeleter>(new DataModel(data_type_register)));
	bool inserted = result.second;
	if (inserted)
		return DataModelConstructor(result.first->second.get());

	Log::Message(Log::LT_ERROR, "Data model name '%s' already exists.", name.c_str());
	return DataModelConstructor();
}

DataModelConstructor Context::GetDataModel(const String& name)
{
	if (DataModel* model = GetDataModelPtr(name))
		return DataModelConstructor(model);

	Log::Message(Log::LT_ERROR, "Data model name '%s' could not be found.", name.c_str());
	return DataModelConstructor();
}

UnorderedMap<String, DataModelConstructor> Context::GetDataModels() const
{
	UnorderedMap<String, DataModelConstructor> result;
	result.reserve(data_models.size());
	for (const auto& pair : data_models)
		result.emplace(pair.first, DataModelConstructor(pair.second.get()));
	return result;
}

bool Context::RemoveDataModel(const String& name)
{
	auto it = data_models.find(name);
	if (it == data_models.end())
		return false;

	DataModel* model = it->second.get();
	ElementList elements = model->GetAttachedModelRootElements();

	for (Element* element : elements)
		element->SetDataModel(nullptr);

	data_models.erase(it);

	return true;
}

DataModel* Context::GetDataModelPtr(const String& name) const
{
	auto it = data_models.find(name);
	if (it != data_models.end())
		return it->second.get();
	return nullptr;
}

} // namespace ui
