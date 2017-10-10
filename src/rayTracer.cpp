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
    sampling_method_ = parser_->getSamplingMethod();
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

    // loop through every spot light
    for (int i = 0; i < spot_lights_.size(); i++) {
        // update light vector
        SpotLight *light = spot_lights_[i];
        vec3 dir = normalize(light->getDirection());
        vec3 l = p - light->getPosition();
        float d = glm::length(l);
        l = normalize(l);

        float angle1 = light->getAngle1();
        float angle2 = light->getAngle2();
        float lAngle = std::acos(dot(l, dir)) * 180 / M_PI;
        // cout << lAngle << endl;
        if (lAngle > angle2)
            continue;
        vec3 I = (1.0/(d*d)) * light->getColor();
        if (lAngle >= angle1) {
            lAngle = 1 - (lAngle - angle1) / (angle2 - angle1);
            // lAngle = std::pow(lAngle, 10);
            // lAngle *= M_PI / 2;
            // I = std::cos(lAngle) * 
            I = lAngle * I;
        }

        l = -l;
        // cast shadow ray
        Ray shadow(p + 0.01*l, l);
        Shape *shadow_obj = Intersect(shadow);
        if (shadow_obj)
            continue;

        // diffuse
        color += I*Kd*std::max(0.f, dot(n, l));
        // specular
        color += I*Ks*std::pow(std::max(0.f, dot(v, reflect(l,n))), s_power);
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
            vec4 newColor;
            // Compute ray
            vec3 p = py + c*dx;
            vec3 dir = normalize(p - pos);
            Ray ray(pos, dir);
            hit_obj = Intersect(ray);

            if (hit_obj) {
                newColor = GetColor(hit_obj, ray);
            } else {
                newColor = background_;
            }

            if (sampling_method_ == 1) {
                int w = 2;
                int h = 2; 
                vec3 dx2 = (1.0/w)*dx;
                vec3 dy2 = (1.0/h)*dy;
                vec3 ulp = p + .5*dx2 + .5*dy2;
                for (int r2 = 0; r2 < h; r2++) {
                    vec3 py2 = ulp + r2*dy2;
                    for (int c2 = 0; c2 < w; c2++) {
                        p = py2 + c2*dx2;
                        dir = normalize(p - pos);
                        ray = Ray(pos, dir);
                        hit_obj = Intersect(ray);

                        if (hit_obj) {
                            newColor += GetColor(hit_obj, ray);
                        } else {
                            newColor += background_;
                        }
                    }
                }
                newColor *= (1.0/(w*h+1));
            } else {
            }
            image_->SetPixel(r, c, newColor);
        }
        if (statusReporter) {
            statusReporter->setValue(r / (float) height);
        }
    }

    // write final image out
    image_->Write(camera_->getOutputImage());
}
