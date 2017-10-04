#ifndef SRC_INCLUDE_MATERIAL_H_
#define SRC_INCLUDE_MATERIAL_H_

#include <iostream>
#include "glm/glm.hpp"

using glm::vec3;

class Material {
    public:
        Material() : Material(vec3(0,0,0), vec3(1,1,1), vec3(0,0,0),
                              5, vec3(0,0,0), 1) {}
        Material(vec3 a, vec3 d, vec3 s, float ns, vec3 t, float ior) {
            ambient_ = a;
            diffuse_ = d;
            specular_ = s;
            power_ = ns;
            transmissive_ = t;
            ior_ = ior;
        }

        // setters
        void setAmbient(vec3 a) { ambient_ = a; }
        void setDiffuse(vec3 d) { diffuse_ = d; }
        void setSpecular(vec3 s) { specular_ = s; }
        void setTransmissive(vec3 t) { transmissive_ = t; }
        void setPower(float p) { power_ = p; }
        void setIOR(float i) { ior_ = i; }

        // getters
        vec3 getAmbient() { return ambient_; }
        vec3 getDiffuse() { return diffuse_; }
        vec3 getSpecular() { return specular_; }
        vec3 getTransmissive() { return transmissive_; }
        float getPower() { return power_; }
        float getIOR() { return ior_; }

    private:
        vec3 ambient_;
        vec3 diffuse_;
        vec3 specular_;
        vec3 transmissive_;
        float power_;
        float ior_;
};

#endif  // SRC_INCLUDE_MATERIAL_H_

