#ifndef SRC_INCLUDE_CANVAS_H_
#define SRC_INCLUDE_CANVAS_H_

#include "include/utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"

using namespace std;
using namespace nanogui;

class MyGLCanvas : public GLCanvas {
    public:
        MyGLCanvas(Widget *parent) : GLCanvas(parent) {
            mShader.init(
                    /* An identifying name */
                    "image shader",

                    /* Vertex shader */
                    "#version 330\n"
                    "in vec2 position;\n"
                    "in vec2 texCoords;\n"
                    "out vec2 itc;\n"
                    "void main() {\n"
                    "   gl_Position = vec4(position, 0.0, 1.0);\n"
                    "   itc = texCoords;\n"
                    "}",

                    /* Fragment shader */
                    "#version 330\n"
                    "uniform sampler2D tex;\n"
                    "uniform bool usingTex;\n"
                    "in vec2 itc;\n"
                    "out vec4 color;\n"
                    "void main() {\n"
                    "   if (usingTex) {\n"
                    "       color = texture(tex, vec2(itc.x, itc.y));\n"
                    "   } else {\n"
                    "       color = vec4(0, 0, 0, 1);\n"
                    "   }"
                    "}"
                    );

            MatrixXu indices(3, 2); /* Draw a cube */
            indices.col( 0) << 0, 1, 2;
            indices.col( 1) << 2, 3, 0;

            MatrixXf positions(2, 4);
            positions.col(0) << -1, -1;
            positions.col(1) <<  1, -1;
            positions.col(2) <<  1,  1;
            positions.col(3) << -1,  1;

            MatrixXf texCoords(2, 4);
            texCoords.col(0) << 0, 1;
            texCoords.col(1) << 1, 1;
            texCoords.col(2) << 1, 0;
            texCoords.col(3) << 0, 0;

            mShader.bind();
            mShader.uploadIndices(indices);

            mShader.uploadAttrib("position", positions);
            mShader.uploadAttrib("texCoords", texCoords);

            mShader.setUniform("usingTex", false);
        }

        void LoadTexture(string path) {
            mShader.bind();
            int w, h, comp;
            unsigned char * image;

            image = stbi_load(path.c_str(), &w, &h, &comp, 4);
            if (image == nullptr) {
                cout << "Failed to load the image: " << path << endl;
                return;
            }
            glGenTextures(1, &mTexture);
            glBindTexture(GL_TEXTURE_2D, mTexture);
            cout << comp << endl;
            //if (comp == 3)
            //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
            //else
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            glBindTexture(GL_TEXTURE_2D, mTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


            stbi_image_free(image);

            mShader.setUniform("usingTex", true);
            cout << "loaded image: " << path  << ", with: w=" << w << ", h=" << h << endl;
        }

        ~MyGLCanvas() {
            mShader.free();
        }

        virtual void drawGL() override {
            mShader.bind();
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mTexture);

            glEnable(GL_DEPTH_TEST);
            mShader.drawIndexed(GL_TRIANGLES, 0, 2);
            glDisable(GL_DEPTH_TEST);
        }

    private:
        GLShader mShader;
        GLuint mTexture;
};

#endif  // SRC_INCLUDE_CANVAS_H_
