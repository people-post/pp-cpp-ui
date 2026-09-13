#include <ui/base/Traits.h>
namespace ui {

int FamilyBase::GetNewId()
{
	static int id = 0;
	return id++;
}

} // namespace ui
