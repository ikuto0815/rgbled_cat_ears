#include "ble.h"
#include "settings.h"
#include "VisualEffectGlobal.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#include <string>

class ServerCallbacks : public BLEServerCallbacks {
	public: 
		bool deviceConnected = false;

		void onConnect(BLEServer *pServer)
		{
			deviceConnected = true;
			Serial.println("connected");
		};

		void onDisconnect(BLEServer *pServer)
		{
			deviceConnected = false;
			Serial.println("disconnected");
			// restart advertising after disconnecting
			pServer->startAdvertising();
		}
};

class Callbacks : public BLECharacteristicCallbacks {
	private:
		void *_data;

	public:
		Callbacks(void *__data)
		{
			_data = __data;
		}

		void onWrite(BLECharacteristic *pCharacteristic) override
		{
			const uint8_t *data = pCharacteristic->getData();
			size_t dataLength = pCharacteristic->getLength();

			/* special handling for the mode selection */
			if (pCharacteristic->getUUID().equals(BLEUUID(CHARACTERISTIC_MODE_UUID))) {
				CurrentMode = mode_index((char*)data);
				return;
			}

			if (dataLength > 0)
				memcpy(_data, data, dataLength);
			else
				Serial.println("Empty value received!");
		}
};

void Ble::createVariableCharacteristic(BLEService *pService, void *var, size_t size, const char *uuid, const String &descriptor)
{
	BLECharacteristic *c = pService->createCharacteristic(uuid,
							      BLECharacteristic::PROPERTY_READ |
							      BLECharacteristic::PROPERTY_WRITE);
	c->setCallbacks(new Callbacks(var));

	BLEDescriptor *serviceNameDescriptor = new BLEDescriptor(CUSTOM_DESCRIPTOR_UUID, 200);
	serviceNameDescriptor->setValue(descriptor);
	c->addDescriptor(serviceNameDescriptor);

	c->setValue((uint8_t*)var, size);
}

Ble::Ble(void)
{
	BLEDevice::init("RGBLED cat ears");
	BLEServer *pServer = BLEDevice::createServer();

	pServer->setCallbacks(new ServerCallbacks());
	BLEService *pService = pServer->createService(CONFIG_SERVICE_UUID);


	createVariableCharacteristic(pService, (void*)&max_brightness, sizeof(uint8_t), CHARACTERISTIC_BRIGHTNESS_UUID,
				     R"({"type":"uint8slider", "order":1, "disabled":false, "label":"max brightness", "minInt":0, "maxInt":255, "stepInt":1})"); 

	createVariableCharacteristic(pService, (void*)&color, sizeof(uint32_t), CHARACTERISTIC_COLOR_UUID,
				     R"({"type":"color", "order":2, "disabled":false, "label":"Color", "alphaSlider":true})");
				     
	String mode_descriptor_value = String(R"({"type":"dropdown", "order":1, "disabled":false, label:"Mode", "options":[)");
	for (int i = 0; i < mode_count(); i++)
		mode_descriptor_value += String("\"") + modes[i].name + String("\",");
	mode_descriptor_value.remove(mode_descriptor_value.length() - 1);
	mode_descriptor_value += String(R"(]})");

	const char *tmp_current_mode = modes[CurrentMode].name.c_str();
	createVariableCharacteristic(pService, (void*)tmp_current_mode, strlen(tmp_current_mode) + 1, CHARACTERISTIC_MODE_UUID,
				     mode_descriptor_value);
	pService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(CONFIG_SERVICE_UUID);
	pAdvertising->start();
}

Ble::~Ble(void)
{

}
