#include <nanogui/nanogui.h>
#include <iostream>
#include "include/rayTracer.h"
#include "include/utils.h"
#include <thread>
#include "include/canvas.h"

using namespace std;
using namespace nanogui;

RayTracer* tracer;

void run(ProgressBar* p, FloatBox<float> * progress, FloatBox<float>* time,
        string s, MyGLCanvas* canvas) {
    tracer = new RayTracer;
    tracer->Parse(s);

    StatusReporter status(p, progress, time);
    tracer->Run(&status);
    
    canvas->SetFileName(tracer->getOutputFileName());
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        tracer = new RayTracer;
        tracer->Parse(argv[1]);
        tracer->Run();
        delete tracer;
        return 0;
    }

    nanogui::init();
    Screen *screen = new Screen(Vector2i(650, 500), "Ray Tracer");
    // screen->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Fill, 10, 10));
    Window *control = new Window(screen, "Control Panel");
    control->setPosition(Vector2i(10, 10));
    control->setLayout(new GroupLayout());

    string scene_file = "../scenes/bear.scn";
    new Label(control, "Current Loaded Scene", "sans-bold");
    TextBox* scene_name = new TextBox(control, scene_file);
    new Label(control, "Scene Selection", "sans-bold");
    Button *open = new Button(control, "Open Scene");

    new Label(control, "Ray Tracing", "sans-bold");
    Button* b = new Button(control, "Run Ray Tracer");
    std::thread trace_thread;
    new Label(control, "Progress", "sans-bold");
    ProgressBar* p = new ProgressBar(control);
    FloatBox<float>* prog_box = new FloatBox<float>(control, 0.0f);
    prog_box->setUnits("%");
    FloatBox<float>* time_box = new FloatBox<float>(control, 0.0f);
    time_box->setUnits("sec");
    new Label(control, "Reset Scene", "sans-bold");
    Button* b2 = new Button(control, "Clear Canvas");
    Window* w = new Window(screen, "Rendered Scene");
    w->setPosition(Vector2i(200, 10));
    w->setLayout(new GroupLayout());

    MyGLCanvas* canvas = new MyGLCanvas(w);
    canvas->setBackgroundColor({100, 100, 100, 255});
    canvas->setSize({400, 400});
    //w->setPosition(Vector2i(30 + control->width(), 10));
    screen->performLayout();
    w->setPosition(Vector2i(30 + control->width(), 10));

    open->setCallback([&] {
        scene_file = file_dialog({ { "scn", "Scene File" }}, false);
        string tmp;
        if (scene_file.length() > 20) {
            tmp = scene_file.substr(scene_file.length() - 20);
        }
        scene_name->setValue(tmp);
        w->setPosition(Vector2i(30 + control->width(), 10));
        screen->setSize(Vector2i(w->position()[0] + w->size()[0] + 10, screen->size()[1]));
    });

    b->setCallback([&] {
        if (scene_file != "") {
            if (trace_thread.joinable()) {
                trace_thread.join();
                delete tracer;
            }
            trace_thread = std::thread(run, p, prog_box, time_box, scene_file, canvas);
        }
    });
    b2->setCallback([&] {
        canvas->Clear();
        time_box->setValue(0);
        prog_box->setValue(0);
        p->setValue(0);
    });

    screen->setVisible(true);
    screen->performLayout();
    screen->setSize(Vector2i(w->position()[0] + w->size()[0] + 10, screen->size()[1]));

    nanogui::mainloop();
    nanogui::shutdown();

    if (trace_thread.joinable()) {
        trace_thread.join();
        delete tracer;
    }

    return 0;
}
