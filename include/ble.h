#pragma once

#include <Arduino.h>

#include <BLECharacteristic.h>

#define CONFIG_SERVICE_UUID "5c538829-50d8-4b0b-9b80-fee97a609e1f"

#define CHARACTERISTIC_CONFIG_SERVICE_NAME_UUID "5c538829-50d8-4b0b-9b80-fee97a6fffff"
#define CHARACTERISTIC_BRIGHTNESS_UUID "5c538829-50d8-4b0b-9b80-fee97a600000"
#define CHARACTERISTIC_COLOR_UUID "5c538829-50d8-4b0b-9b80-fee97a600001"
#define CHARACTERISTIC_MODE_UUID "5c538829-50d8-4b0b-9b80-fee97a600002"

#define CUSTOM_DESCRIPTOR_UUID "2b6fface-0815-4454-be5a-334af5c5f118"

class Ble {
	private:
		
		void createVariableCharacteristic(BLEService *pService, void *var, size_t size, const char *uuid, const String &descriptor);

	public:
		Ble(void);
		~Ble(void);
};
