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
#include <chrono>
#include <filesystem>
#include <functional>
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
              << "  yuengine_cli title-ui <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli title-branches <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli gameplay-frame <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli renderer-submit <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli frame-scheduler <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-obligations <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli material-semantics <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli device-presentation <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli texture-upload <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-state <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli resource-allocation <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli device-execution <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli present-oracle <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli platform-bridge <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-executor <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-device-adapter <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-device-create <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-resource-create <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-upload-bind <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli backend-surface-material-font <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli runtime-contract-suite <project.json> [--repo-root <path>] [--filter <test-name>]\n"
              << "  yuengine_cli mission-event-thread <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli mission-tutorial <project.json> [--repo-root <path>]\n"
              << "  yuengine_cli script <project.json> <module>\n"
              << "  yuengine_cli script-plan <project.json> [module] [function] [--repo-root <path>]\n"
              << "  yuengine_cli script-run <project.json> [module] [function] [--frames N] [--render-frames N] [--input-scenario <name>] [--repo-root <path>]\n"
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
        if (std::string(argv[i]) == "--render-frames" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--input-scenario" && i + 1 < argc) {
            ++i;
            continue;
        }
        if (std::string(argv[i]) == "--filter" && i + 1 < argc) {
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
    options.renderFrames = intOption(argc, argv, "--render-frames", 0);
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

int sqasmClosureBindingCount(const yu::script::SqasmModule& module)
{
    int count = 0;
    for (const auto& function : module.functions) {
        count += static_cast<int>(function.closureBindings.size());
    }
    return count;
}

int failedBootPhaseCount(const yu::runtime::BootReport& report)
{
    return static_cast<int>(std::count_if(report.phases.begin(), report.phases.end(), [](const auto& phase) {
        return phase.status != "ok";
    }));
}

bool resourceReportHasQuery(const yu::resource::ResourceReport& report, const std::string& query)
{
    for (const auto& resource : report.requiredResources) {
        if (resource.query == query && resource.found) {
            return true;
        }
    }
    for (const auto& module : report.scriptModules) {
        for (const auto& resource : module.resourceRefs) {
            if (resource.query == query && resource.found) {
                return true;
            }
        }
    }
    return false;
}

std::filesystem::path resolveRequiredScript(
    const yu::project::ProjectManifest& manifest,
    const std::string& moduleName)
{
    const auto modulePath = yu::script::resolveScriptModule(scriptRoots(manifest), moduleName);
    if (modulePath.empty()) {
        throw std::runtime_error("script module not found: " + moduleName);
    }
    return modulePath;
}

struct ContractSuiteState {
    int contracts = 0;
    int failed = 0;
    long long elapsedMs = 0;
    std::vector<std::string> failures;
};

using ContractSuiteCheck = std::function<void(std::vector<std::string>&)>;

void requireContract(
    std::vector<std::string>& errors,
    bool condition,
    const std::string& message)
{
    if (!condition) {
        errors.push_back(message);
    }
}

bool contractNameMatchesFilter(const std::string& name, const std::string& filter)
{
    return filter.empty() || name == filter || name.find(filter) != std::string::npos;
}

void runContractSuiteCheck(
    ContractSuiteState& suite,
    const std::string& name,
    const std::string& filter,
    const ContractSuiteCheck& check)
{
    if (!contractNameMatchesFilter(name, filter)) {
        return;
    }

    const auto started = std::chrono::steady_clock::now();
    std::vector<std::string> errors;
    try {
        check(errors);
    } catch (const std::exception& ex) {
        errors.push_back(ex.what());
    }
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - started)
        .count();

    ++suite.contracts;
    if (errors.empty()) {
        std::cout << "[pass] " << name << " elapsed_ms=" << elapsed << "\n";
        return;
    }

    ++suite.failed;
    std::cout << "[fail] " << name << " elapsed_ms=" << elapsed << "\n";
    for (const auto& error : errors) {
        std::cout << "  - " << error << "\n";
        suite.failures.push_back(name + ": " + error);
    }
}

int runRuntimeContractSuite(
    const std::filesystem::path& manifestPath,
    const std::filesystem::path& repoRoot,
    const std::string& filter)
{
    const auto suiteStarted = std::chrono::steady_clock::now();
    ContractSuiteState suite;

    const auto touhouManifest = yu::project::loadProjectManifest(manifestPath);
    const auto emptyManifestPath = repoRoot / "samples" / "empty_project" / "project.json";
    const auto registryPath = repoRoot / "docs" / "native_boundary_spec" / "title_first_mission.md";
    const auto titleModuleName = touhouManifest.startup.entryModule;
    const auto missionModuleName = "mission/sc01/main/ms010_0.b64";

    auto makeRegistry = [&]() {
        yu::native::NativeRegistry registry;
        registry.loadMarkdownSurface(registryPath);
        return registry;
    };
    auto loadModule = [&](const std::string& moduleName) {
        return yu::script::loadSqasmModule(resolveRequiredScript(touhouManifest, moduleName));
    };
    auto titleOptions = [](const std::string& scenario, int frames) {
        yu::script::ScriptRunOptions options;
        options.inputScenario = scenario;
        options.frames = frames;
        if (scenario == "title-new-game") {
            options.menuSelectedIndex = 1;
            options.menuDecide = true;
        }
        return options;
    };

    runContractSuiteCheck(suite, "yuengine_validate_touhou_sample", filter, [&](auto& errors) {
        requireContract(errors, touhouManifest.projectId == "touhou_new_world_sample", "unexpected touhou project id");
        requireContract(errors, touhouManifest.renderer == "d3d9_compatible", "unexpected touhou renderer profile");
    });

    runContractSuiteCheck(suite, "yuengine_validate_empty_sample", filter, [&](auto& errors) {
        const auto empty = yu::project::loadProjectManifest(emptyManifestPath);
        requireContract(errors, empty.projectId == "empty_sample", "unexpected empty project id");
        requireContract(errors, empty.renderer == "headless_validation", "unexpected empty renderer profile");
    });

    runContractSuiteCheck(suite, "yuengine_boot_touhou_sample", filter, [&](auto& errors) {
        const auto report = yu::runtime::bootProject(manifestPath, repoRoot);
        requireContract(errors, report.ok, "boot report is not ok");
        requireContract(errors, report.phases.size() == 7, "boot phase count changed");
        requireContract(errors, failedBootPhaseCount(report) == 0, "boot has failed phases");
        requireContract(errors, report.nativeApis == 84, "native api count changed");
        requireContract(errors, report.nativeServices == 11, "native service count changed");
        requireContract(errors, report.obligations.size() == 39, "startup obligation count changed");
    });

    runContractSuiteCheck(suite, "yuengine_boot_empty_sample", filter, [&](auto& errors) {
        const auto report = yu::runtime::bootProject(emptyManifestPath, repoRoot);
        requireContract(errors, report.ok, "empty boot report is not ok");
        requireContract(errors, report.phases.size() == 7, "empty boot phase count changed");
        requireContract(errors, failedBootPhaseCount(report) == 0, "empty boot has failed phases");
        requireContract(errors, report.nativeApis == 84, "empty native api count changed");
        requireContract(errors, report.nativeServices == 11, "empty native service count changed");
        requireContract(errors, report.obligations.empty(), "empty startup has obligations");
    });

    runContractSuiteCheck(suite, "yuengine_resources_touhou_sample", filter, [&](auto& errors) {
        const auto report = yu::resource::inspectProjectResources(manifestPath);
        requireContract(errors, report.ok, "resource report is not ok");
        requireContract(errors, resourceReportHasQuery(report, "menu/title/title_back"), "title background stem missing");
        requireContract(errors, resourceReportHasQuery(report, "menu/title/logo"), "title logo stem missing");
        requireContract(errors, resourceReportHasQuery(report, "map/doujou/doujou.sge"), "first stage missing");
        requireContract(errors, resourceReportHasQuery(report, "map/doujou/doujou.rcm"), "first rail camera missing");
    });

    runContractSuiteCheck(suite, "yuengine_resources_empty_sample", filter, [&](auto& errors) {
        const auto report = yu::resource::inspectProjectResources(emptyManifestPath);
        requireContract(errors, report.ok, "empty resource report is not ok");
        requireContract(errors, report.requiredResources.empty(), "empty sample should not require resources");
    });

    runContractSuiteCheck(suite, "yuengine_script_title_sample", filter, [&](auto& errors) {
        const auto module = loadModule(titleModuleName);
        requireContract(errors, module.functions.size() == 81, "title function count changed");
        requireContract(errors, module.instructionCount == 4466, "title instruction count changed");
        requireContract(errors, module.callCount == 593, "title call count changed");
        requireContract(errors, sqasmClosureBindingCount(module) == 80, "title closure binding count changed");
    });

    runContractSuiteCheck(suite, "yuengine_script_first_mission_sample", filter, [&](auto& errors) {
        const auto module = loadModule(missionModuleName);
        requireContract(errors, module.functions.size() == 62, "mission function count changed");
        requireContract(errors, module.instructionCount == 4068, "mission instruction count changed");
        requireContract(errors, module.callCount == 640, "mission call count changed");
        requireContract(errors, sqasmClosureBindingCount(module) == 61, "mission closure binding count changed");
    });

    runContractSuiteCheck(suite, "yuengine_native_services_surface", filter, [&](auto& errors) {
        const auto registry = makeRegistry();
        yu::native::NativeServiceCatalog catalog;
        const auto unbound = catalog.unboundApis(registry);
        requireContract(errors, catalog.size() == 11, "native service count changed");
        requireContract(errors, registry.size() == 84, "native registry api count changed");
        requireContract(errors, registry.unownedCount() == 0, "native registry has unowned APIs");
        requireContract(errors, unbound.empty(), "native catalog has unbound APIs");
        requireContract(errors, registry.implementationStatusCount("not_started") == 84, "native not_started count changed");
    });

    runContractSuiteCheck(suite, "yuengine_script_plan_title_entry", filter, [&](auto& errors) {
        const auto registry = makeRegistry();
        const auto module = loadModule(titleModuleName);
        const auto plan = yu::script::planEntryExecution(module, touhouManifest.startup.entryFunction, registry);
        requireContract(errors, plan.entryFound, "title entry not found");
        requireContract(errors, plan.directCalls == 2, "title entry direct call count changed");
        requireContract(errors, plan.builtinCalls == 1, "title entry builtin count changed");
        requireContract(errors, plan.nativeObligations == 0, "title entry native obligation count changed");
        requireContract(errors, plan.objectMethodCalls == 1, "title entry object method count changed");
        requireContract(errors, plan.unresolvedCalls == 0, "title entry unresolved calls changed");
        requireContract(errors, plan.classMethodTableCount == 8, "title class method table count changed");
        requireContract(errors, plan.objectBindingCount == 1, "title object binding count changed");
    });

    runContractSuiteCheck(suite, "yuengine_script_run_title_entry", filter, [&](auto& errors) {
        const auto registry = makeRegistry();
        yu::native::NativeServiceCatalog catalog;
        const auto module = loadModule(titleModuleName);
        const auto baselineModules = loadStartupBaselineModules(touhouManifest, titleModuleName);
        const auto report = yu::script::runEntryScript(
            module,
            baselineModules,
            touhouManifest.startup.entryFunction,
            registry,
            catalog,
            titleOptions("passive", 1));
        requireContract(errors, report.entryFound && report.executed, "title entry did not execute");
        requireContract(errors, report.baselineModules == 2, "title entry baseline module count changed");
        requireContract(errors, report.constructedObjects == 3, "title entry constructed object count changed");
        requireContract(errors, report.scriptMethods == 46, "title entry script method count changed");
        requireContract(errors, report.scriptFunctions == 5, "title entry script function count changed");
        requireContract(errors, report.unresolvedCalls == 0, "title entry unresolved calls changed");
        requireContract(errors, !report.truncated, "title entry trace truncated");
    });

    runContractSuiteCheck(suite, "yuengine_script_run_title_new_game", filter, [&](auto& errors) {
        const auto registry = makeRegistry();
        yu::native::NativeServiceCatalog catalog;
        const auto module = loadModule(titleModuleName);
        const auto baselineModules = loadStartupBaselineModules(touhouManifest, titleModuleName);
        const auto report = yu::script::runEntryScript(
            module,
            baselineModules,
            touhouManifest.startup.entryFunction,
            registry,
            catalog,
            titleOptions("title-new-game", 5));
        requireContract(errors, report.entryFound && report.executed, "title new-game did not execute");
        requireContract(errors, report.scriptMethods == 61, "title new-game script method count changed");
        requireContract(errors, report.scriptFunctions == 12, "title new-game script function count changed");
        requireContract(errors, report.nativeObligations == 18, "title new-game native obligation count changed");
        requireContract(errors, report.sceneServiceCommands == 3, "title new-game scene command count changed");
        requireContract(errors, report.unresolvedCalls == 0, "title new-game unresolved calls changed");
        requireContract(errors, !report.truncated, "title new-game trace truncated");
    });

    runContractSuiteCheck(suite, "yuengine_script_run_first_mission_setup", filter, [&](auto& errors) {
        const auto registry = makeRegistry();
        yu::native::NativeServiceCatalog catalog;
        const auto module = loadModule(missionModuleName);
        const auto baselineModules = loadStartupBaselineModules(touhouManifest, missionModuleName);
        yu::script::ScriptRunOptions options;
        options.frames = 1;
        const auto report = yu::script::runEntryScript(
            module,
            baselineModules,
            "setupProcess",
            registry,
            catalog,
            options);
        requireContract(errors, report.entryFound && report.executed, "mission setup did not execute");
        requireContract(errors, report.scriptFunctions == 5, "mission setup script function count changed");
        requireContract(errors, report.builtinCalls == 22, "mission setup builtin count changed");
        requireContract(errors, report.nativeObligations == 21, "mission setup native obligation count changed");
        requireContract(errors, report.sceneServiceCommands == 10, "mission setup scene command count changed");
        requireContract(errors, report.unresolvedCalls == 0, "mission setup unresolved calls changed");
        requireContract(errors, !report.truncated, "mission setup trace truncated");
    });

    runContractSuiteCheck(suite, "yuengine_backend_upload_binding_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendUploadBindingRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "backend upload-binding report is not ok");
        requireContract(errors, report.resourceCreationOk, "resource creation dependency failed");
        requireContract(errors, report.deviceExecutionOk, "device execution dependency failed");
        requireContract(errors, report.backendStateOk, "backend state dependency failed");
        requireContract(errors, report.uploadBindingRuntimeReady, "upload-binding runtime not ready");
        requireContract(errors, report.resourceHandlesCreated == 41, "resource handle count changed");
        requireContract(errors, report.uploadedSubresources == 458, "uploaded subresource count changed");
        requireContract(errors, report.failedUploadRecords == 0, "upload failures reported");
        requireContract(errors, report.executedBindingRecords == 14, "executed binding count changed");
        requireContract(errors, report.setRenderStateCalls == 40, "render state call count changed");
    });

    runContractSuiteCheck(suite, "yuengine_scene_entry_title_new_game", filter, [&](auto& errors) {
        const auto report = yu::runtime::runSceneEntryRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "scene-entry report is not ok");
        requireContract(errors, report.titleNewGameExecuted, "title new-game path did not execute");
        requireContract(errors, report.missionSetupExecuted, "mission setup path did not execute");
        requireContract(errors, report.stageReady && report.actorReady && report.cameraReady && report.eventReady, "scene-entry readiness changed");
        requireContract(errors, report.resourceBindings == 6, "scene-entry resource binding count changed");
        requireContract(errors, report.missingResources == 0, "scene-entry has missing resources");
        requireContract(errors, report.scriptBindings == 5, "scene-entry script binding count changed");
    });

    runContractSuiteCheck(suite, "yuengine_scene_runtime_materialization", filter, [&](auto& errors) {
        const auto report = yu::runtime::runSceneRuntimeMaterialization(manifestPath, repoRoot);
        requireContract(errors, report.ok && report.sceneEntryOk, "scene-runtime report is not ok");
        requireContract(errors, report.stage.ready, "stage handle is not ready");
        requireContract(errors, report.actor.ready, "actor handle is not ready");
        requireContract(errors, report.camera.ready, "camera handle is not ready");
        requireContract(errors, report.eventMarker.ready, "event marker is not ready");
        requireContract(errors, report.stage.dependencyCount == 42, "stage dependency count changed");
        requireContract(errors, report.stage.modelMeshCount == 111, "model mesh count changed");
        requireContract(errors, report.stage.collisionTriangleCount == 150, "collision triangle count changed");
        requireContract(errors, report.camera.railNodeCountCandidate == 3, "rail node count changed");
    });

    runContractSuiteCheck(suite, "yuengine_first_frame_runtime_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runFirstFrameRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok && report.sceneRuntimeOk, "first-frame report is not ok");
        requireContract(errors, report.renderer.ready, "renderer frame is not ready");
        requireContract(errors, report.actor.ready, "actor frame is not ready");
        requireContract(errors, report.camera.ready, "camera frame is not ready");
        requireContract(errors, report.input.ready, "input frame is not ready");
        requireContract(errors, report.event.ready, "event frame is not ready");
        requireContract(errors, report.renderer.meshDrawCandidates == 111, "mesh draw candidate count changed");
        requireContract(errors, report.renderer.textureBindings == 39, "texture binding count changed");
    });

    runContractSuiteCheck(suite, "yuengine_title_ui_command_payload_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runTitleUiRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "title UI report is not ok");
        requireContract(errors, report.titleSetupFound && report.titleSetupExecuted && report.titleRenderExecuted, "title UI execution changed");
        requireContract(errors, report.createdObjects == 26, "title UI object count changed");
        requireContract(errors, report.commandCount == 55, "title UI command count changed");
        requireContract(errors, report.drawCommands == 9, "title UI draw command count changed");
        requireContract(errors, report.backgroundResourceBound && report.logoResourceBound, "title UI key resources not bound");
        requireContract(errors, report.unresolvedCalls == 0, "title UI unresolved calls changed");
    });

    runContractSuiteCheck(suite, "yuengine_title_branches_runtime_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runTitleBranchesRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "title branches report is not ok");
        requireContract(errors, report.scenarioCount == 8, "title branch scenario count changed");
        requireContract(errors, report.executedScenarios == 8, "title branch executed scenario count changed");
        requireContract(errors, report.startGameScenarios == 3, "title branch start-game scenario count changed");
        requireContract(errors, report.loadAutoSaveScenarios == 2, "title branch load-auto-save count changed");
        requireContract(errors, report.makeNewGameScenarios == 1, "title branch make-new-game count changed");
        requireContract(errors, report.shutdownGameScenarios == 1, "title branch shutdown count changed");
        requireContract(errors, report.unresolvedCalls == 0, "title branch unresolved calls changed");
    });

    runContractSuiteCheck(suite, "yuengine_gameplay_frame_runtime_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runGameplayFrameRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "gameplay-frame report is not ok");
        requireContract(errors, report.sceneRuntimeOk && report.titleUiOk && report.titleBranchesOk, "gameplay dependencies changed");
        requireContract(errors, report.missionEventThreadOk && report.missionTutorialOk, "mission dependencies changed");
        requireContract(errors, report.frameUpdates == 2, "gameplay frame update count changed");
        requireContract(errors, report.gameplayCommandCount == 221, "gameplay command count changed");
        requireContract(errors, report.meshDrawCandidates == 111, "gameplay mesh draw count changed");
    });

    runContractSuiteCheck(suite, "yuengine_renderer_backend_submission_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runRendererBackendSubmissionRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "renderer submission report is not ok");
        requireContract(errors, report.backendFrameReady, "backend frame not ready");
        requireContract(errors, report.titlePassReady && report.worldPassReady, "renderer pass readiness changed");
        requireContract(errors, report.backendCommandCount == 181, "backend command count changed");
        requireContract(errors, report.drawSubmissions == 121, "draw submission count changed");
        requireContract(errors, report.resourceUploadSubmissions == 57, "resource upload submission count changed");
    });

    runContractSuiteCheck(suite, "yuengine_frame_scheduler_update_graph_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runFrameSchedulerRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "frame scheduler report is not ok");
        requireContract(errors, report.updateGraphReady, "update graph is not ready");
        requireContract(errors, report.nodeCount == 10, "scheduler node count changed");
        requireContract(errors, report.executedNodes == 10, "scheduler executed node count changed");
        requireContract(errors, report.scheduledWorkItems == 469, "scheduled work item count changed");
        requireContract(errors, report.backendObligations == 6, "scheduler backend obligation count changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_obligations_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendObligationsRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "backend obligations report is not ok");
        requireContract(errors, report.textureUploadContractReady, "texture upload contract not ready");
        requireContract(errors, report.materialBindingContractReady, "material binding contract not ready");
        requireContract(errors, report.ddsTextures == 39, "DDS texture count changed");
        requireContract(errors, report.meshSubmissions == 111, "mesh submission count changed");
        requireContract(errors, report.resolvedBackendContracts == 2, "resolved backend contract count changed");
        requireContract(errors, report.openBackendObligations == 4, "open backend obligation count changed");
    });

    runContractSuiteCheck(suite, "yuengine_material_semantics_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runMaterialSemanticsRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "material semantics report is not ok");
        requireContract(errors, report.materialSemanticsContractReady, "material semantics contract not ready");
        requireContract(errors, report.materials == 16, "material count changed");
        requireContract(errors, report.textureSlots == 39, "texture slot count changed");
        requireContract(errors, report.resolvedTextureSlots == 39, "resolved texture slot count changed");
        requireContract(errors, report.meshMaterialBindings == 110, "mesh material binding count changed");
        requireContract(errors, report.postEffectSamplers == 7, "post effect sampler count changed");
    });

    runContractSuiteCheck(suite, "yuengine_device_presentation_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runDevicePresentationRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "device presentation report is not ok");
        requireContract(errors, report.deviceProfileReady, "device profile not ready");
        requireContract(errors, report.resourceUploadPlanReady, "resource upload plan not ready");
        requireContract(errors, report.drawQueueContractReady, "draw queue contract not ready");
        requireContract(errors, report.backbufferWidthCandidate == 1280, "backbuffer width changed");
        requireContract(errors, report.backbufferHeightCandidate == 720, "backbuffer height changed");
        requireContract(errors, report.drawSubmissions == 121, "device presentation draw submission count changed");
    });

    runContractSuiteCheck(suite, "yuengine_texture_upload_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runTextureUploadRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "texture upload report is not ok");
        requireContract(errors, report.textureUploadRuntimeReady, "texture upload runtime not ready");
        requireContract(errors, report.textureUploadRecords == 39, "texture upload record count changed");
        requireContract(errors, report.validDdsHeaders == 39, "valid DDS count changed");
        requireContract(errors, report.dxt1Textures == 31, "DXT1 count changed");
        requireContract(errors, report.dxt5Textures == 8, "DXT5 count changed");
        requireContract(errors, report.cubeMapTextures == 1, "cube texture count changed");
        requireContract(errors, report.payloadByteTotal == 23768416, "payload byte total changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_state_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendStateRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "backend state report is not ok");
        requireContract(errors, report.backendStateRuntimeReady, "backend state runtime not ready");
        requireContract(errors, report.samplerStateRecords == 7, "sampler state record count changed");
        requireContract(errors, report.passStateRecords == 5, "pass state record count changed");
        requireContract(errors, report.fontQueryRecords == 6, "font query record count changed");
        requireContract(errors, report.textureUploadRecords == 39, "backend state texture upload count changed");
    });

    runContractSuiteCheck(suite, "yuengine_resource_allocation_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendResourceAllocationRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "resource allocation report is not ok");
        requireContract(errors, report.resourceAllocationRuntimeReady, "resource allocation runtime not ready");
        requireContract(errors, report.allocationRecords == 46, "allocation record count changed");
        requireContract(errors, report.readyAllocationRecords == 41, "ready allocation record count changed");
        requireContract(errors, report.stageTextureAllocations == 39, "stage texture allocation count changed");
        requireContract(errors, report.smaaLookupAllocations == 2, "SMAA lookup allocation count changed");
        requireContract(errors, report.readyAllocationPayloadBytes == 23949794, "ready allocation payload bytes changed");
    });

    runContractSuiteCheck(suite, "yuengine_device_execution_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendDeviceExecutionRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "device execution report is not ok");
        requireContract(errors, report.deviceExecutionRuntimeReady, "device execution runtime not ready");
        requireContract(errors, report.resourceCreationRecords == 46, "resource creation record count changed");
        requireContract(errors, report.readyResourceCreationRecords == 41, "ready resource creation count changed");
        requireContract(errors, report.uploadSubresourceRecords == 458, "upload subresource record count changed");
        requireContract(errors, report.bindingRecords == 57, "binding record count changed");
        requireContract(errors, report.renderStateBindingRecords == 5, "render state binding record count changed");
    });

    runContractSuiteCheck(suite, "yuengine_present_oracle_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendPresentationOracleRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "presentation oracle report is not ok");
        requireContract(errors, report.presentationOracleRuntimeReady, "presentation oracle runtime not ready");
        requireContract(errors, report.presentationRecords == 7, "presentation record count changed");
        requireContract(errors, report.readyPresentationRecords == 2, "ready presentation record count changed");
        requireContract(errors, report.backbufferWidth == 1280, "oracle backbuffer width changed");
        requireContract(errors, report.backbufferHeight == 720, "oracle backbuffer height changed");
        requireContract(errors, report.linkedDeviceExecutionRecords == 103, "linked device execution count changed");
    });

    runContractSuiteCheck(suite, "yuengine_platform_bridge_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendPlatformBridgeRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "platform bridge report is not ok");
        requireContract(errors, report.platformBridgeRuntimeReady, "platform bridge runtime not ready");
        requireContract(errors, report.bridgeCallRecords == 10, "bridge call record count changed");
        requireContract(errors, report.resourceCreationCallRecords == 46, "resource creation call count changed");
        requireContract(errors, report.uploadSubresourceCalls == 458, "upload call count changed");
        requireContract(errors, report.stateBindingCallRecords == 57, "state binding call count changed");
        requireContract(errors, report.drawSubmissionCalls == 121, "draw submission call count changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_executor_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendExecutorRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "backend executor report is not ok");
        requireContract(errors, report.executorRuntimeReady, "backend executor runtime not ready");
        requireContract(errors, report.executionResultRecords == 10, "executor result count changed");
        requireContract(errors, report.diagnosticSuccessRecords == 4, "diagnostic success count changed");
        requireContract(errors, report.resultCallCountTotal == 689, "executor call total changed");
        requireContract(errors, report.diagnosticExecutedCalls == 562, "diagnostic executed call count changed");
        requireContract(errors, report.preservedOpenCalls == 127, "preserved open call count changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_device_adapter_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendDeviceAdapterRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "device adapter report is not ok");
        requireContract(errors, report.deviceAdapterRuntimeReady, "device adapter runtime not ready");
        requireContract(errors, report.adapterRecordCount == 10, "adapter record count changed");
        requireContract(errors, report.downstreamRealCallsBlockedUntilDevice == 685, "downstream blocked call count changed");
        requireContract(errors, report.blockedRealCallsTotal == 688, "blocked real call total changed");
        requireContract(errors, report.realExecutedCalls == 0, "device adapter executed real calls unexpectedly");
    });

    runContractSuiteCheck(suite, "yuengine_backend_device_creation_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendDeviceCreationRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "device creation report is not ok");
        requireContract(errors, report.deviceCreationRuntimeReady, "device creation runtime not ready");
        requireContract(errors, report.platformExecutionAttempted, "platform execution was not attempted");
        requireContract(errors, report.windowSurfaceResultRecorded, "window surface result missing");
        requireContract(errors, report.d3dInterfaceResultRecorded, "D3D interface result missing");
        requireContract(errors, report.d3dDeviceResultRecorded, "D3D device result missing");
        requireContract(errors, report.executionResultRecords == 3, "device creation result count changed");
        requireContract(errors, report.downstreamRealCallsDeferred == 685, "downstream deferred call count changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_resource_creation_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendResourceCreationRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "resource creation report is not ok");
        requireContract(errors, report.resourceCreationRuntimeReady, "resource creation runtime not ready");
        requireContract(errors, report.readyResourceCreationExecuted, "ready resource creation was not executed");
        requireContract(errors, report.sourceResourceRecords == 46, "source resource record count changed");
        requireContract(errors, report.realResourceHandlesCreated == 41, "real resource handle count changed");
        requireContract(errors, report.failedResourceHandles == 0, "resource handle failures reported");
        requireContract(errors, report.readyPayloadBytesCreated == 23949794, "ready payload bytes changed");
    });

    runContractSuiteCheck(suite, "yuengine_backend_surface_material_font_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runBackendSurfaceMaterialFontRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok, "surface/material/font report is not ok");
        requireContract(errors, report.surfaceMaterialFontRuntimeReady, "surface/material/font runtime not ready");
        requireContract(errors, report.transientSurfaceCreationExecuted, "transient surface creation did not execute");
        requireContract(errors, report.transientSurfaceBindingExecuted, "transient surface binding did not execute");
        requireContract(errors, report.realTransientSurfacesCreated == 4, "transient surface count changed");
        requireContract(errors, report.renderTargetSurfacesCreated == 3, "render-target surface count changed");
        requireContract(errors, report.depthStencilSurfacesCreated == 1, "depth/stencil surface count changed");
        requireContract(errors, report.executedTransientTextureBindings == 4, "transient texture binding count changed");
        requireContract(errors, report.preservedDepthTextureBindings == 1, "depth texture gate preservation changed");
        requireContract(errors, report.preservedMaterialTextureBindings == 38, "material binding preservation changed");
        requireContract(errors, report.materialShaderEvidenceTracked, "material shader evidence was not tracked");
        requireContract(errors, report.materialShaderSlotBindingDeferred, "material shader slot binding was not deferred");
        requireContract(errors, report.fontAtlasEvidenceTracked, "font atlas evidence was not tracked");
        requireContract(errors, report.fontAtlasCreationDeferred, "font atlas creation was not deferred");
    });

    runContractSuiteCheck(suite, "yuengine_mission_event_thread_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runMissionEventThreadRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok && report.sceneRuntimeOk, "mission event thread report is not ok");
        requireContract(errors, report.eventThreadFound && report.eventThreadExecuted, "mission event thread did not execute");
        requireContract(errors, report.playerControlCommands == 2, "event thread player control count changed");
        requireContract(errors, report.dialogResetCommands == 1, "event thread dialog reset count changed");
        requireContract(errors, report.eventPageSetupCommands == 1, "event page setup count changed");
        requireContract(errors, report.eventVolumeActivationCommands == 1, "event volume activation count changed");
        requireContract(errors, report.unresolvedCalls == 0, "event thread unresolved calls changed");
    });

    runContractSuiteCheck(suite, "yuengine_mission_tutorial_contract", filter, [&](auto& errors) {
        const auto report = yu::runtime::runMissionTutorialRuntime(manifestPath, repoRoot);
        requireContract(errors, report.ok && report.sceneRuntimeOk, "mission tutorial report is not ok");
        requireContract(errors, report.tutorialThreadFound && report.tutorialThreadExecuted, "tutorial thread did not execute");
        requireContract(errors, report.updateUnitsExecuted, "updateUnits did not execute");
        requireContract(errors, report.dialogShowCommands == 3, "tutorial dialog show count changed");
        requireContract(errors, report.tutorialActorCreates == 1, "tutorial actor create count changed");
        requireContract(errors, report.playerControlCommands == 4, "tutorial player control count changed");
        requireContract(errors, report.updateUnitsCommands == 1, "updateUnits command count changed");
        requireContract(errors, report.unresolvedCalls == 0, "tutorial unresolved calls changed");
    });

    suite.elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - suiteStarted)
        .count();

    if (!filter.empty() && suite.contracts == 0) {
        ++suite.failed;
        suite.failures.push_back("no contract matched filter: " + filter);
    }

    std::cout << "contract-suite ok=" << (suite.failed == 0 ? "true" : "false")
              << " contracts=" << suite.contracts
              << " failed=" << suite.failed
              << " elapsed_ms=" << suite.elapsedMs;
    if (!filter.empty()) {
        std::cout << " filter=" << filter;
    }
    std::cout << "\n";

    return suite.failed == 0 ? 0 : 1;
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
        if (command == "title-ui") {
            auto report = yu::runtime::runTitleUiRuntime(manifest, repoRoot);
            std::cout << yu::runtime::titleUiRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "title-branches") {
            auto report = yu::runtime::runTitleBranchesRuntime(manifest, repoRoot);
            std::cout << yu::runtime::titleBranchesRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "gameplay-frame") {
            auto report = yu::runtime::runGameplayFrameRuntime(manifest, repoRoot);
            std::cout << yu::runtime::gameplayFrameRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "renderer-submit") {
            auto report = yu::runtime::runRendererBackendSubmissionRuntime(manifest, repoRoot);
            std::cout << yu::runtime::rendererBackendSubmissionReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "frame-scheduler") {
            auto report = yu::runtime::runFrameSchedulerRuntime(manifest, repoRoot);
            std::cout << yu::runtime::frameSchedulerRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-obligations") {
            auto report = yu::runtime::runBackendObligationsRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendObligationsRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "material-semantics") {
            auto report = yu::runtime::runMaterialSemanticsRuntime(manifest, repoRoot);
            std::cout << yu::runtime::materialSemanticsRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "device-presentation") {
            auto report = yu::runtime::runDevicePresentationRuntime(manifest, repoRoot);
            std::cout << yu::runtime::devicePresentationRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "texture-upload") {
            auto report = yu::runtime::runTextureUploadRuntime(manifest, repoRoot);
            std::cout << yu::runtime::textureUploadRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-state") {
            auto report = yu::runtime::runBackendStateRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendStateRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "resource-allocation") {
            auto report = yu::runtime::runBackendResourceAllocationRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendResourceAllocationRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "device-execution") {
            auto report = yu::runtime::runBackendDeviceExecutionRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendDeviceExecutionRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "present-oracle") {
            auto report = yu::runtime::runBackendPresentationOracleRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendPresentationOracleRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "platform-bridge") {
            auto report = yu::runtime::runBackendPlatformBridgeRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendPlatformBridgeRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-executor") {
            auto report = yu::runtime::runBackendExecutorRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendExecutorRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-device-adapter") {
            auto report = yu::runtime::runBackendDeviceAdapterRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendDeviceAdapterRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-device-create") {
            auto report = yu::runtime::runBackendDeviceCreationRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendDeviceCreationRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-resource-create") {
            auto report = yu::runtime::runBackendResourceCreationRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendResourceCreationRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-upload-bind") {
            auto report = yu::runtime::runBackendUploadBindingRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendUploadBindingRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "backend-surface-material-font") {
            auto report = yu::runtime::runBackendSurfaceMaterialFontRuntime(manifest, repoRoot);
            std::cout << yu::runtime::backendSurfaceMaterialFontRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "runtime-contract-suite") {
            const std::string filter = stringOption(argc, argv, "--filter", "");
            return runRuntimeContractSuite(manifest, repoRoot, filter);
        }
        if (command == "mission-event-thread") {
            auto report = yu::runtime::runMissionEventThreadRuntime(manifest, repoRoot);
            std::cout << yu::runtime::missionEventThreadRuntimeReportToJson(report);
            return report.ok ? 0 : 1;
        }
        if (command == "mission-tutorial") {
            auto report = yu::runtime::runMissionTutorialRuntime(manifest, repoRoot);
            std::cout << yu::runtime::missionTutorialRuntimeReportToJson(report);
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
