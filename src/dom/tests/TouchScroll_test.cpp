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

// A list that scrolls only vertically, a pane that scrolls both ways, and a sideways carousel inside the list.
static const String touch_scroll_doc_rml = R"(
<rml>
<head>
	<title>Test</title>
	<link type="text/rcss" href="/assets/rml.rcss"/>
	<style>
		div { display: block; }
		#list { width: 300px; height: 200px; overflow-y: auto; }
		.row { height: 50px; }
		#carousel { width: 300px; height: 50px; overflow-x: auto; }
		#strip { width: 900px; height: 50px; }
		#pane { width: 300px; height: 200px; overflow: auto; }
		#canvas { width: 900px; height: 900px; }
	</style>
</head>
<body>
<div id="list">
	<div class="row" id="row0"/>
	<div id="carousel"><div id="strip"/></div>
	<div class="row" id="row2"/><div class="row"/><div class="row"/><div class="row"/><div class="row"/>
	<div class="row"/><div class="row"/><div class="row"/><div class="row"/><div class="row"/><div class="row"/>
	<div class="row"/><div class="row"/><div class="row"/><div class="row"/><div class="row"/><div class="row"/>
</div>
<div id="pane"><div id="canvas"/></div>
</body>
</rml>
)";

namespace {
struct EventCounter : public EventListener {
  int count = 0;
  void ProcessEvent(Event& /*event*/) override { ++count; }
};

// Drives one finger through the context on a manual clock, one move per 16ms frame.
struct Finger {
  Context* context;
  TestsSystemInterface* system;
  double time = 10.0;
  Vector2f start;

  void Down(Vector2f at) {
    start = at;
    system->SetManualTime(time);
    context->ProcessTouchStart(TouchList{Touch{TouchId(1), at}}, 0);
  }
  void MoveBy(Vector2f offset) {
    time += 0.016;
    system->SetManualTime(time);
    context->ProcessTouchMove(TouchList{Touch{TouchId(1), start + offset}}, 0);
  }
  void UpAt(Vector2f offset) {
    system->SetManualTime(time);
    context->ProcessTouchEnd(TouchList{Touch{TouchId(1), start + offset}}, 0);
  }
  // Lets any coast run for `seconds` of frames.
  void Coast(double seconds) {
    for (double end = time + seconds; time < end;) {
      time += 0.016;
      system->SetManualTime(time);
      context->Update();
    }
  }
};
} // namespace

TEST_CASE("touch_scroll") {
  Context* context = TestsShell::GetContext();
  REQUIRE(context);
  TestsSystemInterface* system_interface = TestsShell::GetTestsSystemInterface();
  system_interface->SetManualTime(10.0);

  ElementDocument* document = context->LoadDocumentFromMemory(touch_scroll_doc_rml);
  REQUIRE(document);
  document->Show();
  context->Update();
  context->Render();

  Element* list = document->GetElementById("list");
  Element* carousel = document->GetElementById("carousel");
  Element* pane = document->GetElementById("pane");
  REQUIRE(list->GetScrollTop() == 0.f);

  EventCounter clicks, moves;
  list->AddEventListener(EventId::Click, &clicks);
  list->AddEventListener(EventId::Mousemove, &moves);

  Finger finger{context, system_interface};
  const Vector2f on_list = list->GetAbsoluteOffset(BoxArea::Border) + Vector2f(150.f, 150.f);
  const Vector2f on_carousel = carousel->GetAbsoluteOffset(BoxArea::Border) + Vector2f(150.f, 25.f);
  const Vector2f on_pane = pane->GetAbsoluteOffset(BoxArea::Border) + Vector2f(150.f, 150.f);

  SUBCASE("moves inside the slop do not scroll; the scroll then starts without a jump") {
    finger.Down(on_list);
    finger.MoveBy({0.f, -6.f});
    finger.MoveBy({2.f, -9.f});
    CHECK(list->GetScrollTop() == 0.f);
    finger.MoveBy({2.f, -30.f});
    CHECK(list->GetScrollTop() == doctest::Approx(21.f));
    finger.UpAt({2.f, -30.f});
  }

  SUBCASE("a mostly horizontal diagonal swipe does not scroll a vertical list, but the app still sees it") {
    finger.Down(on_list);
    finger.MoveBy({-8.f, -3.f});
    finger.MoveBy({-30.f, -10.f});
    finger.MoveBy({-80.f, -25.f});
    finger.MoveBy({-140.f, -40.f});
    CHECK(list->GetScrollTop() == 0.f);
    CHECK(moves.count >= 4);
    finger.UpAt({-140.f, -40.f});
    finger.Coast(1.0);
    CHECK(list->GetScrollTop() == 0.f);
  }

  SUBCASE("a fast horizontal flick leaves no vertical coast") {
    finger.Down(on_list);
    for (int i = 1; i <= 5; i++)
      finger.MoveBy({-40.f * float(i), -14.f * float(i)});
    finger.UpAt({-200.f, -70.f});
    finger.Coast(1.0);
    CHECK(list->GetScrollTop() == 0.f);
    CHECK(list->GetScrollLeft() == 0.f);
  }

  SUBCASE("a vertical swipe with sideways drift scrolls only vertically, and coasts only vertically") {
    finger.Down(on_pane);
    finger.MoveBy({3.f, -12.f});
    for (int i = 1; i <= 5; i++)
      finger.MoveBy({3.f + 6.f * float(i), -12.f - 30.f * float(i)});
    const float top_at_release = pane->GetScrollTop();
    CHECK(top_at_release > 100.f);
    CHECK(pane->GetScrollLeft() == 0.f);
    finger.UpAt({33.f, -162.f});
    finger.Coast(0.3);
    CHECK(pane->GetScrollTop() > top_at_release);
    CHECK(pane->GetScrollLeft() == 0.f);
  }

  SUBCASE("a horizontal swipe with vertical drift scrolls a two-way pane only sideways") {
    finger.Down(on_pane);
    finger.MoveBy({-12.f, 3.f});
    for (int i = 1; i <= 5; i++)
      finger.MoveBy({-12.f - 30.f * float(i), 3.f + 6.f * float(i)});
    CHECK(pane->GetScrollLeft() > 100.f);
    finger.UpAt({-162.f, 33.f});
    finger.Coast(1.0);
    CHECK(pane->GetScrollTop() == 0.f);
  }

  SUBCASE("a sideways swipe on a carousel inside a list scrolls the carousel; a vertical one the list around it") {
    // Each scroll follows the finger from the event that left the slop on.
    const Vector2f at = carousel->GetAbsoluteOffset(BoxArea::Border) + Vector2f(250.f, 25.f);
    finger.Down(at);
    finger.MoveBy({-12.f, 1.f});
    finger.MoveBy({-62.f, 2.f});
    CHECK(carousel->GetScrollLeft() == doctest::Approx(62.f));
    CHECK(list->GetScrollTop() == 0.f);
    finger.UpAt({-62.f, 2.f});
    finger.Coast(1.0);
    CHECK(list->GetScrollTop() == 0.f);

    finger.Down(on_carousel);
    // The new touch stops whatever is left of the carousel's coast or edge bounce.
    const float carousel_left = carousel->GetScrollLeft();
    finger.MoveBy({1.f, -12.f});
    finger.MoveBy({2.f, -42.f});
    CHECK(list->GetScrollTop() == doctest::Approx(42.f));
    CHECK(carousel->GetScrollLeft() == carousel_left);
    finger.UpAt({2.f, -42.f});
  }

  SUBCASE("a tap on a fast coasting list stops it and clicks nothing; a tap on a settled list clicks") {
    finger.Down(on_list);
    for (int i = 1; i <= 5; i++)
      finger.MoveBy({0.f, -20.f * float(i)});
    finger.UpAt({0.f, -100.f});
    CHECK(clicks.count == 0);
    finger.Coast(0.05);
    const float coasting_top = list->GetScrollTop();
    CHECK(coasting_top > 100.f);

    finger.Down(on_list);
    finger.Coast(0.1);
    CHECK(list->GetScrollTop() == coasting_top);
    finger.UpAt({});
    CHECK(clicks.count == 0);
    finger.Coast(0.5);
    CHECK(list->GetScrollTop() == coasting_top);

    finger.Down(on_list);
    finger.UpAt({});
    CHECK(clicks.count == 1);
  }

  list->RemoveEventListener(EventId::Click, &clicks);
  list->RemoveEventListener(EventId::Mousemove, &moves);
  document->Close();
  TestsShell::ShutdownShell();
}
