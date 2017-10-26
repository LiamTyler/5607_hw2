#include "include/rayTracer.h"
#include "include/ray.h"
#include <stdlib.h>
#include <functional>
#include <chrono>

using glm::vec3;
using glm::vec4;
using glm::normalize;
using glm::dot;
using glm::cross;
using glm::reflect;
using glm::length;

using namespace std;
using namespace std::chrono;
using namespace std::placeholders;

RayTracer::RayTracer() {
    parser_ = nullptr;
    camera_ = nullptr;
    background_ = vec4(0,0,0,1);
    max_depth_ = 5;
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

    triangles_ = parser_->getTriangles();
    normal_triangles_ = parser_->getNormalTriangles();

    vertices_ = parser_->getVertices();
    normals_ = parser_->getNormals();

    materials_ = parser_->getMaterials();
    background_ = vec4(parser_->getBackground(), 1);
    max_depth_ = parser_->getMaxDepth();
    sampling_method_ = parser_->getSamplingMethod();
}

vec4 RayTracer::ComputeLighting(Shape* hit_obj, Ray& ray, int depth) {
    Material * m = hit_obj->getMaterial();
    vec3 I = normalize(ray.dir);
    vec3 p = ray.Evaluate();
    vec3 N = hit_obj->getNormal(p);

    vec3 color = vec3(0,0,0);

    // Pass in intersect function to lighting functions
    auto fp = std::bind(&RayTracer::Intersect, this, _1);

    // Do phong shading
    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end(); ++it) {
        color += (*it)->ComputeLighting(I, p, N, m, fp);
    }

    // Compute reflected and refracting lighting
    if (depth < max_depth_) {
        vec3 reflectColor(0,0,0), transmissiveColor(0,0,0);

        vec3 r = reflect(I, dot(I,N) < 0 ? -N : N);
        Ray mirror(p + 0.001 * r, r);
        reflectColor = m->getSpecular() * vec3(TraceRay(mirror, depth + 1));

        float kr = fresnel(I, N, m->getIOR());
        if (kr < 1) {
            float cosi = max(-1.0f, min(1.0f, dot(I, N)));
            float etai = 1, etat = m->getIOR();
            if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); N = -N; }
            float eta = etai/etat;
            float k = 1 - eta*eta * (1 - cosi*cosi);
            if (k < 0)
                r = vec3(0,0,0);
            else
                r = eta * I + (eta * cosi - sqrt(k)) * N;
            
            Ray refracted(p + 0.0001 * r, r);
            transmissiveColor = m->getTransmissive() * vec3(TraceRay(refracted, depth + 1));
        }

        color += reflectColor * kr + transmissiveColor * (1 - kr);
    }

    // clamp
    color.x = min(1.0f, max(0.f, color.x));
    color.y = min(1.0f, max(0.f, color.y));
    color.z = min(1.0f, max(0.f, color.z));
    return vec4(color, 1);
}


Shape* RayTracer::Intersect(Ray& ray) {
    Ray closest;
    Shape* hit_obj = nullptr;
    // Loop over every shape in scene
    for (int i = 0; i < shapes_.size(); i++) {
        if (shapes_[i]->Intersect(ray)) { 
            if (hit_obj) {
                // Record if object is the closest so far
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

vec4 RayTracer::TraceRay(Ray& ray, int depth) {
    Shape* hit_obj = Intersect(ray);

    if (hit_obj) {
        return ComputeLighting(hit_obj, ray, depth);
    } else {
        return background_;
    }
}

void RayTracer::Run(StatusReporter* statusReporter) {
    high_resolution_clock::time_point start_time = high_resolution_clock::now();
    high_resolution_clock::time_point current_time;
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

    // Get which sampling method is specified
    auto Sample = &RayTracer::BasicSample;
    if (sampling_method_ == SUPER)
        Sample = &RayTracer::SuperSample;
    else if (sampling_method_ == ADAPTIVE)
        Sample = &RayTracer::AdaptiveSample;

    // Loop through each pixel
    #pragma omp parallel for
    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            vec3 p = py + c*dx;
            vec4 color = (this->*Sample)(pos, p, dx, dy);
            image_->SetPixel(r, c, color);
            // if (statusReporter) {
            //     current_time = high_resolution_clock::now();
            //     float dt = duration_cast<milliseconds>(current_time - start_time).count();
            //     statusReporter->Update(r / (float) height, dt / 1000);
            // }
        }
    }
    // current_time = high_resolution_clock::now();
    // float dt = duration_cast<milliseconds>(current_time - start_time).count();
    // statusReporter->Update(1, dt / 1000);

    // Save the final image
    image_->Write(camera_->getOutputImage());
}

vec4 RayTracer::BasicSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec3 dir = normalize(p - pos);
    Ray ray(pos, dir);
    return TraceRay(ray, 0);
}

vec4 RayTracer::SuperSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec3 dir = normalize(p - pos);
    Ray ray(pos, dir);
    vec4 newColor = TraceRay(ray, 0);
    int w = 2;
    int h = 2; 
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
            newColor += TraceRay(ray, 0);
        }
    }
    newColor *= (1.0/(w*h+1));
    return newColor;
}

vec4 RayTracer::AdaptiveSample(vec3& pos, vec3 &p, vec3& dx, vec3& dy) {
    vec3 dir = normalize(p - pos);
    Ray ray(pos,dir);
    vec4 newColor = TraceRay(ray, 0);
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
            vec4 tmp = TraceRay(ray, 0);
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
                vec4 tmp = TraceRay(ray, 0);
                newColor += tmp;
            }
        }
        newColor /= 9.0;
    } else {
        newColor /= 5.0;
    }
    return newColor;
}
