#include "TestNavigator.h"
#include "CaptureScreen.h"
#include "TestConfig.h"
#include "TestSuite.h"
#include "TestViewer.h"
#include <ui/Core/Context.h>
#include <ui/Core/Element.h>
#include <ui/Core/ElementDocument.h>
#include <ui/Core/Math.h>
#include <ui/Core/SystemInterface.h>
#include <Shell.h>
#include <cstdio>

// When capturing frames it seems we need to wait at least an extra frame for the newly submitted
// render to be read back out. If we don't wait, we end up saving a screenshot of the previous test.
constexpr int iteration_wait_frame_count = 2;

TestNavigator::TestNavigator(ui::RenderInterface* render_interface, ui::Context* context, TestViewer* viewer, TestSuiteList _test_suites,
	int start_suite, int start_case) : render_interface(render_interface), context(context), viewer(viewer), test_suites(std::move(_test_suites))
{
	UI_ASSERT(context);
	UI_ASSERTMSG(!test_suites.empty(), "At least one test suite is required.");
	context->GetRootElement()->AddEventListener(ui::EventId::Keydown, this, true);
	context->GetRootElement()->AddEventListener(ui::EventId::Keyup, this, true);
	context->GetRootElement()->AddEventListener(ui::EventId::Keydown, this);
	context->GetRootElement()->AddEventListener(ui::EventId::Textinput, this);
	context->GetRootElement()->AddEventListener(ui::EventId::Change, this);

	suite_index = ui::Math::Clamp(start_suite, 0, (int)test_suites.size() - 1);

	if (start_case > 0)
		CurrentSuite().SetIndex(start_case);
	LoadActiveTest();
}

TestNavigator::~TestNavigator()
{
	context->GetRootElement()->RemoveEventListener(ui::EventId::Keydown, this, true);
	context->GetRootElement()->RemoveEventListener(ui::EventId::Keyup, this, true);
	context->GetRootElement()->RemoveEventListener(ui::EventId::Keydown, this);
	context->GetRootElement()->RemoveEventListener(ui::EventId::Textinput, this);
	context->GetRootElement()->RemoveEventListener(ui::EventId::Change, this);
	ReleaseTextureGeometry(render_interface, reference_geometry);
	ReleaseTextureGeometry(render_interface, reference_highlight_geometry);
}

void TestNavigator::Update()
{
	if (iteration_state != IterationState::None)
	{
		UI_ASSERT(iteration_index >= 0);

		// Capture test document screenshots iteratively every nth frame.
		if (iteration_wait_frames > 0)
		{
			iteration_wait_frames -= 1;
		}
		else
		{
			UI_ASSERT(iteration_index < CurrentSuite().GetNumTests());
			iteration_wait_frames = iteration_wait_frame_count;

			if (iteration_state == IterationState::Capture)
			{
				if (!CaptureCurrentView())
				{
					StopTestSuiteIteration();
					return;
				}
			}
			else if (iteration_state == IterationState::Comparison)
			{
				UI_ASSERT((int)comparison_results.size() == CurrentSuite().GetNumTests());
				int test_index = CurrentSuite().GetIndex();
				comparison_results[test_index] = CompareCurrentView();
			}

			iteration_index += 1;

			if (CurrentSuite().Next())
				LoadActiveTest();
			else
				StopTestSuiteIteration();
		}
	}
}

void TestNavigator::Render()
{
	if (reference_state != ReferenceState::None && source_state == SourceType::None && !viewer->IsHelpVisible())
	{
		const TextureGeometry& geometry =
			(reference_state == ReferenceState::ShowReferenceHighlight ? reference_highlight_geometry : reference_geometry);

		if (const ui::CompiledGeometryHandle handle = render_interface->CompileGeometry(geometry.mesh.vertices, geometry.mesh.indices))
		{
			render_interface->RenderGeometry(handle, ui::Vector2f(0, 0), geometry.texture_handle);
			render_interface->ReleaseGeometry(handle);
		}
	}
}

void TestNavigator::ProcessEvent(ui::Event& event)
{
	// Keydown events in capture phase to override text input
	if (event == ui::EventId::Keydown && event.GetPhase() == ui::EventPhase::Capture)
	{
		const auto key_identifier = (ui::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
		const bool key_ctrl = event.GetParameter<bool>("ctrl_key", false);
		const bool key_shift = event.GetParameter<bool>("shift_key", false);

		ui::Element* element_filter_input = event.GetCurrentElement()->GetElementById("filterinput");
		UI_ASSERT(element_filter_input);

		if (key_identifier == ui::Input::KI_F5)
		{
			if (key_ctrl && key_shift)
				StartTestSuiteIteration(IterationState::Comparison);
			else
			{
				ComparisonResult result = CompareCurrentView();
				const ui::String compare_path = GetCompareInputDirectory() + '/' + GetImageFilenameFromCurrentTest();
				if (result.success)
				{
					if (result.is_equal)
					{
						ui::Log::Message(ui::Log::LT_INFO, "%s compares EQUAL to the reference image %s.", CurrentSuite().GetFilename().c_str(),
							compare_path.c_str());
					}
					else
					{
						ui::Log::Message(ui::Log::LT_INFO, "%s compares NOT EQUAL to the reference image %s.\nSee diff image written to %s.",
							CurrentSuite().GetFilename().c_str(), compare_path.c_str(), GetCaptureOutputDirectory().c_str());
					}

					if (!result.error_msg.empty())
						ui::Log::Message(ui::Log::LT_ERROR, "%s", result.error_msg.c_str());
				}
				else
				{
					ui::Log::Message(ui::Log::LT_ERROR, "Comparison of %s failed.\n%s", CurrentSuite().GetFilename().c_str(),
						result.error_msg.c_str());
				}
			}
		}
		else if (key_identifier == ui::Input::KI_F7)
		{
			if (key_ctrl && key_shift)
			{
				StartTestSuiteIteration(IterationState::Capture);
			}
			else
			{
				const ui::String filepath = GetCaptureOutputDirectory() + '/' + GetImageFilenameFromCurrentTest();
				if (CaptureCurrentView())
					ui::Log::Message(ui::Log::LT_INFO, "Succesfully captured and saved screenshot to %s", filepath.c_str());
				else
					ui::Log::Message(ui::Log::LT_ERROR, "Could not capture screenshot to %s", filepath.c_str());
			}
		}
		else if (key_identifier == ui::Input::KI_F1)
		{
			ShowReference(ReferenceState::None);
			viewer->ShowHelp(!viewer->IsHelpVisible());
		}
		else if (key_identifier == ui::Input::KI_F && key_ctrl)
		{
			element_filter_input->Focus();
			context->ProcessKeyDown(ui::Input::KI_A, ui::Input::KeyModifier::KM_CTRL);
			context->ProcessKeyUp(ui::Input::KI_A, ui::Input::KeyModifier::KM_CTRL);
		}
		else if (key_identifier == ui::Input::KI_R && key_ctrl)
		{
			LoadActiveTest(true);
		}
		else if (key_identifier == ui::Input::KI_S && key_ctrl)
		{
			if (source_state == SourceType::None)
			{
				source_state = (key_shift ? SourceType::Reference : SourceType::Test);
			}
			else
			{
				if (key_shift)
					source_state = (source_state == SourceType::Reference ? SourceType::Test : SourceType::Reference);
				else
					source_state = SourceType::None;
			}
			viewer->ShowSource(source_state);
			ShowReference(ReferenceState::None);
		}
		else if (key_identifier == ui::Input::KI_Q && key_ctrl)
		{
			if (reference_state != ReferenceState::None)
				ShowReference(ReferenceState::None);
			else
				ShowReference(key_shift ? ReferenceState::ShowReferenceHighlight : ReferenceState::ShowReference);
		}
		else if (key_identifier == ui::Input::KI_LSHIFT || key_identifier == ui::Input::KI_RSHIFT)
		{
			if (reference_state == ReferenceState::ShowReference)
				ShowReference(ReferenceState::ShowReferenceHighlight);
		}
		else if (key_identifier == ui::Input::KI_ESCAPE)
		{
			if (iteration_state != IterationState::None)
			{
				StopTestSuiteIteration();
			}
			else if (viewer->IsHelpVisible())
			{
				viewer->ShowHelp(false);
			}
			else if (source_state != SourceType::None)
			{
				source_state = SourceType::None;
				viewer->ShowSource(source_state);
			}
			else if (reference_state != ReferenceState::None)
			{
				ShowReference(ReferenceState::None);
			}
			else if (element_filter_input->IsPseudoClassSet("focus"))
			{
				element_filter_input->Blur();
			}
			else if (viewer->IsNavigationLocked())
			{
				element_filter_input->GetOwnerDocument()->Focus();
			}
			else if (goto_index >= 0)
			{
				CancelGoTo();
				UpdateGoToText();
			}
		}
		else if (key_identifier == ui::Input::KI_RETURN || key_identifier == ui::Input::KI_NUMPADENTER)
		{
			element_filter_input->Blur();
		}
		else if (key_identifier == ui::Input::KI_G && key_ctrl)
		{
			if (goto_index < 0)
			{
				element_filter_input->Blur();
				StartGoTo();
				UpdateGoToText();
			}
		}
	}

	if (event == ui::EventId::Keyup && event.GetPhase() == ui::EventPhase::Capture)
	{
		const auto key_identifier = (ui::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);
		if (key_identifier == ui::Input::KI_LSHIFT || key_identifier == ui::Input::KI_RSHIFT)
		{
			if (reference_state == ReferenceState::ShowReferenceHighlight)
				ShowReference(ReferenceState::ShowReference);
		}
	}

	// Keydown events in target/bubble phase ignored when focusing on input.
	if (event == ui::EventId::Keydown && event.GetPhase() != ui::EventPhase::Capture && !viewer->IsNavigationLocked())
	{
		const auto key_identifier = (ui::Input::KeyIdentifier)event.GetParameter<int>("key_identifier", 0);

		if (key_identifier == ui::Input::KI_LEFT)
		{
			if (CurrentSuite().Previous())
			{
				LoadActiveTest();
			}
		}
		else if (key_identifier == ui::Input::KI_RIGHT)
		{
			if (CurrentSuite().Next())
			{
				LoadActiveTest();
			}
		}
		else if (key_identifier == ui::Input::KI_UP)
		{
			const ui::String& filter = CurrentSuite().GetFilter();
			int new_index = std::max(0, suite_index - 1);
			if (new_index != suite_index)
			{
				suite_index = new_index;
				CurrentSuite().SetFilter(filter);
				LoadActiveTest();
			}
		}
		else if (key_identifier == ui::Input::KI_DOWN)
		{
			const ui::String& filter = CurrentSuite().GetFilter();
			int new_index = std::min((int)test_suites.size() - 1, suite_index + 1);
			if (new_index != suite_index)
			{
				suite_index = new_index;
				CurrentSuite().SetFilter(filter);
				LoadActiveTest();
			}
		}
		else if (key_identifier == ui::Input::KI_HOME)
		{
			CurrentSuite().SetIndex(0, TestSuite::Direction::Forward);
			LoadActiveTest();
		}
		else if (key_identifier == ui::Input::KI_END)
		{
			CurrentSuite().SetIndex(CurrentSuite().GetNumTests() - 1, TestSuite::Direction::Backward);
			LoadActiveTest();
		}
		else if (goto_index >= 0 && key_identifier == ui::Input::KI_BACK)
		{
			if (goto_index <= 0)
				CancelGoTo();
			else
				goto_index = goto_index / 10;

			UpdateGoToText();
		}
	}

	if (event == ui::EventId::Textinput && goto_index >= 0)
	{
		const ui::String text = event.GetParameter<ui::String>("text", "");

		for (const char c : text)
		{
			if (c >= '0' && c <= '9')
			{
				goto_index = goto_index * 10 + int(c - '0');
				UpdateGoToText();
			}
			else if (goto_index >= 0 && c == '\n')
			{
				bool out_of_bounds = false;

				if (goto_index > 0)
				{
					if (CurrentSuite().SetIndex(goto_index - 1))
						LoadActiveTest();
					else
						out_of_bounds = true;
				}

				CancelGoTo();
				UpdateGoToText(out_of_bounds);
			}
		}
	}

	if (event == ui::EventId::Change)
	{
		ui::Element* element = event.GetTargetElement();
		if (element->GetId() == "filterinput")
		{
			CurrentSuite().SetFilter(event.GetParameter<ui::String>("value", ""));
			LoadActiveTest();
		}
	}
}

void TestNavigator::LoadActiveTest(bool keep_scroll_position)
{
	const TestSuite& suite = CurrentSuite();
	viewer->LoadTest(suite.GetDirectory(), suite.GetFilename(), suite.GetIndex(), suite.GetNumTests(), suite.GetFilterIndex(),
		suite.GetNumFilteredTests(), suite_index, (int)test_suites.size(), keep_scroll_position);
	viewer->ShowSource(source_state);
	ShowReference(ReferenceState::None);
	UpdateGoToText();
}

ui::String TestNavigator::GetImageFilenameFromCurrentTest()
{
	const ui::String& filename = CurrentSuite().GetFilename();
	return filename.substr(0, filename.rfind('.')) + ".png";
}

ComparisonResult TestNavigator::CompareCurrentView()
{
	const ui::String filename = GetImageFilenameFromCurrentTest();

	ComparisonResult result = CompareScreenToPreviousCapture(render_interface, filename, nullptr, nullptr);

	return result;
}

bool TestNavigator::CaptureCurrentView()
{
	const ui::String filename = GetImageFilenameFromCurrentTest();

	bool result = CaptureScreenshot(filename, 1060);

	return result;
}

void TestNavigator::StartTestSuiteIteration(IterationState new_iteration_state)
{
	if (iteration_state != IterationState::None || new_iteration_state == IterationState::None)
		return;

	source_state = SourceType::None;

	TestSuite& suite = CurrentSuite();

	if (new_iteration_state == IterationState::Comparison)
	{
		comparison_results.clear();
		comparison_results.resize(suite.GetNumTests());
	}

	iteration_initial_index = suite.GetIndex();
	iteration_wait_frames = iteration_wait_frame_count;

	iteration_state = new_iteration_state;
	iteration_index = 0;
	suite.SetIndex(iteration_index, TestSuite::Direction::Forward);
	LoadActiveTest();
}

static bool SaveFile(const ui::String& file_path, const ui::String& contents)
{
	std::FILE* file = std::fopen(file_path.c_str(), "wt");
	if (!file)
		return false;

	std::fputs(contents.c_str(), file);
	std::fclose(file);

	return true;
}

void TestNavigator::StopTestSuiteIteration()
{
	if (iteration_state == IterationState::None)
		return;

	const ui::String output_directory = GetCaptureOutputDirectory();
	TestSuite& suite = CurrentSuite();
	const int num_tests = suite.GetNumTests();
	const int num_filtered_tests = suite.GetNumFilteredTests();

	if (iteration_state == IterationState::Capture)
	{
		if (iteration_index == num_tests)
		{
			ui::Log::Message(ui::Log::LT_INFO, "Successfully captured %d document screenshots to directory: %s", iteration_index,
				output_directory.c_str());
		}
		else if (iteration_index == num_filtered_tests)
		{
			ui::Log::Message(ui::Log::LT_INFO, "Successfully captured %d document screenshots (filtered out of %d total tests) to directory: %s",
				iteration_index, num_tests, output_directory.c_str());
		}
		else
		{
			ui::Log::Message(ui::Log::LT_ERROR, "Test suite capture aborted after %d of %d test(s). Output directory: %s", iteration_index,
				num_tests, output_directory.c_str());
		}
	}
	else if (iteration_state == IterationState::Comparison)
	{
		UI_ASSERT(num_tests == (int)comparison_results.size());

		// Indices
		ui::Vector<int> equal;
		ui::Vector<int> not_equal;
		ui::Vector<int> failed;
		ui::Vector<int> skipped;

		for (int i = 0; i < (int)comparison_results.size(); i++)
		{
			const ComparisonResult& comparison = comparison_results[i];

			if (comparison.skipped)
				skipped.push_back(i);
			else if (!comparison.success)
				failed.push_back(i);
			else if (comparison.is_equal)
				equal.push_back(i);
			else
				not_equal.push_back(i);
		}

		ui::String summary = ui::CreateString("  Total tests: %d\n  Not equal: %d\n  Failed: %d\n  Skipped: %d\n  Equal: %d", num_tests,
			(int)not_equal.size(), (int)failed.size(), (int)skipped.size(), (int)equal.size());

		if (!suite.GetFilter().empty())
			summary += "\n  Filter applied: " + suite.GetFilter();

		if (iteration_index == num_tests)
		{
			ui::Log::Message(ui::Log::LT_INFO, "Compared all test documents to their screenshot captures.\n%s", summary.c_str());
		}
		else if (iteration_index == num_filtered_tests)
		{
			ui::Log::Message(ui::Log::LT_INFO, "Compared all filtered test documents to their screenshot captures.\n%s", summary.c_str());
		}
		else
		{
			ui::Log::Message(ui::Log::LT_ERROR, "Test suite comparison aborted after %d of %d test(s).\n%s", iteration_index, num_tests,
				summary.c_str());
		}

		ui::String log;
		log.reserve(comparison_results.size() * 100);

		log += "pp-cpp-ui VisualTests comparison log output\n---------------------------------------\n\n" + summary;
		log += "\n\nNot Equal:\n";

		if (!not_equal.empty())
			log += "    #   similarity scores (%)   max pixel difference   filename\n\n";
		for (int i : not_equal)
		{
			suite.SetIndex(i);
			log += ui::CreateString("%5d   %5.1f%%  %4d   %s\n", i + 1, comparison_results[i].similarity_score * 100.0,
				(int)comparison_results[i].max_absolute_difference_single_pixel, suite.GetFilename().c_str());
			if (!comparison_results[i].error_msg.empty())
				log += "          " + comparison_results[i].error_msg + "\n";
		}
		log += "\nFailed:\n";
		for (int i : failed)
		{
			suite.SetIndex(i);
			log += ui::CreateString("%5d   %s\n", i + 1, suite.GetFilename().c_str());
			log += "          " + comparison_results[i].error_msg + "\n";
		}
		log += "\nSkipped:\n";
		for (int i : skipped)
		{
			suite.SetIndex(i);
			log += ui::CreateString("%5d   %s\n", i + 1, suite.GetFilename().c_str());
		}
		log += "\nEqual:\n";
		for (int i : equal)
		{
			suite.SetIndex(i);
			log += ui::CreateString("%5d   %s\n", i + 1, suite.GetFilename().c_str());
		}

		const ui::String log_path = GetCaptureOutputDirectory() + "/comparison.log";
		bool save_result = SaveFile(log_path, log);
		if (save_result && failed.empty())
			ui::Log::Message(ui::Log::LT_INFO, "Comparison log output written to %s", log_path.c_str());
		else if (save_result && !failed.empty())
			ui::Log::Message(ui::Log::LT_ERROR, "Comparison log output written to %s.\nSome comparisons failed, see log output for details.",
				log_path.c_str());
		else
			ui::Log::Message(ui::Log::LT_ERROR, "Failed writing comparison log output to file %s", log_path.c_str());
	}

	iteration_index = -1;
	iteration_initial_index = -1;
	iteration_wait_frames = -1;
	iteration_state = IterationState::None;

	suite.SetIndex(iteration_initial_index);
	LoadActiveTest();
}

void TestNavigator::StartGoTo()
{
	goto_index = 0;
	const ui::Rectanglef area = viewer->GetGoToArea();
	ui::GetSystemInterface()->ActivateKeyboard(area.TopLeft(), area.Height());
}

void TestNavigator::CancelGoTo()
{
	ui::GetSystemInterface()->DeactivateKeyboard();
	goto_index = -1;
}

void TestNavigator::UpdateGoToText(bool out_of_bounds)
{
	if (out_of_bounds)
		viewer->SetGoToText("Go To out of bounds");
	else if (goto_index > 0)
		viewer->SetGoToText(ui::CreateString("Go To: %d", goto_index));
	else if (goto_index == 0)
		viewer->SetGoToText("Go To:");
	else if (iteration_state == IterationState::Capture)
		viewer->SetGoToText("Capturing all tests");
	else if (iteration_state == IterationState::Comparison)
		viewer->SetGoToText("Comparing all tests");
	else if (reference_state == ReferenceState::ShowReference)
		viewer->SetGoToText(ui::CreateString("Showing reference capture (%.1f%% similar)", reference_comparison.similarity_score * 100.));
	else if (reference_state == ReferenceState::ShowReferenceHighlight)
		viewer->SetGoToText("Showing reference capture (highlight differences)");
	else
		viewer->SetGoToText("Press 'F1' for keyboard shortcuts.");
}

void TestNavigator::ShowReference(ReferenceState new_reference_state)
{
	if (new_reference_state == reference_state || viewer->IsHelpVisible())
		return;

	ui::String error_msg;

	if (reference_state == ReferenceState::None)
	{
		reference_comparison =
			CompareScreenToPreviousCapture(render_interface, GetImageFilenameFromCurrentTest(), &reference_geometry, &reference_highlight_geometry);

		if (!reference_comparison.success || !reference_geometry.texture_handle || !reference_highlight_geometry.texture_handle)
		{
			error_msg = reference_comparison.error_msg;
			new_reference_state = ReferenceState::None;
		}
	}

	if (new_reference_state == ReferenceState::None)
	{
		ReleaseTextureGeometry(render_interface, reference_geometry);
		ReleaseTextureGeometry(render_interface, reference_highlight_geometry);
		reference_comparison = {};
	}

	reference_state = new_reference_state;
	viewer->SetAttention(reference_state != ReferenceState::None);

	if (!error_msg.empty())
		viewer->SetGoToText(error_msg);
	else if (reference_comparison.is_equal)
		viewer->SetGoToText("EQUAL to reference capture");
	else
		UpdateGoToText();
}
