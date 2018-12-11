#include "include/bvh2.h"
#include <functional>
#include <iostream>

using namespace std;

BVH2::BVH2() {
    left_ = 0;
    right_ = 0;
    min_ = vec3(0,0,0);
    max_ = vec3(0,0,0);
}


Shape* IntersectBVH2(BVH2* arr, Intersection& inter) {
    Shape* closest_shape = nullptr;
    Intersection closest;
    int stack[64];
    int idx = 0;
    stack[idx++] = 0;
    const auto& shapes = *inter.shapes;

    while (idx) {
        int i = stack[--idx];

        auto node = arr[i];
        if (!node.RayHitsBB(inter.ray))
            continue;

        // if not a leaf node
        if (!node.isLeaf()) {
            if (node.left_)
                stack[idx++] = node.left_;
            if (node.right_)
                stack[idx++] = node.right_;
        } else { // if leaf
            if (shapes[node.left_]->Intersect(inter)) { 
                if (inter.ray.tmin < closest.ray.tmin || !closest_shape) {
                    closest = inter;
                    closest_shape = shapes[node.left_];
                }
            }
            if (node.getNumShapes() == 2) {
                if (shapes[node.right_]->Intersect(inter)) { 
                    if (inter.ray.tmin < closest.ray.tmin || !closest_shape) {
                        closest = inter;
                        closest_shape = shapes[node.right_];
                    }
                }
            }
        }
    }

    inter = closest;
    return closest_shape;

    /*
    // cout << "ray hit bb" << endl;
    if (shapes_.size() != 0) {
        Intersection closest;
        Shape* hit_obj = nullptr;
        // Loop over every sphere in node
        for (int i = 0; i < shapes_.size(); i++) {
            if (shapes_[i]->Intersect(inter)) { 
                if (hit_obj) {
                    // Record if object is the closest so far
                    if (inter.ray.tmin < closest.ray.tmin) {
                        closest = inter;
                        hit_obj = shapes_[i];
                    }
                } else {
                    closest = inter;
                    hit_obj = shapes_[i];
                }
            }
        }
        if (hit_obj) {
            inter = closest;
        }
        return hit_obj;
    } else {
        Intersection l = inter;
        Intersection r = inter;
        Shape* ls = nullptr; 
        Shape* rs = nullptr; 
        if (left_)
            ls = left_->Intersect(l);
        if (right_)
            rs = right_->Intersect(r);
        if (ls) {
            if (rs) {
                if (l.ray.tmin < r.ray.tmin) {
                    inter = l;
                    return ls;
                } else {
                    inter = r;
                    return rs;
                }
            } else {
                inter = l;
                return ls;
            }
        } else {
            inter = r;
            return rs;
        }
    }
    */
    return nullptr;
}

// Intersection code. NOTE!!! Used scratchapixel as a reference for
// this intersection code / math since I kept screwing it up on my own
bool BVH2::RayHitsBB(Ray& ray) {
    vec3 p = ray.p;
    vec3 d = ray.dir;
    float tmin = (min_.x - p.x) / d.x;
    float tmax = (max_.x - p.x) / d.x;
    if (tmin > tmax)
        swap(tmin, tmax);
    float tymin = (min_.y - p.y) / d.y;
    float tymax = (max_.y - p.y) / d.y;
    if (tymin > tymax)
        swap(tymin, tymax);
    if ((tmin > tymax) || (tymin > tmax))
        return false;

    tmin = max(tymin, tmin);
    tmax = min(tymax, tmax);

    float tzmin = (min_.z - p.z) / d.z;
    float tzmax = (max_.z - p.z) / d.z;
    if (tzmin > tzmax)
        swap(tzmin, tzmax);
    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    // tmin = max(tzmin, tmin);
    // tmax = min(tzmax, tmax);
    // ray.tmin = tmin;
    // ray.tmax = tmax;
    return true;
}

BVH2::BVH2(BVH* node) {
    node->getBB(min_, max_);
    numShapes_ = node->getNumShapes();
    left_ = 0;
    right_ = 0;
}

int insertChildren(BVH2* arr, BVH* node, const std::vector<Shape*>& shapes, int parent, int size) {
    BVH* l = node->getLeft(), *r = node->getRight();
    int numS = node->getNumShapes();

    if (numS) {
        if (numS > 2) {
            std::cout << "more than 2 shapes in this bin, error" << std::endl;
            exit(EXIT_FAILURE);
        }
        auto v = std::find(shapes.begin(), shapes.end(), node->shapes_[0]);
        if (v == shapes.end()) {
            std::cout << "could not find this node's shape inside the array" << std::endl;
            exit(EXIT_FAILURE);
        } else {
            arr[parent].left_ = v - shapes.begin();
        }

        if (numS == 2) {
            v = std::find(shapes.begin(), shapes.end(), node->shapes_[1]);
            if (v == shapes.end()) {
                std::cout << "could not find this node's shape inside the array" << std::endl;
                exit(EXIT_FAILURE);
            } else {
                arr[parent].right_ = v - shapes.begin();
            }
        }

        return size;
    }

    if (l) {
        arr[parent].left_ = size;
        arr[size++] = BVH2(l);
    }
    if (r) {
        arr[parent].right_ = size;
        arr[size++] = BVH2(r);
    }

    if (l)
        size = insertChildren(arr, l, shapes, arr[parent].left_, size);
    if (r)
        size = insertChildren(arr, r, shapes, arr[parent].right_, size);

    return size;
}

BVH2* CreateFromBVH(BVH* root, const std::vector<Shape*>& shapes) {
    int nodes = root->count();
    std::cout << "nodes: " << nodes << std::endl;
    BVH2* list = new BVH2[nodes];
    list[0] = BVH2(root);
    int inserted = insertChildren(list, root, shapes, 0, 1);
    std::cout << "inserted nodes: " << inserted << std::endl;

    return list;
}
