#ifndef SRC_INCLUDE_RAY_H_
#define SRC_INCLUDE_RAY_H_

#include <iostream>
#include "include/utils.h"

class Ray {
    public:
        Ray() : Ray(vec3(0,0,0), vec3(0,0,0)) {}
        Ray(vec3 pos, vec3 d) : p(pos), dir(glm::normalize(d)), tmin(-1), tmax(-1) {}

        vec3 Evaluate() {
            return p + tmin * dir;
        }
        vec3 Evaluate(float t) {
            return p + t * dir;
        }


        vec3 p;
        vec3 dir;

        float tmin;
        float tmax;
};

#endif  // SRC_INCLUDE_RAY_H_
