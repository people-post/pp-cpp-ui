#include "TestsShell.h"
#include <ui/base/StringUtilities.h>
#include <ui/core/Core.h>
#include <ui/dom/Context.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementText.h>
#include <ui/font/FontEngineInterface.h>
#include <doctest.h>

using namespace ui;

namespace {

// Every code point is 10px wide, so line contents are fully predictable.
class FixedWidthFontEngine : public FontEngineInterface {
public:
	bool LoadFontFace(const String& /*file_name*/, int /*face_index*/, bool /*fallback_face*/, Style::FontWeight /*weight*/) override { return true; }
	bool LoadFontFace(Span<const byte> /*data*/, int /*face_index*/, const String& /*family*/, Style::FontStyle /*style*/,
		Style::FontWeight /*weight*/, bool /*fallback_face*/) override
	{
		return true;
	}
	FontFaceHandle GetFontFaceHandle(const String& /*family*/, Style::FontStyle /*style*/, Style::FontWeight /*weight*/, int /*size*/) override
	{
		return 1;
	}
	const FontMetrics& GetFontMetrics(FontFaceHandle /*handle*/) override
	{
		static const FontMetrics metrics = {10, 8.f, 2.f, 12.f, 5.f, 1.f, 1.f, false};
		return metrics;
	}
	int GetStringWidth(FontFaceHandle /*handle*/, StringView string, const TextShapingContext& /*text_shaping_context*/,
		Character /*prior_character*/) override
	{
		return 10 * (int)StringUtilities::LengthUTF8(string);
	}
	int GenerateString(RenderManager& /*render_manager*/, FontFaceHandle /*face_handle*/, FontEffectsHandle /*font_effects_handle*/,
		StringView string, Vector2f /*position*/, ColourbPremultiplied /*colour*/, float /*opacity*/,
		const TextShapingContext& /*text_shaping_context*/, TexturedMeshList& /*mesh_list*/) override
	{
		return 10 * (int)StringUtilities::LengthUTF8(string);
	}
};

struct FixedWidthFontScope {
	FixedWidthFontScope()
	{
		TestsShell::ShutdownShell();
		SetFontEngineInterface(&font_engine);
		context = TestsShell::GetContext();
	}
	~FixedWidthFontScope() { TestsShell::ShutdownShell(); }

	FixedWidthFontEngine font_engine;
	Context* context = nullptr;
};

// Lays out 'text' in a 100px wide block (ten 10px characters per line) and returns the generated lines.
Vector<String> LayoutLines(Context* context, const String& text, const String& extra_style = "")
{
	const String rml = "<rml><head><style>body { display: block; width: 400px; } #t { display: block; width: 100px; font-size: 10px; " +
		extra_style + " }</style></head><body><div id=\"t\">" + text + "</div></body></rml>";

	ElementDocument* document = context->LoadDocumentFromMemory(rml);
	REQUIRE(document);
	document->Show();
	context->Update();

	Vector<String> result;
	Element* div = document->GetElementById("t");
	REQUIRE(div);
	ElementText* text_element = ui_dynamic_cast<ElementText*>(div->GetFirstChild());
	REQUIRE(text_element);
	for (const ElementText::Line& line : text_element->GetLines())
		result.push_back(line.text);

	document->Close();
	context->Update();
	return result;
}

bool StartsWithAny(const String& line, std::initializer_list<const char*> prefixes)
{
	for (const char* prefix : prefixes)
		if (StringUtilities::StartsWith(line, String(prefix)))
			return true;
	return false;
}

void CheckLinesFit(const Vector<String>& lines)
{
	for (const String& line : lines)
		CHECK_MESSAGE(StringUtilities::LengthUTF8(line) <= 10, "line overflows: ", line);
}

} // namespace

TEST_CASE("ElementText.LineBreak.CjkRunWrapsAtContainerWidth")
{
	FixedWidthFontScope scope;

	// 12 + space + 12 characters: the run before the space must be split, not moved or left overflowing as a whole.
	const Vector<String> lines = LayoutLines(scope.context, "一二三四五六七八九十甲乙 丙丁戊己庚辛壬癸子丑寅卯");
	REQUIRE(lines.size() == 3);
	CHECK(lines[0] == "一二三四五六七八九十");
	CHECK(lines[1] == "甲乙 丙丁戊己庚辛壬");
	CHECK(lines[2] == "癸子丑寅卯");
}

TEST_CASE("ElementText.LineBreak.KinsokuClosingPunctuation")
{
	FixedWidthFontScope scope;

	for (const char* text : {"一二三四五六七八九十？好", "一二三四五六七八九十。好", "一二三四五六七八九十，好", "一二三四五六七八九十」好"})
	{
		const String text_string = text;
		CAPTURE(text_string);
		const Vector<String> lines = LayoutLines(scope.context, text);
		REQUIRE(lines.size() == 2);
		CheckLinesFit(lines);
		for (const String& line : lines)
			CHECK_FALSE(StartsWithAny(line, {"？", "。", "，", "」"}));
		CHECK(lines[0] == "一二三四五六七八九");
	}
}

TEST_CASE("ElementText.LineBreak.KinsokuAsciiPunctuationAfterCjk")
{
	FixedWidthFontScope scope;

	const Vector<String> lines = LayoutLines(scope.context, "一二三四五六七八九十?好");
	REQUIRE(lines.size() == 2);
	CHECK(lines[0] == "一二三四五六七八九");
	CHECK(lines[1] == "十?好");
}

TEST_CASE("ElementText.LineBreak.KinsokuOpeningPunctuation")
{
	FixedWidthFontScope scope;

	const Vector<String> lines = LayoutLines(scope.context, "一二三四五六七八九（十）");
	REQUIRE(lines.size() == 2);
	CheckLinesFit(lines);
	CHECK(lines[0] == "一二三四五六七八九");
	CHECK(lines[1] == "（十）");
	for (const String& line : lines)
		CHECK_FALSE(StringUtilities::EndsWith(line, String("（")));
}

TEST_CASE("ElementText.LineBreak.MixedCjkAndLatin")
{
	FixedWidthFontScope scope;

	const Vector<String> lines = LayoutLines(scope.context, "我们在GitHub上发布", "width: 80px;");
	REQUIRE(lines.size() == 3);
	CHECK(lines[0] == "我们在");
	CHECK(lines[1] == "GitHub上发");
	CHECK(lines[2] == "布");
}

TEST_CASE("ElementText.LineBreak.AsciiParagraphUnchanged")
{
	FixedWidthFontScope scope;

	const Vector<String> lines = LayoutLines(scope.context, "The quick brown fox jumps over the lazy dog");
	REQUIRE(lines.size() == 5);
	CHECK(lines[0] == "The quick");
	CHECK(lines[1] == "brown fox");
	CHECK(lines[2] == "jumps over");
	CHECK(lines[3] == "the lazy");
	CHECK(lines[4] == "dog");
}

TEST_CASE("ElementText.LineBreak.BreakAllLatin")
{
	FixedWidthFontScope scope;

	const Vector<String> lines = LayoutLines(scope.context, "abcdefghijklmnopqrstuvwxyz", "word-break: break-all;");
	REQUIRE(lines.size() == 3);
	CHECK(lines[0] == "abcdefghij");
	CHECK(lines[1] == "klmnopqrst");
	CHECK(lines[2] == "uvwxyz");
}

TEST_CASE("ElementText.LineBreak.BreakAllRespectsKinsoku")
{
	FixedWidthFontScope scope;

	// A forced split of 'abcdefghij。' must not leave '。' at the start of the next line.
	const Vector<String> lines = LayoutLines(scope.context, "abcdefghij。", "word-break: break-all;");
	REQUIRE(lines.size() == 2);
	CHECK(lines[0] == "abcdefghi");
	CHECK(lines[1] == "j。");
}
