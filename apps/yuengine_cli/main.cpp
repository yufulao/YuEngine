#include "yuengine/project/ProjectManifest.h"
#include "yuengine/resource/ResourceDiagnostics.h"
#include "yuengine/runtime/EngineRuntime.h"

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

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
              << "  yuengine_cli resources <project.json>\n";
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
    } catch (const std::exception& ex) {
        std::cerr << "yuengine_cli: " << ex.what() << "\n";
        return 1;
    }

    usage();
    return 2;
}
