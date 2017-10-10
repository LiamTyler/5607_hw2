#include <nanogui/nanogui.h>
#include <iostream>
#include "include/rayTracer.h"
#include "include/utils.h"
#include <thread>
#include "include/canvas.h"

using namespace std;
using namespace nanogui;

RayTracer* tracer;

void run(ProgressBar* p, TextBox* t, string s, MyGLCanvas* canvas) {
    tracer = new RayTracer;
    tracer->Parse(s);

    StatusReporter status(p, t);
    tracer->Trace(&status);
    
    canvas->SetFileName(tracer->getOutputFileName());
}

int main(int argc, char* argv[]) {
    if (argc == 2) {
        tracer = new RayTracer;
        tracer->Parse(argv[1]);
        tracer->Trace();
        delete tracer;
        return 0;
    }

    nanogui::init();
    Screen *screen = new Screen(Vector2i(650, 500), "Ray Tracer");
    // screen->setLayout(new BoxLayout(Orientation::Horizontal, Alignment::Fill, 10, 10));
    Window *w = new Window(screen, "Control Panel");
    w->setPosition(Vector2i(10, 10));
    w->setLayout(new GroupLayout());

    new Label(w, "Scene Selection", "sans-bold");
    Button *b = new Button(w, "Open Scene");
    string scene_file = "../scenes/scene2.scn";
    b->setCallback([&] {
        scene_file = file_dialog({ { "scn", "Scene File" }}, false);
    });

    new Label(w, "Ray Tracing", "sans-bold");
    b = new Button(w, "Run Ray Tracer");
    std::thread trace_thread;
    new Label(w, "Progress", "sans-bold");
    ProgressBar* p = new ProgressBar(w);
    TextBox * textBox = new TextBox(w);
    textBox->setValue("0");
    textBox->setUnits("%");
    Button* b2 = new Button(w, "Clear Canvas");
    w = new Window(screen, "Rendered Scene");
    w->setPosition(Vector2i(200, 10));
    w->setLayout(new GroupLayout());

    MyGLCanvas* canvas = new MyGLCanvas(w);
    canvas->setBackgroundColor({100, 100, 100, 255});
    canvas->setSize({400, 400});

    b->setCallback([&] {
        if (scene_file != "") {
            if (trace_thread.joinable()) {
                trace_thread.join();
                delete tracer;
            }
            trace_thread = std::thread(run, p, textBox, scene_file, canvas);
        }
    });
    b2->setCallback([&] {
        canvas->Clear();
    });

    screen->setVisible(true);
    screen->performLayout();

    nanogui::mainloop();
    nanogui::shutdown();

    if (trace_thread.joinable()) {
        trace_thread.join();
        delete tracer;
    }

    return 0;
}
