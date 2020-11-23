#include "OBJ_Loader.hpp"
#include "global.hpp"
#include "Scene.hpp"

#include <cstdio>
#include <cstring>

#include <glm/glm.hpp>
#include <opencv2/opencv.hpp>

void show_help(char *selfname) {
    printf("%s: zbuffer demo\n\n", selfname);
    printf("    usage:\n");
    printf("        -h|--help               print this message\n");
    printf(
        "        -o|--object <file>      load .obj file stored in <file>\n");
    printf("\n");
}

int main(int argc, char **argv) {
    std::string objfile; // Path to the obj file

    /****** Parse arguments ******/
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            show_help(argv[0]);
            return 0;
        } else {
            objfile = argv[i];
        }
    }
    if (objfile.size() == 0) {
        fprintf(stderr, " [x] No obj file specified\n");
        return 1;
    }

    /****** Load model *****/
    objl::Loader loader;
    printf(" [*] loading object fro file '%s' ..\n", objfile.c_str());
    if (!loader.LoadFile(objfile)) {
        fprintf(stderr, " [x] failed to load object %s\n", objfile.c_str());
    }
    printf(" [v] object loaded\n");

    /****** playground ******/
    printf("%lu meshes loaded\n", loader.LoadedMeshes.size());
    Scene scene(loader.LoadedMeshes[0]);

    return 0;
}

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Nov 18 2020, 17:33 [CST]
