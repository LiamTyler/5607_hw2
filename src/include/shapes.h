#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/material.h"
#include "glm/glm.hpp"

using glm::vec3;

class Sphere {
    public:
        Sphere() : Sphere(vec3(1,1,1), 1, nullptr) {}
        Sphere(vec3 pos, float r, Material* m) {
            position_ = pos;
            radius_ = r;
            material_ = m;
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
