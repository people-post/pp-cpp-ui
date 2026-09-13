#include "TestsInterface.h"
#include "TypesToString.h"
#include <ui/base/Log.h>
#include <ui/base/StringUtilities.h>
#include <doctest.h>

TestsSystemInterface::~TestsSystemInterface()
{
	SetNumExpectedWarnings(0);
}

double TestsSystemInterface::GetElapsedTime()
{
	if (manual_time)
		return elapsed_time;

	return ui::SystemInterface::GetElapsedTime();
}

bool TestsSystemInterface::LogMessage(ui::Log::Type type, const ui::String& message)
{
	static const char* message_type_str[ui::Log::Type::LT_MAX] = {"Always", "Error", "Assert", "Warning", "Info", "Debug"};
	const bool result = ui::SystemInterface::LogMessage(type, message);

	if (type <= ui::Log::Type::LT_WARNING)
	{
		const ui::String warning = "pp-cpp-ui " + ui::String(message_type_str[type]) + ": " + message;

		if (num_expected_warnings > 0)
		{
			num_logged_warnings += 1;
			warnings.push_back(warning);
		}
		else
		{
			FAIL_CHECK(warning);
		}
	}

	return result;
}

void TestsSystemInterface::SetNumExpectedWarnings(int in_num_expected_warnings)
{
	if (num_expected_warnings > 0)
	{
		// Check and clear previous warnings
		if (num_logged_warnings != num_expected_warnings)
		{
			ui::String str = "Got unexpected number of warnings: \n";
			ui::StringUtilities::JoinString(str, warnings, '\n');
			if (warnings.empty())
				str += "(no warnings logged)";
			CHECK_MESSAGE(num_logged_warnings == num_expected_warnings, str);
		}

		num_expected_warnings = 0;
		num_logged_warnings = 0;
		warnings.clear();
	}
	num_expected_warnings = in_num_expected_warnings;
}

void TestsSystemInterface::SetManualTime(double t)
{
	manual_time = true;
	elapsed_time = t;
}

void TestsSystemInterface::Reset()
{
	SetManualTime(0);
	manual_time = false;

	SetNumExpectedWarnings(0);
}

ui::CompiledGeometryHandle TestsRenderInterface::CompileGeometry(ui::Span<const ui::Vertex> vertices, ui::Span<const int> indices)
{
	counters.compile_geometry += 1;

	if (meshes_set)
	{
		INFO("Got vertices:\n", vertices);
		INFO("Got indices:\n", indices);
		REQUIRE_MESSAGE(!meshes.empty(), "No CompileGeometry expected, but one was passed to us");

		ui::Mesh mesh = std::move(meshes.front());
		meshes.erase(meshes.begin());
		INFO("Expected mesh:\n", mesh);

		CHECK(mesh.vertices.size() == vertices.size());
		CHECK(mesh.indices.size() == indices.size());

		for (size_t i = 0; i < mesh.vertices.size(); i++)
		{
			CHECK(mesh.vertices[i].position == vertices[i].position);
			CHECK(mesh.vertices[i].colour == vertices[i].colour);
			CHECK(mesh.vertices[i].tex_coord == vertices[i].tex_coord);
		}

		for (size_t i = 0; i < mesh.indices.size(); i++)
		{
			CHECK(mesh.indices[i] == indices[i]);
		}
	}

	return ui::CompiledGeometryHandle(counters.compile_geometry);
}

void TestsRenderInterface::RenderGeometry(ui::CompiledGeometryHandle /*geometry*/, ui::Vector2f /*translation*/, ui::TextureHandle /*texture*/)
{
	counters.render_geometry += 1;
}

void TestsRenderInterface::ReleaseGeometry(ui::CompiledGeometryHandle /*geometry*/)
{
	counters.release_geometry += 1;
}

void TestsRenderInterface::EnableScissorRegion(bool /*enable*/)
{
	counters.enable_scissor += 1;
}

void TestsRenderInterface::SetScissorRegion(ui::Rectanglei /*region*/)
{
	counters.set_scissor += 1;
}

void TestsRenderInterface::EnableClipMask(bool /*enable*/)
{
	counters.enable_clip_mask += 1;
}

void TestsRenderInterface::RenderToClipMask(ui::ClipMaskOperation /*mask_operation*/, ui::CompiledGeometryHandle /*geometry*/,
	ui::Vector2f /*translation*/)
{
	counters.render_to_clip_mask += 1;
}

ui::TextureHandle TestsRenderInterface::LoadTexture(ui::Vector2i& texture_dimensions, const ui::String& source)
{
	counters.load_texture += 1;
	if (source.find("invalid") != ui::String::npos)
		return 0;

	texture_dimensions.x = 512;
	texture_dimensions.y = 256;
	return 1;
}

ui::TextureHandle TestsRenderInterface::GenerateTexture(ui::Span<const ui::byte> /*source*/, ui::Vector2i /*source_dimensions*/)
{
	counters.generate_texture += 1;
	return 1;
}

void TestsRenderInterface::ReleaseTexture(ui::TextureHandle /*texture_handle*/)
{
	counters.release_texture += 1;
}

void TestsRenderInterface::SetTransform(const ui::Matrix4f* /*transform*/)
{
	counters.set_transform += 1;
}

ui::CompiledFilterHandle TestsRenderInterface::CompileFilter(const ui::String& /*name*/, const ui::Dictionary& /*parameters*/)
{
	counters.compile_filter += 1;
	return 1;
}

void TestsRenderInterface::ReleaseFilter(ui::CompiledFilterHandle /*filter*/)
{
	counters.release_filter += 1;
}

ui::CompiledShaderHandle TestsRenderInterface::CompileShader(const ui::String& /*name*/, const ui::Dictionary& /*parameters*/)
{
	counters.compile_shader += 1;
	return 1;
}

void TestsRenderInterface::RenderShader(ui::CompiledShaderHandle /*shader*/, ui::CompiledGeometryHandle /*geometry*/, ui::Vector2f /*translation*/,
	ui::TextureHandle /*texture*/)
{
	counters.render_shader += 1;
}

void TestsRenderInterface::ReleaseShader(ui::CompiledShaderHandle /*shader*/)
{
	counters.release_shader += 1;
}
void TestsRenderInterface::ResetCounters()
{
	counters_from_previous_reset = std::exchange(counters, Counters());
}

void TestsRenderInterface::ExpectCompileGeometry(ui::Vector<ui::Mesh> in_meshes)
{
	VerifyMeshes();
	meshes = std::move(in_meshes);
	meshes_set = true;
}

void TestsRenderInterface::Reset()
{
	VerifyMeshes();
	meshes_set = false;
	ResetCounters();
}
void TestsRenderInterface::VerifyMeshes()
{
	if (!meshes.empty())
	{
		// Use FAIL instead of REQUIRE here as this may be executed outside the context of a doctest test case.
		FAIL("CompileGeometry: Expected meshes not passed to us");
	}
}
