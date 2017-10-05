#include "include/parser.h"

using namespace std;

Parser::Parser() {
    filename_ = "scene.scn";
    Init();
}

Parser::Parser(string filename) {
    filename_ = filename;
    Init();
}

Parser::~Parser() {
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

void Parser::Init() {
    camera_ = new Camera();
    ambient_light_ = new AmbientLight();
    current_material_ = new Material();
}

bool Parser::Parse() {
    infile_.open(filename_.c_str());
    if(infile_.fail()){
        cout << "Can't open file '" << filename_ << "'" << endl;
        return false;
    }

    string command;
    string line;
    while(infile_ >> command) {
        if (command[0] == '#'){
            getline(infile_, line);
            continue;
        } else if (command == "camera") {
            vec3 pos, dir, up;
            infile_ >> pos;
            infile_ >> dir;
            infile_ >> up;
            float ha;
            infile_ >> ha;
            camera_->setPos(pos);
            camera_->setDir(dir);
            camera_->setUp(up);
            camera_->setHalfAngle(ha);
        } else if (command == "film_resolution") {
            int w, h;
            infile_ >> w >> h;
            camera_->setWidth(w);
            camera_->setHeight(h);
        } else if (command == "sphere") {
            vec3 p;
            float r;
            infile_ >> p >> r;
            spheres_.push_back(new Sphere(p, r, current_material_));
        } else if (command == "background") {
            vec3 c;
            infile_ >> c;
            background_ = c;
        } else if (command == "material") {
            vec3 a, d, s, t;
            float ns, ior;
            infile_ >> a >> d >> s >> ns >> t >> ior;
            current_material_ = new Material(a, d, s, ns, t, ior);
            materials_.push_back(current_material_);
        } else if (command == "output_image"){
            string o;
            infile_ >> o;
            camera_->setOutputImage(o);
        } else if (command == "directional_light"){
            vec3 c, d;
            infile_ >> c >> d;
            directional_lights_.push_back(new DirectionalLight(c, d));
        } else if (command == "point_light"){
            vec3 c, p;
            infile_ >> c >> p;
            point_lights_.push_back(new PointLight(c, p));
        } else if (command == "spot_light"){
            vec3 c, p, d;
            float a1, a2;
            infile_ >> c >> p >> d >> a1 >> a2;
            spot_lights_.push_back(new SpotLight(c,p,d,a1,a2));
        } else if (command == "ambient_light"){
            vec3 c;
            infile_ >> c;
            ambient_light_->setColor(c);
        } else if (command == "max_depth"){
            infile_ >> max_depth_;
        } else {
            getline(infile_, line);
            cout << "WARNING. Do not know command: " << command << endl;
        }
    }
}
