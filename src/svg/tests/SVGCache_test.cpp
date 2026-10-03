#include "TestsShell.h"
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <doctest.h>

using namespace ui;

TEST_CASE("svg.max_texture_size")
{
	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(R"(
<rml>
<head><title>svg-size</title></head>
<body>
	<svg id="small" src="icon_a.svg" style="width: 16px; height: 16px;"></svg>
	<svg id="large" src="icon_a.svg" style="width: 20000px; height: 20000px;"></svg>
	<svg id="pixels" src="icon_b.svg" style="width: 8000px; height: 8000px;"></svg>
</body>
</rml>
)",
		"assets/");
	REQUIRE(document);
	document->Show();

	TestsShell::SetNumExpectedWarnings(2);
	TestsShell::RenderLoop();
	TestsShell::SetNumExpectedWarnings(0);

	document->Close();
	TestsShell::ShutdownShell();
}
