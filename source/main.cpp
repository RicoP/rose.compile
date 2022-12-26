#include <cstdarg>
#include <cstdio>
#include <vector>
#include <cstring>
#include <windows.h>

#define IMPL_ROSE_COMPILER
#include "../include/rose.compiler.h"

int main(int argc, char** argv) {
    if(argc < 1) return 1; //Shouldn't happen

    RoseCompiler compiler;

	char* g_app_name = "game.dll";

    enum class State {
        None = 0,
        Name
    } state = State::None;

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
