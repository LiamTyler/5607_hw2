#include <iostream>
#include "include/rayTracer.h"
#include "include/utils.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/stb_image_write.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Please enter in just the name of the scene file" << endl;
        return -1;
    }
    RayTracer* tracer = new RayTracer;
    tracer->Parse(argv[1]);
    tracer->Trace();

    return 0;
}
