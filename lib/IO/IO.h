// Input/output module
// Communication between an processing system and the outside world
#ifndef lib_IO
	#define lib_IO
	#include <Arduino.h>

	class IO {
		public:
			struct Settings {
				struct Pin {
					int relay;
					int buzzer;
					int greenLed;
					int reset;
					int exitButton;
				} pin;
				
				struct OnState {
					int greenLed;
					int buzzer;
				} onState;

				int openTimespan;
				unsigned long int systemRestartTimespan;
			};

			Settings settings;
			IO(Settings settings);

			void Unlock();
			void Lock();
			void Open(int code = 3);
			void Reset();
			void Init();
			void Update();
			bool IsLocked();
			void PlaySuccessCode(int code);
			void PlayErrorCode(int code);
			void PlayStartupCode();
		private:
			bool locked = true;
			int lastExitButtonPress = 0;
			void CheckExitButton();
	};
#endif