#include "include/camera.h"

Camera::Camera() {
    Init();
}

void Camera::Init() {
    camera_pos_ = vec3(0, 0, 0);
    camera_dir_ = vec3(0, 0, 1);
    camera_up_ =  vec3(0, 1, 0);
    width_ = 640;
    height_ = 480;
    ha_ = 45;
    output_image_ = "raytraced.bmp";
}
