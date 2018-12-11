#pragma once

#include <iostream>
#include <vector>
#include "include/shapes.h"
#include "glm/glm.hpp"
#include "include/bvh.h"

class BVH2 {
    public:
        BVH2();
        BVH2(BVH* node);
        bool RayHitsBB(Ray& ray);
        int getLeft() { return left_; }
        int getRight() { return right_; }
        int getNumShapes() { return numShapes_; }
        bool isLeaf() { return numShapes_ != 0; }
        void getBB(vec3& mi, vec3& ma) { mi = min_; ma = max_; }

        int numShapes_;
        int left_;
        int right_;
        glm::vec3 min_;
        glm::vec3 max_;
};

BVH2* CreateFromBVH(BVH* root, const std::vector<Shape*>& shapes);

Shape* IntersectBVH2(BVH2* arr, Intersection& inter);
