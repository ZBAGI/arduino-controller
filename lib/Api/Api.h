#ifndef lib_Api
	#define lib_Api

	#include <Arduino.h>
	#include <SPI.h>
	#include <Ethernet.h>
	#include <ArduinoHttpClient.h>

	typedef void (*OnConnectionErrorCallback) ();
	typedef void (*OnResponseErrorCallback) ();
	typedef void (*OnStatusChangeCallback) (bool locked, bool requirePin);
	enum AccessRequestResponse {
		Allowed,
		ConnectionError,
		ResponseError,
		Denied
	};

	class Api {
		public:
			struct Settings {
				OnConnectionErrorCallback onConnectionError;
				OnResponseErrorCallback onResponseError;
				OnStatusChangeCallback onStatusChange;
				
				byte macAddress[6];
				char const*ip;
				char const*apiKey;
				char const*apiHost;
				int accessPointId;
				int timeout;
			};

			Settings settings;
			Api(Settings settings);
			void Update();
			void Init();
			AccessRequestResponse CodeCanAccess(const String& code);
			AccessRequestResponse TagAndPinCanAccess(const String& tagId, const String& pin);
			AccessRequestResponse TagCanAccess(const String& tagId);
			bool checkStatus = true;
		private:
			int failedRequests = 0;
			int failedConnections = 0;
			unsigned long lastStatusCheck = 0;
			bool locked = true;
			bool requirePin = true;
			AccessRequestResponse Request(String url);
			int RawRequest(String url);
			EthernetClient ethernetClient;
			HttpClient httpClient = HttpClient(ethernetClient, "xxxxxxxxxxxxxxx", 80);
	};
#endif