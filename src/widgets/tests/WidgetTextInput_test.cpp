#include "TestsShell.h"
#include "TestsInterface.h"
#include <ui/core/Core.h>
#include <ui/base/TextInputContext.h>
#include <ui/base/TextInputHandler.h>
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <ui/widgets/ElementFormControl.h>
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

TEST_SUITE_END();
