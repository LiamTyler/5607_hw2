#include <iostream>
#include "include/parser.h"
#include "include/utils.h"

using namespace std;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cout << "Please enter in just the name of the scene file" << endl;
        return -1;
    }
    Parser p(argv[1]);
    p.Parse();
    // cout << "---------------------------------------------" << endl;
    // cout << "camera pos: " << p.getCameraPos() << endl;
    // cout << "camera dir: " << p.getCameraDir() << endl;
    // cout << "camera up: " << p.getCameraUp() << endl;
    // cout << "half angle: " << p.getHalfAngle() << endl;
    // cout << "width: " << p.getWidth() << endl;
    // cout << "height: " << p.getHeight() << endl;
    // cout << "filename: " << p.getFilename() << endl;
    // cout << "output image: " << p.getOutputImage() << endl;

    return 0;
}
