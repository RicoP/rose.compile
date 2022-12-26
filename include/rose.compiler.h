#ifndef ROSE_COMPILER_H_GUARD
#define ROSE_COMPILER_H_GUARD

#include <vector>

struct RoseCompiler {
    std::vector<const char*> files;
    std::vector<const char*> defines;
    std::vector<const char*> includes;
    const char* app_name = "";
    bool verbose = false;
    bool execute = true;

    bool compile();
};

#endif

#ifdef IMPL_ROSE_COMPILER
#ifndef IMPL_ROSE_COMPILER_GUARD
#define IMPL_ROSE_COMPILER_GUARD

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <windows.h>

template<size_t SIZE>
struct SafePrinter {
    char buffer[SIZE] = "";
    char * p = buffer;
    int left = SIZE;

    SafePrinter() = default;
    SafePrinter(const SafePrinter &) = delete;
    SafePrinter operator=(const SafePrinter &) = delete;

    bool printf (char const* const format, ...) {
        va_list args;
        va_start(args, format);
        int n = std::vsnprintf(p, left, format, args);
        va_end(args);

        if (n == -1 || n >= left) {
            std::fprintf(stderr, "Buffer overflow \n");
            left = 0;
            return false;
        }
        p += n;
        left -= n;

        return true;
    };
};

inline char* get_program_path(const char* program_name) {
  static char path[MAX_PATH];
  if (SearchPathA(NULL, program_name, ".exe", MAX_PATH, path, NULL) != 0) {
    return path;
  }
  else {
    return NULL;
  }
}

// Function to generate a temporary filename ending with the file type
inline void get_temp_file_name_ext(char *temp_file_name, const char * file_type) {
    // Generate a temporary filename and store it in the provided buffer
    std::tmpnam(temp_file_name);

    // Append ".xxx" to the end of the filename
    std::strcat(temp_file_name, file_type);
}

bool RoseCompiler::compile() {
    SafePrinter<4096> command;

    std::vector<const char*> libs {
        "../rose/.build/bin/Release/raylib.lib",
        "../rose/.build/bin/Release/imgui.lib"
    };

    std::vector<const char*> extraIncludes {
        "."
        ,"../include/maths"
        ,"../include/mathc"
        ,"../include"
        ,"../include/imgui"
        ,"../roselib/include"
        ,"../raylib/src"
        ,"../premake-comppp/include/"
    };

    std::vector<const char*> extraFiles {"../rose/source/systems/source/roseimpl.cpp"};
    std::vector<const char*> extraDefines { "IMGUI_API=__declspec(dllimport)" };

    bool ok = true;

    char pdb_name[512] = "";
    get_temp_file_name_ext(pdb_name, ".pdb");

    ok = command.printf("CL /nologo /MP /std:c++17 /wd\"4530\" ");
    if(!ok) return false;

    for (auto& def : defines) {
        ok = command.printf("/D%s ", def);
        if(!ok) return false;
    }

    for (auto& def : extraDefines) {
        ok = command.printf("/D%s ", def);
        if(!ok) return false;
    }

    ok = command.printf("/Zi /LD /MD ");
    if(!ok) return false;

    for (auto& include : includes) {
        ok = command.printf("/I %s ", include);
        if(!ok) return false;
    }

    for (auto& include : extraIncludes) {
        ok = command.printf("/I %s ", include);
        if(!ok) return false;
    }

    ok = command.printf("/Fe:\"%s\" ", app_name);
    if(!ok) return false;

    for (auto& file : files) {
        ok = command.printf("%s ", file);
        if(!ok) return false;
    }

    for (auto& file : extraFiles) {
        ok = command.printf("%s ", file);
        if(!ok) return false;
    }

    for (auto& lib : libs) {
        ok = command.printf("%s ", lib);
        if(!ok) return false;
    }

    ok = command.printf("/link /incremental /PDB:\"%s\" ", pdb_name);
    if(!ok) return false;

    if (verbose) {
        std::printf("%s \n", command.buffer);
    }

    if (execute) {
        ok = 0 == std::system(command.buffer);
    }

    return ok;
}

#endif
#endif