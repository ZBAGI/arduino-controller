#include "IO.h"

IO::IO(Settings settings)
{
   this->settings = settings;
}

void IO::Init() 
{
   digitalWrite(this->settings.pin.reset, HIGH);
	digitalWrite(this->settings.pin.relay, HIGH);

   delay(200);

	pinMode(this->settings.pin.greenLed, OUTPUT);
	pinMode(this->settings.pin.buzzer, OUTPUT);
	pinMode(this->settings.pin.relay, OUTPUT);

	delay(200);

   digitalWrite(this->settings.pin.reset, HIGH);
	digitalWrite(this->settings.pin.relay, HIGH);
	digitalWrite(this->settings.pin.greenLed, !this->settings.onState.greenLed);
   digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
}

void IO::Unlock()
{
   this->locked = false;
   digitalWrite(this->settings.pin.relay, HIGH);
	digitalWrite(this->settings.pin.greenLed, this->settings.onState.greenLed);
}

void IO::Lock()
{
   this->locked = true;
   digitalWrite(this->settings.pin.relay, LOW);
	digitalWrite(this->settings.pin.greenLed, !this->settings.onState.greenLed);
}

// 2 = Exit button pressed
// 3 = Ordinary access granted
// 5 = No internet/connection with server, special access granted
// 6 = Incorrect server response, special access granted
void IO::Open(int code) 
{
   digitalWrite(this->settings.pin.relay, HIGH);
   digitalWrite(this->settings.pin.greenLed, this->settings.onState.greenLed);

   this->PlaySuccessCode(code);
   delay(this->settings.openTimespan);
   
   digitalWrite(this->settings.pin.relay, LOW);
   digitalWrite(this->settings.pin.greenLed, !this->settings.onState.greenLed);
}

void IO::Update() 
{
   if(this->IsLocked())
      IO::CheckExitButton();

   if(millis() > this->settings.systemRestartTimespan)
		this->Reset();
}

void IO::Reset() 
{
   digitalWrite(this->settings.pin.reset, LOW);
}

void IO::CheckExitButton() 
{
   int buttonValue = digitalRead(this->settings.pin.exitButton);
   if (buttonValue == LOW){
      lastExitButtonPress = 0;
      return;
   }

   lastExitButtonPress += 1;
   if(lastExitButtonPress > 200)
      this->Open(2);
}

bool IO::IsLocked() 
{
   return this->locked;
}

// Codes:
// 2 = Exit button pressed
// 3 = Ordinary access granted
// 4 = No internet/connection with server, special access granted
// 5 = Incorrect server response, special access granted
void IO::PlaySuccessCode(int code) {
   for( int c = 0; c <= code; c = c + 1 ) {
      if(c > 0)
         delay(80);
      digitalWrite(this->settings.pin.buzzer, this->settings.onState.buzzer);
      delay(80);
      digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
   }
}

// Codes:
// 0 = User action denied / Access denied by server
// 1 = Timeout, no action taken
// 2 = Internet problem - no connection
// 3 = Incorrect server responses
void IO::PlayErrorCode(int code) {
   if(code == 0) {
      digitalWrite(this->settings.pin.buzzer, this->settings.onState.buzzer);
      delay(1000);
      digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
      return;
   }

   for( int c = 0; c <= code; c = c + 1 ) {
      if(c > 0)
         delay(300);
      digitalWrite(this->settings.pin.buzzer, this->settings.onState.buzzer);
      delay(300);
      digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
   }
}

void IO::PlayStartupCode() {
   	digitalWrite(this->settings.pin.buzzer, this->settings.onState.buzzer);
      delay(500);
      digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
      delay(100);
      digitalWrite(this->settings.pin.buzzer, this->settings.onState.buzzer);
      delay(100);
      digitalWrite(this->settings.pin.buzzer, !this->settings.onState.buzzer);
}