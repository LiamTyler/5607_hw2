#ifndef SRC_INCLUDE_LIGHTS_H_
#define SRC_INCLUDE_LIGHTS_H_

#include "include/utils.h"
#include "include/material.h"
#include "include/shapes.h"
#include <cmath>
#include <functional>

using std::max;
using std::pow;
using glm::dot;
using glm::length;
using glm::normalize;

class Light {
    public:
        Light() : Light(vec3(0,0,0)) {}
        Light(vec3 c) : color_(c) {}

        virtual vec3 ComputeLighting(vec3& v, vec3& p, vec3& n, Material* m,
                Intersection& inter, std::function<Shape*(Intersection&)> Intersect) = 0;
        void setColor(vec3 c) { color_ = c; }
        vec3 getColor() { return color_; }

    protected:
        vec3 color_;
};

class DirectionalLight : public Light {
    public:
        DirectionalLight() : DirectionalLight(vec3(0,0,0), vec3(0,0,0)) {}
        DirectionalLight(vec3 c, vec3 d) : Light(c), direction_(normalize(d)) {}
        virtual vec3 ComputeLighting(vec3& v, vec3& p, vec3& n, Material* m,
                Intersection& inter, std::function<Shape*(Intersection&)> Intersect) {
            vec3 l = -direction_;
            vec3 ret(0,0,0);
            Ray shadow(p + 0.01*l, l);
            Intersection i = inter;
            i.ray = shadow;
            Shape* shadow_obj = Intersect(i);
            if (shadow_obj)
                return ret;
            ret += color_*m->getDiffuse()*std::max(0.f, dot(n, l));
            ret += color_*m->getSpecular()*pow(max(0.f,
                        dot(v, glm::reflect(l,n))), m->getPower());
            return ret;
        }

        void setDirection(vec3 d) { direction_ = d; }
        vec3 getDirection() { return direction_; }

    private:
        vec3 direction_;
};

class PointLight : public Light {
    public:
        PointLight() : PointLight(vec3(0,0,0), vec3(0,0,0)) {}
        PointLight(vec3 c, vec3 p) : Light(c), position_(p) {}
        virtual vec3 ComputeLighting(vec3& v, vec3& p, vec3& n, Material* m,
                Intersection& inter, std::function<Shape*(Intersection&)> Intersect) {
            vec3 ret(0,0,0);
            vec3 l = position_ - p;
            float d = length(l);
            l = normalize(l);
            Ray shadow(p + 0.01*l, l);
            Intersection i = inter;
            i.ray = shadow;
            Shape* shadow_obj = Intersect(i);
            if (shadow_obj && length(i.ray.Evaluate() - p) < d)
                return ret;

            vec3 I = (1.0 / (d*d)) * color_;
            ret += I*m->getDiffuse()*max(0.f, dot(n, l));
            ret += I*m->getSpecular()*pow(max(0.f, dot(v, glm::reflect(l,n))), m->getPower());;
            return ret;
        }

        void setPosition(vec3 p) { position_ = p; }
        vec3 getPosition() { return position_; }

    private:
        vec3 position_;
};

class SpotLight : public Light {
    public:
        SpotLight() : SpotLight(vec3(0,0,0), vec3(0,0,0), vec3(0,0,0), 0, 0) {}
        SpotLight(vec3 c, vec3 p, vec3 d, float a1, float a2) :
            Light(c), position_(p), direction_(normalize(d)), angle1_(a1), angle2_(a2) {}

        virtual vec3 ComputeLighting(vec3& v, vec3& p, vec3& n, Material* m,
                Intersection& inter, std::function<Shape*(Intersection&)> Intersect) {

            vec3 ret(0,0,0);
            vec3 l = p - position_;
            float d = length(l);
            l = normalize(l);

            float angle = std::acos(dot(l, direction_)) * 180 / M_PI;
            if (angle > angle2_)
                return ret;
            vec3 I = (1.0/(d*d)) * color_; 
            if (angle >= angle1_) {
                angle = 1 - (angle - angle1_) / (angle2_ - angle1_);
                I = angle * I;
            }

            l = -l;
            // cast shadow ray
            Ray shadow(p + 0.01*l, l);
            Intersection i = inter;
            i.ray = shadow;
            Shape* shadow_obj = Intersect(i);
            if (shadow_obj && length(i.ray.Evaluate() - p) < d)
                return ret;

            ret += I*m->getDiffuse()*max(0.f, dot(n, l));
            ret += I*m->getSpecular()*pow(max(0.f, dot(v, glm::reflect(l,n))), m->getPower());
            return ret;
        }

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
        virtual vec3 ComputeLighting(vec3& v, vec3& p, vec3& n, Material* m,
                Intersection& inter, std::function<Shape*(Intersection&)> Intersect) {
            return m->getAmbient() * color_;
        }
};

#endif  // SRC_INCLUDE_LIGHTS_H_
