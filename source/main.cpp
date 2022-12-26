#include <cstdarg>
#include <cstdio>
#include <vector>
#include <cstring>

static bool g_verbose = false;
static bool g_execute = true;
static char * g_app_name = "game.exe";
static char g_pdb_name[512] = "1234";


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

    std::vector<const char*> files;
    std::vector<const char*> defines;
    std::vector<const char*> includes;
    std::vector<const char*> libs;

    GetTempFileName(g_pdb_name, ".pdb");

    for(char ** parg = argv + 1; parg != argv + argc; parg++) {
        char * arg = *parg;

        switch(state) {
            case State::None: 
                if(arg[0] == '-') {
                    ++arg;
                    //option
                    if(std::strcmp(arg, "v") == 0 || std::strcmp(arg, "-verbose")) {
                        g_verbose = true;
                    } else if(std::strcmp(arg, "ne") == 0 || std::strcmp(arg, "-no_execute")) {
                        g_execute = false;
                    } else if(std::strcmp(arg, "n") == 0 || std::strcmp(arg, "-name")) {
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
        char command[4096];
        int left = 4096;
        char * p = command;

        auto safe_sprintf = [&] (char const* const format, ...) {
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

        safe_sprintf("CL /nologo /MP /std:c++17 /wd\"4530\" ");

        for(auto & def : defines) {
            safe_sprintf("%s ", def);
        }

        safe_sprintf("/Zi /LD /MD ");

        for(auto & include : includes) {
            safe_sprintf("%s ", include);
        }

        safe_sprintf("/FE:\"%s\" ", g_app_name);
        
        for(auto & file : files) {
            safe_sprintf("%s ", file);
        }

        for(auto & lib : libs) {
            safe_sprintf("%s ", lib);
        }

        safe_sprintf("/link /incremental /PDB:\"%s\" ", g_pdb_name);
        

        //print command
        if(g_verbose) {
            std::printf("%s \n %d \n", command, left);
        }
        
        if(g_execute) {
            std::system(command);
        }
    }

    return 0;
}

