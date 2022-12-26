#include <cstdarg>
#include <cstdio>
#include <vector>
#include <cstring>
#include <windows.h>

template<size_t SIZE>
struct SafePrinter {
    char buffer[SIZE] = "";
    char * p = buffer;
    int left = SIZE;

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

char* get_program_path(const char* program_name) {
  static char path[MAX_PATH];
  if (SearchPathA(NULL, program_name, ".exe", MAX_PATH, path, NULL) != 0) {
    return path;
  }
  else {
    return NULL;
  }
}

// Function to generate a temporary filename ending with the file type
void GetTempFileName(char *temp_file_name, const char * file_type) {
    // Generate a temporary filename and store it in the provided buffer
    std::tmpnam(temp_file_name);

    // Append ".xxx" to the end of the filename
    std::strcat(temp_file_name, file_type);
}

struct RoseCompiler {
    std::vector<const char*> files;
    std::vector<const char*> defines;
    std::vector<const char*> includes;
    char* app_name = "";
    bool verbose = false;
    bool execute = true;

    bool compile();
};

int main(int argc, char** argv) {
    if(argc < 1) return 1; //Shouldn't happen

    RoseCompiler compiler;

	char* g_app_name = "game.dll";

    enum class State {
        None = 0,
        Name
    } state = State::None;

    compiler.includes = std::vector<const char*> {
        "."
        ,"../include/maths"
        ,"../include/mathc"
        ,"../include"
        ,"../include/imgui"
        ,"../roselib/include"
        ,"../raylib/src"
        ,"../premake-comppp/include/"
    };

    for(char ** parg = argv + 1; parg != argv + argc; parg++) {
        char * arg = *parg;

        switch(state) {
            case State::None: 
                if(arg[0] == '-') {
                    //options
                    if(std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--verbose") == 0) {
                        compiler.verbose = true;
                    } else if(std::strcmp(arg, "-ne") == 0 || std::strcmp(arg, "--no_execute") == 0) {
                        compiler.execute = false;
                    } else if(std::strcmp(arg, "-pwd") == 0 || std::strcmp(arg, "--current_path") == 0) {
                        printf("PWD %s \n", argv[0]);
                    } else if(std::strcmp(arg, "--buildtime") == 0) {
                        printf(__DATE__ " " __TIME__ "\n");
                    } else if(std::strcmp(arg, "-o") == 0 || std::strcmp(arg, "--output") == 0) {
                        state = State::Name; continue;
                    } else if(std::strcmp(arg, "-D") == 0) {
                        compiler.defines.push_back(arg + 2);
                    } else {
                        fprintf(stderr, "Unknown option %s \n", arg);
                        return 1;
                    }
                }
                else {
                    compiler.files.push_back(arg);
                }
                break;
            case State::Name: g_app_name = arg; break;
        }
        
        state = State::None;
    }

    bool ok = compiler.compile();

    if (ok) {
        printf("\n\nOK! \n");
    }
    else {
        printf("\n\nError! \n");
    }

    return 0;
}


bool RoseCompiler::compile() {
    SafePrinter<4096> command;
    bool ok;

    std::vector<const char*> libs {
        "../rose/.build/bin/Release/raylib.lib",
        "../rose/.build/bin/Release/imgui.lib"
    };

    files.push_back("../rose/source/systems/source/roseimpl.cpp");
    defines.push_back("IMGUI_API=__declspec(dllimport)");

    char pdb_name[512] = "";
    GetTempFileName(pdb_name, ".pdb");

    ok = command.printf("CL /nologo /MP /std:c++17 /wd\"4530\" ");
    if(!ok) return false;

    for (auto& def : defines) {
        ok = command.printf("/D%s ", def);
        if(!ok) return false;
    }

    ok = command.printf("/Zi /LD /MD ");
    if(!ok) return false;

    for (auto& include : includes) {
        ok = command.printf("/I %s ", include);
        if(!ok) return false;
    }

    ok = command.printf("/Fe:\"%s\" ", app_name);
    if(!ok) return false;

    for (auto& file : files) {
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
