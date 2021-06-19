#include "InputProcessor.h"

InputProcessor::InputProcessor(Settings settings)
{
    this->settings = settings;
}

bool InputProcessor::EscapeButtonClicked() {
    this->emptyBuffers();
    this->resetRequests+=1;
    if(this->resetRequests >= 10) {
        this->settings.onSystemResetRequest();
        return true;
    }
    return false;
}

bool InputProcessor::ButtonClicked(const uint8_t button) {
    if(this->status == Disabled)
        return false;

    this->lastActionMillis = millis();

    if(strlen(this->codeBuffer) != 0 || strlen(this->tagBuffer) == 0) {
        strncat(this->codeBuffer, (char*)button, MAX_CODE_LENGTH);

        if(strlen(this->codeBuffer) == 16) {
            this->settings.onCodeAccessRequest(this->codeBuffer);
            this->emptyBuffers();
            return true;
        }
    }

    if(strlen(this->tagBuffer) != 0) {
        strncat(this->pinBuffer, (char*)button, MAX_PIN_LENGTH);

        Serial.println(this->pinBuffer);
        if(strlen(this->pinBuffer) == 4) {
            this->settings.onTagPinAccessRequest(this->tagBuffer, this->pinBuffer);
            this->emptyBuffers();
        }
    }
    return true;
}

bool InputProcessor::TagScanned(const char* tagId) {
    if(this->status == Disabled)
        return false;
    
    this->emptyBuffers();
    strncat (this->tagBuffer, tagId, MAX_TAG_LENGTH);

    if(this->status == OnlyTag) {
        this->settings.onTagAccessRequest(this->tagBuffer);
        this->emptyBuffers();
    }
    return true;
}

void InputProcessor::emptyBuffers() {
    this->codeBuffer[0] = '\0';
    this->tagBuffer[0] = '\0';
    this->pinBuffer[0] = '\0';
    this->lastActionMillis = 0;
}

void InputProcessor::Update() {
    if(this->lastActionMillis != 0 && (this->lastActionMillis + this->settings.timeout) < millis()) {
        if(strlen(this->codeBuffer) != 0 || strlen(this->tagBuffer) != 0 || strlen(this->pinBuffer) != 0)
            this->settings.onTimeout();
        this->emptyBuffers();
    }
}
