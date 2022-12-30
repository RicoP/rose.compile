#ifndef ROSE_COMPILER_H_GUARD
#define ROSE_COMPILER_H_GUARD

#include <vector>

struct RoseCompiler {
    enum {
        BUFFER_SIZE = 4096
    };

    char buffer[BUFFER_SIZE] = "";
    int left = BUFFER_SIZE;

    std::vector<const char*> files;
    std::vector<const char*> defines;
    std::vector<const char*> includes;
    const char* app_name = "game.dll";
    bool verbose = false;
    bool execute = true;
    bool ok = false;

    bool construct();
    bool compile();
    void clear() { left = BUFFER_SIZE; ok = false; }

    bool printf (char const* const format, ...);
};

#endif

#ifdef IMPL_ROSE_COMPILER
#ifndef IMPL_ROSE_COMPILER_GUARD
#define IMPL_ROSE_COMPILER_GUARD

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <windows.h>

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

bool RoseCompiler::printf (char const* const format, ...) {
    char * p = buffer + (BUFFER_SIZE - left);

    va_list args;
    va_start(args, format);
    int n = std::vsnprintf(p, left, format, args);
    va_end(args);

    if (n == -1 || n >= left) {
        std::fprintf(stderr, "Buffer overflow \n");
        left = 0;
        return false;
    }
    left -= n;

    return true;
};

bool RoseCompiler::construct() {
    clear();

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

    char pdb_name[512] = "";
    get_temp_file_name_ext(pdb_name, ".pdb");

    ok = printf("CL /nologo /MP /std:c++17 /wd\"4530\" ");

    for (auto& def : defines) {
        ok = ok && printf("/D%s ", def);
    }

    for (auto& def : extraDefines) {
        ok = ok && printf("/D%s ", def);
    }

    ok = ok && printf("/Zi /LD /MD ");

    for (auto& include : includes) {
        ok = ok && printf("/I %s ", include);
    }

    for (auto& include : extraIncludes) {
        ok = ok && printf("/I %s ", include);
    }

    ok = ok && printf("/Fe:\"%s\" ", app_name);

    for (auto& file : files) {
        ok = ok && printf("%s ", file);
    }

    for (auto& file : extraFiles) {
        ok = ok && printf("%s ", file);
    }

    for (auto& lib : libs) {
        ok = ok && printf("%s ", lib);
    }

    ok = ok && printf("/link /incremental /PDB:\"%s\" ", pdb_name);
    
    return ok;
}

bool RoseCompiler::compile() {
    if (ok && verbose) {
        std::printf("%s \n", buffer);
    }

    if (ok && execute) {
        ok = 0 == std::system(buffer);
    }

    return ok;
}

#endif
#endif