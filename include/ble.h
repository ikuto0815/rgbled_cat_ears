#pragma once

#include <Arduino.h>

#include <BLECharacteristic.h>

class Ble {
	private:
		const char *CONFIG_SERVICE_UUID = "5c538829-50d8-4b0b-9b80-fee97a609e1f";
		const char *CHARACTERISTIC_BRIGHTNESS_UUID = "5c538829-50d8-4b0b-9b80-fee97a600000";
		const char *CHARACTERISTIC_COLOR_UUID = "5c538829-50d8-4b0b-9b80-fee97a600001";

		const char *CUSTOM_DESCRIPTOR_UUID = "2b6fface-0815-4454-be5a-334af5c5f118";
		
		void createVariableCharacteristic(BLEService *pService, void *var, size_t size, const char *uuid, const String &descriptor);

	public:
		Ble(void);
		~Ble(void);
};
