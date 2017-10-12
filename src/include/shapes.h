#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/material.h"
#include "include/ray.h"
#include "glm/glm.hpp"
#include <cmath>
#include <algorithm>

using glm::vec3;

class Shape {
    public:
        Shape() : Shape(nullptr) {}
        Shape(Material *m) : material_(m) {}
        virtual bool Intersect(Ray& ray) = 0;
        virtual vec3 getNormal(vec3 p) = 0;
        void setMaterial(Material *m) { material_ = m; }
        Material* getMaterial() { return material_; }

    protected:
        Material * material_;
};

class Sphere : public Shape {
    public:
        Sphere() : Sphere(vec3(1,1,1), 1, nullptr) {}
        Sphere(vec3 pos, float r, Material* m)  : Shape(m) {
            position_ = pos;
            radius_ = r;
        }

        bool Intersect(Ray& ray) {
            float t0 = -1, t1 = -1;
            vec3 OC = ray.p - position_;
            float b = 2*glm::dot(ray.dir, OC);
            float c = glm::dot(OC,OC) - radius_*radius_;
            float disc = b*b - 4*c;
            if (disc < 0) {
                return false;
            }
            t0 = (-b + std::sqrt(disc)) / (2.0);
            t1 = (-b - std::sqrt(disc)) / (2.0);
            if (t0 > t1) {
                float tmp = t0;
                t0 = t1;
                t1 = t0;
            }
            if (t0 < 0) {
                t0 = t1;
                if (t0 < 0)
                    return false;
            }
            ray.tmin = std::min(t0, t1);
            ray.tmax = std::max(t0, t1);
            return true;
        }

        vec3 getNormal(vec3 pos) {
            return glm::normalize(pos - position_);
        }

        void setPosition(vec3 p) { position_ = p; }
        void setRadius(float r) { radius_ = r; }

        vec3 getPosition() { return position_; }
        float getRadius() { return radius_; }

    protected:
        vec3 position_;
        float radius_;
};

#endif  // SRC_INCLUDE_SHAPES_H_
