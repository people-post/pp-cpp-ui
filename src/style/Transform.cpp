#include <ui/Core/Transform.h>
#include <ui/Core/Property.h>
#include <ui/Core/StyleSheetSpecification.h>
#include <ui/Core/TransformPrimitive.h>

namespace ui {

Transform::Transform() {}

Transform::Transform(PrimitiveList primitives) : primitives(std::move(primitives)) {}

Property Transform::MakeProperty(PrimitiveList primitives)
{
	Property p(MakeShared<Transform>(std::move(primitives)), Unit::TRANSFORM);
	p.definition = StyleSheetSpecification::GetProperty(PropertyId::Transform);
	return p;
}

void Transform::ClearPrimitives()
{
	primitives.clear();
}

void Transform::AddPrimitive(const TransformPrimitive& p)
{
	primitives.push_back(p);
}

int Transform::GetNumPrimitives() const noexcept
{
	return (int)primitives.size();
}

const TransformPrimitive& Transform::GetPrimitive(int i) const noexcept
{
	return primitives[i];
}

} // namespace ui
