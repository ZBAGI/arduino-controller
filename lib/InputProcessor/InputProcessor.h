#ifndef lib_InputProcessor
	#define lib_InputProcessor

	#include <Arduino.h>
	#include <Ethernet.h>

	typedef void (*OnCodeAccessRequestCallback) (const char* code);
	typedef void (*OnTagPinAccessRequestCallback) (const char* tagId, const char* pin);
	typedef void (*OnTagAccessRequestCallback) (const char* tagId);
	typedef void (*OnFailCallback) (const char* reason);
	typedef void (*OnTimeoutCallback) ();
	typedef void (*OnResetRequestCallback) ();

	enum Status {
		RequirePin,
		OnlyTag,
		Disabled
	};

	const size_t MAX_PIN_LENGTH = 4;
	const size_t MAX_TAG_LENGTH = 10;
	const size_t MAX_CODE_LENGTH = 12;

	class InputProcessor {
		public:
			struct Settings {
				OnCodeAccessRequestCallback onCodeAccessRequest;
				OnTagPinAccessRequestCallback onTagPinAccessRequest;
				OnTagAccessRequestCallback onTagAccessRequest;
				OnResetRequestCallback onSystemResetRequest;

				OnFailCallback onFail;
				OnTimeoutCallback onTimeout;
				
				int timeout;
			};

			Settings settings;
			Status status = RequirePin;
			InputProcessor(Settings settings);
			void Update();
			bool ButtonClicked(const uint8_t button);
			bool TagScanned(const char* tagId);
			bool EscapeButtonClicked();

		private:
			void emptyBuffers();
			int resetRequests = 0;

			char pinBuffer[MAX_PIN_LENGTH+1];
			char tagBuffer[MAX_TAG_LENGTH+1];
			char codeBuffer[MAX_CODE_LENGTH+1];
			unsigned long lastActionMillis = 0;
	};
#endif