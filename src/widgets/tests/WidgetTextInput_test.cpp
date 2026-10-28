#include "TestsShell.h"
#include "TestsInterface.h"
#include <ui/core/Core.h>
#include <ui/base/TextInputContext.h>
#include <ui/base/TextInputHandler.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/EventListener.h>
#include <ui/widgets/ElementFormControl.h>
#include <ui/widgets/ElementFormControlInput.h>
#include <ui/widgets/ElementFormControlTextArea.h>
#include <doctest.h>

using namespace ui;

static const String text_input_doc_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<style>
		input, textarea { display: block; width: 300px; }
		textarea { height: 100px; }
	</style>
</head>
<body>
<input type="text" id="input"/>
<textarea id="area"></textarea>
</body>
</rml>
)";

namespace {

// Test system interface with an in-memory clipboard.
class ClipboardSystemInterface : public TestsSystemInterface {
public:
	void SetClipboardText(const String& text) override { clipboard = text; }
	void GetClipboardText(String& text) override { text = clipboard; }
	String clipboard;
};

// Captures the text input context of the focused widget, like a platform IME handler would.
class CapturingTextInputHandler : public TextInputHandler {
public:
	void OnActivate(TextInputContext* c) override { context = c; }
	void OnDeactivate(TextInputContext*) override {}
	void OnDestroy(TextInputContext* c) override
	{
		if (context == c)
			context = nullptr;
	}
	TextInputContext* context = nullptr;
};

struct Fixture {
	Context* context = nullptr;
	ElementDocument* document = nullptr;
	ElementFormControl* control = nullptr;

	explicit Fixture(const char* id, Context* in_context = nullptr)
	{
		context = in_context ? in_context : TestsShell::GetContext();
		REQUIRE(context);
		document = context->LoadDocumentFromMemory(text_input_doc_rml);
		REQUIRE(document);
		document->Show();
		context->Update();
		context->Render();
		control = ui_dynamic_cast<ElementFormControl*>(document->GetElementById(id));
		REQUIRE(control);
		control->Focus();
		context->Update();
	}
	~Fixture()
	{
		document->Close();
		TestsShell::ShutdownShell();
	}

	void Type(const String& s) { context->ProcessTextInput(s); }
	void Key(Input::KeyIdentifier key, int mods = 0)
	{
		context->ProcessKeyDown(key, mods);
		context->ProcessKeyUp(key, mods);
	}
	String Value() const { return control->GetValue(); }
};

} // namespace

TEST_SUITE_BEGIN("WidgetTextInput");

TEST_CASE("backspace removes CJK characters one at a time")
{
	for (const char* id : {"input", "area"})
	{
		Fixture f(id);
		f.Type("\xe4\xbd\xa0\xe5\xa5\xbd"); // 你好
		CHECK(f.Value() == "\xe4\xbd\xa0\xe5\xa5\xbd");
		f.Key(Input::KI_BACK);
		CHECK(f.Value() == "\xe4\xbd\xa0");
		f.Key(Input::KI_BACK);
		CHECK(f.Value() == "");
	}
}

TEST_CASE("backspace on mixed ASCII and CJK")
{
	Fixture f("input");
	f.Type("a\xe4\xbd\xa0"
		   "b");
	f.Key(Input::KI_BACK);
	CHECK(f.Value() == "a\xe4\xbd\xa0");
	f.Key(Input::KI_BACK);
	CHECK(f.Value() == "a");
	f.Key(Input::KI_BACK);
	CHECK(f.Value() == "");
}

TEST_CASE("delete forward removes a CJK character")
{
	Fixture f("input");
	f.Type("\xe4\xbd\xa0\xe5\xa5\xbd");
	f.Key(Input::KI_HOME);
	f.Key(Input::KI_DELETE);
	CHECK(f.Value() == "\xe5\xa5\xbd");
}

TEST_CASE("arrow keys move over whole CJK characters")
{
	Fixture f("input");
	f.Type("\xe4\xbd\xa0\xe5\xa5\xbd");
	f.Key(Input::KI_LEFT);
	f.Type("X");
	CHECK(f.Value() == "\xe4\xbd\xa0"
					   "X\xe5\xa5\xbd");
	f.Key(Input::KI_HOME);
	f.Key(Input::KI_RIGHT);
	f.Type("Y");
	CHECK(f.Value() == "\xe4\xbd\xa0"
					   "YX\xe5\xa5\xbd");
}

TEST_CASE("backspace removes an emoji with skin tone and a flag in one press")
{
	Fixture f("input");
	f.Type("\xf0\x9f\x91\x8d\xf0\x9f\x8f\xbd"); // thumbs up + medium skin tone
	f.Key(Input::KI_BACK);
	CHECK(f.Value() == "");
	f.Type("\xf0\x9f\x87\xa8\xf0\x9f\x87\xb3"); // CN flag
	f.Key(Input::KI_BACK);
	CHECK(f.Value() == "");
}

TEST_CASE("select all, copy, paste with the platform command modifier")
{
	ClipboardSystemInterface clipboard_interface;

	for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
	{
		// The shell installs its own system interface on initialization, so swap ours in afterwards.
		REQUIRE(TestsShell::GetContext());
		SetSystemInterface(&clipboard_interface);
		clipboard_interface.clipboard.clear();
		Fixture f("input");
		f.Type("\xe4\xbd\xa0\xe5\xa5\xbd");
		f.Key(Input::KI_A, mod);
		f.Key(Input::KI_C, mod);
		CHECK(clipboard_interface.clipboard == "\xe4\xbd\xa0\xe5\xa5\xbd");
		f.Key(Input::KI_RIGHT);
		f.Key(Input::KI_V, mod);
		CHECK(f.Value() == "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xbd\xa0\xe5\xa5\xbd");

		// Cut.
		f.Key(Input::KI_A, mod);
		f.Key(Input::KI_X, mod);
		CHECK(f.Value() == "");
		CHECK(clipboard_interface.clipboard == "\xe4\xbd\xa0\xe5\xa5\xbd\xe4\xbd\xa0\xe5\xa5\xbd");
	}
}

TEST_CASE("IME composition then commit does not block backspace")
{
	REQUIRE(TestsShell::GetContext());
	CapturingTextInputHandler handler;
	Context* ime_context = CreateContext("ime", Vector2i(1024, 768), nullptr, &handler);
	REQUIRE(ime_context);
	{
		Fixture f("input", ime_context);
		REQUIRE(handler.context);

		// Simulated IME: pre-edit "ni" shown as composition, then committed as "你好".
		handler.context->SetText("ni", 0, 0);
		handler.context->SetCompositionRange(0, 2);
		handler.context->CommitComposition("\xe4\xbd\xa0\xe5\xa5\xbd");
		CHECK(f.Value() == "\xe4\xbd\xa0\xe5\xa5\xbd");

		f.Key(Input::KI_END);
		f.Key(Input::KI_BACK);
		CHECK(f.Value() == "\xe4\xbd\xa0");
		f.Key(Input::KI_BACK);
		CHECK(f.Value() == "");
	}
}

TEST_CASE("dragging a selection handle moves that end of the selection")
{
	Fixture f("input");
	f.Type("hello world, hello world");
	auto* input = ui_dynamic_cast<ElementFormControlInput*>(f.control);
	REQUIRE(input);
	input->Select();
	f.context->Update();

	int start = -1, end = -1;
	input->GetSelection(&start, &end, nullptr);
	REQUIRE(start == 0);
	REQUIRE(end == 24);

	// The start handle sits at the first character: press it and drag to the right.
	const Vector2f origin = f.control->GetAbsoluteOffset(BoxArea::Content);
	f.context->ProcessMouseMove(int(origin.x) + 2, int(origin.y) + 4, 0);
	f.context->ProcessMouseButtonDown(0, 0);
	for (int dx = 10; dx <= 60; dx += 10)
		f.context->ProcessMouseMove(int(origin.x) + 2 + dx, int(origin.y) + 4, 0);
	f.context->ProcessMouseButtonUp(0, 0);

	input->GetSelection(&start, &end, nullptr);
	CHECK(start > 0);
	CHECK(end == 24);
}

TEST_CASE("a handle drag ends on release and works right after a double click")
{
	Fixture f("input");
	f.Type("hello world, hello world");
	auto* input = ui_dynamic_cast<ElementFormControlInput*>(f.control);
	REQUIRE(input);
	const Vector2f origin = f.control->GetAbsoluteOffset(BoxArea::Content);
	const int y = int(origin.y) + 4;
	const auto drag = [&](int from_x, int to_x) {
		f.context->ProcessMouseMove(from_x, y, 0);
		f.context->ProcessMouseButtonDown(0, 0);
		const int step = to_x > from_x ? 10 : -10;
		for (int x = from_x + step; (to_x - x) * step >= 0; x += step)
			f.context->ProcessMouseMove(x, y, 0);
		f.context->ProcessMouseButtonUp(0, 0);
	};
	int start = -1, end = -1;

	// A double click selects a word; the start handle must then follow a drag (it used to be ignored).
	f.context->ProcessMouseMove(int(origin.x) + 10, y, 0);
	f.context->ProcessMouseButtonDown(0, 0);
	f.context->ProcessMouseButtonUp(0, 0);
	f.context->ProcessMouseButtonDown(0, 0);
	f.context->ProcessMouseButtonUp(0, 0);
	input->GetSelection(&start, &end, nullptr);
	REQUIRE(start == 0);
	REQUIRE(end == 5);
	const int old_end = end;
	drag(int(origin.x) + 2, int(origin.x) + 22);
	input->GetSelection(&start, &end, nullptr);
	CHECK(start > 0);
	CHECK(end == old_end);

	// The handle drag is over: a press far from both handles starts a new selection there instead of
	// moving the old handle.
	const int far_x = int(origin.x) + 90;
	drag(far_x, far_x + 30);
	int new_start = -1, new_end = -1;
	input->GetSelection(&new_start, &new_end, nullptr);
	CHECK(new_start >= old_end);
	CHECK(new_end > new_start);
}

TEST_CASE("a click in a scrolled textarea lands on the line under the pointer")
{
	Fixture f("area");
	String value;
	for (int i = 0; i < 20; ++i)
		value += "line" + ToString(i % 10) + "\n";
	f.control->SetValue(value);
	f.context->Update();
	f.context->Render();
	auto* area = ui_dynamic_cast<ElementFormControlTextArea*>(f.control);
	REQUIRE(area);

	// Scroll to the end, as typing there does.
	f.control->SetScrollTop(f.control->GetScrollHeight());
	f.context->Update();
	const float scroll_top = f.control->GetScrollTop();
	REQUIRE(scroll_top > 0.f);

	const Vector2f origin = f.control->GetAbsoluteOffset(BoxArea::Content);
	const auto click_line_offset = [&](float dy) {
		f.context->ProcessMouseMove(int(origin.x) + 2, int(origin.y + dy), 0);
		f.context->ProcessMouseButtonDown(0, 0);
		f.context->ProcessMouseButtonUp(0, 0);
		int start = -1, end = -1;
		area->GetSelection(&start, &end, nullptr);
		return start;
	};

	// The first visible line is not the last line of the text, and a lower click lands further down.
	const int top = click_line_offset(4.f);
	const int lower = click_line_offset(60.f);
	const int last_line_start = 20 * 6;
	CHECK(top < last_line_start - 6);
	CHECK(lower > top);
}

namespace {

// Clipboard interface whose clock the test moves by hand, so pauses between keystrokes need no sleeping.
struct UndoEnv {
	ClipboardSystemInterface system;
	double now = 100.0;
	UndoEnv()
	{
		REQUIRE(TestsShell::GetContext());
		// The shell installs its own system interface on initialization, so swap ours in afterwards.
		SetSystemInterface(&system);
		system.SetManualTime(now);
	}
	void Wait(double seconds)
	{
		now += seconds;
		system.SetManualTime(now);
	}
};

const char* const CJK = "\xe4\xbd\xa0\xe5\xa5\xbd"; // 你好

constexpr int kShift = Input::KM_SHIFT;

} // namespace

TEST_CASE("undo after typing restores the empty field, redo brings the text back")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			for (char c : String("hello"))
				f.Type(String(1, c));
			CHECK(f.Value() == "hello");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
			// Nothing left to undo.
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");

			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == "hello");
			// Caret is back at the end: typing continues there.
			f.Type("!");
			CHECK(f.Value() == "hello!");
		}
}

TEST_CASE("typing is undone word by word")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			for (char c : String("one two three"))
				f.Type(String(1, c));
			CHECK(f.Value() == "one two three");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "one two");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "one");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
		}
}

TEST_CASE("a pause of more than a second starts a new undo step")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type("a");
			env.Wait(0.5);
			f.Type("b");
			env.Wait(0.9);
			f.Type("c");
			// Under a second between keystrokes: one step.
			env.Wait(1.5);
			f.Type("d");
			f.Type("e");
			CHECK(f.Value() == "abcde");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "abc");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
		}
}

TEST_CASE("moving the caret or switching between typing and deleting starts a new undo step")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type("a");
			f.Type("b");
			f.Type("c");
			f.Key(Input::KI_BACK);
			f.Key(Input::KI_BACK);
			CHECK(f.Value() == "a");
			f.Type("x");
			CHECK(f.Value() == "ax");

			f.Key(Input::KI_Z, mod); // typing x
			CHECK(f.Value() == "a");
			f.Key(Input::KI_Z, mod); // the two backspaces together
			CHECK(f.Value() == "abc");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");

			f.Key(Input::KI_Z, mod | kShift);
			f.Key(Input::KI_Z, mod | kShift);
			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == "ax");

			// Moving the caret ends the step even without a pause.
			f.Key(Input::KI_HOME);
			f.Type("1");
			f.Key(Input::KI_END);
			f.Type("2");
			CHECK(f.Value() == "1ax2");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "1ax");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "ax");
		}
}

TEST_CASE("undo after a paste removes exactly the paste")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			env.system.clipboard = "PASTED";
			Fixture f(id);
			f.Type("a");
			f.Type("b");
			f.Key(Input::KI_V, mod);
			f.Type("c");
			CHECK(f.Value() == "abPASTEDc");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "abPASTED");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "ab");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
		}
}

TEST_CASE("undo of a cut brings back the text and its selection")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type("hello world");
			f.Key(Input::KI_A, mod);
			f.Key(Input::KI_X, mod);
			CHECK(f.Value() == "");
			CHECK(env.system.clipboard == "hello world");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "hello world");
			int start = -1, end = -1;
			auto* widget_control = ui_dynamic_cast<ElementFormControlInput*>(f.control);
			auto* area_control = ui_dynamic_cast<ElementFormControlTextArea*>(f.control);
			if (widget_control)
				widget_control->GetSelection(&start, &end, nullptr);
			else
				area_control->GetSelection(&start, &end, nullptr);
			CHECK(start == 0);
			CHECK(end == 11);

			// Backspace over a selection is one step too.
			f.Key(Input::KI_BACK);
			CHECK(f.Value() == "");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "hello world");
		}
}

TEST_CASE("a new edit after undo clears redo")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type("a");
			env.Wait(2);
			f.Type("b");
			CHECK(f.Value() == "ab");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "a");

			f.Type("c");
			CHECK(f.Value() == "ac");
			// Redo is gone; the edit is on the undo list.
			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == "ac");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "a");
		}
}

TEST_CASE("ctrl+Y redoes")
{
	for (const char* id : {"input", "area"})
	{
		UndoEnv env;
		Fixture f(id);
		f.Type("a");
		env.Wait(2);
		f.Type("b");
		f.Key(Input::KI_Z, Input::KM_CTRL);
		CHECK(f.Value() == "a");
		f.Key(Input::KI_Y, Input::KM_CTRL);
		CHECK(f.Value() == "ab");
		f.Key(Input::KI_Z, Input::KM_META);
		f.Key(Input::KI_Z, Input::KM_META);
		CHECK(f.Value() == "");
		f.Key(Input::KI_Y, Input::KM_CTRL);
		f.Key(Input::KI_Y, Input::KM_CTRL);
		CHECK(f.Value() == "ab");
	}
}

TEST_CASE("a value set by the app clears the undo history")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type("send me");
			CHECK(f.Value() == "send me");

			// The app empties the composer after sending.
			f.control->SetValue("");
			CHECK(f.Value() == "");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == "");

			// Editing works again afterwards, and only goes back as far as the new value.
			f.Type("new");
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");

			// Same for a value replaced while redo was available.
			f.Type("a");
			env.Wait(2);
			f.Type("b");
			f.Key(Input::KI_Z, mod);
			f.control->SetValue("replaced");
			f.Key(Input::KI_Z, mod | kShift);
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "replaced");
		}
}

namespace {
// Plays the app that empties its field from the change handler, like a chat composer that sends on change.
struct ClearOnChange : public EventListener {
	ElementFormControl* control = nullptr;
	bool armed = false;
	void ProcessEvent(Event&) override
	{
		if (armed)
			control->SetValue("");
	}
};
} // namespace

TEST_CASE("a value cleared from a change handler is not resurrected by undo")
{
	UndoEnv env;
	Fixture f("input");
	f.Type("keep");
	ClearOnChange listener;
	listener.control = f.control;
	f.control->AddEventListener(EventId::Change, &listener);

	listener.armed = true;
	f.Type("!");
	listener.armed = false;
	CHECK(f.Value() == "");

	f.Key(Input::KI_Z, Input::KM_CTRL);
	CHECK(f.Value() == "");
	f.control->RemoveEventListener(EventId::Change, &listener);
}

TEST_CASE("Chinese text round-trips through undo and redo")
{
	for (const char* id : {"input", "area"})
		for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
		{
			UndoEnv env;
			Fixture f(id);
			f.Type(CJK);
			env.Wait(2);
			f.Type(String("\xe4\xb8\x96\xe7\x95\x8c")); // 世界
			CHECK(f.Value() == String(CJK) + "\xe4\xb8\x96\xe7\x95\x8c");

			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == CJK);
			f.Key(Input::KI_Z, mod);
			CHECK(f.Value() == "");
			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == CJK);
			f.Key(Input::KI_Z, mod | kShift);
			CHECK(f.Value() == String(CJK) + "\xe4\xb8\x96\xe7\x95\x8c");

			// The caret came back after the last character.
			f.Type("!");
			CHECK(f.Value() == String(CJK) + "\xe4\xb8\x96\xe7\x95\x8c!");
		}
}

TEST_CASE("a line break in a textarea is its own undo step")
{
	for (int mod : {(int)Input::KM_CTRL, (int)Input::KM_META})
	{
		UndoEnv env;
		Fixture f("area");
		f.Type("a");
		f.Type("\n");
		f.Type("b");
		CHECK(f.Value() == "a\nb");
		f.Key(Input::KI_Z, mod);
		CHECK(f.Value() == "a\n");
		f.Key(Input::KI_Z, mod);
		CHECK(f.Value() == "a");
	}
}

TEST_CASE("the undo history is capped")
{
	UndoEnv env;
	Fixture f("input");
	for (int i = 0; i < 150; ++i)
	{
		f.Type("x");
		env.Wait(2); // every character its own step
	}
	CHECK(f.Value().size() == 150);
	for (int i = 0; i < 200; ++i)
		f.Key(Input::KI_Z, Input::KM_CTRL);
	CHECK(f.Value().size() == 50);
}

TEST_CASE("undo is ignored while an IME composition is active and covers the commit as one step")
{
	REQUIRE(TestsShell::GetContext());
	CapturingTextInputHandler handler;
	Context* ime_context = CreateContext("ime_undo", Vector2i(1024, 768), nullptr, &handler);
	REQUIRE(ime_context);
	{
		Fixture f("input", ime_context);
		REQUIRE(handler.context);
		f.Type("a");
		// Move on from the typing step.
		f.Key(Input::KI_END);

		handler.context->SetText("n", 1, 1);
		handler.context->SetCompositionRange(1, 2);
		handler.context->SetText("ni", 1, 2);
		handler.context->SetCompositionRange(1, 3);
		CHECK(f.Value() == "ani");
		f.Key(Input::KI_Z, Input::KM_CTRL);
		CHECK(f.Value() == "ani");

		handler.context->CommitComposition(String(CJK));
		CHECK(f.Value() == String("a") + CJK);
		f.Key(Input::KI_Z, Input::KM_CTRL);
		CHECK(f.Value() == "a");
	}
}

TEST_SUITE_END();

TEST_CASE("WidgetTextInput.textarea_max_rows_grows_with_text")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);
	ElementDocument* document = context->LoadDocumentFromMemory(R"(
<rml>
<head>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<link type="text/rcss" href="/assets/invader.rcss"/>
	<style>
		textarea { display: block; width: 300px; line-height: 20px; padding: 0; border-width: 0; }
	</style>
</head>
<body>
<textarea id="grow" rows="2" max-rows="6"></textarea>
<textarea id="fixed" rows="2"></textarea>
</body>
</rml>
)");
	REQUIRE(document);
	document->Show();

	auto* grow = ui_dynamic_cast<ElementFormControl*>(document->GetElementById("grow"));
	auto* fixed = ui_dynamic_cast<ElementFormControl*>(document->GetElementById("fixed"));
	REQUIRE(grow);
	REQUIRE(fixed);

	auto height_with = [&](ElementFormControl* control, const String& value) {
		control->SetValue(value);
		// One update notices the new line count, the next lays out with it.
		for (int i = 0; i < 3; i++)
		{
			context->Update();
			context->Render();
		}
		return control->GetClientHeight();
	};

	CHECK(height_with(grow, "") == doctest::Approx(40.f));
	CHECK(height_with(grow, "a\nb") == doctest::Approx(40.f));
	CHECK(height_with(grow, "a\nb\nc\nd") == doctest::Approx(80.f));
	CHECK(height_with(grow, "1\n2\n3\n4\n5\n6\n7\n8\n9") == doctest::Approx(120.f));
	CHECK(height_with(grow, "a\nb\nc") == doctest::Approx(60.f));
	CHECK(height_with(grow, "") == doctest::Approx(40.f));

	// Without max-rows the height stays at 'rows'.
	CHECK(height_with(fixed, "1\n2\n3\n4\n5") == doctest::Approx(40.f));

	document->Close();
	TestsShell::ShutdownShell();
}
