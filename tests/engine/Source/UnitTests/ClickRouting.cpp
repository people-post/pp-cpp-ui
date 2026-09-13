#include "ClickRouting.h"

#include <ui/core/Core.h>
#include <ui/dom/Element.h>
#include <ui/dom/Factory.h>
#include <ui/core/SystemInterface.h>
#include <ui/base/Types.h>
#include <doctest.h>

#include <unordered_map>

namespace {

ui::SystemInterface g_system_interface;
std::unordered_map<ui::Element*, bool> g_contains_point;
// Initialise Core without TestsShell; shut down on scope exit so later tests can re-init.
struct UiCoreGuard {
	UiCoreGuard()
	{
		ui::SetSystemInterface(&g_system_interface);
		REQUIRE(ui::Initialise());
	}
	~UiCoreGuard() { ui::Shutdown(); }
};

ui::ElementPtr MakeElement(const char* tag)
{
	ui::ElementPtr element = ui::Factory::InstanceElement(nullptr, "*", tag, ui::XMLAttributes());
	REQUIRE(static_cast<bool>(element));
	return element;
}

ui::Element* AppendChild(ui::Element& parent, const char* tag)
{
	ui::ElementPtr child = ui::Factory::InstanceElement(&parent, "*", tag, ui::XMLAttributes());
	REQUIRE(static_cast<bool>(child));
	ui::Element* inserted = parent.AppendChild(std::move(child), false);
	REQUIRE(inserted != nullptr);
	return inserted;
}

bool MockPointWithin(ui::Element* element, ui::Vector2f /*point*/, void* /*context*/)
{
	const auto it = g_contains_point.find(element);
	if (it == g_contains_point.end())
		return false;
	return it->second;
}

void SetContains(ui::Element* element, bool contains) { g_contains_point[element] = contains; }

ui::Element* NullFocus(ui::Element* /*element*/) { return nullptr; }

ui::Element* g_focus_result = nullptr;
ui::Element* ReturnFocusResult(ui::Element* /*element*/) { return g_focus_result; }

ui::Element* Resolve(ui::Element* press_hover, ui::Element* release_hover, ui::Vector2f point,
	ui::ClickRouting::FindFocusElementFn find_focus)
{
	return ui::ClickRouting::ResolveClickTargetWithPredicate(press_hover, release_hover, point, find_focus, MockPointWithin, nullptr);
}

} // namespace

TEST_CASE("ClickRouting.TreeHelpers")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr root_ptr = MakeElement("div");
	REQUIRE(static_cast<bool>(root_ptr));
	ui::Element& root = *root_ptr;
	ui::Element* child = AppendChild(root, "span");
	REQUIRE(child != nullptr);
	ui::Element* grandchild = AppendChild(*child, "option");
	REQUIRE(grandchild != nullptr);

	CHECK(ui::ClickRouting::IsAncestorOf(&root, grandchild));
	CHECK(ui::ClickRouting::IsAncestorOf(child, grandchild));
	CHECK_FALSE(ui::ClickRouting::IsAncestorOf(grandchild, &root));

	CHECK(ui::ClickRouting::InSameClickTree(grandchild, grandchild));
	CHECK(ui::ClickRouting::InSameClickTree(child, grandchild));
	CHECK(ui::ClickRouting::InSameClickTree(grandchild, child));

	ui::Element* sibling_b = AppendChild(root, "span");
	REQUIRE(sibling_b != nullptr);
	CHECK_FALSE(ui::ClickRouting::InSameClickTree(child, sibling_b));
}

TEST_CASE("ClickRouting.FindInteractiveElement")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr div_ptr = MakeElement("div");
	REQUIRE(static_cast<bool>(div_ptr));
	ui::Element& div = *div_ptr;
	ui::Element* button = AppendChild(div, "button");
	REQUIRE(button != nullptr);
	ui::Element* text = AppendChild(*button, "span");
	REQUIRE(text != nullptr);

	CHECK(ui::ClickRouting::FindInteractiveElement(text) == button);

	ui::ElementPtr scrim_ptr = MakeElement("div");
	REQUIRE(static_cast<bool>(scrim_ptr));
	ui::Element& scrim = *scrim_ptr;
	scrim.SetAttribute("data-event-click", "close()");
	CHECK(ui::ClickRouting::FindInteractiveElement(&scrim) == &scrim);
}

TEST_CASE("ClickRouting.ResolveClickTargetTier1Option")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr select_ptr = MakeElement("select");
	REQUIRE(static_cast<bool>(select_ptr));
	ui::Element& select = *select_ptr;
	ui::Element* selectbox = AppendChild(select, "selectbox");
	REQUIRE(selectbox != nullptr);
	ui::Element* option = AppendChild(*selectbox, "option");
	REQUIRE(option != nullptr);

	const ui::Vector2f point{10.f, 10.f};
	CHECK(Resolve(option, option, point, NullFocus) == option);
}

TEST_CASE("ClickRouting.ResolveClickTargetTier1ButtonChild")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr button_ptr = MakeElement("button");
	REQUIRE(static_cast<bool>(button_ptr));
	ui::Element& button = *button_ptr;
	ui::Element* label = AppendChild(button, "span");
	REQUIRE(label != nullptr);

	const ui::Vector2f point{4.f, 4.f};
	CHECK(Resolve(label, label, point, NullFocus) == label);
}

TEST_CASE("ClickRouting.ResolveClickTargetTier2SiblingChildren")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr button_ptr = MakeElement("button");
	REQUIRE(static_cast<bool>(button_ptr));
	ui::Element& button = *button_ptr;
	ui::Element* press = AppendChild(button, "span");
	ui::Element* release = AppendChild(button, "span");
	REQUIRE(press != nullptr);
	REQUIRE(release != nullptr);

	SetContains(&button, true);
	SetContains(press, false);
	SetContains(release, false);

	const ui::Vector2f point{8.f, 8.f};
	CHECK(Resolve(press, release, point, NullFocus) == &button);
}

TEST_CASE("ClickRouting.ResolveClickTargetUnrelated")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr press_ptr = MakeElement("div");
	ui::ElementPtr release_ptr = MakeElement("div");
	REQUIRE(static_cast<bool>(press_ptr));
	REQUIRE(static_cast<bool>(release_ptr));
	ui::Element& press = *press_ptr;
	ui::Element& release = *release_ptr;
	SetContains(&press, true);
	SetContains(&release, true);

	const ui::Vector2f point{1.f, 1.f};
	CHECK(Resolve(&press, &release, point, NullFocus) == nullptr);
}

TEST_CASE("ClickRouting.ResolveClickTargetTier3Geometry")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr press_ptr = MakeElement("div");
	REQUIRE(static_cast<bool>(press_ptr));
	ui::Element& press = *press_ptr;
	SetContains(&press, true);

	const ui::Vector2f point{2.f, 2.f};
	CHECK(Resolve(&press, nullptr, point, NullFocus) == &press);
}

TEST_CASE("ClickRouting.ResolveClickTargetTier3Focus")
{
	UiCoreGuard ui_core_guard;
	g_contains_point.clear();

	ui::ElementPtr press_ptr = MakeElement("div");
	ui::ElementPtr release_ptr = MakeElement("span");
	REQUIRE(static_cast<bool>(press_ptr));
	REQUIRE(static_cast<bool>(release_ptr));
	ui::Element& press = *press_ptr;
	ui::Element& release = *release_ptr;
	g_focus_result = &press;
	SetContains(&press, false);

	const ui::Vector2f point{0.f, 0.f};
	CHECK(Resolve(&press, &release, point, ReturnFocusResult) == &press);
	g_focus_result = nullptr;
}
