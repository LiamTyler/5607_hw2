#ifndef SRC_INCLUDE_STATUSREPORTER_H_
#define SRC_INCLUDE_STATUSREPORTER_H_

#include <iostream>
#include <string>
#include <nanogui/nanogui.h>

using namespace nanogui;

class StatusReporter {
    public:
        StatusReporter() : StatusReporter(nullptr, nullptr) {}
        StatusReporter(ProgressBar* p, TextBox* t) {
            pbar_ = p;
            tbox_ = t;
            progress_ = 0;
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
        void setValue(float val) {
            progress_ = val;
            Update();
        }
        void Update() {
            pbar_->setValue(progress_);
            tbox_->setValue(std::to_string((int) 100* progress_));
        }
                
    protected:
        ProgressBar* pbar_;
        TextBox* tbox_;
        float progress_;
};

#endif  // SRC_INCLUDE_MATERIAL_H_

