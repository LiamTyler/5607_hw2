#ifndef SRC_INCLUDE_SHAPES_H_
#define SRC_INCLUDE_SHAPES_H_

#include "include/material.h"
#include "include/ray.h"
#include "glm/glm.hpp"
#include <cmath>
#include <algorithm>
#include <vector>

using glm::vec3;
using glm::dot;
using glm::length;
using glm::cross;
using std::vector;

typedef struct {
    vector<vec3>& verts;
    vector<vec3>& norms;
    Ray & ray;
    float u, v;
} Intersection;

class Shape {
    public:
        Shape() : Shape(nullptr) {}
        Shape(Material *m) : material_(m) {}
        virtual bool Intersect(Ray& ray) { return false; }
        virtual vec3 getNormal(vec3 p) { return vec3(0,0,0); }
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
        /*
        virtual vec3 getNormal1(vector<vec3>& vertices) {
            vec3 v13 = vertices[v3] - 
            return glm::cross(
        */
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
        bool Intersect(vector<vec3>& verts, vector<vec3>& norms, Ray& ray, float& u, float& v) {
            // get vertices and normals
            vec3 v1 = verts[v1_];
            vec3 v2 = verts[v2_];
            vec3 v3 = verts[v3_];
            vec3 n1 = norms[n1_];
            vec3 n2 = norms[n2_];
            vec3 n3 = norms[n3_];

            // get edges and vectors for barycentric coordinates
            vec3 e12 = v2 - v1;
            vec3 e23 = v3 - v2;
            vec3 e31 = v1 - v3;
            vec3 v12 = e12;
            vec3 v13 = v3 - v1;

            vec3 N = cross(v12, v13);
            
            float d = dot(v1, N);
            float t = -(dot(ray.p, N) + d) / dot(ray.dir, N);
            // check if triangle is behind the ray's origin
            if (t < 0)
                return false;
            ray.tmin = t;

            vec3 P = ray.Evaluate();
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

            // v3 to v1
            vp = P - v3;
            vpCross = cross(e31, vp);
            if (dot(vpCross, N) < 0)
                return false;

            return true;
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
