#ifndef SRC_INCLUDE_LIGHTS_H_
#define SRC_INCLUDE_LIGHTS_H_

#include "glm/glm.hpp"

using glm::vec3;

class Light {
    public:
        Light() : Light(vec3(0,0,0)) {}
        Light(vec3 c) : color_(c) {}

        void setColor(vec3 c) { color_ = c; }
        vec3 getColor() { return color_; }

    protected:
        vec3 color_;
};

class DirectionalLight : public Light {
    public:
        DirectionalLight() : DirectionalLight(vec3(0,0,0), vec3(0,0,0)) {}
        DirectionalLight(vec3 c, vec3 d) : Light(c), direction_(d) {}

        void setDirection(vec3 d) { direction_ = d; }
        vec3 getDirection() { return direction_; }

    private:
        vec3 direction_;
};

class PointLight : public Light {
    public:
        PointLight() : PointLight(vec3(0,0,0), vec3(0,0,0)) {}
        PointLight(vec3 c, vec3 p) : Light(c), position_(p) {}

        void setPosition(vec3 p) { position_ = p; }
        vec3 getPosition() { return position_; }

    private:
        vec3 position_;
};

class SpotLight : public Light {
    public:
        SpotLight() : SpotLight(vec3(0,0,0), vec3(0,0,0), vec3(0,0,0), 0, 0) {}
        SpotLight(vec3 c, vec3 p, vec3 d, float a1, float a2) :
            Light(c), position_(p), direction_(d), angle1_(a1), angle2_(a2) {}

        void setDirection(vec3 d) { direction_ = d; }
        vec3 getDirection() { return direction_; }
        void setPosition(vec3 p) { position_ = p; }
        vec3 getPosition() { return position_; }
        void setAngle1(float a) { angle1_ = a; }
        void setAngle2(float a) { angle2_ = a; }
        float getAngle1() { return angle1_; }
        float getAngle2() { return angle2_; }

    private:
        vec3 position_;
        vec3 direction_;
        float angle1_;
        float angle2_;
};

class AmbientLight : public Light {
    public:
        AmbientLight() : AmbientLight(vec3(0,0,0)) {}
        AmbientLight(vec3 c) : Light(c) {}
};

#endif  // SRC_INCLUDE_LIGHTS_H_
