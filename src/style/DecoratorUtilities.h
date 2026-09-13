#pragma once

#include <ui/base/NumericValue.h>
#include <ui/base/Types.h>
namespace ui {

using Vector2Numeric = Vector2<NumericValue>;

// Compute a 2d-position property value into a percentage-length vector.
Vector2Numeric ComputePosition(Array<const Property*, 2> p_position);

} // namespace ui
