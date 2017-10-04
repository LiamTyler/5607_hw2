#include <iostream>
#include "include/parser.h"
#include "include/utils.h"
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image.h"
#include "include/stb_image_write.h"

using namespace std;

class Pixel {
    public:
        Pixel() : Pixel(0, 0, 0, 0) {}
        Pixel(float rr, float gg, float bb, float aa) {
            r = rr;
            g = gg;
            b = bb;
            a = aa;
        }

        float r, g, b, a;
};

class Image {
    public:
        Image() : Image(640, 480) {}
        Image(int w, int h) {
            width = w;
            height = h;
            pixels = new Pixel[w*h];
            for (int i = 0; i < w*h; i++)
                pixels[i] = Pixel(0,0,0,0);
        }

        ~Image() {
            delete [] pixels;
        }

        Pixel GetPixel(int r, int c) { return pixels[r*width + c]; }
        void SetPixel(int r, int c, Pixel p) { pixels[r*width + c] = p; }

        void Write(string fname) {
            unsigned char out[width*height*3];
            int b = 0;
            for (int r = 0; r < height; r++) {
                for (int c = 0; c < width; c++) {
                    Pixel p = GetPixel(r,c);
                    out[b++] = (unsigned char) 255*p.r;
                    out[b++] = (unsigned char) 255*p.g;
                    out[b++] = (unsigned char) 255*p.b;
                }
            }

            stbi_write_bmp(fname.c_str(), width, height, 3, out);
        }

        int width, height;
        Pixel* pixels;
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Please enter in just the name of the scene file" << endl;
        return -1;
    }
    Parser p(argv[1]);
    p.Parse();
    Camera* camera = p.getCamera();
    int width = camera->getWidth();
    int height = camera->getHeight();
    Image img(width, height);

    vec3 pos = camera->getPos();
    vec3 dir = camera->getDir();
    vec3 up = camera->getUp();
    float half_angle = camera->getHalfAngle() * M_PI / 180.0;
    float d = height / (2.0 * tan(half_angle));
    vec3 dx = glm::normalize(glm::cross(up, dir));
    vec3 dy = glm::normalize(-up);
    vec3 ul = pos + d * dir + up * (height / 2.0) - (width / 2.0) * dx;
    // cout << pos << endl;
    // cout << dir << endl;
    // cout << up << endl;
    // cout << dx << endl;
    // cout << dy << endl;
    // cout << ul << endl;
    // cout << half_angle << endl;
    // cout << d << endl;

    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            vec3 p = py + c*dx;
            vec ray = glm::normalize(p - pos);
        }
    }

    // write final image out
    img.Write(camera->getOutputImage());


    return 0;
}
