#ifndef SRC_INCLUDE_PARSER_H_
#define SRC_INCLUDE_PARSER_H_

#include <iostream>
#include <fstream>
#include <vector>
#include "glm/glm.hpp"
#include "include/utils.h"
#include "include/camera.h"
#include "include/lights.h"
#include "include/material.h"
#include "include/shapes.h"

using std::string;
using glm::vec3;
using std::vector;

class Parser {
    public:
        Parser();
        Parser(string filename);
        ~Parser();

        // sets all the variables to their default values
        void Init();

        // Parses the file and updates the variables
        bool Parse();

        // Getter functions
        void setFilename(string f) { filename_ = f; };
        string getFilename() { return filename_; }
        void setMaxDepth(int d) { max_depth_ = d; }
        int getMaxDepth() { return max_depth_; }
        void setBackground(vec3 b) { background_ = b; }
        vec3 getBackground() { return background_; }

        int getMaxVertices() { return max_vertices_; }
        int getMaxNormals() { return max_normals_; }
        Camera* getCamera() { return camera_; }

        vector<DirectionalLight *> getDirectionalLights() { return directional_lights_; }
        vector<PointLight *> getPointLights() { return point_lights_; }
        vector<SpotLight *> getSpotLights() { return spot_lights_; }
        AmbientLight* getAmbientLight() { return ambient_light_; }

        vector<Sphere*>& getSpheres() { return spheres_; }
        vector<Triangle*>& getTriangles() { return triangles_; }
        vector<NormalTriangle*>& getNormalTriangles() { return normal_triangles_; }

        vector<vec3>& getVertices() { return vertices_; }
        vector<vec3>& getNormals() { return normals_; }

        vector<Material*> getMaterials() { return materials_; }
        SamplingMethod getSamplingMethod() { return sampling_method_; }
        string getEnvironmentMap() { return env_map_; }

    private:
        string filename_;
        std::ifstream infile_;
        Camera * camera_;
        vector<DirectionalLight *> directional_lights_;
        vector<PointLight *> point_lights_;
        vector<SpotLight *> spot_lights_;
        AmbientLight* ambient_light_;
        
        vector<Material*> materials_;
        Material* current_material_;
        vector<Sphere*> spheres_;
        vector<Triangle*> triangles_;
        vector<NormalTriangle*> normal_triangles_;

        vector<vec3> vertices_;
        vector<vec3> normals_;
        int max_vertices_;
        int max_normals_;

        SamplingMethod sampling_method_;
        vec3 background_;
        int max_depth_;
        string env_map_;
};

#endif  // SRC_INCLUDE_PARSER_H_

