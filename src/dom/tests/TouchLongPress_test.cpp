#include "TestsInterface.h"
#include "TestsShell.h"
#include <ui/core/Core.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/Event.h>
#include <ui/dom/EventListener.h>
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

TEST_CASE("keep_focus.press_on_chrome_keeps_the_field_focused")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<style>
		div, button, textarea { display: block; width: 200px; height: 40px; }
		#composer { height: auto; }
	</style>
</head>
<body>
<div id="composer" keep-focus="">
	<textarea id="field"></textarea>
	<button id="send">send</button>
</div>
<div id="outside"/>
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

	// The send button is chrome of the field: pressing it must not blur the field.
	press("send");
	CHECK(context->GetFocusElement() == field);

	// A press elsewhere still moves the focus, and a press on the field itself gives it back.
	press("outside");
	CHECK(context->GetFocusElement() != field);
	press("field");
	CHECK(context->GetFocusElement() == field);

	document->Close();
	TestsShell::ShutdownShell();
}

// A composer's text field above a scrollable transcript: on a phone a touch on the transcript that turns
// into a scroll keeps the field focused (the on-screen keyboard stays up); a plain tap blurs it on release.
static const String touch_focus_doc_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<style>
		div, button, textarea { display: block; width: 200px; height: 40px; }
		#transcript { height: 100px; overflow-y: auto; }
		#message { height: 400px; }
		#composer { height: auto; }
	</style>
</head>
<body>
<div id="transcript"><div id="message"/></div>
<div id="composer" keep-focus="">
	<textarea id="field"></textarea>
	<button id="send">send</button>
</div>
</body>
</rml>
)";

namespace {
struct ClickRecorder : public EventListener {
	Context* context = nullptr;
	Element* field = nullptr;
	int clicks = 0;
	bool field_focused_at_click = false;
	void ProcessEvent(Event& /*event*/) override
	{
		++clicks;
		field_focused_at_click = context->GetFocusElement() == field;
	}
};
} // namespace

TEST_CASE("touch_focus.scroll_keeps_the_field_focused_tap_blurs_on_release")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(touch_focus_doc_rml);
	REQUIRE(document);
	document->Show();
	context->Update();
	context->Render();

	Element* field = document->GetElementById("field");
	Element* message = document->GetElementById("message");
	ClickRecorder recorder;
	recorder.context = context;
	recorder.field = field;
	message->AddEventListener(EventId::Click, &recorder);

	const Vector2f on_message = message->GetAbsoluteOffset(BoxArea::Border) + Vector2f(20.f, 60.f);

	SUBCASE("a touch that scrolls the transcript keeps the field focused")
	{
		field->Focus();
		REQUIRE(context->GetFocusElement() == field);

		context->ProcessTouchStart(TouchList{Touch{TouchId(1), on_message}}, 0);
		CHECK(context->GetFocusElement() == field);
		context->ProcessTouchMove(TouchList{Touch{TouchId(1), on_message + Vector2f(0.f, -15.f)}}, 0);
		context->ProcessTouchMove(TouchList{Touch{TouchId(1), on_message + Vector2f(0.f, -40.f)}}, 0);
		context->ProcessTouchEnd(TouchList{Touch{TouchId(1), on_message + Vector2f(0.f, -40.f)}}, 0);
		CHECK(context->GetFocusElement() == field);
		CHECK(recorder.clicks == 0);
	}

	SUBCASE("a tap on the transcript blurs the field on release, before the click")
	{
		field->Focus();
		REQUIRE(context->GetFocusElement() == field);

		context->ProcessTouchStart(TouchList{Touch{TouchId(1), on_message}}, 0);
		// Still focused while the finger is down: it may yet become a scroll.
		CHECK(context->GetFocusElement() == field);
		context->ProcessTouchEnd(TouchList{Touch{TouchId(1), on_message}}, 0);
		CHECK(context->GetFocusElement() != field);
		CHECK(recorder.clicks == 1);
		CHECK_FALSE(recorder.field_focused_at_click);
	}

	SUBCASE("a tap on keep-focus chrome keeps the field focused")
	{
		field->Focus();
		REQUIRE(context->GetFocusElement() == field);

		const Vector2f on_send = document->GetElementById("send")->GetAbsoluteOffset(BoxArea::Border) + Vector2f(5.f, 5.f);
		context->ProcessTouchStart(TouchList{Touch{TouchId(1), on_send}}, 0);
		context->ProcessTouchEnd(TouchList{Touch{TouchId(1), on_send}}, 0);
		CHECK(context->GetFocusElement() == field);
	}

	SUBCASE("a mouse press on the transcript blurs the field at once")
	{
		field->Focus();
		REQUIRE(context->GetFocusElement() == field);

		context->ProcessMouseMove(int(on_message.x), int(on_message.y), 0);
		context->ProcessMouseButtonDown(0, 0);
		CHECK(context->GetFocusElement() != field);
		context->ProcessMouseButtonUp(0, 0);
		CHECK(recorder.clicks == 1);
	}

	message->RemoveEventListener(EventId::Click, &recorder);
	document->Close();
	TestsShell::ShutdownShell();
}
