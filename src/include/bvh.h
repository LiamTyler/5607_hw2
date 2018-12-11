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
        BVH* getLeft() { return left_; }
        BVH* getRight() { return right_; }
        int getNumShapes() { return shapes_.size(); }
        void getBB(vec3& mi, vec3& ma) { mi = min_; ma = max_; }
        void PrintTree(int curr_depth, int max_depth);
        int count();

        BVH* left_;
        BVH* right_;
        vector<Shape*> shapes_;
        glm::vec3 min_;
        glm::vec3 max_;
};

#endif  // SRC_INCLUDE_BVH_H_

