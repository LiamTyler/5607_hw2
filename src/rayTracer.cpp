#include "include/rayTracer.h"
#include "include/ray.h"
#include <stdlib.h>
#include <functional>

using std::cout;
using std::endl;
using std::string;
using std::vector;
using glm::vec3;
using glm::vec4;
using glm::normalize;
using glm::dot;
using glm::cross;
using glm::reflect;
using glm::length;

using namespace std::placeholders;

RayTracer::RayTracer() {
    parser_ = nullptr;
    camera_ = nullptr;
    background_ = vec4(0,0,0,1);
    max_depth_ = 1;
}

RayTracer::~RayTracer() {
    if (parser_)
        delete parser_;
    if (image_)
        delete image_;
    if (camera_)
        delete camera_;
    for (int i = 0; i < lights_.size(); i++)
        delete lights_[i];
    for (int i = 0; i < shapes_.size(); i++)
        delete shapes_[i];
    for (int i = 0; i < materials_.size(); i++)
        delete materials_[i];
}

void RayTracer::Parse(string filename) {
    parser_ = new Parser(filename);
    parser_->Parse();
    camera_ = parser_->getCamera();

    lights_.push_back(parser_->getAmbientLight());

    vector<DirectionalLight*> dl = parser_->getDirectionalLights();
    for (int i = 0; i < dl.size(); i++)
        lights_.push_back(dl[i]);

    vector<PointLight*> pl = parser_->getPointLights();
    for (int i = 0; i < pl.size(); i++)
        lights_.push_back(pl[i]);

    vector<SpotLight*> sl = parser_->getSpotLights();
    for (int i = 0; i < sl.size(); i++)
        lights_.push_back(sl[i]);

    vector<Sphere*> s = parser_->getSpheres();
    for (int i = 0; i < s.size(); i++)
        shapes_.push_back(s[i]);

    materials_ = parser_->getMaterials();
    background_ = vec4(parser_->getBackground(), 1);
    max_depth_ = parser_->getMaxDepth();
    sampling_method_ = parser_->getSamplingMethod();
}

vec4 RayTracer::ComputeLighting(Shape* hit_obj, Ray& ray) {
    Material * m = hit_obj->getMaterial();
    vec3 v = normalize(ray.dir);
    vec3 p = ray.Evaluate();
    vec3 n = hit_obj->getNormal(p);

    vec3 color = vec3(0,0,0);
    auto fp = std::bind(&RayTracer::Intersect, this, _1);

    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end(); ++it) {
        color += (*it)->ComputeLighting(v, p, n, m, fp);
    }
    color.x = std::min(1.0f, std::max(0.f, color.x));
    color.y = std::min(1.0f, std::max(0.f, color.y));
    color.z = std::min(1.0f, std::max(0.f, color.z));
    return vec4(color, 1);
}


Shape* RayTracer::Intersect(Ray& ray) {
    Ray closest;
    Shape* hit_obj = nullptr;
    for (int i = 0; i < shapes_.size(); i++) {
        if (shapes_[i]->Intersect(ray)) { 
            if (hit_obj) {
                if (ray.tmin < closest.tmin) {
                    closest = ray;
                    hit_obj = shapes_[i];
                }
            } else {
                closest = ray;
                hit_obj = shapes_[i];
            }
        }
    }
    if (hit_obj) {
        ray = closest;
    }
    return hit_obj;
}

vec4 RayTracer::TraceRay(Ray& ray) {
    Shape* hit_obj = Intersect(ray);

    if (hit_obj) {
        return ComputeLighting(hit_obj, ray);
    } else {
        return background_;
    }
}

void RayTracer::Run(StatusReporter* statusReporter) {
    // Get screen dimensions, construct image
    int width = camera_->getWidth();
    int height = camera_->getHeight();
    image_ = new Image(width, height);

    // Get camera info
    vec3 pos = camera_->getPos();
    vec3 dir = normalize(camera_->getDir());
    vec3 up = normalize(camera_->getUp());
    float half_angle = camera_->getHalfAngle() * M_PI / 180.0;

    // Calculate position of screen corner,
    // and delta vectors to iterate over the screen
    float d = height / (2.0 * tan(half_angle));
    vec3 dx = normalize(cross(up, dir));
    vec3 dy = -up;
    vec3 ul = pos + d * dir + up * (height / 2.0) - (width / 2.0) * dx;

    // Get which 
    auto Sample = &RayTracer::BasicSample;
    if (sampling_method_ == SUPER)
        Sample = &RayTracer::SuperSample;
    else if (sampling_method_ == ADAPTIVE)
        Sample = &RayTracer::AdaptiveSample;

    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            vec3 p = py + c*dx;
            vec4 color = (this->*Sample)(pos, p, dx, dy);
            image_->SetPixel(r, c, color);
        }
        if (statusReporter) {
            statusReporter->setValue(r / (float) height);
        }
    }

    image_->Write(camera_->getOutputImage());
}

vec4 RayTracer::BasicSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec3 dir = normalize(p - pos);
    Ray ray(pos, dir);
    return TraceRay(ray);
}

vec4 RayTracer::SuperSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec4 newColor(0,0,0,0);
    int w = 3;
    int h = 3; 
    vec3 dir;
    Ray ray;
    vec3 dx2 = (1.0/w)*dx;
    vec3 dy2 = (1.0/h)*dy;
    vec3 ulp = p - .5*dx - .5*dy;
    ulp += .5*dx2 + .5*dy2;
    for (int r2 = 0; r2 < h; r2++) {
        vec3 py2 = ulp + r2*dy2;
        for (int c2 = 0; c2 < w; c2++) {
            p = py2 + c2*dx2;
            dir = normalize(p - pos);
            ray = Ray(pos, dir);
            newColor += TraceRay(ray);
        }
    }
    newColor *= (1.0/(w*h+1));
    return newColor;
}

vec4 RayTracer::AdaptiveSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec3 dir = normalize(p - pos);
    Ray ray(pos,dir);
    vec4 newColor = TraceRay(ray);
    vec4 deltaColor(0,0,0,0);
    vec4 prevColor = newColor;
    vec3 dx2 = dx;
    vec3 dy2 = dy;
    vec3 ulp = p - .5*dx - .5*dy;
    for (int r2 = 0; r2 < 2; r2++) {
        vec3 py2 = ulp + r2*dy2;
        for (int c2 = 0; c2 < 2; c2++) {
            vec3 p2 = py2 + c2*dx2;
            dir = normalize(p2 - pos);
            ray = Ray(pos, dir);
            vec4 tmp = TraceRay(ray);
            deltaColor += tmp - prevColor;
            prevColor = tmp;
            newColor += tmp;
        }
    }
    if (length(deltaColor) > 1) {
        dx2 = dx / 2;
        dy2 = dy / 2;
        ulp = p - .5*dx2 - .5*dy2;
        for (int r2 = 0; r2 < 2; r2++) {
            vec3 py2 = ulp + r2*dy2;
            for (int c2 = 0; c2 < 2; c2++) {
                p = py2 + c2*dx2;
                dir = normalize(p - pos);
                ray = Ray(pos, dir);
                vec4 tmp = TraceRay(ray);
                newColor += tmp;
            }
        }
        newColor /= 9.0;
    } else {
        newColor /= 5.0;
    }
    return newColor;
}
