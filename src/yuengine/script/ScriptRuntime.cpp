#include "yuengine/script/ScriptRuntime.h"

#include "yuengine/core/Json.h"

#include <map>
#include <sstream>
#include <utility>

namespace yu::script {
namespace {

std::map<std::string, std::vector<int>> buildFunctionNameIndex(const SqasmModule& module)
{
    std::map<std::string, std::vector<int>> index;
    for (const auto& function : module.functions) {
        index[function.name].push_back(function.ordinal);
    }
    return index;
}

const SqasmFunction* findUniqueEntryFunction(const SqasmModule& module, const std::string& name)
{
    const SqasmFunction* found = nullptr;
    for (const auto& function : module.functions) {
        if (function.name != name) {
            continue;
        }
        if (found) {
            return nullptr;
        }
        found = &function;
    }
    return found;
}

void writeOrdinalArray(std::ostringstream& out, const std::vector<int>& values)
{
    out << "[";
    for (size_t i = 0; i < values.size(); ++i) {
        out << (i ? ", " : "") << values[i];
    }
    out << "]";
}

} // namespace

ScriptExecutionPlan planEntryExecution(
    const SqasmModule& module,
    const std::string& entryFunction,
    const native::NativeRegistry& registry)
{
    ScriptExecutionPlan plan;
    plan.modulePath = module.path.string();
    plan.entryFunction = entryFunction;

    const auto functionIndex = buildFunctionNameIndex(module);
    const SqasmFunction* entry = findUniqueEntryFunction(module, entryFunction);
    if (!entry) {
        plan.status = "entry_not_found_or_ambiguous";
        return plan;
    }

    plan.entryFound = true;
    plan.entryOrdinal = entry->ordinal;
    plan.entryInstructions = entry->instructionCount;
    plan.status = "vm_not_implemented";
    plan.directCalls = static_cast<int>(entry->calls.size());

    for (const auto& call : entry->calls) {
        ScriptCallResolution resolution;
        resolution.name = call.name;
        resolution.pc = call.pc;
        resolution.sourceLine = call.sourceLine;

        const native::ApiSurface* api = registry.find(call.name);
        if (api) {
            resolution.category = "native_obligation";
            resolution.service = api->service;
            resolution.implementationStatus =
                api->implementationStatus.empty() ? "not_started" : api->implementationStatus;
            ++plan.nativeObligations;
            plan.callResolutions.push_back(std::move(resolution));
            continue;
        }

        const auto candidates = functionIndex.find(call.name);
        if (candidates == functionIndex.end()) {
            resolution.category = "unresolved_or_builtin";
            ++plan.unresolvedCalls;
            plan.callResolutions.push_back(std::move(resolution));
            continue;
        }

        resolution.candidateOrdinals = candidates->second;
        if (candidates->second.size() == 1) {
            resolution.category = "script_function";
            ++plan.scriptCalls;
        } else {
            resolution.category = "ambiguous_script_function";
            ++plan.ambiguousCalls;
        }
        plan.callResolutions.push_back(std::move(resolution));
    }

    return plan;
}

std::string scriptExecutionPlanToJson(const ScriptExecutionPlan& plan)
{
    std::ostringstream out;
    out << "{\n";
    out << "  \"schema\": \"yuengine.script_execution_plan.v1\",\n";
    out << "  \"module\": \"" << core::jsonEscape(plan.modulePath) << "\",\n";
    out << "  \"entry_function\": \"" << core::jsonEscape(plan.entryFunction) << "\",\n";
    out << "  \"metrics\": \"entry=" << core::jsonEscape(plan.entryFunction)
        << " found=" << (plan.entryFound ? "true" : "false") << " direct_calls=" << plan.directCalls
        << " native_obligations=" << plan.nativeObligations << " script_calls=" << plan.scriptCalls
        << " ambiguous_calls=" << plan.ambiguousCalls << " unresolved_calls=" << plan.unresolvedCalls
        << " status=" << core::jsonEscape(plan.status) << "\",\n";
    out << "  \"entry_found\": " << (plan.entryFound ? "true" : "false") << ",\n";
    out << "  \"entry_ordinal\": " << plan.entryOrdinal << ",\n";
    out << "  \"entry_instructions\": " << plan.entryInstructions << ",\n";
    out << "  \"status\": \"" << core::jsonEscape(plan.status) << "\",\n";
    out << "  \"direct_calls\": " << plan.directCalls << ",\n";
    out << "  \"native_obligations\": " << plan.nativeObligations << ",\n";
    out << "  \"script_calls\": " << plan.scriptCalls << ",\n";
    out << "  \"ambiguous_calls\": " << plan.ambiguousCalls << ",\n";
    out << "  \"unresolved_calls\": " << plan.unresolvedCalls << ",\n";
    out << "  \"call_resolutions\": [\n";
    for (size_t i = 0; i < plan.callResolutions.size(); ++i) {
        const auto& resolution = plan.callResolutions[i];
        out << "    {\"name\": \"" << core::jsonEscape(resolution.name) << "\", \"pc\": " << resolution.pc
            << ", \"source_line\": " << resolution.sourceLine << ", \"category\": \""
            << core::jsonEscape(resolution.category) << "\", \"service\": \""
            << core::jsonEscape(resolution.service) << "\", \"implementation_status\": \""
            << core::jsonEscape(resolution.implementationStatus) << "\", \"candidate_ordinals\": ";
        writeOrdinalArray(out, resolution.candidateOrdinals);
        out << "}";
        out << (i + 1 == plan.callResolutions.size() ? "\n" : ",\n");
    }
    out << "  ]\n";
    out << "}\n";
    return out.str();
}

} // namespace yu::script
