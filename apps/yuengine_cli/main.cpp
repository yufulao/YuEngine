#include "yuengine/native/NativeRegistry.h"
#include "yuengine/native/NativeServices.h"
#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/ResourceDiagnostics.h"
#include "yuengine/runtime/EngineRuntime.h"
#include "yuengine/runtime/FrameRuntime.h"
#include "yuengine/runtime/SceneRuntime.h"
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
              << "  yuengine_cli scene-entry <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli scene-runtime <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli first-frame <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli mission-event-thread <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli script <project.json> <module>\n"
              << "  yuengine_cli script-plan <project.json> [module] [function] [--repo-root <path>]\n"
              << "  yuengine_cli script-run <project.json> [module] [function] [--frames N] [--input-scenario <name>] [--repo-root <path>]\n"
              << "  yuengine_cli native-services <project.json> [--repo-root <path>]\n";
}

bool traceEnabled()
{
#if defined(_MSC_VER)
    char* value = nullptr;
    size_t size = 0;
    _dupenv_s(&value, &size, "YUENGINE_TRACE_BOOT");
    const bool enabled = value != nullptr && value[0] != '\0';
    std::free(value);
    return enabled;
#else
    const char* value = std::getenv("YUENGINE_TRACE_BOOT");
    return value != nullptr && value[0] != '\0';
#endif
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
        if (std::string(argv[i]) == "--input-scenario" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--menu-select" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--menu-decide" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--menu-down" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--menu-up" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--save-empty" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--continue-disabled" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--can-shutdown" && i + 1 < argc) {
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

std::string stringOption(int argc, char** argv, const std::string& option, std::string fallback)
{
    for (int i = 3; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return argv[i + 1];
        }
    }
    return fallback;
}

bool parseBoolText(const std::string& value)
{
    if (value == "1" || value == "true" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "0" || value == "false" || value == "no" || value == "off") {
        return false;
    }
    throw std::runtime_error("invalid boolean option value: " + value);
}

bool boolOption(int argc, char** argv, const std::string& option, bool fallback)
{
    for (int i = 3; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return parseBoolText(argv[i + 1]);
        }
    }
    return fallback;
}

bool hasOption(int argc, char** argv, const std::string& option)
{
    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == option) {
            return true;
        }
    }
    return false;
}

yu::script::ScriptRunOptions scriptRunOptions(int argc, char** argv)
{
    yu::script::ScriptRunOptions options;
    options.frames = intOption(argc, argv, "--frames", 0);
    options.inputScenario = stringOption(argc, argv, "--input-scenario", "passive");

    if (options.inputScenario == "passive") {
        options.menuSelectedIndex = 0;
        options.menuDecide = false;
    } else if (options.inputScenario == "title-continue-disabled") {
        options.menuSelectedIndex = 0;
        options.menuDecide = true;
        options.saveListEmpty = true;
        options.continueDisabled = true;
    } else if (options.inputScenario == "title-continue") {
        options.menuSelectedIndex = 0;
        options.menuDecide = true;
        options.saveListEmpty = false;
        options.continueDisabled = false;
    } else if (options.inputScenario == "title-new-game") {
        options.menuSelectedIndex = 1;
        options.menuDecide = true;
    } else if (options.inputScenario == "title-load-empty") {
        options.menuSelectedIndex = 2;
        options.menuDecide = true;
        options.saveListEmpty = true;
    } else if (options.inputScenario == "title-load") {
        options.menuSelectedIndex = 2;
        options.menuDecide = true;
        options.saveListEmpty = false;
    } else if (options.inputScenario == "title-option") {
        options.menuSelectedIndex = 3;
        options.menuDecide = true;
    } else if (options.inputScenario == "title-exit-denied") {
        options.menuSelectedIndex = 4;
        options.menuDecide = true;
        options.canShutdown = false;
    } else if (options.inputScenario == "title-exit-allowed") {
        options.menuSelectedIndex = 4;
        options.menuDecide = true;
        options.canShutdown = true;
    } else {
        throw std::runtime_error("unknown script input scenario: " + options.inputScenario);
    }

    if (hasOption(argc, argv, "--menu-select")) {
        options.menuSelectedIndex = intOption(argc, argv, "--menu-select", options.menuSelectedIndex);
    }
    if (hasOption(argc, argv, "--menu-decide")) {
        options.menuDecide = boolOption(argc, argv, "--menu-decide", options.menuDecide);
    }
    if (hasOption(argc, argv, "--menu-down")) {
        options.menuDown = boolOption(argc, argv, "--menu-down", options.menuDown);
    }
    if (hasOption(argc, argv, "--menu-up")) {
        options.menuUp = boolOption(argc, argv, "--menu-up", options.menuUp);
    }
    if (hasOption(argc, argv, "--save-empty")) {
        options.saveListEmpty = boolOption(argc, argv, "--save-empty", options.saveListEmpty);
    }
    if (hasOption(argc, argv, "--continue-disabled")) {
        options.continueDisabled = boolOption(argc, argv, "--continue-disabled", options.continueDisabled);
    }
    if (hasOption(argc, argv, "--can-shutdown")) {
        options.canShutdown = boolOption(argc, argv, "--can-shutdown", options.canShutdown);
    }

    return options;
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
        if (command == "scene-entry") {
            auto report = yu::runtime::runSceneEntryRuntime(manifest, repoRoot);
            std::cout << yu::runtime::sceneEntryRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "scene-runtime") {
            auto report = yu::runtime::runSceneRuntimeMaterialization(manifest, repoRoot);
            std::cout << yu::runtime::sceneRuntimeMaterializationReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "first-frame") {
            auto report = yu::runtime::runFirstFrameRuntime(manifest, repoRoot);
            std::cout << yu::runtime::firstFrameRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "mission-event-thread") {
            auto report = yu::runtime::runMissionEventThreadRuntime(manifest, repoRoot);
            std::cout << yu::runtime::missionEventThreadRuntimeReportToJson(report);
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
            traceCli("script-run load manifest");
            const auto loaded = yu::project::loadProjectManifest(manifest);
            traceCli("script-run parse args");
            const auto args = positionalArgs(argc, argv, 3);
            const std::string moduleName = args.empty() ? loaded.startup.entryModule : args[0];
            const std::string entryFunction = args.size() < 2 ? loaded.startup.entryFunction : args[1];
            const yu::script::ScriptRunOptions options = scriptRunOptions(argc, argv);
            traceCli("script-run resolve module");
            const auto modulePath = yu::script::resolveScriptModule(scriptRoots(loaded), moduleName);
            if (modulePath.empty()) {
                std::cerr << "yuengine_cli: script module not found: " << moduleName << "\n";
                return 1;
            }
            traceCli("script-run load native surface");
            yu::native::NativeRegistry registry;
            registry.loadMarkdownSurface(repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md");
            yu::native::NativeServiceCatalog catalog;
            traceCli("script-run load module");
            const auto module = yu::script::loadSqasmModule(modulePath);
            traceCli("script-run load baseline modules");
            const auto baselineModules = loadStartupBaselineModules(loaded, moduleName);
            traceCli("script-run execute runtime");
            const auto report =
                yu::script::runEntryScript(module, baselineModules, entryFunction, registry, catalog, options);
            traceCli("script-run serialize report");
            const auto json = yu::script::scriptExecutionReportToJson(report);
            traceCli("script-run write report");
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
