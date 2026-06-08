#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/ResourceDiagnostics.h"
#include "yuengine/runtime/EngineRuntime.h"
#include "yuengine/script/ScriptRuntime.h"
#include "yuengine/script/SqasmModule.h"

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::filesystem::path findRepoRoot(const char* argv0)
{
    std::filesystem::path current = std::filesystem::weakly_canonical(std::filesystem::path(argv0)).parent_path();
    for (int i = 0; i < 8; ++i) {
        if (std::filesystem::exists(current / "AGENTS.md")) {
            return current;
        }
        current = current.parent_path();
    }
    return std::filesystem::current_path();
}

void usage()
{
    std::cout << "usage:\n"
              << "  yuengine_cli validate <project.json>\n"
              << "  yuengine_cli boot <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli resources <project.json>\n"
              << "  yuengine_cli script <project.json> <module>\n"
              << "  yuengine_cli script-plan <project.json> [module] [function] [--repo-root <path>]\n"
              << "  yuengine_cli script-run <project.json> [module] [function] [--frames N] [--repo-root <path>]\n"
              << "  yuengine_cli native-services <project.json> [--repo-root <path>]\n";
}

bool traceEnabled()
{
    const char* value = std::getenv("YUENGINE_TRACE_BOOT");
    return value != nullptr && value[0] != '\0';
}

void traceCli(const std::string& phase)
{
    if (traceEnabled()) {
        std::cerr << "[yuengine cli] " << phase << std::endl;
    }
}

std::vector<std::filesystem::path> scriptRoots(const yu::project::ProjectManifest& manifest)
{
    std::vector<std::filesystem::path> roots;
    for (const auto& root : manifest.scriptRoots) {
        roots.push_back(root.path);
    }
    return roots;
}

void appendUnique(std::vector<std::string>& values, const std::string& value)
{
    if (std::find(values.begin(), values.end(), value) == values.end()) {
        values.push_back(value);
    }
}

std::vector<yu::script::SqasmModule> loadStartupBaselineModules(
    const yu::project::ProjectManifest& manifest,
    const std::string& entryModule)
{
    std::vector<std::string> moduleNames;
    for (const auto& preload : manifest.startup.preloadScripts) {
        appendUnique(moduleNames, preload);
    }
    for (const auto& dependency : manifest.startup.dependencyScripts) {
        appendUnique(moduleNames, dependency);
    }

    std::vector<yu::script::SqasmModule> modules;
    const auto roots = scriptRoots(manifest);
    for (const auto& moduleName : moduleNames) {
        if (moduleName == entryModule) {
            continue;
        }
        const auto modulePath = yu::script::resolveScriptModule(roots, moduleName);
        if (modulePath.empty()) {
            throw std::runtime_error("startup baseline script module not found: " + moduleName);
        }
        modules.push_back(yu::script::loadSqasmModule(modulePath));
    }
    return modules;
}

std::vector<std::string> positionalArgs(int argc, char** argv, int begin)
{
    std::vector<std::string> args;
    for (int i = begin; i < argc; ++i) {
        if (std::string(argv[i]) == "--repo-root" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--frames" && i + 1 < argc) {
            ++i;
            continue;
        }
        args.push_back(argv[i]);
    }
    return args;
}

int intOption(int argc, char** argv, const std::string& option, int fallback)
{
    for (int i = 3; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return std::stoi(argv[i + 1]);
        }
    }
    return fallback;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc < 3) {
        usage();
        return 2;
    }

    const std::string command = argv[1];
    const std::filesystem::path manifest = argv[2];
    std::filesystem::path repoRoot = findRepoRoot(argv[0]);

    for (int i = 3; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--repo-root") {
            repoRoot = argv[i + 1];
        }
    }

    try {
        if (command == "validate") {
            auto loaded = yu::project::loadProjectManifest(manifest);
            std::cout << "project-manifest: " << loaded.projectId << " ok\n";
            return 0;
        }
        if (command == "boot") {
            traceCli("boot command start");
            auto report = yu::runtime::bootProject(manifest, repoRoot);
            traceCli("boot report built");
            std::string json = yu::runtime::bootReportToJson(report);
            traceCli("boot json built");
            std::cout << json;
            traceCli("boot json written");
            return report.ok ? 0 : 1;
        }
        if (command == "resources") {
            auto report = yu::resource::inspectProjectResources(manifest);
            std::cout << yu::resource::resourceReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "script") {
            if (argc < 4) {
                usage();
                return 2;
            }
            const auto loaded = yu::project::loadProjectManifest(manifest);
            const auto roots = scriptRoots(loaded);
            const auto modulePath = yu::script::resolveScriptModule(roots, argv[3]);
            if (modulePath.empty()) {
                std::cerr << "yuengine_cli: script module not found: " << argv[3] << "\n";
                return 1;
            }
            const auto module = yu::script::loadSqasmModule(modulePath);
            std::cout << yu::script::sqasmModuleReportToJson(module);
            return 0;
        }
        if (command == "script-plan") {
            const auto loaded = yu::project::loadProjectManifest(manifest);
            const auto args = positionalArgs(argc, argv, 3);
            const std::string moduleName = args.empty() ? loaded.startup.entryModule : args[0];
            const std::string entryFunction = args.size() < 2 ? loaded.startup.entryFunction : args[1];
            const auto modulePath = yu::script::resolveScriptModule(scriptRoots(loaded), moduleName);
            if (modulePath.empty()) {
                std::cerr << "yuengine_cli: script module not found: " << moduleName << "\n";
                return 1;
            }
            yu::native::NativeRegistry registry;
            registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
            const auto module = yu::script::loadSqasmModule(modulePath);
            const auto plan = yu::script::planEntryExecution(module, entryFunction, registry);
            std::cout << yu::script::scriptExecutionPlanToJson(plan);
            return plan.entryFound ? 0 : 1;
        }
        if (command == "script-run") {
            const auto loaded = yu::project::loadProjectManifest(manifest);
            const auto args = positionalArgs(argc, argv, 3);
            const std::string moduleName = args.empty() ? loaded.startup.entryModule : args[0];
            const std::string entryFunction = args.size() < 2 ? loaded.startup.entryFunction : args[1];
            const int frames = intOption(argc, argv, "--frames", 0);
            const auto modulePath = yu::script::resolveScriptModule(scriptRoots(loaded), moduleName);
            if (modulePath.empty()) {
                std::cerr << "yuengine_cli: script module not found: " << moduleName << "\n";
                return 1;
            }
            yu::native::NativeRegistry registry;
            registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
            yu::native::NativeServiceCatalog catalog;
            const auto module = yu::script::loadSqasmModule(modulePath);
            const auto baselineModules = loadStartupBaselineModules(loaded, moduleName);
            const auto report =
                yu::script::runEntryScript(module, baselineModules, entryFunction, registry, catalog, frames);
            const auto json = yu::script::scriptExecutionReportToJson(report);
            const size_t written = std::fwrite(json.data(), 1, json.size(), stdout);
            std::fflush(stdout);
            if (written != json.size()) {
                std::cerr << "yuengine_cli: failed to write script-run report\n";
                return 1;
            }
            return report.entryFound && report.executed ? 0 : 1;
        }
        if (command == "native-services") {
            yu::project::loadProjectManifest(manifest);
            yu::native::NativeRegistry registry;
            registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
            yu::native::NativeServiceCatalog catalog;
            const auto unbound = catalog.unboundApis(registry);
            std::cout << yu::native::nativeServiceReportToJson(registry, catalog);
            return unbound.empty() && registry.unownedCount() == 0 ? 0 : 1;
        }
    } catch (const std::exception& ex) {
        std::cerr << "yuengine_cli: " << ex.what() << "\n";
        return 1;
    }

    usage();
    return 2;
}
