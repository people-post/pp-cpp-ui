#pragma once

#include <ui/style/Decorator.h>
#include <ui/paint/Geometry.h>
#include <ui/base/ID.h>
#include <ui/style/Spritesheet.h>
#include "base/Pool.h"

namespace ui {

class DecoratorShader : public Decorator {
public:
	DecoratorShader();
	virtual ~DecoratorShader();

	bool Initialise(String&& value);

	DecoratorDataHandle GenerateElementData(Element* element, BoxArea paint_area) const override;
	void ReleaseElementData(DecoratorDataHandle element_data) const override;

	void RenderElement(Element* element, DecoratorDataHandle element_data) const override;

private:
	String value;
};

class DecoratorShaderInstancer : public DecoratorInstancer {
public:
	DecoratorShaderInstancer();
	~DecoratorShaderInstancer();

	SharedPtr<Decorator> InstanceDecorator(const String& name, const PropertyDictionary& properties,
		const DecoratorInstancerInterface& instancer_interface) override;

private:
	struct PropertyIds {
		PropertyId value;
	};
	PropertyIds ids;
};

struct ShaderElementData {
	ShaderElementData(Geometry&& geometry, CompiledShader&& shader) : geometry(std::move(geometry)), shader(std::move(shader)) {}
	Geometry geometry;
	CompiledShader shader;
};
Pool<ShaderElementData>& GetShaderElementDataPool();

} // namespace ui
