#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/material.h"
#include "glm/glm.hpp"
#include <cmath>
#include <algorithm>

using glm::vec3;

class Sphere {
    public:
        Sphere() : Sphere(vec3(1,1,1), 1, nullptr) {}
        Sphere(vec3 pos, float r, Material* m) {
            position_ = pos;
            radius_ = r;
            material_ = m;
        }

        bool Hit(vec3 p, vec3 ray, float& t) {
            float t0 = 1, t1 = 1;
            vec3 g = p - position_;
            float b = 2*glm::dot(ray, g);
            float c = glm::dot(g,g) - radius_*radius_;
            float disc = b*b - 4*c;
            if (disc < 0) {
                return false;
            }
            t0 = (-b + std::sqrt(disc)) / (2.0);
            t1 = (-b - std::sqrt(disc)) / (2.0);
            t = std::min(t0, t1);
            return true;
        }

        void setPosition(vec3 p) { position_ = p; }
        void setRadius(float r) { radius_ = r; }
        void setMaterial(Material *m) { material_ = m; }

        vec3 getPosition() { return position_; }
        float getRadius() { return radius_; }
        Material* getMaterial() { return material_; }

    protected:
        vec3 position_;
        float radius_;
        Material* material_;
};

#endif  // SRC_INCLUDE_SHAPES_H_
