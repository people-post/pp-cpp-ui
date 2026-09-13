#include <ui/dom/Element.h>
#include <ui/dom/Context.h>
#include <ui/base/Dictionary.h>
#include <ui/dom/ElementDocument.h>
#include <ui/dom/ElementInstancer.h>
#include <ui/dom/ElementScroll.h>
#include <ui/dom/ElementUtilities.h>
#include <ui/dom/Factory.h>
#include <ui/base/Math.h>
#include <ui/base/Profiling.h>
#include <ui/font/FontEngineInterface.h>
#include <ui/style/PropertiesIteratorView.h>
#include <ui/style/PropertyDefinition.h>
#include <ui/style/PropertyIdSet.h>
#include <ui/style/StyleSheet.h>
#include <ui/style/StyleSheetSpecification.h>
#include <ui/style/TransformPrimitive.h>
#include "base/Clock.h"
#include "style/ComputeProperty.h"
#include "ElementAnimation.h"
#include "ElementBackgroundBorder.h"
#include "ElementDefinition.h"
#include "ElementEffects.h"
#include "ElementMeta.h"
#include "ElementStyle.h"
#include "EventDispatcher.h"
#include "EventSpecification.h"
#include "layout/LayoutEngine.h"
#include "dom/PluginRegistry.h"
#include "base/Pool.h"
#include "style/PropertiesIterator.h"
#include "SelectionContentBuilder.h"
#include "StyleSheetNode.h"
#include "StyleSheetParser.h"
#include "style/TransformState.h"
#include "style/TransformUtilities.h"
#include <algorithm>
#include <cmath>
#include <limits>

namespace ui {

// Determines how many levels up in the hierarchy the OnChildAdd and OnChildRemove are called (starting at the child itself)
static constexpr int ChildNotifyLevels = 2;

// Helper function to select scroll offset delta

Element::Element(const String& tag) :
	local_stacking_context(false), local_stacking_context_forced(false), stacking_context_dirty(false), computed_values_are_default_initialized(true),
	visible(true), offset_fixed(false), absolute_offset_dirty(true), rounded_main_padding_size_dirty(true), dirty_definition(false),
	dirty_child_definitions(false), dirty_animation(false), dirty_transition(false), dirty_transform(false), dirty_perspective(false), tag(tag),
	relative_offset_base(0, 0), relative_offset_position(0, 0), absolute_offset(0, 0), scroll_offset(0, 0)
{
	UI_ASSERT(tag == StringUtilities::ToLower(tag));
	parent = nullptr;
	focus = nullptr;
	instancer = nullptr;
	owner_document = nullptr;
	offset_parent = nullptr;

	clip_area = BoxArea::Padding;

	baseline = 0.0f;

	num_non_dom_children = 0;

	z_index = 0;

	meta = ElementMetaPool::element_meta_pool->pool.AllocateAndConstruct(this);
	data_model = nullptr;
}

Element::~Element()
{
	UI_ASSERT(parent == nullptr);

	PluginRegistry::NotifyElementDestroy(this);

	// A simplified version of RemoveChild() for destruction.
	for (ElementPtr& child : children)
	{
		Element* child_ancestor = child.get();
		for (int i = 0; i <= ChildNotifyLevels && child_ancestor; i++, child_ancestor = child_ancestor->GetParentNode())
			child_ancestor->OnChildRemove(child.get());

		child->SetParent(nullptr);
	}

	children.clear();
	num_non_dom_children = 0;

	ElementMetaPool::element_meta_pool->pool.DestroyAndDeallocate(meta);
}

void Element::Update(float dp_ratio, Vector2f vp_dimensions)
{
#ifdef UI_TRACY_PROFILING
	auto name = GetAddress(false, false);
	UI_ZoneScoped;
	UI_ZoneText(name.c_str(), name.size());
#endif

	OnUpdate();

	HandleTransitionProperty();
	HandleAnimationProperty();
	AdvanceAnimations();

	Scroll().Update();

	UpdateProperties(dp_ratio, vp_dimensions);

	// Do en extra pass over the animations and properties if the 'animation' property was just changed.
	if (dirty_animation)
	{
		HandleAnimationProperty();
		AdvanceAnimations();
		UpdateProperties(dp_ratio, vp_dimensions);
	}

	Effects().InstanceEffects();

	for (size_t i = 0; i < children.size(); i++)
		children[i]->Update(dp_ratio, vp_dimensions);

	if (HasAnimations() && IsVisible(true))
	{
		if (Context* ctx = GetContext())
			ctx->RequestNextUpdate(0);
	}
}

void Element::UpdateProperties(const float dp_ratio, const Vector2f vp_dimensions)
{
	UpdateDefinition();

	if (Style().AnyPropertiesDirty())
	{
		const ComputedValues* parent_values = parent ? &parent->GetComputedValues() : nullptr;
		const ComputedValues* document_values = owner_document ? &owner_document->GetComputedValues() : nullptr;

		// Compute values and clear dirty properties
		PropertyIdSet dirty_properties = Style().ComputeValues(meta->computed_values, parent_values, document_values,
			computed_values_are_default_initialized, dp_ratio, vp_dimensions);

		computed_values_are_default_initialized = false;

		// Computed values are just calculated and can safely be used in OnPropertyChange.
		// However, new properties set during this call will not be available until the next update loop.
		if (!dirty_properties.Empty())
			OnPropertyChange(dirty_properties);
	}
}

void Element::Render()
{
#ifdef UI_TRACY_PROFILING
	auto name = GetAddress(false, false);
	UI_ZoneScoped;
	UI_ZoneText(name.c_str(), name.size());
#endif

	UpdateAbsoluteOffsetAndRenderBoxData();

	// Rebuild our stacking context if necessary.
	if (stacking_context_dirty)
		BuildLocalStackingContext();

	UpdateTransformState();

	// Apply our transform
	ElementUtilities::ApplyTransform(*this);

	Effects().RenderEffects(RenderStage::Enter);

	// Set up the clipping region for this element.
	if (ElementUtilities::SetClippingRegion(this))
	{
		BackgroundBorder().Render(this);
		Effects().RenderEffects(RenderStage::Decoration);

		{
			UI_ZoneScopedNC("OnRender", 0x228B22);

			OnRender();
		}
	}

	// Render all elements in our local stacking context.
	if (stacking_context)
	{
		for (Element* element : *stacking_context)
			element->Render();
	}

	Effects().RenderEffects(RenderStage::Exit);
}






















bool Element::GetIntrinsicDimensions(Vector2f& /*dimensions*/, float& /*ratio*/)
{
	return false;
}

bool Element::IsReplaced()
{
	Vector2f unused_dimensions;
	float unused_ratio = 0.f;
	return GetIntrinsicDimensions(unused_dimensions, unused_ratio);
}


bool Element::IsVisible(bool include_ancestors) const
{
	if (!include_ancestors)
		return visible;
	const Element* element = this;
	while (element)
	{
		if (!element->visible)
			return false;
		element = element->parent;
	}
	return true;
}

void Element::ApplyLocalVisibilityOverrides()
{
	// DataViewIf / DataViewVisible mutate display/visibility during DataModel::Update, which runs
	// before Element::UpdateProperties. Without an eager sync, `visible` and stacking stay stale for
	// the rest of the frame (SVG icons keep painting after data-if hides them).
	bool new_visibility = true;
	if (const Property* display = GetLocalProperty(PropertyId::Display))
	{
		if (display->Get<int>() == static_cast<int>(Style::Display::None))
			new_visibility = false;
	}
	if (new_visibility)
	{
		if (const Property* visibility = GetLocalProperty(PropertyId::Visibility))
		{
			if (visibility->Get<int>() == static_cast<int>(Style::Visibility::Hidden))
				new_visibility = false;
		}
	}

	if (visible == new_visibility)
		return;

	visible = new_visibility;
	if (parent)
		parent->DirtyStackingContext();
	DirtyLayout();
	if (!visible)
		Blur();
}

float Element::GetZIndex() const
{
	return z_index;
}

FontFaceHandle Element::GetFontFaceHandle() const
{
	return meta->computed_values.font_face_handle();
}

const FontMetrics& Element::GetFontMetrics() const
{
	if (FontFaceHandle handle = GetFontFaceHandle())
		return GetFontEngineInterface()->GetFontMetrics(handle);

	// Return a default font metrics so layout can still proceed when the font face is missing.
	static const FontMetrics font_metrics = {};
	return font_metrics;
}

















const TransformState* Element::GetTransformState() const noexcept
{
	return transform_state.get();
}






Variant* Element::GetAttribute(const String& name)
{
	return GetIf(attributes, name);
}

const Variant* Element::GetAttribute(const String& name) const
{
	return GetIf(attributes, name);
}

bool Element::HasAttribute(const String& name) const
{
	return attributes.find(name) != attributes.end();
}

void Element::RemoveAttribute(const String& name)
{
	auto it = attributes.find(name);
	if (it != attributes.end())
	{
		attributes.erase(it);

		ElementAttributes changed_attributes;
		changed_attributes.emplace(name, Variant());
		OnAttributeChange(changed_attributes);
	}
}


Context* Element::GetContext() const
{
	ElementDocument* document = GetOwnerDocument();
	if (document != nullptr)
		return document->GetContext();

	return nullptr;
}

RenderManager* Element::GetRenderManager() const
{
	if (Context* context = GetContext())
		return &context->GetRenderManager();
	return nullptr;
}

void Element::SetAttributes(const ElementAttributes& _attributes)
{
	attributes.reserve(attributes.size() + _attributes.size());
	for (auto& pair : _attributes)
		attributes[pair.first] = pair.second;

	OnAttributeChange(_attributes);
}

int Element::GetNumAttributes() const
{
	return (int)attributes.size();
}





















ElementStyle* Element::GetStyle() const
{
	return &meta->style;
}

ElementStyle& Element::Style()
{
	return meta->style;
}

const ElementStyle& Element::Style() const
{
	return meta->style;
}

ElementBox Element::BoxModel()
{
	return ElementBox(this);
}

ElementScroll& Element::Scroll()
{
	return meta->scroll;
}

const ElementScroll& Element::Scroll() const
{
	return meta->scroll;
}

EventDispatcher& Element::Events()
{
	return meta->event_dispatcher;
}

const EventDispatcher& Element::Events() const
{
	return meta->event_dispatcher;
}

ElementEffects& Element::Effects()
{
	return meta->effects;
}

const ElementEffects& Element::Effects() const
{
	return meta->effects;
}

ElementBackgroundBorder& Element::BackgroundBorder()
{
	return meta->background_border;
}

const ElementBackgroundBorder& Element::BackgroundBorder() const
{
	return meta->background_border;
}







































String Element::GetEventDispatcherSummary() const
{
	return Events().ToString();
}

DataModel* Element::GetDataModel() const
{
	return data_model;
}

void Element::SetInstancer(ElementInstancer* _instancer)
{
	// Only record the first instancer being set as some instancers call other instancers to do their dirty work, in
	// which case we don't want to update the lowest level instancer.
	if (!instancer)
	{
		instancer = _instancer;
	}
}

void Element::OnUpdate() {}

void Element::OnRender() {}

void Element::OnResize() {}

void Element::OnLayout() {}

void Element::OnDpRatioChange() {}

void Element::OnStyleSheetChange() {}

void Element::OnAttributeChange(const ElementAttributes& changed_attributes)
{
	for (const auto& element_attribute : changed_attributes)
	{
		const auto& attribute = element_attribute.first;
		const auto& value = element_attribute.second;
		if (attribute == "id")
		{
			id = value.Get<String>();
		}
		else if (attribute == "class")
		{
			Style().SetClassNames(value.Get<String>());
		}
		else if (((attribute == "colspan" || attribute == "rowspan") && meta->computed_values.display() == Style::Display::TableCell) ||
			(attribute == "span" &&
				(meta->computed_values.display() == Style::Display::TableColumn ||
					meta->computed_values.display() == Style::Display::TableColumnGroup)))
		{
			DirtyLayout();
		}
		else if (attribute.size() > 2 && attribute[0] == 'o' && attribute[1] == 'n')
		{
			static constexpr size_t on_length = 2;
			static constexpr size_t capture_length = 7;
			const bool in_capture_phase = StringUtilities::EndsWith(attribute, "capture");
			auto& attribute_event_listeners = meta->attribute_event_listeners;
			auto& event_dispatcher = meta->event_dispatcher;
			const size_t event_name_length = attribute.size() - on_length - (in_capture_phase ? capture_length : 0);
			const auto event_id = EventSpecificationInterface::GetIdOrInsert(attribute.substr(on_length, event_name_length));
			const auto remove_event_listener_if_exists = [&attribute_event_listeners, &event_dispatcher, event_id, in_capture_phase]() {
				const auto listener_it = attribute_event_listeners.find(event_id);
				if (listener_it != attribute_event_listeners.cend())
				{
					event_dispatcher.DetachEvent(event_id, listener_it->second, in_capture_phase);
					attribute_event_listeners.erase(listener_it);
				}
			};

			if (value.GetType() == Variant::Type::STRING)
			{
				remove_event_listener_if_exists();

				const auto value_as_string = value.Get<String>();
				auto insertion_result = attribute_event_listeners.emplace(event_id, Factory::InstanceEventListener(value_as_string, this));
				if (auto* listener = insertion_result.first->second)
					event_dispatcher.AttachEvent(event_id, listener, in_capture_phase);
			}
			else if (value.GetType() == Variant::Type::NONE)
				remove_event_listener_if_exists();
		}
		else if (attribute == "style")
		{
			if (value.GetType() == Variant::STRING)
			{
				PropertyDictionary properties;
				StyleSheetParser parser;
				parser.ParseProperties(properties, value.GetReference<String>());

				for (const auto& name_value : properties.GetProperties())
					Style().SetProperty(name_value.first, name_value.second);
			}
			else if (value.GetType() != Variant::NONE)
				Log::Message(Log::LT_WARNING, "Invalid 'style' attribute, string type required. In element: %s", GetAddress().c_str());
		}
		else if (attribute == "lang")
		{
			if (value.GetType() == Variant::STRING)
				Style().SetProperty(PropertyId::Ui_Language, Property(value.GetReference<String>(), Unit::STRING));
			else if (value.GetType() != Variant::NONE)
				Log::Message(Log::LT_WARNING, "Invalid 'lang' attribute, string type required. In element: %s", GetAddress().c_str());
		}
		else if (attribute == "dir")
		{
			if (value.GetType() == Variant::STRING)
			{
				const String& dir_value = value.GetReference<String>();

				if (dir_value == "auto")
					Style().SetProperty(PropertyId::Ui_Direction, Property(Style::Direction::Auto));
				else if (dir_value == "ltr")
					Style().SetProperty(PropertyId::Ui_Direction, Property(Style::Direction::Ltr));
				else if (dir_value == "rtl")
					Style().SetProperty(PropertyId::Ui_Direction, Property(Style::Direction::Rtl));
				else
					Log::Message(Log::LT_WARNING, "Invalid 'dir' attribute '%s', value must be 'auto', 'ltr', or 'rtl'. In element: %s",
						dir_value.c_str(), GetAddress().c_str());
			}
			else if (value.GetType() != Variant::NONE)
				Log::Message(Log::LT_WARNING, "Invalid 'dir' attribute, string type required. In element: %s", GetAddress().c_str());
		}
	}

	// Any change to the attributes may affect which styles apply to the current element, in particular due to attribute selectors, ID selectors, and
	// class selectors. This can further affect all siblings or descendants due to sibling or descendant combinators.
	DirtyDefinition(DirtyNodes::SelfAndSiblings);
}

void Element::OnPropertyChange(const PropertyIdSet& changed_properties)
{
	UI_ZoneScoped;
	const bool top_right_bottom_left_changed = (           //
		changed_properties.Contains(PropertyId::Top) ||    //
		changed_properties.Contains(PropertyId::Right) ||  //
		changed_properties.Contains(PropertyId::Bottom) || //
		changed_properties.Contains(PropertyId::Left)      //
	);

	// See if the document layout needs to be updated.
	if (!IsLayoutDirty())
	{
		// Force a relayout if any of the changed properties require it.
		const PropertyIdSet changed_properties_forcing_layout =
			(changed_properties & StyleSheetSpecification::GetRegisteredPropertiesForcingLayout());

		if (!changed_properties_forcing_layout.Empty())
		{
			DirtyLayout();
		}
		else if (top_right_bottom_left_changed)
		{
			// Normally, the position properties only affect the position of the element and not the layout. Thus, these properties are not registered
			// as affecting layout. However, when absolutely positioned elements with both left & right, or top & bottom are set to definite values,
			// they affect the size of the element and thereby also the layout. This layout-dirtying condition needs to be registered manually.
			using namespace Style;
			const ComputedValues& computed = GetComputedValues();
			const bool absolutely_positioned = (computed.position() == Position::Absolute || computed.position() == Position::Fixed);
			const bool sized_width =
				(computed.width().type == Width::Auto && computed.left().type != Left::Auto && computed.right().type != Right::Auto);
			const bool sized_height =
				(computed.height().type == Height::Auto && computed.top().type != Top::Auto && computed.bottom().type != Bottom::Auto);

			if (absolutely_positioned && (sized_width || sized_height))
				DirtyLayout();
		}
	}

	// Update the position.
	if (top_right_bottom_left_changed)
	{
		UpdateOffset();
		DirtyAbsoluteOffset();
	}

	// Update the visibility.
	if (changed_properties.Contains(PropertyId::Visibility) || changed_properties.Contains(PropertyId::Display))
	{
		bool new_visibility =
			(meta->computed_values.display() != Style::Display::None && meta->computed_values.visibility() == Style::Visibility::Visible);

		if (visible != new_visibility)
		{
			visible = new_visibility;

			if (parent != nullptr)
				parent->DirtyStackingContext();

			if (!visible)
				Blur();
		}
	}

	const bool border_radius_changed = (                                    //
		changed_properties.Contains(PropertyId::BorderTopLeftRadius) ||     //
		changed_properties.Contains(PropertyId::BorderTopRightRadius) ||    //
		changed_properties.Contains(PropertyId::BorderBottomRightRadius) || //
		changed_properties.Contains(PropertyId::BorderBottomLeftRadius)     //
	);
	const bool filter_or_mask_changed = (changed_properties.Contains(PropertyId::Filter) || changed_properties.Contains(PropertyId::BackdropFilter) ||
		changed_properties.Contains(PropertyId::MaskImage));

	// Update the z-index and stacking context.
	if (changed_properties.Contains(PropertyId::ZIndex) || filter_or_mask_changed)
	{
		const Style::ZIndex z_index_property = meta->computed_values.z_index();

		const float new_z_index = (z_index_property.type == Style::ZIndex::Auto ? 0.f : z_index_property.value);
		const bool enable_local_stacking_context = (z_index_property.type != Style::ZIndex::Auto || local_stacking_context_forced ||
			meta->computed_values.has_filter() || meta->computed_values.has_backdrop_filter() || meta->computed_values.has_mask_image());

		if (z_index != new_z_index || local_stacking_context != enable_local_stacking_context)
		{
			z_index = new_z_index;

			if (local_stacking_context != enable_local_stacking_context)
			{
				local_stacking_context = enable_local_stacking_context;

				// If we are no longer acting as a local stacking context, then we clear the list and are all set. Otherwise, we need to rebuild our
				// local stacking context.
				stacking_context.reset();
				stacking_context_dirty = local_stacking_context;
			}

			// When our z-index or local stacking context changes, then we must dirty our parent stacking context so we are re-indexed.
			if (parent)
				parent->DirtyStackingContext();
		}
	}

	// Dirty the background if it's changed.
	if (border_radius_changed ||                                    //
		changed_properties.Contains(PropertyId::BackgroundColor) || //
		changed_properties.Contains(PropertyId::Opacity) ||         //
		changed_properties.Contains(PropertyId::ImageColor) ||      //
		changed_properties.Contains(PropertyId::BoxShadow))         //
	{
		BackgroundBorder().DirtyBackground();
	}

	// Dirty the border if it's changed.
	if (border_radius_changed ||                                      //
		changed_properties.Contains(PropertyId::BorderTopWidth) ||    //
		changed_properties.Contains(PropertyId::BorderRightWidth) ||  //
		changed_properties.Contains(PropertyId::BorderBottomWidth) || //
		changed_properties.Contains(PropertyId::BorderLeftWidth) ||   //
		changed_properties.Contains(PropertyId::BorderTopColor) ||    //
		changed_properties.Contains(PropertyId::BorderRightColor) ||  //
		changed_properties.Contains(PropertyId::BorderBottomColor) || //
		changed_properties.Contains(PropertyId::BorderLeftColor) ||   //
		changed_properties.Contains(PropertyId::Opacity))
	{
		BackgroundBorder().DirtyBorder();
	}

	// Dirty the effects if they've changed.
	if (border_radius_changed || filter_or_mask_changed || changed_properties.Contains(PropertyId::Decorator))
	{
		Effects().DirtyEffects();
	}

	const bool font_changed = (changed_properties.Contains(PropertyId::FontFamily) || changed_properties.Contains(PropertyId::FontStyle) ||
		changed_properties.Contains(PropertyId::FontWeight) || changed_properties.Contains(PropertyId::FontSize) ||
		changed_properties.Contains(PropertyId::FontKerning) || changed_properties.Contains(PropertyId::LetterSpacing));

	// Dirty the effects data when their visual looks may have changed.
	if (border_radius_changed ||                            //
		font_changed ||                                     //
		changed_properties.Contains(PropertyId::Opacity) || //
		changed_properties.Contains(PropertyId::Color) ||   //
		changed_properties.Contains(PropertyId::ImageColor))
	{
		Effects().DirtyEffectsData();
	}

	// Check for `perspective' and `perspective-origin' changes
	if (changed_properties.Contains(PropertyId::Perspective) ||        //
		changed_properties.Contains(PropertyId::PerspectiveOriginX) || //
		changed_properties.Contains(PropertyId::PerspectiveOriginY))
	{
		DirtyTransformState(true, false);
	}

	// Check for `transform' and `transform-origin' changes
	if (changed_properties.Contains(PropertyId::Transform) ||        //
		changed_properties.Contains(PropertyId::TransformOriginX) || //
		changed_properties.Contains(PropertyId::TransformOriginY) || //
		changed_properties.Contains(PropertyId::TransformOriginZ))
	{
		DirtyTransformState(false, true);
	}

	// Check for `animation' changes
	if (changed_properties.Contains(PropertyId::Animation))
	{
		dirty_animation = true;
	}
	// Check for `transition' changes
	if (changed_properties.Contains(PropertyId::Transition))
	{
		dirty_transition = true;
	}
}

void Element::OnPseudoClassChange(const String& /*pseudo_class*/, bool /*activate*/) {}



namespace {

bool IsBlockTagForSelection(const String& tag)
{
	return tag == "p" || tag == "div" || tag == "h1" || tag == "h2" || tag == "h3" || tag == "li" || tag == "ul" || tag == "ol" ||
		tag == "blockquote" || tag == "pre";
}

} // namespace

SelectionDisposition Element::QuerySelection(const SelectionQuery& query)
{
	if (query.phase == SelectionQuery::Phase::PointerDown && BlocksSelectionInteraction())
		return SelectionDisposition::Block;
	return SelectionDisposition::Default;
}

bool Element::BlocksSelectionInteraction() const
{
	if (HasAttribute("data-event-click"))
		return true;

	const String& tag = GetTagName();
	return tag == "button" || tag == "a" || tag == "input" || tag == "textarea" || tag == "select";
}

void Element::BuildSelectionContent(SelectionContentBuilder& builder)
{
	if (BlocksSelectionInteraction())
	{
		builder.AppendGap();
		return;
	}

	for (int i = 0; i < GetNumChildren(); ++i)
	{
		Element* child = GetChild(i);
		if (!child)
			continue;

		if (!builder.GetFlatText().empty() && builder.GetFlatText().back() != '\n' && IsBlockTagForSelection(child->GetTagName()))
			builder.AppendBlockSeparator();

		child->BuildSelectionContent(builder);

		if (IsBlockTagForSelection(child->GetTagName()) && (builder.GetFlatText().empty() || builder.GetFlatText().back() != '\n'))
			builder.AppendBlockSeparator();
	}
}

SelectionEndpoint Element::HitTestSelection(Vector2f absolute_position) const
{
	SelectionEndpoint best;
	float best_distance = std::numeric_limits<float>::max();

	for (int i = 0; i < GetNumChildren(); ++i)
	{
		Element* child = GetChild(i);
		if (!child)
			continue;

		SelectionEndpoint hit = child->HitTestSelection(absolute_position);
		if (!hit.IsValid())
			continue;

		const Vector2f offset = child->BoxModel().GetAbsoluteOffset(BoxArea::Border);
		const float distance = (offset - absolute_position).Magnitude();
		if (distance < best_distance)
		{
			best_distance = distance;
			best = hit;
		}
	}

	return best;
}

void Element::RenderSelectionSlice(int /*local_start*/, int /*local_end*/) {}

String Element::GetSelectionSlice(int /*local_start*/, int /*local_end*/) const
{
	return {};
}

void Element::DirtyLayout()
{
	if (Element* document = GetOwnerDocument())
		document->DirtyLayout();
}

bool Element::IsLayoutDirty()
{
	if (Element* document = GetOwnerDocument())
		return document->IsLayoutDirty();
	return false;
}



const Style::ComputedValues& Element::GetComputedValues() const
{
	return meta->computed_values;
}

void Element::GetRML(String& content)
{
	// First we start the open tag, add the attributes then close the open tag.
	// Then comes the children in order, then we add our close tag.
	content += "<";
	content += tag;

	for (auto& pair : attributes)
	{
		const String& name = pair.first;
		if (name == "style")
			continue;

		const Variant& variant = pair.second;
		String value;
		if (variant.GetInto(value))
		{
			content += ' ';
			content += name;
			content += "=\"";
			content += value;
			content += "\"";
		}
	}

	const PropertyMap& local_properties = Style().GetLocalStyleProperties();
	if (!local_properties.empty())
		content += " style=\"";

	for (const auto& pair : local_properties)
	{
		const PropertyId id = pair.first;
		const Property& property = pair.second;

		content += StyleSheetSpecification::GetPropertyName(id);
		content += ": ";
		content += StringUtilities::EncodeRml(property.ToString());
		content += "; ";
	}

	if (!local_properties.empty())
		content.back() = '\"';

	if (HasChildNodes())
	{
		content += ">";

		GetInnerRML(content);

		content += "</";
		content += tag;
		content += ">";
	}
	else
	{
		content += " />";
	}
}



void Element::Release()
{
	if (instancer)
		instancer->ReleaseElement(this);
	else
		Log::Message(Log::LT_WARNING, "Leak detected: element %s not instanced via pp-cpp-ui Factory. Unable to release.", GetAddress().c_str());
}


void Element::DirtyAbsoluteOffset()
{
	if (!absolute_offset_dirty)
		DirtyAbsoluteOffsetRecursive();
}

void Element::DirtyAbsoluteOffsetRecursive()
{
	if (!absolute_offset_dirty)
	{
		absolute_offset_dirty = true;

		if (transform_state)
			DirtyTransformState(true, true);
	}

	for (size_t i = 0; i < children.size(); i++)
		children[i]->DirtyAbsoluteOffsetRecursive();
}













} // namespace ui
