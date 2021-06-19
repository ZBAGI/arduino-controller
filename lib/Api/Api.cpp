#include "Api.h"

Api::Api(Settings settings)
{
    this->settings = settings;
}

void Api::Init() {
    IPAddress ip = IPAddress();
	ip.fromString(this->settings.ip);
	Ethernet.begin((uint8_t *)this->settings.macAddress, ip);

	if (Ethernet.hardwareStatus() == EthernetNoHardware)
	{
        Serial.println("No ethernet hardware detected");
		while (true)
		{
			digitalWrite(LED_BUILTIN, HIGH);
            delay(20);
            digitalWrite(LED_BUILTIN, LOW);
            delay(20);
		}
	}

	ethernetClient.setTimeout((this->settings.timeout/2));
	ethernetClient.setConnectionTimeout((this->settings.timeout/2));

	this->httpClient.setTimeout((this->settings.timeout/2));
	this->httpClient.setHttpResponseTimeout((this->settings.timeout/2));

	Serial.print("Selected IP: ");
	Serial.println(Ethernet.localIP());

	delay(1000);
}

void Api::Update() {
    if(this->lastStatusCheck == 0 || (this->lastStatusCheck + 20000) < millis()) {
        this->lastStatusCheck = millis();

        if(!this->checkStatus) {
            Serial.println("Skipped status check");
            if((this->lastStatusCheck + 120000) < millis())
                this->checkStatus = true; // something is wrong, status check should not be stopped for more then 2 min;
            return;
        }

        Serial.println("Updating status...");
        char url[100];
        snprintf(url, sizeof(url), "/v1/access-points/%d", this->settings.accessPointId);
        int status = this->RawRequest(url);
        Serial.print("Request Status:");
        Serial.println(status);
        delay(100);
        if(status == HTTP_SUCCESS) {
            this->failedConnections = 0;
            int responseCode = httpClient.responseStatusCode();
            String responseBody = httpClient.responseBody();

            Serial.print("Response Code:");
            Serial.println(responseCode);
            Serial.print("Response body:" + responseBody);
            Serial.println(responseBody);


            if(responseCode != 200) {
                Serial.println("Skipping due to response not being 200 OK");
                if(this->failedRequests >= 5) {
                    this->settings.onResponseError();
                } else {
                    this->failedRequests += 1;
                }
                return;
            }
		    this->failedRequests = 0;

            bool webLockStatus = responseBody.indexOf("\"locked\":true") != -1;
            bool webRequirePinStatus = responseBody.indexOf("\"requirePin\":true") != -1;
            if(webLockStatus != this->locked || this->requirePin != webRequirePinStatus) {
                this->locked = webLockStatus;
                this->requirePin = webRequirePinStatus;
                this->settings.onStatusChange(this->locked, this->requirePin);
            }
        } else {
            if(this->failedConnections >= 5) {
                this->settings.onConnectionError();
            } else {
                this->failedConnections += 1;
            }
        }
    }
}

AccessRequestResponse Api::CodeCanAccess(const char* code) {
    char url[100];
    snprintf(url, sizeof(url), "/v1/access-points/can-access()\?accessPointId=%d&code=%s", this->settings.accessPointId, code);
    Serial.println("checking:");
    Serial.println(url);
    return this->Request(url);
}

AccessRequestResponse Api::TagAndPinCanAccess(const char* tagId, const char* pin) {
    char url[100];
    snprintf(url, sizeof(url), "/v1/access-points/can-access()\?accessPointId=%d&tagId=%s&pin=%s", this->settings.accessPointId, tagId, pin);
    Serial.println("checking:");
    Serial.println(url);
    return this->Request(url);
}

AccessRequestResponse Api::TagCanAccess(const char* tagId) {
    char url[100];
    snprintf(url, sizeof(url), "/v1/access-points/can-access()\?accessPointId=%d&tagId=%s", this->settings.accessPointId, tagId);
    Serial.println("checking:");
    Serial.println(url);
    return this->Request(url);
}

AccessRequestResponse Api::Request(const char* url) {
    Serial.print("access requesting: ");
    Serial.println(url);

	int status = this->RawRequest(url);
    Serial.println("Request Status:");
	Serial.println(status);
	if(status == HTTP_SUCCESS) {
		int responseCode = httpClient.responseStatusCode();
		String responseBody = httpClient.responseBody();

		Serial.print("Response Code:");
		Serial.println(responseCode);
        Serial.print("Response body:");
		Serial.println(responseBody);

		if(responseBody.equalsIgnoreCase("True"))
			return Allowed;
		else
			return Denied;
	} else {
		return ConnectionError;
	}
}

int Api::RawRequest(const char* url) {
    Serial.print("raw Requesting: ");
    Serial.println(url);
    httpClient.beginRequest();
	int status = httpClient.get(url);
	httpClient.sendHeader("api-key", this->settings.apiKey);
	httpClient.beginBody();
	httpClient.endRequest();

    return status;
}