#include "TestsInterface.h"
#include "TestsShell.h"
#include <ui/core/Core.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <doctest.h>

using namespace ui;

static const String long_press_doc_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<style>
		#area { display: block; width: 300px; height: 100px; }
	</style>
</head>
<body>
<div id="area"/>
</body>
</rml>
)";

TEST_CASE("touch.long_press_wakes_idle_host")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);
	TestsSystemInterface* system_interface = TestsShell::GetTestsSystemInterface();
	system_interface->SetManualTime(10.0);

	ElementDocument* document = context->LoadDocumentFromMemory(long_press_doc_rml);
	REQUIRE(document);
	document->Show();
	context->Update();
	context->Render();

	int fired = 0;
	context->SetTouchLongPressCallback([&fired](Vector2i, Element*) { ++fired; });

	const Vector2f inside = document->GetElementById("area")->GetAbsoluteOffset(BoxArea::Border) + Vector2f(20.f, 20.f);
	context->ProcessTouchStart(TouchList{Touch{TouchId(1), inside}}, 0);

	// A still finger sends no further events: the context itself must ask to be updated when the hold is due.
	context->Update();
	CHECK(fired == 0);
	CHECK(context->GetNextUpdateDelay() > 0.0);
	CHECK(context->GetNextUpdateDelay() <= 0.5);

	system_interface->SetManualTime(10.6);
	context->Update();
	CHECK(fired == 1);

	context->ProcessTouchEnd(TouchList{Touch{TouchId(1), inside}}, 0);
	context->SetTouchLongPressCallback({});
	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("context_menu.press_keeps_focus")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<style>
		div { display: block; width: 200px; height: 40px; }
	</style>
</head>
<body>
<div id="field"/>
<div id="outside"/>
<div id="context-menu-layer"><div id="item"/></div>
</body>
</rml>
)");
	REQUIRE(document);
	document->Show();
	context->Update();
	context->Render();

	Element* field = document->GetElementById("field");
	const auto press = [&](const char* id) {
		const Vector2f at = document->GetElementById(id)->GetAbsoluteOffset(BoxArea::Border) + Vector2f(5.f, 5.f);
		context->ProcessMouseMove(int(at.x), int(at.y), 0);
		context->ProcessMouseButtonDown(0, 0);
		context->ProcessMouseButtonUp(0, 0);
	};

	field->Focus();
	REQUIRE(context->GetFocusElement() == field);

	// The menu acts on the focused field; pressing one of its items must not blur that field.
	press("item");
	CHECK(context->GetFocusElement() == field);

	press("outside");
	CHECK(context->GetFocusElement() != field);

	document->Close();
	TestsShell::ShutdownShell();
}
