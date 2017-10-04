#ifndef SRC_INCLUDE_CAMERA_H_
#define SRC_INCLUDE_CAMERA_H_

#include <iostream>
#include "glm/glm.hpp"
using glm::vec3;

class Camera {
    public:
        Camera();
        void Init();

        // setters
        void setCameraPos(vec3 v) { camera_pos_ = v; }
        void setCameraDir(vec3 v) { camera_dir_ = v; }
        void setCameraUp(vec3 v) { camera_up_ = v; }
        void setHalfAngle(float a) { ha_ = a; }
        void setWidth(int w) { width_ = w; }
        void setHeight(int h) { height_ = h; }
        void setOutputImage(std::string s) { output_image_ = s; }

        // getters
        vec3 getCameraPos() { return camera_pos_; }
        vec3 getCameraDir() { return camera_dir_; }
        vec3 getCameraUp() { return camera_up_; }
        float getHalfAngle() { return ha_; }
        int getWidth() { return width_; }
        int getHeight() { return height_; }
        std::string getOutputImage() { return output_image_; }

    protected:
        vec3 camera_pos_;
        vec3 camera_dir_;
        vec3 camera_up_;
        float ha_;
        int width_;
        int height_;
        std::string output_image_;
};

#endif  // SRC_INCLUDE_CAMERA_H_

