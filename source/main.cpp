#include <cstdarg>
#include <cstdio>
#include <vector>
#include <cstring>
#include <windows.h>

#define IMPL_ROSE_COMPILER
#include "../include/rose.compiler.h"

//https://de.wikipedia.org/wiki/FNV_(Informatik)#FNV-Implementation,_64-bit-Schl%C3%BCssel
constexpr uint64_t fnFNV (const char* pBuffer)
{
    const uint64_t  MagicPrime = 0x00000100000001b3;
    uint64_t        Hash       = 0xcbf29ce484222325;

    for (; *pBuffer; pBuffer++)
        Hash = (Hash ^ *pBuffer) * MagicPrime;   // bitweises XOR und dann Multiplikation

    return Hash;
}

int main(int argc, char** argv) {
    if(argc < 1) return 1; //Shouldn't happen

    RoseCompiler compiler;

	const char* app_name = "game.dll";
	const char* arg = "";
	const char* lastarg = "";
    uint64_t state = 0;
    bool proceed = false;

    for(char ** parg = argv + 1; parg != argv + argc; ) {
        lastarg = arg;
        arg = *parg;

        switch(state) {
            case 0:
                if(proceed) {
                    proceed = false;
                    parg++;
                    continue;
                } 
                if(arg[0] == '-') {
                    state = fnFNV(arg);
                    proceed = true;
                    continue;
                }
                else {
                    compiler.files.push_back(arg);
                    parg++;
                }
            break; case fnFNV("-v"): case fnFNV("--verbose"):
                compiler.verbose = true;  
            break; case fnFNV("-ne"): case fnFNV("--no_execute"):
                compiler.execute = false; 
            break; case fnFNV("-pwd"): case fnFNV("--current_path"):
                printf("PWD %s \n", argv[0]); 
            break; case fnFNV("--buildtime"): 
                printf(__DATE__ " " __TIME__ "\n"); 
            break; case fnFNV("-o"): case fnFNV("--output"):
				parg++; state = __LINE__; continue; case __LINE__:
                app_name = arg; 
            break; case fnFNV("-D"):
                compiler.defines.push_back(arg + 2); 
            break; default:
                std::fprintf(stderr, "unknown command %s \n", lastarg);
                return 1;
        }
        
        state = 0;
    }

    if(state != 0) {
        std::fprintf(stderr, "expected argument after %s \n", arg);
        return 1;
    }

    compiler.app_name = app_name;
    bool ok = compiler.compile();

    if (ok) {
        printf("\n\nOK! \n");
    }
    else {
        printf("\n\nError! \n");
    }

    return 0;
}
