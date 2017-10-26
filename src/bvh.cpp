#include "include/bvh.h"
#include <functional>
#include <iostream>

using namespace std;

BVH::BVH() {
    left_ = nullptr;
    right_ = nullptr;
    min_ = vec3(0,0,0);
    max_ = vec3(0,0,0);
}


Shape* BVH::Intersect(Intersection& inter) {
    if (!RayHitsBB(inter.ray))
        return nullptr;
    // cout << "Intersecting shapes!" << endl;

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
}

// Intersection code. NOTE!!! Used scratchapixel as a reference for
// this intersection code / math since I kept screwing it up on my own
bool BVH::RayHitsBB(Ray& ray) {
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

void BVH::Partition(const vector<Shape*>& shapes) {
    if (shapes.size() == 0) {
        cout << "ERROR, TRYING TO PARTITION 0 SHAPES" << endl;
        return;
    }

    // find the bounding box for all shapes combined
    vec3 curr_min, curr_max;
    float minX, maxX, minY, maxY, minZ, maxZ;
    shapes[0]->GetBB(curr_min, curr_max);
    minX = curr_min.x; maxX = curr_max.x;
    minY = curr_min.y; maxY = curr_max.y;
    minZ = curr_min.z; maxZ = curr_max.z;
    vec3 minC = shapes[0]->getCenter();
    vec3 maxC = shapes[0]->getCenter();

    int i = 0;
    for (vector<Shape*>::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        // get bounding box for shape
        (*it)->GetBB(curr_min, curr_max);
        minX = min(minX, curr_min.x);
        minY = min(minY, curr_min.y);
        minZ = min(minZ, curr_min.z);
        maxX = max(maxX, curr_max.x);
        maxY = max(maxY, curr_max.y);
        maxZ = max(maxZ, curr_max.z);

        // find the min / max components of all of the shape centers
        vec3 c = (*it)->getCenter();
        minC.x = min(minC.x, c.x);
        minC.y = min(minC.y, c.y);
        minC.z = min(minC.z, c.z);
        maxC.x = max(maxC.x, c.x);
        maxC.y = max(maxC.y, c.y);
        maxC.z = max(maxC.z, c.z);

    }
    min_ = vec3(minX, minY, minZ);
    max_ = vec3(maxX, maxY, maxZ);

    vec3 mid = (minC + maxC) / 2;
    vec3 d = maxC - minC;
    function<bool(const vec3&, const vec3&)> cmp;
    // split on largest axis
    if (d.x >= d.y && d.x >= d.z) {
        cmp = [&](vec3 mid, vec3 center) { return mid.x >= center.x; };
    } else if (d.y >= d.x && d.y >= d.z) {
        cmp = [&](vec3 mid, vec3 center) { return mid.y >= center.y; };
    } else {
        cmp = [&](vec3 mid, vec3 center) { return mid.z >= center.z; };
    }

    vector<Shape*> left;
    vector<Shape*> right;
    i = 0;
    for (vector<Shape*>::const_iterator it = shapes.begin(); it != shapes.end(); ++it) {
        if (cmp(mid, (*it)->getCenter()))
            left.push_back(*it);
        else
            right.push_back(*it);
    }
    if (left.size() == 0 || right.size() == 0) {
        shapes_ = shapes;
    } else {
        left_ = new BVH;
        right_ = new BVH;
        left_->Partition(left);
        right_->Partition(right);
    }
}

void BVH::PrintTree(int curr_depth, int max_depth) {
    if (curr_depth > max_depth)
        return;
    string t(curr_depth, '\t');
    cout << t << "num shapes: " << shapes_.size() << endl;
    cout << t << "min: " << min_ << endl;
    cout << t << "max: " << max_ << endl;
    curr_depth++;
    if (curr_depth <= max_depth) {
        if (left_) {
            cout << t << "left:" << endl;
            left_->PrintTree(curr_depth, max_depth);
        } if (right_) {
            cout << t << "right:" << endl;
            right_->PrintTree(curr_depth, max_depth);
        }
    }
}

