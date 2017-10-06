#include "include/image.h"

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
    unsigned char out[width_*height_*3];
    int b = 0;
    for (int r = 0; r < height_; r++) {
        for (int c = 0; c < width_; c++) {
            vec4 p = GetPixel(r,c);
            out[b++] = (unsigned char) 255*p.r;
            out[b++] = (unsigned char) 255*p.g;
            out[b++] = (unsigned char) 255*p.b;
        }
    }

    stbi_write_bmp(fname.c_str(), width_, height_, 3, out);
}
