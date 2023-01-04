#ifndef ROSE_COMPILER_H_GUARD
#define ROSE_COMPILER_H_GUARD

#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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
    bool silent = false;
    bool pipe = false;
    bool ok = false;

    bool construct();
    bool compile();
    void clear() { left = BUFFER_SIZE; ok = false; }

    bool buffer_printf (char const* const format, ...);
    void reset_buffer() { left = BUFFER_SIZE; }
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

bool RoseCompiler::buffer_printf (char const* const format, ...) {
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
        ,"../imgui"
        ,"../roselib/include"
        ,"../raylib/src"
        ,"../premake-comppp/include/"
    };

    std::vector<const char*> extraFiles {"../rose/source/systems/source/roseimpl.cpp"};
    std::vector<const char*> extraDefines { "IMGUI_API=__declspec(dllimport)" };

    char pdb_name[512] = "";
    get_temp_file_name_ext(pdb_name, ".pdb");

    ok = buffer_printf("CL /nologo /MP /std:c++17 /wd\"4530\" ");

    for (auto& def : defines) {
        ok = ok && buffer_printf("/D%s ", def);
    }

    for (auto& def : extraDefines) {
        ok = ok && buffer_printf("/D%s ", def);
    }

    ok = ok && buffer_printf("/Zi /LD /MD ");

    for (auto& include : includes) {
        ok = ok && buffer_printf("/I %s ", include);
    }

    for (auto& include : extraIncludes) {
        ok = ok && buffer_printf("/I %s ", include);
    }

    ok = ok && buffer_printf("/Fe:\"%s\" ", app_name);

    for (auto& file : files) {
        ok = ok && buffer_printf("%s ", file);
    }

    for (auto& file : extraFiles) {
        ok = ok && buffer_printf("%s ", file);
    }

    for (auto& lib : libs) {
        ok = ok && buffer_printf("%s ", lib);
    }

    ok = ok && buffer_printf("/link /incremental /PDB:\"%s\" ", pdb_name);
    
    return ok;
}

inline void clean_command(char * message) {
    char * p = message;
    //Replace all " with '
    while ((p = strchr(p, '"'))) *p = '\'';
}

inline void announce(const char * kind, const char * message) {
    char command[1024];
    sprintf(command, "rose -broadcast%s \"%s\"", kind, message);
    std::system(command);
}

bool RoseCompiler::compile() {
    if (ok && verbose) {
        std::printf("%s \n", buffer);
    }
    reset_buffer();

    if (ok && execute) {
        FILE * fp = _popen(buffer, "r");

        while (fgets(buffer, BUFFER_SIZE, fp) != nullptr) {
            if(!silent) {
                std::printf("%s", buffer);
            }
            if(pipe) {
                bool error = strstr(buffer, "error") != nullptr;
                bool warning = strstr(buffer, "warning") != nullptr;

                char * kind = nullptr;

                if (error) {
                    kind = "Error";
                }

                if (warning) {
                    kind = "Warning";
                }

                if(error || warning) {
                    clean_command(buffer);
                    announce(kind, buffer);
                }
            }
        }

        int return_value = _pclose(fp);

        if(pipe) {
            if(return_value) {
                announce("Error", "Compiling Failed");
            }
            else {
                announce("Success", "Compiling Success");
            }
        }

        return return_value == 0;
    }

    return ok;
}

#endif
#endif