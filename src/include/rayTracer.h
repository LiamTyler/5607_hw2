#ifndef SRC_INCLUDE_RAYTRACER_H_
#define SRC_INCLUDE_RAYTRACER_H__

#include <iostream>
#include <vector>
#include "include/utils.h"
#include "include/camera.h"
#include "include/lights.h"
#include "include/material.h"
#include "include/shapes.h"
#include "include/parser.h"
#include "include/image.h"

using std::string;
using glm::vec3;
using std::vector;

class RayTracer {
    public:
        RayTracer();
        ~RayTracer();
        void Parse(string filename);
        void Trace();
        vec4 GetColor(Shape* hit_obj, Ray ray);
        Shape* Intersect(Ray &ray);

    private:
        Image* image_;
        Parser* parser_;

        Camera * camera_;
        vector<DirectionalLight *> directional_lights_;
        vector<PointLight *> point_lights_;
        vector<SpotLight *> spot_lights_;
        AmbientLight* ambient_light_;
        
        vector<Material*> materials_;
        vector<Sphere*> spheres_;

        vec4 background_;
        int max_depth_;
};

#endif  // SRC_INCLUDE_RAYTRACER_H_
