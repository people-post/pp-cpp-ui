#include "TestsInterface.h"
#include "TestsShell.h"
#include "font/harfbuzz/FontEngineInterfaceHarfBuzz.h"
#include <ui/base/FileInterface.h>
#include <ui/core/Core.h>
#include <ui/font/TextShapingContext.h>
#include <Shell.h>
#include <doctest.h>

using namespace ui;

namespace {

// A HarfBuzz font engine for one test case, outside the shared tests shell (which uses the default engine).
class HarfBuzzFonts {
public:
  HarfBuzzFonts() {
    TestsShell::ShutdownShell();
    REQUIRE(Shell::Initialize());
    SetSystemInterface(TestsShell::GetTestsSystemInterface());
    SetRenderInterface(TestsShell::GetTestsRenderInterface());
    SetFontEngineInterface(&engine);
    REQUIRE(Initialise());
  }
  ~HarfBuzzFonts() {
    Shutdown();
    Shell::Shutdown();
  }

  // Reads a test asset into memory that lives as long as the engine.
  Span<const byte> Read(const String& path) {
    FileInterface* files = GetFileInterface();
    FileHandle handle = files->Open(path);
    REQUIRE(handle);
    Vector<byte>& bytes = buffers.emplace_back(files->Length(handle));
    files->Read(bytes.data(), bytes.size(), handle);
    files->Close(handle);
    return {bytes.data(), bytes.size()};
  }

  int Width(const String& family, Style::FontWeight weight, const String& text) {
    const FontFaceHandle handle = engine.GetFontFaceHandle(family, Style::FontStyle::Normal, weight, 32);
    REQUIRE(handle);
    const String language = "en";
    return engine.GetStringWidth(handle, text, TextShapingContext{language}, Character::Null);
  }

private:
  FontEngineInterfaceHarfBuzz engine;
  Vector<Vector<byte>> buffers;
};

const String kEmoji = "assets/NotoEmoji-Regular.ttf";
const String kRegular = "assets/LatoLatin-Regular.ttf";
const String kBold = "assets/LatoLatin-Bold.ttf";
// Noto Emoji has no Latin letters, so these come from the fallback faces.
const String kText = "Hamburgefonstiv";

} // namespace

TEST_CASE("font.harfbuzz.fallback_glyphs_follow_the_weight_of_the_text") {
  HarfBuzzFonts fonts;
  REQUIRE(LoadFontFace(kEmoji, false, Style::FontWeight::Normal));
  REQUIRE(LoadFontFace(kEmoji, false, Style::FontWeight::Bold));
  // The regular face is added first, as the CJK regular face is in the app.
  REQUIRE(LoadFontFace(kRegular, true));
  REQUIRE(LoadFontFace(kBold, true));

  // Two faces in one family: each weight gets its own face, and bold is wider.
  REQUIRE(fonts.Width("latolatin", Style::FontWeight::Normal, kText) <
          fonts.Width("latolatin", Style::FontWeight::Bold, kText));

  // Text in the emoji family takes its letters from the fallback face of its own weight. (Fallback glyphs are
  // measured with unshaped advances, so compare them with each other, not with the widths above.)
  const int regular = fonts.Width("noto emoji", Style::FontWeight::Normal, kText);
  const int bold = fonts.Width("noto emoji", Style::FontWeight::Bold, kText);
  CHECK(regular < bold);
  // 600 is nearer to bold (700) than to regular (400).
  CHECK(fonts.Width("noto emoji", Style::FontWeight(600), kText) == bold);
}

TEST_CASE("font.harfbuzz.fallback_families_keep_their_order_whatever_the_weight") {
  HarfBuzzFonts fonts;
  REQUIRE(LoadFontFace(kEmoji, false, Style::FontWeight::Normal));
  REQUIRE(LoadFontFace(kEmoji, false, Style::FontWeight::Bold));
  // Two fallback families: the first only has a regular face, the second a bold one.
  REQUIRE(LoadFontFace(fonts.Read(kRegular), "first", Style::FontStyle::Normal, Style::FontWeight::Normal, true));
  REQUIRE(LoadFontFace(fonts.Read(kBold), "second", Style::FontStyle::Normal, Style::FontWeight::Bold, true));

  // Bold text still takes its letters from the first family: the weight only picks a face within a family.
  CHECK(fonts.Width("noto emoji", Style::FontWeight::Bold, kText) ==
        fonts.Width("noto emoji", Style::FontWeight::Normal, kText));
}
