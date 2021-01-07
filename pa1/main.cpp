#include "OBJ_Loader.hpp"
#include "Scene.hpp"
#include "Timer.hpp"
#include "Triangle.hpp"
#include "Zbuf.hpp"
#include "global.hpp"
#include "shaders.hpp"

#include <cctype>
#include <cstdio>
#include <cstring>

void show_help(char const *selfname) {
    int l = strlen(selfname);
    int o;
    for (o = l - 1; o >= 0; --o) {
        if (selfname[o] == '/') {
            break;
        }
    }
    ++o;
    printf("%s: zbuffer demo\n\n", selfname + o);
    printf("    usage: %s <objfile> [-n|--normal]\n", selfname + o);
    printf("                             [-i|--interpolation]\n");
    printf("                             [-r|--resolution <WxH>]\n");
    printf("                             [-f|--field-of-view <fov>]\n");
    printf("                             [-o|--output <path>]\n");
    printf("\n");
    printf("    options:\n");
    printf("        -h|--help                 Show this message and quit\n");
    printf("        -n|--normal               Use normal fragment shader "
           "(default)\n");
    printf("        -i|--interpolation        Use interpolation fragment "
           "shader\n");
    printf("        -r|--resolution <WxH>     Use given resolution, default: "
           "1920x1080\n");
    printf("        -f|--field-of-view <fov>  Use given field of view (in "
           "degrees), default: 45\n");
    printf("        -o|--output <path>        Save render result (ppm "
           "format) to <path>, default: zbuffer.ppm\n");
    printf("\n");
}

int main(int argc, char **argv) {
    // Path to the obj file
    std::string objfile;
    // Path of the file to save render result
    std::string outfile{"zbuffer.ppm"};
    // Shader function to use
    std::function<Color(Triangle const &, Triangle const &,
                        std::tuple<flt, flt, flt> const &barycentric)>
        selected_fragment_shader = shdr::normal_shader;
    // Resolution (horizontal)
    int width = 1920;
    // Resolution (vertical)
    int height = 1080;
    // Field of view (in degrees)
    flt fovy = 45;

    /*************************** Parse arguments ****************************/
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            show_help(argv[0]);
            return 0;
        } else if (!strcmp(argv[i], "-n") || !strcmp(argv[i], "--normal")) {
            selected_fragment_shader = shdr::normal_shader;
        } else if (!strcmp(argv[i], "-i") ||
                   !strcmp(argv[i], "--interpolation")) {
            selected_fragment_shader = shdr::vertex_interpolation_shader;
        } else if (!strcmp(argv[i], "-r") ||
                   !strcmp(argv[i], "--resolution")) {
            ++i;
            if (i >= argc) {
                break;
            }
            int reslen = strlen(argv[i]);
            int split  = -1;
            for (int j = 0; j < reslen; ++j) {
                if (split == -1 && argv[i][j] == 'x') {
                    split = j;
                } else if (!std::isdigit(argv[i][j])) {
                    split = -1;
                    break;
                }
            }
            if (split == -1) {
                continue;
            } else {
                argv[i][split] = '\0';
                width          = atoi(argv[i]);
                height         = atoi(argv[i] + split + 1);
                // debugm("width is %d, height is %d\n", width, height);
            }
        } else if (!strcmp(argv[i], "-f") ||
                   !strcmp(argv[i], "--field-of-view")) {
            ++i;
            if (i >= argc) {
                break;
            }
            fovy = atof(argv[i]);
        } else if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) {
            ++i;
            if (i >= argc) {
                break;
            }
            outfile = argv[i];
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unrecognized option '%s'\n", argv[i]);
        } else {
            objfile = argv[i];
        }
    }
    if (objfile.size() == 0) {
        show_help(argv[0]);
        return 0;
    }

    /****************************** Load model ******************************/
    objl::Loader loader;
    debugm("loading object from file '%s' ..\n", objfile.c_str());
    if (!loader.LoadFile(objfile)) {
        errorm("Failed to load object from '%s'\n", objfile.c_str());
    }
    msg("Object loaded\n");
    Scene world{loader.LoadedMeshes[0]};

    // Create a renderer on scene
    Zbuf zbuf{world, static_cast<size_t>(height), static_cast<size_t>(width)};

    // Set fragment shader
    zbuf.set_shader(selected_fragment_shader);

    // Camera extrinsincs
    vec3 eye{-3, 5, 10};
    // vec3 gaze(0, 0, -1);
    vec3 gaze = glm::normalize(-eye);
    vec3 up{2, 3, -.9};
    flt  aspect_ratio = 1.0 * height / width;
    flt  znear        = -.1;
    flt  zfar         = -50;
    // Camera
    Camera camera{eye, fovy, aspect_ratio, znear, zfar, gaze, up};

    // Initialize camera
    zbuf.init_cam(camera);

    // Use no model transformation
    zbuf.set_model_transformation(glm::identity<mat4>());

    msg("Rendering scene ..\n");
    Timer timer;
    timer.start();
    zbuf.render();
    timer.end();

    msg("Scene (%dx%d) rendered in %.0f milliseconds\n", width, height,
        timer.elapsedms());

    write_ppm(outfile, zbuf.img);

    return 0;
}

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Nov 18 2020, 17:33 [CST]
