#define STBI_IMAGE_WRITE_IMPLEMENTATION
#define STBI_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/stb_image_write.h"
#include "include/utils.h"
#include <string>

class Image {
    public:
        Image();
        Image(int w, int h);

        ~Image();

        vec4 GetPixel(int r, int c) { return pixels_[r*width_ + c]; }
        void SetPixel(int r, int c, vec4 p) { pixels_[r*width_ + c] = p; }

        void Write(std::string fname);

    protected:
        int width_;
        int height_;
        vec4* pixels_;
};
