#pragma once

#include <ui/style/ComputedValues.h>
#include <ui/dom/Element.h>
#include <ui/dom/ElementScroll.h>
#include <ui/base/Traits.h>
#include <ui/base/Types.h>
#include "base/ControlledLifetimeResource.h"
#include <ui/dom/ElementBackgroundBorder.h>
#include <ui/dom/ElementEffects.h>
#include <ui/dom/ElementStyle.h>
#include <ui/dom/EventDispatcher.h>
#include "base/Pool.h"

namespace ui {

// Meta objects for element collected in a single struct to reduce memory allocations
struct ElementMeta {
	explicit ElementMeta(Element* el) : event_dispatcher(el), style(el), background_border(), effects(el), scroll(el), computed_values(el) {}
	SmallUnorderedMap<EventId, EventListener*> attribute_event_listeners;
	EventDispatcher event_dispatcher;
	ElementStyle style;
	ElementBackgroundBorder background_border;
	ElementEffects effects;
	ElementScroll scroll;
	Style::ComputedValues computed_values;
};

struct ElementMetaPool {
	Pool<ElementMeta> pool{50, true};

	static ControlledLifetimeResource<ElementMetaPool> element_meta_pool;
	static void Initialize();
	static void Shutdown();
};

} // namespace ui
