#pragma once

#include <ui/base/Header.h>
#include <ui/dom/Input.h>
#include <ui/base/Types.h>
#include <ui/base/Vector2.h>

namespace ui {

class Context;
class Element;

class UI_CORE_API SelectionController {
public:
	bool CanSelectStaticText(Element* target) const;
	bool BlocksTarget(Element* target) const;
	void SelectWordAt(Vector2i position);
	void SelectAll();
	void FinalizeSelection();
	void OnPointerUp();
	String GetSelectedText();
	bool HasSelection() const;
};

} // namespace ui
