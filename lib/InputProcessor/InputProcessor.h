#ifndef lib_InputProcessor
	#define lib_InputProcessor

	#include <Arduino.h>
	#include <Ethernet.h>

	typedef void (*OnCodeAccessRequestCallback) (String code);
	typedef void (*OnTagPinAccessRequestCallback) (String tagId, String pin);
	typedef void (*OnTagAccessRequestCallback) (String tagId);
	typedef void (*OnFailCallback) (String reason);
	typedef void (*OnTimeoutCallback) ();
	typedef void (*OnResetRequestCallback) ();

	enum Status {
		RequirePin,
		OnlyTag,
		Disabled
	};

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
			void ButtonClicked(String button);
			void TagScanned(String tagId);
			void EscapeButtonClicked();

		private:
			void clearBuilders();
			int resetRequests = 0;
			String codeBuilder = "";
			String pinBuilder = "";
			String tagBuilder = "";
			unsigned long lastActionMillis = 0;
	};
#endif