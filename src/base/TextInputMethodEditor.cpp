#include <ui/base/TextInputMethodEditor.h>
#include <ui/base/TextInputContext.h>

namespace ui {

void TextInputMethodEditor::OnActivate(TextInputContext* context) {
  if (context == input_context)
    return;
  Cancel();
  input_context = context;
}

void TextInputMethodEditor::OnDeactivate(TextInputContext* context) {
  if (context != input_context)
    return;
  // The platform drops its composition when text input stops, so the field drops it too.
  Cancel();
  input_context = nullptr;
}

void TextInputMethodEditor::OnDestroy(TextInputContext* context) {
  if (context != input_context)
    return;
  input_context = nullptr;
  composing = false;
  range_start = range_end = 0;
}

void TextInputMethodEditor::SetComposition(StringView text, int caret) {
  if (!input_context)
    return;

  if (text.empty()) {
    Cancel();
    return;
  }

  if (!IsComposing()) {
    input_context->GetSelectionRange(range_start, range_end);
    composing = true;
  }

  input_context->SetText(text, range_start, range_end);
  const int length = (int)StringUtilities::LengthUTF8(text);
  range_end = range_start + length;
  input_context->SetCompositionRange(range_start, range_end);

  if (caret < 0 || caret > length)
    caret = length;
  input_context->SetCursorPosition(range_start + caret);
}

bool TextInputMethodEditor::IsComposing() {
  if (!input_context || !composing)
    return false;

  // The app may have replaced the value meanwhile, which also ends the widget's composition. The range then no longer
  // holds the composition and must not be overwritten.
  int start = 0, end = 0;
  input_context->GetCompositionRange(start, end);
  if (start != range_start || end != range_end) {
    composing = false;
    range_start = range_end = 0;
  }
  return composing;
}

bool TextInputMethodEditor::Commit(StringView text) {
  if (!IsComposing())
    return false;

  const int caret = range_start + (int)StringUtilities::LengthUTF8(text);
  input_context->CommitComposition(text);
  EndComposition();
  input_context->SetCursorPosition(caret);
  return true;
}

void TextInputMethodEditor::Cancel() {
  if (!IsComposing())
    return;

  const int start = range_start;
  input_context->SetText("", range_start, range_end);
  EndComposition();
  input_context->SetCursorPosition(start);
}

void TextInputMethodEditor::EndComposition() {
  if (input_context)
    input_context->SetCompositionRange(0, 0);
  composing = false;
  range_start = range_end = 0;
}

} // namespace ui
