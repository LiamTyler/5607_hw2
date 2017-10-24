#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/material.h"
#include "include/ray.h"
#include "glm/glm.hpp"
#include <cmath>
#include <algorithm>
#include <vector>
#include <iostream>

using glm::vec3;
using glm::dot;
using glm::length;
using glm::cross;
using std::vector;
using std::endl;
using std::cout;

typedef struct {
    vector<vec3>* verts;
    vector<vec3>* norms;
    vec3 cameraDir;
    Ray ray;
    float u, v;
} Intersection;

class Shape {
    public:
        Shape() : Shape(nullptr) {}
        Shape(Material *m) : material_(m) {}
        virtual bool Intersect(Intersection& inter) = 0;
        virtual vec3 getNormal(vec3& p, Intersection& inter) = 0;
        void setMaterial(Material *m) { material_ = m; }
        Material* getMaterial() { return material_; }

    protected:
        Material * material_;
};

class Triangle : public Shape {
    public:
        Triangle() : Shape(nullptr), v1_(0), v2_(0), v3_(0) {}
        Triangle(Material* m, int v1, int v2, int v3) : Shape(m) {
            v1_ = v1;
            v2_ = v2;
            v3_ = v3;
        }
        int getVertex1() { return v1_; }
        int getVertex2() { return v2_; }
        int getVertex3() { return v3_; }
        virtual vec3 getNormal(vec3& p, Intersection& inter) {
            vec3 v1 = (*inter.verts)[v1_];
            vec3 v2 = (*inter.verts)[v2_];
            vec3 v3 = (*inter.verts)[v3_];
            vec3 e12 = v2 - v1;
            vec3 e13 = v3 - v1;
            vec3 N = normalize(cross(e12, e13));

            if (dot(inter.cameraDir, N) < 0)
                return N;
            else
                return -N;
        }
        virtual bool Intersect(Intersection& inter) {
            // get vertices and normals
            vec3 v1 = (*inter.verts)[v1_];
            vec3 v2 = (*inter.verts)[v2_];
            vec3 v3 = (*inter.verts)[v3_];

            // get edges and vectors for barycentric coordinates
            vec3 e12 = v2 - v1;
            vec3 e23 = v3 - v2;
            vec3 e31 = v1 - v3;
            vec3 v12 = e12;
            vec3 v13 = v3 - v1;

            vec3 N = cross(v12, v13);
            
            float d = dot(v1, N);
            float t = -(dot(inter.ray.p, N) - d) / dot(inter.ray.dir, N);
            // check if triangle is behind the ray's origin
            if (t < 0)
                return false;
            inter.ray.tmin = t;

            vec3 P = inter.ray.Evaluate();
            float area = length(N) / 2;
            
            vec3 vp;
            vec3 vpCross;
            // check to see if the point is on the correct side of all 3 edges
            // v1 to v2
            vp = P - v1;
            vpCross = cross(e12, vp);
            if (dot(vpCross, N) < 0)
                return false;

            // v2 to v3
            vp = P - v2;
            vpCross = cross(e23, vp);
            if (dot(vpCross, N) < 0)
                return false;
            inter.u = (length(vpCross) / 2) / area;

            // v3 to v1
            vp = P - v3;
            vpCross = cross(e31, vp);
            if (dot(vpCross, N) < 0)
                return false;
            inter.v = (length(vpCross) / 2) / area;

            return true;
        }

    protected:
        int v1_;
        int v2_;
        int v3_;
};

class NormalTriangle : public Triangle {
    public:
        NormalTriangle() : Triangle(), n1_(0), n2_(0), n3_(0) {}
        NormalTriangle(Material* m, int v1, int v2, int v3, int n1, int n2, int n3) :
            Triangle(m, v1, v2, v3) {
            n1_ = n1; n2_ = n2; n3_ = n3;
        }
        int getNormal1() { return n1_; }
        int getNormal2() { return n2_; }
        int getNormal3() { return n3_; }
        virtual vec3 getNormal(vec3& p, Intersection& inter) {
            vec3 v1 = (*inter.verts)[v1_];
            vec3 v2 = (*inter.verts)[v2_];
            vec3 v3 = (*inter.verts)[v3_];
            vec3 n1 = (*inter.norms)[n1_];
            vec3 n2 = (*inter.norms)[n2_];
            vec3 n3 = (*inter.norms)[n3_];
            float u = inter.u, v = inter.v;

            return normalize(n1 * u + n2 * v + n3 * (1 - u - v));
            // return normalize(n1 (1- u - v) + n2 * u + n3 * v);
        }

    protected:
        int n1_;
        int n2_;
        int n3_;
};


class Sphere : public Shape {
    public:
        Sphere() : Sphere(vec3(1,1,1), 1, nullptr) {}
        Sphere(vec3 pos, float r, Material* m)  : Shape(m) {
            position_ = pos;
            radius_ = r;
        }

        bool Intersect(Intersection& inter) {
            float t0 = -1, t1 = -1;
            vec3 OC = inter.ray.p - position_;
            float b = 2*glm::dot(inter.ray.dir, OC);
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
            inter.ray.tmin = std::min(t0, t1);
            inter.ray.tmax = std::max(t0, t1);
            return true;
        }

        virtual vec3 getNormal(vec3& pos, Intersection& inter) {
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
