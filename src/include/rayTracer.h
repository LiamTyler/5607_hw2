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
#include "include/statusReporter.h"

using std::string;
using glm::vec3;
using std::vector;

class RayTracer {
    public:
        RayTracer();
        ~RayTracer();
        void Parse(string filename);
        void Run(StatusReporter* statusReporter);
        void Run() { Run(nullptr); }
        vec4 TraceRay(Ray& ray, int depth);
        vec4 ComputeLighting(Shape* hit_obj, Ray& ray, int depth);
        Shape* Intersect(Ray &ray);
        vec4 BasicSample(vec3& pos, vec3& p, vec3& dx, vec3& dy);
        vec4 SuperSample(vec3& pos, vec3& p, vec3& dx, vec3& dy);
        vec4 AdaptiveSample(vec3& pos, vec3& p, vec3& dx, vec3& dy);
        string getOutputFileName() { return camera_->getOutputImage(); }

    private:
        Image* image_;
        Parser* parser_;

        Camera * camera_;
        vector<Light*> lights_;
        vector<Shape*> shapes_;
        
        vector<Material*> materials_;

        vec4 background_;
        int max_depth_;
        SamplingMethod sampling_method_;
};

#endif  // SRC_INCLUDE_RAYTRACER_H_
