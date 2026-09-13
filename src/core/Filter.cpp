#include <ui/Core/Filter.h>
#include <ui/Core/RenderManager.h>

namespace ui {

Filter::Filter() {}

Filter::~Filter() {}

void Filter::ExtendInkOverflow(Element* /*element*/, Rectanglef& /*scissor_region*/) const {}

FilterInstancer::FilterInstancer() {}

FilterInstancer::~FilterInstancer() {}

} // namespace ui
