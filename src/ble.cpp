#include "ble.h"
#include "settings.h"
#include "VisualEffect.h"

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
				CurrentMode = VisualEffect::mode_index((char*)data);
				return;
			}

			/* special handline for the save button */
			if (pCharacteristic->getUUID().equals(BLEUUID(CHARACTERISTIC_SAVE_UUID))) {
				save_settings();
				return;
			}

			/* special handline for the system save button */
			/* restart needed when changing LED settings */
			if (pCharacteristic->getUUID().equals(BLEUUID(CHARACTERISTIC_SYSTEM_SAVE_UUID))) {
				save_settings();
				esp_restart();
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

	/* main config service */
	BLEService *pService = pServer->createService(CONFIG_SERVICE_UUID);

	BLECharacteristic *pCharacteristicServiceName = pService->createCharacteristic(
		CHARACTERISTIC_CONFIG_SERVICE_NAME_UUID,
		BLECharacteristic::PROPERTY_READ);
	BLEDescriptor *serviceNameDescriptor = new BLEDescriptor(CUSTOM_DESCRIPTOR_UUID, 200);
	serviceNameDescriptor->setValue(R"({"type":"serviceName", "order":1})");
	pCharacteristicServiceName->addDescriptor(serviceNameDescriptor);
	pCharacteristicServiceName->setValue("effect settings");

	createVariableCharacteristic(pService, (void*)&max_brightness, sizeof(uint8_t), CHARACTERISTIC_BRIGHTNESS_UUID,
				     R"({"type":"uint8slider", "order":1, "disabled":false, "label":"max brightness", "minInt":0, "maxInt":255, "stepInt":1})");

	createVariableCharacteristic(pService, (void*)&color, sizeof(uint32_t), CHARACTERISTIC_COLOR_UUID,
				     R"({"type":"color", "order":2, "disabled":false, "label":"Color", "alphaSlider":false})");

	String mode_descriptor_value = String(R"({"type":"dropdown", "order":3, "disabled":false, label:"Mode", "options":[)");
	for (int i = 0; i < VisualEffect::mode_count(); i++)
		mode_descriptor_value += String("\"") + modes[i].name + String("\",");
	mode_descriptor_value.remove(mode_descriptor_value.length() - 1);
	mode_descriptor_value += String(R"(]})");

	const char *tmp_current_mode = modes[CurrentMode].name.c_str();
	createVariableCharacteristic(pService, (void*)tmp_current_mode, strlen(tmp_current_mode) + 1, CHARACTERISTIC_MODE_UUID,
				     mode_descriptor_value);

	uint8_t save_dummy;
	createVariableCharacteristic(pService, (void*)&save_dummy, sizeof(uint8_t), CHARACTERISTIC_SAVE_UUID,
				     R"({"type":"button", "order":4, "disabled":false, "label":"Save"})");

	pService->start();

	/* system settings service (LED parameters, maybe more) */

	BLEService *pSystemService = pServer->createService(SYSTEM_CONFIG_SERVICE_UUID);

	BLECharacteristic *pCharacteristicSystemServiceName = pSystemService->createCharacteristic(
		CHARACTERISTIC_CONFIG_SERVICE_NAME_UUID,
		BLECharacteristic::PROPERTY_READ);
	BLEDescriptor *systemServiceNameDescriptor = new BLEDescriptor(CUSTOM_DESCRIPTOR_UUID, 200);
	systemServiceNameDescriptor->setValue(R"({"type":"serviceName", "order":1})");
	pCharacteristicSystemServiceName->addDescriptor(systemServiceNameDescriptor);
	pCharacteristicSystemServiceName->setValue("system settings");

	createVariableCharacteristic(pSystemService, (void*)&led_strip_len, sizeof(uint16_t), CHARACTERISTIC_LED_STRIP_LEN_UUID,
				     R"({"type":"uint16", "order":1, "disabled":false, "label":"LED strip length"})");

	createVariableCharacteristic(pSystemService, (void*)&led_offset_left, sizeof(uint16_t), CHARACTERISTIC_LED_OFFSET_LEFT_UUID,
				     R"({"type":"uint16", "order":1, "disabled":false, "label":"LED left ear offset"})");

	createVariableCharacteristic(pSystemService, (void*)&led_ear_len, sizeof(uint16_t), CHARACTERISTIC_LED_EAR_LEN_UUID,
				     R"({"type":"uint16", "order":1, "disabled":false, "label":"LED ear length"})");

	createVariableCharacteristic(pSystemService, (void*)&led_middle_gap, sizeof(uint16_t), CHARACTERISTIC_LED_MIDDLE_GAP_UUID,
				     R"({"type":"uint16", "order":1, "disabled":false, "label":"LED middle gap"})");

	createVariableCharacteristic(pSystemService, (void*)&save_dummy, sizeof(uint8_t), CHARACTERISTIC_SYSTEM_SAVE_UUID,
				     R"({"type":"button", "order":4, "disabled":false, "label":"Save"})");

	pSystemService->start();

	BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
	pAdvertising->addServiceUUID(CONFIG_SERVICE_UUID);
	pAdvertising->addServiceUUID(SYSTEM_CONFIG_SERVICE_UUID);
	pAdvertising->start();
}

Ble::~Ble(void)
{

}
