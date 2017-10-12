#ifndef SRC_INCLUDE_STATUSREPORTER_H_
#define SRC_INCLUDE_STATUSREPORTER_H_

#include <iostream>
#include <string>
#include <nanogui/nanogui.h>

using namespace nanogui;

class StatusReporter {
    public:
        StatusReporter() : StatusReporter(nullptr, nullptr, nullptr) {}
        StatusReporter(ProgressBar* p, FloatBox<float>* prog, FloatBox<float>* time) {
            pbar_ = p;
            progress_gui_ = prog;
            time_gui_ = time;
            progress_ = 0;
            time_ = 0;
        }
        ~StatusReporter() {
            progress_ = 1;
            Update();
        }
        void Done() {
            progress_ = 1;
            Update();
        }
        void Clear() {
            progress_ = 0;
            Update();
        }

        float getProgress() { return progress_; }

        void Update(float percent, float time) {
            progress_ = percent;
            time_ = time;
            Update();
        }
        void Update() {
            pbar_->setValue(progress_);
            progress_gui_->setValue(100 * progress_);
            time_gui_->setValue(time_);
        }
                
    protected:
        ProgressBar* pbar_;
        FloatBox<float>* progress_gui_;
        FloatBox<float>* time_gui_;
        float progress_;
        float time_;
};

#endif  // SRC_INCLUDE_MATERIAL_H_
