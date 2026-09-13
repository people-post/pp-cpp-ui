#pragma once

#include <ui/base/Header.h>
#include <ui/base/Traits.h>
#include <ui/base/Types.h>

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
