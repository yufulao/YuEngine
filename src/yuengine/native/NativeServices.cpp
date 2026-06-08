#include "yuengine/native/NativeServices.h"

#include "yuengine/core/Json.h"

#include <sstream>
#include <utility>

namespace yu::native {
namespace {

template <typename ServiceInterface>
class TrackingNativeService final : public ServiceInterface {
public:
    explicit TrackingNativeService(std::string serviceName)
        : serviceName_(std::move(serviceName))
    {
    }

    std::string name() const override
    {
        return serviceName_;
    }

    NativeDispatchResult dispatch(const ApiSurface& api, const NativeCallContext& context) const override
    {
        NativeDispatchResult result;
        result.api = api.name;
        result.service = api.service;
        result.ownerLevel = api.ownerLevel;
        result.implementationStatus = api.implementationStatus.empty() ? "not_started" : api.implementationStatus;
        result.evidence = api.evidence;
        result.apiKnown = true;
        result.serviceBound = true;
        result.implemented = result.implementationStatus == "implemented";
        if (!result.implemented) {
            result.obligation = "tracked obligation from " + context.module + ":" + context.function + ":pc"
                + std::to_string(context.pc);
        }
        return result;
    }

private:
    std::string serviceName_;
};

void writeStringArray(std::ostringstream& out, const std::vector<std::string>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << "\"" << core::jsonEscape(values[i]) << "\"";
    }
    out << "]";
}

} // namespace

NativeServiceCatalog::NativeServiceCatalog()
{
    registerService(std::make_unique<TrackingNativeService<ActorTaskService>>("Actor And Task Service"));
    registerService(std::make_unique<TrackingNativeService<AudioService>>("Audio Service"));
    registerService(std::make_unique<TrackingNativeService<CameraService>>("Camera Service"));
    registerService(
        std::make_unique<TrackingNativeService<CollisionPhysicsLiteService>>("Collision And Physics-Lite Service"));
    registerService(std::make_unique<TrackingNativeService<EventQuestFlagService>>("Event/Quest/Flag Service"));
    registerService(std::make_unique<TrackingNativeService<PlatformService>>("Platform Service"));
    registerService(std::make_unique<TrackingNativeService<ResourceService>>("Resource Service"));
    registerService(std::make_unique<TrackingNativeService<SaveProfileScenarioService>>("Save/Profile/Scenario Service"));
    registerService(std::make_unique<TrackingNativeService<SceneStageService>>("Scene And Stage Service"));
    registerService(std::make_unique<TrackingNativeService<ScriptService>>("Script Service"));
    registerService(std::make_unique<TrackingNativeService<UiRender2dService>>("UI And 2D Render Service"));
}

void NativeServiceCatalog::registerService(std::unique_ptr<NativeService> service)
{
    services_[service->name()] = std::move(service);
}

const NativeService* NativeServiceCatalog::find(const std::string& serviceName) const
{
    auto it = services_.find(serviceName);
    return it == services_.end() ? nullptr : it->second.get();
}

NativeDispatchResult NativeServiceCatalog::dispatch(const ApiSurface& api, const NativeCallContext& context) const
{
    const NativeService* service = find(api.service);
    if (service) {
        return service->dispatch(api, context);
    }

    NativeDispatchResult result;
    result.api = api.name;
    result.service = api.service;
    result.ownerLevel = api.ownerLevel;
    result.implementationStatus = "service_unbound";
    result.evidence = api.evidence;
    result.apiKnown = true;
    result.serviceBound = false;
    result.implemented = false;
    result.obligation = "native service interface is not registered";
    return result;
}

std::vector<std::string> NativeServiceCatalog::serviceNames() const
{
    std::vector<std::string> names;
    for (const auto& [name, _] : services_) {
        names.push_back(name);
    }
    return names;
}

std::vector<std::string> NativeServiceCatalog::unboundApis(const NativeRegistry& registry) const
{
    std::vector<std::string> apis;
    for (const auto& [name, api] : registry.apis()) {
        if (!find(api.service)) {
            apis.push_back(name);
        }
    }
    return apis;
}

size_t NativeServiceCatalog::size() const
{
    return services_.size();
}

std::string nativeServiceReportToJson(const NativeRegistry& registry, const NativeServiceCatalog& catalog)
{
    const auto serviceApiCounts = registry.serviceApiCounts();
    const auto serviceCallCounts = registry.serviceCallCounts();
    const auto unbound = catalog.unboundApis(registry);
    const size_t notStarted = registry.implementationStatusCount("not_started");
    const size_t unowned = registry.unownedCount();

    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.native_service_report.v1\",\n";
    out << "  \"metrics\": \"services=" << catalog.size() << " native_apis=" << registry.size()
        << " unowned_apis=" << unowned << " unbound_apis=" << unbound.size() << " not_started=" << notStarted
        << "\",\n";
    out << "  \"services\": " << catalog.size() << ",\n";
    out << "  \"native_apis\": " << registry.size() << ",\n";
    out << "  \"unowned_apis\": " << unowned << ",\n";
    out << "  \"unbound_apis\": " << unbound.size() << ",\n";
    out << "  \"not_started\": " << notStarted << ",\n";
    out << "  \"unbound_api_names\": ";
    writeStringArray(out, unbound);
    out << ",\n";
    out << "  \"service_interfaces\": [\n";
    auto names = catalog.serviceNames();
    for (size_t i = 0; i < names.size(); ++i) {
        const auto& name = names[i];
        const auto apiIt = serviceApiCounts.find(name);
        const auto callIt = serviceCallCounts.find(name);
        out << "    {\"name\": \"" << core::jsonEscape(name) << "\", \"apis\": "
            << (apiIt == serviceApiCounts.end() ? 0 : apiIt->second) << ", \"documented_calls\": "
            << (callIt == serviceCallCounts.end() ? 0 : callIt->second) << "}";
        out << (i + 1 == names.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::native
