#include "include/rayTracer.h"

using std::string;
using glm::vec3;
using std::vector;
using glm::vec4;
using glm::normalize;
using glm::dot;
using glm::cross;

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

vec4 RayTracer::GetColor(Sphere* hit_s, vec3 pos, vec3 ray, float t) {
    PointLight *light = point_lights_[0];
    vec3 l = light->getPosition();
    float r = glm::length(l);
    vec3 p = pos + t * ray;
    vec3 L = normalize(l - p);
    Material * m = hit_s->getMaterial();
    vec3 color = m->getDiffuse();
    vec3 N = normalize(p - hit_s->getPosition());
    return vec4(std::max(0.f, dot(N, L)) * color, 1);

    // return vec4(hit_s->GetColor(pos, ray, t), 1);
}

void RayTracer::Trace() {
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

    float earliest_hit;
    Sphere* hit_sphere;

    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            hit_sphere = nullptr;
            vec3 p = py + c*dx;
            vec3 ray = normalize(p - pos);
            // vector<float> intersections
            for (int i = 0; i < spheres_.size(); i++) {
                float t;
                if (spheres_[i]->Hit(pos, ray, t)) { 
                    if (hit_sphere) {
                        if (t < earliest_hit) {
                            earliest_hit = t;
                            hit_sphere = spheres_[i];
                        }
                    } else {
                        earliest_hit = t;
                        hit_sphere = spheres_[i];
                    }
                }
            }
            if (hit_sphere) {
                image_->SetPixel(r, c, GetColor(hit_sphere, pos, ray, earliest_hit));
            } else {
                image_->SetPixel(r, c, background_);
            }
        }
    }

    // write final image out
    image_->Write(camera_->getOutputImage());
}
