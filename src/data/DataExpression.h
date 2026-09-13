#pragma once

#include <ui/data/DataTypes.h>
#include <ui/base/Header.h>
#include <ui/base/Types.h>
namespace ui {

class Element;
class DataModel;
struct InstructionData;
using Program = Vector<InstructionData>;
using AddressList = Vector<DataAddress>;

class DataExpressionInterface {
public:
	DataExpressionInterface() = default;
	DataExpressionInterface(DataModel* data_model, Element* element, Event* event = nullptr);

	DataAddress ParseAddress(const String& address_str) const;
	Variant GetValue(const DataAddress& address) const;
	bool SetValue(const DataAddress& address, const Variant& value) const;
	bool CallTransform(const String& name, const VariantList& arguments, Variant& out_result);
	bool EventCallback(const String& name, const VariantList& arguments);

private:
	DataModel* data_model = nullptr;
	Element* element = nullptr;
	Event* event = nullptr;
};

class DataExpression {
public:
	DataExpression(String expression);
	~DataExpression();

	bool Parse(const DataExpressionInterface& expression_interface, bool is_assignment_expression);

	bool Run(const DataExpressionInterface& expression_interface, Variant& out_value);

	// Available after Parse()
	StringList GetVariableNameList() const;

private:
	String expression;

	Program program;
	AddressList addresses;
};

} // namespace ui
