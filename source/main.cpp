#include <cstdarg>
#include <cstdio>
#include <vector>
#include <cstring>

static bool g_verbose = false;
static bool g_execute = true;
static char * g_app_name = "game.dll";
static char g_pdb_name[512] = "";

template<size_t SIZE>
struct SafePrinter {
    char buffer[SIZE] = "";
    char * p = buffer;
    int left = SIZE;

    void printf (char const* const format, ...) {
        va_list args;
        va_start(args, format);
        int n = std::vsnprintf(p, left, format, args);
        va_end(args);

        if (n == -1 || n >= left) {
            std::fprintf(stderr, "Buffer overflow \n");
            std::exit(1);
        }
        p += n;
        left -= n;
    };
};

// Function to generate a temporary filename ending with ".pdb"
void GetTempFileName(char *temp_file_name, const char * file_type) {
    // Generate a temporary filename and store it in the provided buffer
    std::tmpnam(temp_file_name);

    // Append ".pdb" to the end of the filename
    std::strcat(temp_file_name, file_type);
}

int main(int argc, char ** argv) {
    if(argc < 1) return 1; //Shouldn't happen

    enum class State {
        None = 0,
        Name
    } state = State::None;

    std::vector<const char*> includes {
        "."
        ,"../include/maths"
        ,"../include/mathc"
        ,"../include"
        ,"../include/imgui"
        ,"../roselib/include"
        ,"../raylib/src"
        ,"../premake-comppp/include/"
    };
    std::vector<const char*> files { "../rose/source/systems/source/roseimpl.cpp"};
    std::vector<const char*> defines {"IMGUI_API=__declspec(dllimport)"};
    std::vector<const char*> libs {
        "A:/rose_repo/rose/.build/bin/Release/raylib.lib", 
        "A:/rose_repo/rose/.build/bin/Release/imgui.lib"
    };

    GetTempFileName(g_pdb_name, ".pdb");

    for(char ** parg = argv + 1; parg != argv + argc; parg++) {
        char * arg = *parg;

        switch(state) {
            case State::None: 
                if(arg[0] == '-') {
                    //options
                    if(std::strcmp(arg, "-v") == 0 || std::strcmp(arg, "--verbose") == 0) {
                        g_verbose = true;
                    } else if(std::strcmp(arg, "-ne") == 0 || std::strcmp(arg, "--no_execute") == 0) {
                        g_execute = false;
                    } else if(std::strcmp(arg, "-pwd") == 0 || std::strcmp(arg, "--current_path") == 0) {
                        printf("PWD %s \n", argv[0]);
                    } else if(std::strcmp(arg, "--buildtime") == 0) {
                        printf(__DATE__ " " __TIME__ "\n");
                    } else if(std::strcmp(arg, "-o") == 0 || std::strcmp(arg, "--output") == 0) {
                        state = State::Name; continue;
                    } else {
                        fprintf(stderr, "Unknown option -%s \n", arg);
                        return 1;
                    }
                }
                else {
                    files.push_back(arg);
                }
                break;
            case State::Name: g_app_name = arg; break;
        }
        
        state = State::None;
    }

    //
	// error = execute(f'{compiler} /nologo /MP /std:c++17 /wd"4530"
    // {arg_defines} 
    // /Zi {dll_stuff}
    // {INCLUDES}
    // /Fe:"{APP_NAME}"
    // {arg_c_files}
    // {libs}
    // /link /incremental /PDB:"{PDB_NAME}" > {TMP}/clout.txt')

    {
        SafePrinter<4096> command;
        command.printf("CL /nologo /MP /std:c++17 /wd\"4530\" ");

        for(auto & def : defines) {
            command.printf("/D%s ", def);
        }

        command.printf("/Zi /LD /MD ");

        for(auto & include : includes) {
            command.printf("/I %s ", include);
        }

        command.printf("/Fe:\"%s\" ", g_app_name);
        
        for(auto & file : files) {
            command.printf("%s ", file);
        }

        for(auto & lib : libs) {
            command.printf("%s ", lib);
        }

        command.printf("/link /incremental /PDB:\"%s\" ", g_pdb_name);

        //print command
        if(g_verbose) {
            std::printf("%s \n", command.buffer);
        }
        
        if(g_execute) {
            std::system(command.buffer);
        }
    }

    return 0;
}

