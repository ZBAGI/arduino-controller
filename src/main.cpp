#include <Arduino.h>
#include <InputProcessor.h>
#include <IO.h>
#include <Wiegand.h>
#include <Api.h>

#define PIN_D0 2
#define PIN_D1 3

// Forward definition just to keep everything clean in order
static void tryAccessViaCode(const char* code);
static void tryAccessViaTagAndPin(const char* tagId, const char* pin);
static void tryAccessViaTag(const char* tagId);
static void resetSystem();
static void actionFailed(const char* reason);
static void actionTimeout();
static void readBytes(uint8_t* data, uint8_t bits, const char* message);
static void readBytesError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message);
static void stateChanged(bool plugged, const char* message);
static void apiConnectionError();
static void apiResponseError();
static void apiStatusChange(bool locked, bool requirePin);
///////

IO::Settings IOSettings = {
	.pin = {
		.relay = A0,
		.buzzer = 7,
		.greenLed = 8,
		.reset = A5,
		.exitButton = A4 // Other end of lead should go to GRND
	},
	.onState = {
		.greenLed = LOW,
		.buzzer = LOW
	},
	.openTimespan = 4000, // 4 sec
	.systemRestartTimespan = 86400000 // 24h
};
IO io = IO(IOSettings);

InputProcessor::Settings inputProcessorSettings = {
	.onCodeAccessRequest = tryAccessViaCode,
	.onTagPinAccessRequest = tryAccessViaTagAndPin,
	.onTagAccessRequest = tryAccessViaTag,
	.onSystemResetRequest = resetSystem,
	
	.onFail = actionFailed,
	.onTimeout = actionTimeout,
	
	.timeout = 10000, // 10 sec
};
InputProcessor inputProcessor = InputProcessor(inputProcessorSettings);

Api::Settings apiReceptionSettings = {
	.onConnectionError = apiConnectionError,
	.onResponseError = apiResponseError,
	.onStatusChange = apiStatusChange,
	
	.macAddress = {222,173,254,171,253,253},
	.ip = "192.168.1.177",
	.apiKey = "xxxxxxxxxxxxxx",
	.apiHost = "xxxxxxxxxxxxxx",
	.accessPointId = 3,
	.timeout = 14000
};

Api::Settings apiUpperDoorSettings = {
	.onConnectionError = apiConnectionError,
	.onResponseError = apiResponseError,
	.onStatusChange = apiStatusChange,

	.macAddress = {222,173,254,171,253,254},
	.ip = "192.168.1.178",
	.apiKey = "xxxxxxxxxxxxxx",
	.apiHost = "xxxxxxxxxxxxxx",
	.accessPointId = 2,
	.timeout = 14000
};

Api::Settings apiFrontDoorSettings = {
	.onConnectionError = apiConnectionError,
	.onResponseError = apiResponseError,
	.onStatusChange = apiStatusChange,

	.macAddress = {222,173,254,171,253,255},
	.ip = "192.168.1.179",
	.apiKey = "xxxxxxxxxxxxxx",
	.apiHost = "xxxxxxxxxxxxxx",
	.accessPointId = 1,
	.timeout = 14000
};

Api api = Api(apiReceptionSettings);

Wiegand wiegand;

void setup() {
	Serial.begin(9600);
	wiegand.onReceive(readBytes);
	wiegand.onReceiveError(readBytesError);
	wiegand.onStateChange(stateChanged);
	wiegand.begin(Wiegand::LENGTH_ANY, true);
	pinMode(PIN_D0, INPUT);
	pinMode(PIN_D1, INPUT);
	api.Init();
	io.Init();
	io.PlayStartupCode();
}

void loop() {
	wiegand.flush();
	// Check for changes on the the wiegand input pins
	wiegand.setPin0State(digitalRead(PIN_D0));
	wiegand.setPin1State(digitalRead(PIN_D1));

	io.Update();
	inputProcessor.Update();
	api.Update();
}

void actionFailed(const char* reason) {
	Serial.println("Input processor failed due to: ");
	Serial.print(reason);
	io.PlayErrorCode(0);
	api.checkStatus = true;
}

void actionTimeout() {
	Serial.println("Input processor timeout.");
	io.PlayErrorCode(1);
	api.checkStatus = true;
}

void processApiResponse(AccessRequestResponse response) {
	if(response == Denied) {
		io.PlayErrorCode(0);
	}else if(response == Allowed) {
		io.Open(3);
	}else if(response == ConnectionError) {
		io.Open(5);
	}else if(response == ResponseError) {
		io.Open(6);
	}
}

void tryAccessViaTag(const char* tagId) {
	Serial.print("Checking access for tag: ");
	Serial.println(tagId);
	processApiResponse(api.TagCanAccess(tagId));
	api.checkStatus = true;
}

void tryAccessViaTagAndPin(const char* tagId, const char* pin) {
	Serial.print("Checking access for tag: ");
	Serial.print(tagId);
	Serial.print(" and pin ");
	Serial.println(pin);
	processApiResponse(api.TagAndPinCanAccess(tagId, pin));
	api.checkStatus = true;
}

void tryAccessViaCode(const char* code) {
	Serial.print("Checking access for code: ");
	Serial.println(code);
	processApiResponse(api.CodeCanAccess(code));
	api.checkStatus = true;
}

void resetSystem() {
	io.PlayStartupCode();
	delay(100);
	io.Reset();
	api.checkStatus = true; // Just in case it did not reset for some reason ;)
}

void apiConnectionError() {
	io.PlayErrorCode(2);
}

void apiResponseError() {
	io.PlayErrorCode(3);
}

void apiStatusChange(bool locked, bool requirePin) {
	if(!locked) {
		io.Unlock();
		inputProcessor.status = Disabled;
		return;
	}
		
	io.Lock();
	inputProcessor.status = requirePin ? RequirePin : OnlyTag;
}


void readBytes(uint8_t* data, uint8_t bits, const char* message) {
	if(bits == 4) {
		if(data[0] == 0xA || data[0] == 0xB)
		{
			inputProcessor.EscapeButtonClicked();
			return;
		}
		api.checkStatus = false;
		if(!inputProcessor.ButtonClicked(data[0])) {
			api.checkStatus = true;
			io.PlayErrorCode(0);
		}
	} else {
		uint32_t uid = (uint32_t) data[3] << 0
			 | (uint32_t) data[2] << 8
			 | (uint32_t) data[1] << 16
			 | (uint32_t) data[0] << 24;
		char tagId[11];  // max = 10 digits + terminating NUL
		ultoa(uid, tagId, 10);  // convert to base 10
		api.checkStatus = false;
		if(!inputProcessor.TagScanned(tagId)) {
			api.checkStatus = true;
			io.PlayErrorCode(0);
		}
	}
}

void readBytesError(Wiegand::DataError error, uint8_t* rawData, uint8_t rawBits, const char* message) {
	Serial.print(message);
	Serial.print(Wiegand::DataErrorStr(error));
	Serial.print(" - Raw data: ");
	Serial.print(rawBits);
	Serial.print("bits / ");

	uint8_t bytes = (rawBits+7)/8;
	for (int i=0; i<bytes; i++) {
		Serial.print(rawData[i] >> 4, 16);
		Serial.print(rawData[i] & 0xF, 16);
	}
	Serial.println();

	io.PlayErrorCode(0);
}

void stateChanged(bool plugged, const char* message) {
	Serial.print("Reader stat changed to ");
	Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}