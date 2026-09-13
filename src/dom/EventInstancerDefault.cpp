#include "EventInstancerDefault.h"
#include <ui/dom/Event.h>
namespace ui {

EventInstancerDefault::EventInstancerDefault() {}

EventInstancerDefault::~EventInstancerDefault() {}

EventPtr EventInstancerDefault::InstanceEvent(Element* target, EventId id, const String& type, const Dictionary& parameters, bool interruptible)
{
	return EventPtr(new Event(target, id, type, parameters, interruptible));
}

void EventInstancerDefault::ReleaseEvent(Event* event)
{
	delete event;
}

void EventInstancerDefault::Release()
{
	delete this;
}

} // namespace ui
