#pragma once

#include "Header.h"
#include "Traits.h"
#include "Types.h"

namespace ui {

/**
    Base class for all objects that hold a scriptable object.
 */

class UI_CORE_API ScriptInterface : public Releasable {
public:
	UI_RTTI_Define(ScriptInterface)

	virtual ~ScriptInterface() {}

	virtual ScriptObject GetScriptObject() const { return nullptr; }
};

} // namespace ui
