#include "include/rayTracer.h"
#include "include/ray.h"
#include <stdlib.h>

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
    if (ambient_light_)
        delete ambient_light_;
    for (int i = 0; i < directional_lights_.size(); i++)
        delete directional_lights_[i];
    for (int i = 0; i < point_lights_.size(); i++)
        delete point_lights_[i];
    for (int i = 0; i < spot_lights_.size(); i++)
        delete spot_lights_[i];
    for (int i = 0; i < spheres_.size(); i++)
        delete spheres_[i];
    for (int i = 0; i < materials_.size(); i++)
        delete materials_[i];
}

void RayTracer::Parse(string filename) {
    parser_ = new Parser(filename);
    parser_->Parse();
    camera_ = parser_->getCamera();

    directional_lights_ = parser_->getDirectionalLights();
    point_lights_ = parser_->getPointLights();
    spot_lights_ = parser_->getSpotLights();
    ambient_light_ = parser_->getAmbientLight();
    spheres_ = parser_->getSpheres();

    materials_ = parser_->getMaterials();

    background_ = vec4(parser_->getBackground(), 1);
    max_depth_ = parser_->getMaxDepth();
}

vec4 RayTracer::GetColor(Shape* hit_obj, Ray ray) {
    // Get material Colors and properties
    Material * m = hit_obj->getMaterial();
    vec3 Ka = m->getAmbient();
    vec3 Kd = m->getDiffuse();
    vec3 Ks = m->getSpecular();
    float s_power = m->getPower();

    // calculate vectors: view, normal, and reflected
    vec3 v = normalize(ray.dir);
    vec3 p = ray.Evaluate();
    vec3 n = hit_obj->getNormal(p);
    vec3 r = reflect(v,n);

    // color (starts as black)
    vec3 color = vec3(0,0,0);
    // ambient
    color += Ka * ambient_light_->getColor();

    // loop through every point light
    for (int i = 0; i < point_lights_.size(); i++) {
        // update light vector
        PointLight *light = point_lights_[i];
        vec3 l = light->getPosition() - p;
        float d = glm::length(l);
        l = normalize(l);
        vec3 I = (1.0 / (d*d)) * light->getColor();

        // cast shadow ray
        Ray shadow(p + 0.01*l, l);
        Shape *shadow_obj = Intersect(shadow);
        if (shadow_obj && length(shadow.Evaluate() - p) < d)
            continue;

        // diffuse
        color += I*Kd*std::max(0.f, dot(n, l));
        // specular
        color += I*Ks*std::pow(std::max(0.f, dot(v, reflect(l,n))), s_power);
    }

    // loop through every directional light
    for (int i = 0; i < directional_lights_.size(); i++) {
        // update light vector
        DirectionalLight *light = directional_lights_[i];
        vec3 l = normalize(-light->getDirection());
        vec3 lcolor = light->getColor();

        // cast shadow ray
        Ray shadow(p + 0.01*l, l);
        Shape *shadow_obj = Intersect(shadow);
        if (shadow_obj)
            continue;

        // diffuse
        color += lcolor*Kd*std::max(0.f, dot(n, l));
        // specular
        color += lcolor*Ks*std::pow(std::max(0.f, dot(v, reflect(l,n))), s_power);
    }
    color.x = std::min(1.0f, std::max(0.f, color.x));
    color.y = std::min(1.0f, std::max(0.f, color.y));
    color.z = std::min(1.0f, std::max(0.f, color.z));

    return vec4(color, 1);
}

Shape* RayTracer::Intersect(Ray& ray) {
    Ray closest;
    Shape* hit_obj = nullptr;
    for (int i = 0; i < spheres_.size(); i++) {
        if (spheres_[i]->Intersect(ray)) { 
            if (hit_obj) {
                if (ray.tmin < closest.tmin) {
                    closest = ray;
                    hit_obj = spheres_[i];
                }
            } else {
                closest = ray;
                hit_obj = spheres_[i];
            }
        }
    }
    if (hit_obj) {
        ray = closest;
    }
    return hit_obj;
}

void RayTracer::Trace(StatusReporter* statusReporter) {
    int width = camera_->getWidth();
    int height = camera_->getHeight();
    image_ = new Image(width, height);

    vec3 pos = camera_->getPos();
    vec3 dir = normalize(camera_->getDir());
    vec3 up = normalize(camera_->getUp());
    float half_angle = camera_->getHalfAngle() * M_PI / 180.0;
    float d = height / (2.0 * tan(half_angle));
    vec3 dx = normalize(cross(up, dir));
    vec3 dy = -up;
    vec3 ul = pos + d * dir + up * (height / 2.0) - (width / 2.0) * dx;

    Shape* hit_obj;
    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            // Compute ray
            vec3 p = py + c*dx;
            vec3 dir = normalize(p - pos);
            Ray ray(pos, dir);

            // See if ray hits anything
            hit_obj = Intersect(ray);

            // Compute color
            if (hit_obj) {
                image_->SetPixel(r, c, GetColor(hit_obj, ray));
            } else {
                image_->SetPixel(r, c, background_);
            }
        }
        if (statusReporter) {
            statusReporter->setValue(r / (float) height);
        }

        /*
        if (rand() % 50 == 0) {
            int start;
            int end = time(NULL);
            start = end;
            while(end - start < 1) {
                end = time(NULL);
            }
        }
        */
    }

    // write final image out
    image_->Write(camera_->getOutputImage());
}
