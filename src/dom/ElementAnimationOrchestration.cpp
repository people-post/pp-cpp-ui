// Element — animation / transition orchestration (definitions only).
#include <ui/dom/Element.h>
#include <ui/base/Dictionary.h>
#include <ui/style/StyleSheetSpecification.h>
#include <ui/base/Types.h>
#include "base/Clock.h"
#include "ElementAnimation.h"
#include "ElementMeta.h"
#include "ElementStyle.h"
#include "EventSpecification.h"
#include <algorithm>

namespace ui {

ElementAnimationList& Element::EnsureAnimations()
{
	if (!animations)
		animations = MakeUnique<ElementAnimationList>();
	return *animations;
}

bool Element::Animate(const String& property_name, const Property& target_value, float duration, Tween tween, int num_iterations,
	bool alternate_direction, float delay, const Property* start_value)
{
	return Animate(StyleSheetSpecification::GetPropertyId(property_name), target_value, duration, tween, num_iterations, alternate_direction, delay,
		start_value);
}

bool Element::Animate(PropertyId id, const Property& target_value, float duration, Tween tween, int num_iterations, bool alternate_direction,
	float delay, const Property* start_value)
{
	auto& animations = EnsureAnimations();

	bool result = false;
	auto it_animation = StartAnimation(id, start_value, num_iterations, alternate_direction, delay, false);
	if (it_animation != animations.end())
	{
		result = it_animation->AddKey(duration, target_value, *this, tween, true);
		if (!result)
			animations.erase(it_animation);
	}

	return result;
}

bool Element::AddAnimationKey(const String& property_name, const Property& target_value, float duration, Tween tween)
{
	return AddAnimationKey(StyleSheetSpecification::GetPropertyId(property_name), target_value, duration, tween);
}

bool Element::AddAnimationKey(PropertyId id, const Property& target_value, float duration, Tween tween)
{
	auto& animations = EnsureAnimations();

	ElementAnimation* animation = nullptr;
	for (auto& existing_animation : animations)
	{
		if (existing_animation.GetPropertyId() == id)
		{
			animation = &existing_animation;
			break;
		}
	}
	if (!animation)
		return false;

	bool result = animation->AddKey(animation->GetDuration() + duration, target_value, *this, tween, true);

	return result;
}

ElementAnimationList::iterator Element::StartAnimation(PropertyId property_id, const Property* start_value, int num_iterations,
	bool alternate_direction, float delay, bool initiated_by_animation_property)
{
	auto& animations = EnsureAnimations();

	auto it = std::find_if(animations.begin(), animations.end(), [&](const ElementAnimation& el) { return el.GetPropertyId() == property_id; });

	if (it != animations.end())
	{
		const bool allow_overwriting_animation = !initiated_by_animation_property;
		if (!allow_overwriting_animation)
		{
			Log::Message(Log::LT_WARNING,
				"Could not animate property '%s' on element: %s. "
				"Please ensure that the property does not appear in multiple animations on the same element.",
				StyleSheetSpecification::GetPropertyName(property_id).c_str(), GetAddress().c_str());
			return it;
		}

		*it = ElementAnimation{};
	}
	else
	{
		animations.emplace_back();
		it = animations.end() - 1;
	}

	Property value;

	if (start_value)
	{
		value = *start_value;
		if (!value.definition)
			if (auto default_value = GetProperty(property_id))
				value.definition = default_value->definition;
	}
	else if (auto default_value = GetProperty(property_id))
	{
		value = *default_value;
	}

	if (value.definition)
	{
		ElementAnimationOrigin origin = (initiated_by_animation_property ? ElementAnimationOrigin::Animation : ElementAnimationOrigin::User);
		double start_time = Clock::GetElapsedTime() + (double)delay;
		*it = ElementAnimation{property_id, origin, value, *this, start_time, 0.0f, num_iterations, alternate_direction};
	}

	if (!it->IsInitalized())
	{
		animations.erase(it);
		it = animations.end();
	}

	return it;
}

bool Element::AddAnimationKeyTime(PropertyId property_id, const Property* target_value, float time, Tween tween)
{
	auto& animations = EnsureAnimations();

	if (!target_value)
		target_value = Style().GetProperty(property_id);
	if (!target_value)
		return false;

	ElementAnimation* animation = nullptr;

	for (auto& existing_animation : animations)
	{
		if (existing_animation.GetPropertyId() == property_id)
		{
			animation = &existing_animation;
			break;
		}
	}
	if (!animation)
		return false;

	bool result = animation->AddKey(time, *target_value, *this, tween, true);

	return result;
}

bool Element::StartTransition(const Transition& transition, const Property& start_value, const Property& target_value)
{
	auto& animations = EnsureAnimations();

	auto it = std::find_if(animations.begin(), animations.end(), [&](const ElementAnimation& el) { return el.GetPropertyId() == transition.id; });

	if (it != animations.end() && !it->IsTransition())
		return false;

	float duration = transition.duration;
	double start_time = Clock::GetElapsedTime() + (double)transition.delay;

	if (it == animations.end())
	{
		// Add transition as new animation
		animations.push_back(ElementAnimation{transition.id, ElementAnimationOrigin::Transition, start_value, *this, start_time, 0.0f, 1, false});
		it = (animations.end() - 1);
	}
	else
	{
		// Compress the duration based on the progress of the current animation
		float f = it->GetInterpolationFactor();
		f = 1.0f - (1.0f - f) * transition.reverse_adjustment_factor;
		duration = duration * f;
		// Replace old transition
		*it = ElementAnimation{transition.id, ElementAnimationOrigin::Transition, start_value, *this, start_time, 0.0f, 1, false};
	}

	bool result = it->AddKey(duration, target_value, *this, transition.tween, true);

	if (result)
		SetProperty(transition.id, start_value);
	else
		animations.erase(it);

	return result;
}

void Element::HandleTransitionProperty()
{
	if (dirty_transition)
	{
		dirty_transition = false;

		if (!animations)
			return;

		auto& animations = *this->animations;

		// Remove all transitions that are no longer in our local list
		const TransitionList* keep_transitions = GetComputedValues().transition();

		if (keep_transitions && keep_transitions->all)
			return;

		auto it_remove = animations.end();

		if (!keep_transitions || keep_transitions->none)
		{
			// All transitions should be removed, but only touch the animations that originate from the 'transition' property.
			// Move all animations to be erased in a valid state at the end of the list, and erase later.
			it_remove = std::partition(animations.begin(), animations.end(),
				[](const ElementAnimation& animation) -> bool { return !animation.IsTransition(); });
		}
		else
		{
			UI_ASSERT(keep_transitions);

			// Only remove the transitions that are not in our keep list.
			const auto& keep_transitions_list = keep_transitions->transitions;

			it_remove = std::partition(animations.begin(), animations.end(), [&keep_transitions_list](const ElementAnimation& animation) -> bool {
				if (!animation.IsTransition())
					return true;
				auto it = std::find_if(keep_transitions_list.begin(), keep_transitions_list.end(),
					[&animation](const Transition& transition) { return animation.GetPropertyId() == transition.id; });
				bool keep_animation = (it != keep_transitions_list.end());
				return keep_animation;
			});
		}

		// We can decide what to do with cancelled transitions here.
		for (auto it = it_remove; it != animations.end(); ++it)
			RemoveProperty(it->GetPropertyId());

		animations.erase(it_remove, animations.end());
	}
}

void Element::HandleAnimationProperty()
{
	auto& animations = EnsureAnimations();

	// Note: We are effectively restarting all animations whenever 'dirty_animation' is set. Use the dirty flag with care,
	// or find another approach which only updates actual "dirty" animations.
	if (dirty_animation)
	{
		dirty_animation = false;

		const AnimationList* animation_list = meta->computed_values.animation();
		bool element_has_animations = ((animation_list && !animation_list->empty()) || HasAnimations());
		const StyleSheet* stylesheet = nullptr;

		if (element_has_animations)
			stylesheet = GetStyleSheet();

		if (stylesheet)
		{
			// Remove existing animations
			{
				// We only touch the animations that originate from the 'animation' property.
				auto it_remove = std::partition(animations.begin(), animations.end(),
					[](const ElementAnimation& animation) { return animation.GetOrigin() != ElementAnimationOrigin::Animation; });

				// We can decide what to do with cancelled animations here.
				for (auto it = it_remove; it != animations.end(); ++it)
					RemoveProperty(it->GetPropertyId());

				animations.erase(it_remove, animations.end());
			}

			// Start animations
			if (animation_list)
			{
				for (const auto& animation : *animation_list)
				{
					const Keyframes* keyframes_ptr = stylesheet->GetKeyframes(animation.name);
					if (keyframes_ptr && keyframes_ptr->blocks.size() >= 1 && !animation.paused)
					{
						auto& property_ids = keyframes_ptr->property_ids;
						auto& blocks = keyframes_ptr->blocks;

						bool has_from_key = (blocks[0].normalized_time == 0);
						bool has_to_key = (blocks.back().normalized_time == 1);

						// If the first key defines initial conditions for a given property, use those values, else, use this element's current
						// values.
						for (PropertyId id : property_ids)
							StartAnimation(id, (has_from_key ? blocks[0].properties.GetProperty(id) : nullptr), animation.num_iterations,
								animation.alternate, animation.delay, true);

						// Add middle keys: Need to skip the first and last keys if they set the initial and end conditions, respectively.
						for (int i = (has_from_key ? 1 : 0); i < (int)blocks.size() + (has_to_key ? -1 : 0); i++)
						{
							// Add properties of current key to animation
							float time = blocks[i].normalized_time * animation.duration;
							for (auto& property : blocks[i].properties.GetProperties())
								AddAnimationKeyTime(property.first, &property.second, time, animation.tween);
						}

						// If the last key defines end conditions for a given property, use those values, else, use this element's current values.
						float time = animation.duration;
						for (PropertyId id : property_ids)
							AddAnimationKeyTime(id, (has_to_key ? blocks.back().properties.GetProperty(id) : nullptr), time, animation.tween);
					}
				}
			}
		}
	}
}

void Element::AdvanceAnimations()
{
	if (!HasAnimations())
		return;

	auto& animations = *this->animations;
	double time = Clock::GetElapsedTime();

	for (auto& animation : animations)
	{
		Property property = animation.UpdateAndGetProperty(time, *this);
		if (property.unit != Unit::UNKNOWN)
			SetProperty(animation.GetPropertyId(), property);
	}

	// Move all completed animations to the end of the list
	auto it_completed =
		std::partition(animations.begin(), animations.end(), [](const ElementAnimation& animation) { return !animation.IsComplete(); });

	Vector<Dictionary> dictionary_list;
	Vector<bool> is_transition;
	dictionary_list.reserve(animations.end() - it_completed);
	is_transition.reserve(animations.end() - it_completed);

	for (auto it = it_completed; it != animations.end(); ++it)
	{
		const String& property_name = StyleSheetSpecification::GetPropertyName(it->GetPropertyId());

		dictionary_list.emplace_back();
		dictionary_list.back().emplace("property", Variant(property_name));
		is_transition.push_back(it->IsTransition());

		// Remove completed transition- and animation-initiated properties.
		// Should behave like in HandleTransitionProperty() and HandleAnimationProperty() respectively.
		if (it->GetOrigin() != ElementAnimationOrigin::User)
			RemoveProperty(it->GetPropertyId());
	}

	// Need to erase elements before submitting event, as iterators might be invalidated when calling external code.
	animations.erase(it_completed, animations.end());

	for (size_t i = 0; i < dictionary_list.size(); i++)
		DispatchEvent(is_transition[i] ? EventId::Transitionend : EventId::Animationend, dictionary_list[i]);
}

} // namespace ui
