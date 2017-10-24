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

    // Lights
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

    // Shapes
    vertices_ = parser_->getVertices();
    normals_ = parser_->getNormals();
    vector<Sphere*> s = parser_->getSpheres();
    for (int i = 0; i < s.size(); i++)
        shapes_.push_back(s[i]);
    vector<Triangle*> t = parser_->getTriangles();
    for (int i = 0; i < t.size(); i++)
        shapes_.push_back(t[i]);
    vector<NormalTriangle*> nt = parser_->getNormalTriangles();
    for (int i = 0; i < nt.size(); i++)
        shapes_.push_back(nt[i]);

    // triangles_ = parser_->getTriangles();
    // normal_triangles_ = parser_->getNormalTriangles();

    materials_ = parser_->getMaterials();
    background_ = vec4(parser_->getBackground(), 1);
    max_depth_ = parser_->getMaxDepth();
    sampling_method_ = parser_->getSamplingMethod();

    if (parser_->getEnvironmentMap() != "") {
        env_map_ = new Image(parser_->getEnvironmentMap());
    } else {
        env_map_ = nullptr;
    }
    cout << "Made it past loading envmap" << endl;
}

vec3 refract(vec3& I, vec3& N, float& ior) {
    float cosi = max(-1.0f, min(1.0f, dot(I, N))); 
    float etai = 1, etat = ior; 
    vec3 n = N; 
    if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; } 
    float eta = etai / etat; 
    float k = 1 - eta * eta * (1 - cosi * cosi); 
    return k < 0 ? vec3(0,0,0) : eta * I + (eta * cosi - sqrt(k)) * n;
}
/*
void fresnel(const vec3 &I, const vec3 &N, const float &ior, float &kr) 
{ 
    float cosi = max(-1.0f, min(1.0f, dot(I, N))); 
    float etai = 1, etat = ior; 
    if (cosi > 0) { std::swap(etai, etat); } 
    float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi)); 
    // Total internal reflection
    if (sint >= 1) { 
        kr = 1; 
    } 
    else { 
        float cost = sqrtf(std::max(0.f, 1 - sint * sint)); 
        cosi = fabsf(cosi); 
        float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost)); 
        float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost)); 
        kr = (Rs * Rs + Rp * Rp) / 2; 
    } 
}
*/

vec4 RayTracer::ComputeLighting(Shape* hit_obj, Intersection& inter, int depth) {
    Material * m = hit_obj->getMaterial();
    vec3 I = inter.ray.dir;
    vec3 p = inter.ray.Evaluate();
    vec3 N = normalize(hit_obj->getNormal(p, inter));

    vec3 color = vec3(0,0,0);

    // Pass in intersect function to lighting functions
    auto fp = std::bind(&RayTracer::Intersect, this, _1);

    // Do phong shading
    for (vector<Light*>::iterator it = lights_.begin(); it != lights_.end(); ++it) {
        color += (*it)->ComputeLighting(I, p, N, m, inter, fp);
    }

    // Compute reflected and refracting lighting
    // NOTE!! Using www.scratchapixel.com's lesson on refraction as reference
    // cout << depth << " " << max_depth_ << endl;
    if (depth < max_depth_) {
        vec3 reflectColor(0,0,0), transmissiveColor(0,0,0);

        // reflection
        float n1 = 1.0, n2 = m->getIOR();
        float c = dot(N, I);
        if (c > 0) { 
            N = -N;
            swap(n1, n2);
        }
        vec3 r = normalize(reflect(I, N));
        Ray mirror(p + 0.001 * r, r);
        reflectColor = m->getSpecular() * vec3(TraceRay(mirror, depth + 1));

        // refraction


        float ratio = n1 / n2;
        r = normalize(glm::refract(I, N, ratio));
        // r = normalize(refract(I, N, m->getIOR()));
        if (m->getTransmissive() != vec3(0,0,0)) {
            // cout << "ray dir: " << I << endl;
            // cout << "normal: " << N << endl;
            // cout << "dot product: " << c << endl;
            // cout << "refracted: " << r << endl;
            // cout << "depth: " << depth << " " << max_depth_ << endl;
            // cout << endl;
        }
        Ray transmissive(p + 0.001 * r, r);
        transmissiveColor = m->getTransmissive() * vec3(TraceRay(transmissive, depth + 1));

        // color += reflectColor * kr + transmissiveColor * (1 - kr);
        color += reflectColor + transmissiveColor;
    }

    // clamp
    color.x = min(1.0f, max(0.f, color.x));
    color.y = min(1.0f, max(0.f, color.y));
    color.z = min(1.0f, max(0.f, color.z));
    return vec4(color, 1);
}

//float R0 = (n1 - n2) / (n1 + n2);
//R0 *= R0;
//kr = R0 + (1-R0)*pow(1-c, 5);
//if (kr < 1) {

Shape* RayTracer::Intersect(Intersection& inter) {
    Intersection closest;
    Shape* hit_obj = nullptr;
    // Loop over every sphere in scene
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
}

vec4 RayTracer::TraceRay(Ray& ray, int depth) {
    Intersection inter;
    inter.ray = ray;
    inter.verts = &vertices_;
    inter.norms = &normals_;
    inter.cameraDir = normalize(camera_->getDir());

    Shape* hit_obj = Intersect(inter);

    if (hit_obj) {
        //if (depth > 0) {
        //    string s;
        //    cin >> s;
        //    cout << "refracted hit something" << endl;
        //}
        return ComputeLighting(hit_obj, inter, depth);
    } else {
        if (env_map_) {
            vec3 d = ray.dir;
            float lat = asin(d.y), lon = atan2(d.x,d.z);
            float u = lon/(2*M_PI) + 0.5, v = lat/M_PI + 0.5;
            // return texture(skyTexture, vec2(u, v)).rgb;
            int c = u * env_map_->Width();
            int r = (1-v) * env_map_->Height();
            return env_map_->GetPixel(r,c);
        } else {
            return background_;
        }
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
    #pragma omp parallel for schedule(dynamic) 
    for (int r = 0; r < height; r++) {
        vec3 py = ul + r * dy;
        for (int c = 0; c < width; c++) {
            vec3 p = py + c*dx;
            vec4 color = (this->*Sample)(pos, p, dx, dy);
            image_->SetPixel(r, c, color);
            if (statusReporter) {
                current_time = high_resolution_clock::now();
                float dt = duration_cast<milliseconds>(current_time - start_time).count();
                statusReporter->Update(r / (float) height, dt / 1000);
            }
        }
    }

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
