#include "InputProcessor.h"

InputProcessor::InputProcessor(Settings settings)
{
    this->settings = settings;
}

void InputProcessor::EscapeButtonClicked() {
    this->clearBuilders();
    this->resetRequests+=1;
    if(this->resetRequests >= 10)
        this->settings.onSystemResetRequest();
}

void InputProcessor::ButtonClicked(String button) {
    if(this->status == Disabled)
        return;

    this->lastActionMillis = millis();

    if(this->codeBuilder != "" || this->tagBuilder == "") {
        this->codeBuilder =  this->codeBuilder + button;
        if(this->codeBuilder.length() == 16) {
            this->settings.onCodeAccessRequest(this->codeBuilder);
            this->clearBuilders();
            return;
        }
    }

    if(this->tagBuilder != "") {
        this->pinBuilder = this->pinBuilder + button;
        if(this->pinBuilder.length() == 4) {
            this->settings.onTagPinAccessRequest(this->tagBuilder, this->pinBuilder);
            this->clearBuilders();
            return;
        }
    }
}

void InputProcessor::TagScanned(String tagId) {
    if(this->status == Disabled)
        return;
    
    if(this->tagBuilder != "" || this->codeBuilder != "")
        this->clearBuilders();

    this->tagBuilder = tagId;

    if(this->status == OnlyTag) {
        this->settings.onTagAccessRequest(this->tagBuilder);
        this->clearBuilders();
    }
}

void InputProcessor::clearBuilders() {
    this->codeBuilder = "";
    this->tagBuilder = "";
    this->pinBuilder = "";
    this->lastActionMillis = 0;
}

void InputProcessor::Update() {
    if(this->lastActionMillis != 0 && (this->lastActionMillis + this->settings.timeout) < millis()) {
        if(this->codeBuilder != "" || this->tagBuilder != "" || this->pinBuilder != "")
            this->settings.onTimeout();
        this->clearBuilders();
    }
}
