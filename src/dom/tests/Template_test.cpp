#include "TestsShell.h"
#include <ui/dom/Context.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementDocument.h>
#include <algorithm>
#include <doctest.h>

using namespace ui;

TEST_CASE("template.body")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<style>
		body.window
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
	</style>
</head>

<body id="body" class="overridden" template="window">
	<p id="p">A paragraph</p>
</body>
</rml>
)";

	static const String p_address = "p#p < div#content < div#window < body#body.window < #root#main";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	INFO("Expected warning: Body 'class' attribute overridden by template.");
	TestsShell::SetNumExpectedWarnings(1);
	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	TestsShell::SetNumExpectedWarnings(0);

	document->Show();
	TestsShell::RenderLoop();

	Element* el_p = document->GetElementById("p");
	REQUIRE(el_p);
	CHECK(el_p->GetAddress() == p_address);

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.body+inline")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/template_basic.rml"/>
	<style>
		body.window
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
	</style>
</head>

<body id="body" template="window">
	<p id="p">A paragraph</p>
	<div id="basic_wrapper">
		<template src="basic">
			Hello!<span id="span">World</span>
		</template>
	</div>
</body>
</rml>
)";

	static const String p_address = "p#p < div#content < div#window < body#body.window < #root#main";
	static const String span_address = "span#span < p#text < div#basic_wrapper < div#content < div#window < body#body.window < #root#main";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	document->Show();
	TestsShell::RenderLoop();

	Element* el_p = document->GetElementById("p");
	Element* el_span = document->GetElementById("span");
	REQUIRE(el_p);
	REQUIRE(el_span);
	CHECK(el_p->GetAddress() == p_address);
	CHECK(el_span->GetAddress() == span_address);

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.inline")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<style>
		body
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
	</style>
</head>

<body id="body" class="inline">
<p>Paragraph outside the window.</p>
<div id="template_parent">
	<template src="window">
		<p id="p">A paragraph</p>
	</template>
</div>
</body>
</rml>
)";

	static const String p_address = "p#p < div#content < div#window < div#template_parent < body#body.inline < #root#main";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	document->Show();
	TestsShell::RenderLoop();

	Element* el_p = document->GetElementById("p");
	REQUIRE(el_p);
	CHECK(el_p->GetAddress() == p_address);

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.inline+inline.unique")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/template_basic.rml"/>
	<style>
		body
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
	</style>
</head>

<body id="body" class="inline">
<p>Paragraph outside the window.</p>
<div id="template_parent">
	<template src="window">
		<p id="p">A paragraph</p>
	</template>
	<div id="basic_wrapper">
		<template src="basic">
			Hello!<span id="span">World</span>
		</template>
	</div>
</div>
</body>
</rml>
)";

	static const String p_address = "p#p < div#content < div#window < div#template_parent < body#body.inline < #root#main";
	static const String span_address = "span#span < p#text < div#basic_wrapper < div#template_parent < body#body.inline < #root#main";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	document->Show();
	TestsShell::RenderLoop();

	Element* el_p = document->GetElementById("p");
	Element* el_span = document->GetElementById("span");
	REQUIRE(el_p);
	REQUIRE(el_span);
	CHECK(el_p->GetAddress() == p_address);
	CHECK(el_span->GetAddress() == span_address);

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.inline+inline.identical.wrapped")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/template_basic.rml"/>
	<style>
		body
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
		p {
			border: 1px aqua;
			padding: 5px;
			margin: 10px;
		}
	</style>
</head>

<body id="body" class="inline">
<div id="wrap_t1">
	<template src="basic">
		Enable<span id="s1">X</span>
	</template>
</div>
<div id="wrap_t2">
	<template src="basic">
		Disable<span id="s2">Y</span>
	</template>
</div>
</body>
</rml>
)";

	static const String s1_address = "span#s1 < p#text < div#wrap_t1 < body#body.inline < #root#main";
	static const String s2_address = "span#s2 < p#text < div#wrap_t2 < body#body.inline < #root#main";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	document->Show();
	TestsShell::RenderLoop();

	CHECK(document->GetElementById("s1")->GetAddress() == s1_address);
	CHECK(document->GetElementById("s2")->GetAddress() == s2_address);
	CHECK(StringUtilities::StripWhitespace(document->QuerySelector("#wrap_t1 p#text")->GetInnerRML()) == R"(Enable<span id="s1">X</span>)");

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.inline+inline.identical.siblings")
{
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/window.rml"/>
	<link type="text/template" href="/../Tests/Data/UnitTests/template_basic.rml"/>
	<style>
		body
		{
			top: 100px;
			left: 200px;
			width: 600px;
			height: 450px;
		}
		p {
			border: 1px aqua;
			padding: 5px;
			margin: 10px;
		}
	</style>
</head>

<body id="body" class="inline">
<template src="basic">
	Enable<span id="s1">X</span>
</template>
<template src="basic">
	Disable<span id="s2">Y</span>
</template>
</body>
</rml>
)";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	document->Show();
	TestsShell::RenderLoop();

	ElementList p_elements;
	document->GetElementsByTagName(p_elements, "p");
	REQUIRE(p_elements.size() == 2);
	CHECK(StringUtilities::StripWhitespace(p_elements[0]->GetInnerRML()) == R"(Enable<span id="s1">X</span>)");
	CHECK(StringUtilities::StripWhitespace(p_elements[1]->GetInnerRML()) == R"(Disable<span id="s2">Y</span>)");

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.self_recursive")
{
	// Expansion stops at the maximum template expansion depth.
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/recursive_template.rml"/>
</head>
<body id="body">
<template src="recursive_template">
</template>
</body>
</rml>
)";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	INFO("Expected error: template expansion depth exceeded.");
	TestsShell::SetNumExpectedWarnings(1);
	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	TestsShell::SetNumExpectedWarnings(0);

	document->Show();
	TestsShell::RenderLoop();

	document->Close();
	TestsShell::ShutdownShell();
}

static int CountDescendants(Element* element)
{
	int count = 0;
	for (int i = 0; i < element->GetNumChildren(); i++)
		count += 1 + CountDescendants(element->GetChild(i));
	return count;
}

static int GetMaxDepth(Element* element)
{
	int max_child_depth = 0;
	for (int i = 0; i < element->GetNumChildren(); i++)
		max_child_depth = std::max(max_child_depth, GetMaxDepth(element->GetChild(i)));
	return 1 + max_child_depth;
}

TEST_CASE("template.self_recursive_fanout")
{
	// Expansion stops at the maximum number of template expansions.
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/recursive_template_fanout.rml"/>
</head>
<body id="body">
<template src="recursive_template_fanout">
</template>
</body>
</rml>
)";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	TestsShell::SetNumExpectedWarnings(2);
	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	TestsShell::SetNumExpectedWarnings(0);
	REQUIRE(document);

	Element* body = document->GetElementById("body");
	REQUIRE(body);
	CHECK(CountDescendants(body) <= 256);

	document->Show();
	TestsShell::RenderLoop();

	document->Close();
	TestsShell::ShutdownShell();
}

TEST_CASE("template.max_document_depth")
{
	// Document depth is counted across nested template expansions.
	static const String document_rml = R"(
<rml>
<head>
	<link type="text/template" href="/assets/deep_recursive_template.rml"/>
</head>
<body id="body">
<template src="deep_recursive_template">
</template>
</body>
</rml>
)";

	Context* context = TestsShell::GetContext();
	REQUIRE(context);

	TestsShell::SetNumExpectedWarnings(1);
	ElementDocument* document = context->LoadDocumentFromMemory(document_rml);
	TestsShell::SetNumExpectedWarnings(0);
	REQUIRE(document);

	CHECK(GetMaxDepth(document) <= 512);

	document->Show();
	TestsShell::RenderLoop();

	document->Close();
	TestsShell::ShutdownShell();
}
