#include <ui/core/Core.h>
#include <ui/dom/ElementInstancer.h>
#include <ui/dom/Factory.h>
#include <ui/base/Log.h>
#include <ui/dom/Plugin.h>
#include <ui/svg/ElementSVG.h>
#include "DecoratorSVG.h"
#include "SVGCache.h"
#include "XMLNodeHandlerSVG.h"

namespace ui {
namespace SVG {

	class SVGPlugin : public Plugin {
	public:
		void OnInitialise() override
		{
			SVGCache::Initialize();

			element_instancer = MakeUnique<ElementInstancerGeneric<ElementSVG>>();
			Factory::RegisterElementInstancer("svg", element_instancer.get());

			decorator_instancer = MakeUnique<DecoratorSVGInstancer>();
			Factory::RegisterDecoratorInstancer("svg", decorator_instancer.get());

			XMLParser::RegisterNodeHandler("svg", MakeShared<XMLNodeHandlerSVG>());
			XMLParser::RegisterPersistentCDATATag("svg");

			Log::Message(Log::LT_INFO, "SVG plugin initialised.");
		}

		void OnShutdown() override
		{
			delete this;
			SVGCache::Shutdown();
		}

		int GetEventClasses() override { return Plugin::EVT_BASIC; }

	private:
		UniquePtr<ElementInstancerGeneric<ElementSVG>> element_instancer;
		UniquePtr<DecoratorSVGInstancer> decorator_instancer;
	};

	void Initialise()
	{
		RegisterPlugin(new SVGPlugin);
	}

} // namespace SVG
} // namespace ui
