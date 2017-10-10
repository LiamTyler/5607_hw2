#include "include/image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

Image::Image() : Image(640, 480) {}

Image::Image(int w, int h) {
    width_ = w;
    height_ = h;
    pixels_ = new vec4[w*h];
    for (int i = 0; i < w*h; i++)
        pixels_[i] = vec4(0,0,0,0);
}

Image::~Image() {
    delete [] pixels_;
}

void Image::Write(std::string fname) {
    unsigned char out[width_*height_*4];
    int b = 0;
    for (int r = 0; r < height_; r++) {
        for (int c = 0; c < width_; c++) {
            vec4 p = GetPixel(r,c);
            out[b++] = (unsigned char) 255*p.r;
            out[b++] = (unsigned char) 255*p.g;
            out[b++] = (unsigned char) 255*p.b;
            out[b++] = (unsigned char) 255*p.a;
        }
    }

    int i = fname.length();
    while (fname[--i] != '.' && i >= 0);
    if (i < 0) {
        std::cout << "Invalid image file name: " << fname << std::endl;
        return;
    }

    switch(fname[i+1]) {
        case 'p':
            stbi_write_png(fname.c_str(), width_, height_, 4, out, width_*4);
            break;
        case 'b':
            stbi_write_bmp(fname.c_str(), width_, height_, 4, out);
            break;
        default:
            std::cout << "Invalid image file name: " << fname << std::endl;
    }
}
