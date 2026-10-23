// Paths.cpp
#include "Paths.h"
#include <algorithm>
#include <fstream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

std::string executableDirectory() {
    std::string path;
#ifdef _WIN32
    char buffer[MAX_PATH];
    const DWORD length = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    if (length == 0 || length >= MAX_PATH) return "";
    path.assign(buffer, length);
    std::replace(path.begin(), path.end(), '\\', '/');
#else
    char buffer[4096];
    const ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (length <= 0) return "";
    path.assign(buffer, static_cast<size_t>(length));
#endif
    const size_t slash = path.find_last_of('/');
    return slash == std::string::npos ? "" : path.substr(0, slash + 1);
}

// ЎукаЇмо в поточн≥й тец≥ та в тец≥ програми, п≥дн≥маючись до 3 р≥вн≥в угору
// (щоб знаходити файл ≥ при запуску з проЇкту, ≥ з bin/Debug).
std::vector<std::string> dataFileCandidates(const std::string& fileName) {
    std::vector<std::string> bases = {""};
    const std::string exeDir = executableDirectory();
    if (!exeDir.empty()) bases.push_back(exeDir);

    std::vector<std::string> result;
    auto add = [&](const std::string& path) {
        if (std::find(result.begin(), result.end(), path) == result.end()) result.push_back(path);
    };

    for (const auto& base : bases) {
        std::string prefix = base;
        for (int up = 0; up <= 3; ++up) {
            add(prefix + "data/" + fileName);
            add(prefix + fileName);
            prefix += "../";
        }
    }
    return result;
}

std::string findDataFile(const std::string& fileName) {
    for (const auto& path : dataFileCandidates(fileName)) {
        std::ifstream file(path, std::ios::binary);
        if (file.good()) return path;
    }
    return "";
}
