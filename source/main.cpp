#include <vector>
#include <cstring>

static bool g_verbose = false;
static char * g_app_name = "game";
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
        char buffer[4096];
        int left = 4096;
        int n = 0;

        char * p = buffer;

        auto err_check = [&] () {
            if (n == -1 || n >= left) {
                std::fprintf(stderr, "Buffer overflow \n");
                std::exit(1);
            }
            p += n;
            left -= n;
        };

        n = std::snprintf(p, left, "CL /nologo /MP /std:c++17 /wd\"4530\" ");
        err_check();

        for(auto & def : defines) {
            n = std::snprintf(p, left, "%s ", def);
            err_check();
        }

        n = std::snprintf(p, left, "/Zi /LD /MD ");
        err_check();

        for(auto & include : includes) {
            n = std::snprintf(p, left, "%s ", include);
            err_check();
        }

        n = std::snprintf(p, left, "/FE:\"%s\" ", g_app_name);
        err_check();

        for(auto & file : files) {
            n = std::snprintf(p, left, "%s ", file);
            err_check();
        }

        for(auto & lib : libs) {
            n = std::snprintf(p, left, "%s ", lib);
            err_check();
        }

        n = std::snprintf(p, left, "/link /incremental /PDB:\"%s\" ", g_pdb_name);
        err_check();

        //print command
        std::printf("%s \n %d \n", buffer, left);
    }

    return 0;
}
