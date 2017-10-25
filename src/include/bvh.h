#ifndef SRC_INCLUDE_BVH_
#define SRC_INCLUDE_BVH_

#include <iostream>
#include <vector>
#include "include/shapes.h"
#include "glm/glm.hpp"

class BVH {
    public:
        BVH();
        void Partition(const std::vector<Shape*>& shapes);
        Shape* Intersect(Intersection& inter);
        bool RayHitsBB(Ray& ray);

    private:
        BVH* left_;
        BVH* right_;
        vector<Shape*> shapes_;
        glm::vec3 min_;
        glm::vec3 max_;
};

#endif  // SRC_INCLUDE_BVH_H_

