#include "TestsShell.h"
#include <ui/dom/Context.h>
#include <ui/style/ConvolutionFilter.h>
#include <doctest.h>

using namespace ui;

TEST_CASE("convolution_filter.max_radius")
{
	REQUIRE(TestsShell::GetContext());

	constexpr int max_radius = ConvolutionFilter::MAX_KERNEL_RADIUS;
	CHECK(max_radius <= 64);

	ConvolutionFilter filter;
	CHECK(filter.Initialise(max_radius, FilterOperation::Dilation));
	CHECK(filter.Initialise(Vector2i(max_radius, 0), FilterOperation::Sum));

	TestsShell::SetNumExpectedWarnings(3);
	CHECK_FALSE(filter.Initialise(max_radius + 1, FilterOperation::Dilation));
	CHECK_FALSE(filter.Initialise(Vector2i(0, 100000), FilterOperation::Sum));
	CHECK_FALSE(filter.Initialise(Vector2i(1 << 30, 0), FilterOperation::Sum));
	TestsShell::SetNumExpectedWarnings(0);

	TestsShell::ShutdownShell();
}
