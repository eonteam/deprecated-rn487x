#ifndef __RN487X
#define __RN487X

#include "eonOS.h"
#include "rn487x_defines.h"
#include <stdbool.h>
#include <string.h>

/** 
 ===============================================================================
            ##### Macro definitions #####
 ===============================================================================
 */
#define RN487X_DEFAULT_BAUDRATE 115200

typedef struct
{
  uint16_t index;
  uint16_t length;
} ble_charact_t;

// Advertising types
#define BLE_ADTYPE_FLAGS 0x01
#define BLE_ADTYPE_INCOMPLETE_16_UUID 0x02
#define BLE_ADTYPE_COMPLETE_16_UUID 0x03
#define BLE_ADTYPE_INCOMPLETE_32_UUID 0x04
#define BLE_ADTYPE_COMPLETE_32_UUID 0x05
#define BLE_ADTYPE_INCOMPLETE_128_UUID 0x06
#define BLE_ADTYPE_COMPLETE_128_UUID 0x07
#define BLE_ADTYPE_SHORTENED_LOCAL_NAME 0x08
#define BLE_ADTYPE_COMPLETE_LOCAL_NAME 0x09
#define BLE_ADTYPE_TX_POWER_LEVEL 0x0A
#define BLE_ADTYPE_CLASS_OF_DEVICE 0x0D
#define BLE_ADTYPE_SIMPLE_PAIRING_HASH 0x0E
#define BLE_ADTYPE_SIMPLE_PAIRING_RANDOMIZER 0x0F
#define BLE_ADTYPE_TK_VALUE 0x10
#define BLE_ADTYPE_SECURITY_OOB_FLAG 0x11
#define BLE_ADTYPE_SLAVE_CONNECTION_INTERVAL 0x12
#define BLE_ADTYPE_LIST_16_SERVICE_UUID 0x14
#define BLE_ADTYPE_LIST_128_SERVICE_UUID 0x15
#define BLE_ADTYPE_SERVICE_DATA 0x16
#define BLE_ADTYPE_MANUFACTURE_SPECIFIC_DATA 0xFF

// BLE Properties
#define BLE_PROPERTY_INDICATE 0b00100000
#define BLE_PROPERTY_NOTIFY 0b00010000
#define BLE_PROPERTY_WRITE 0b00001000
#define BLE_PROPERTY_WRITE_NO_RESP 0b00000100
#define BLE_PROPERTY_READ 0b00000010

/** 
 ===============================================================================
            ##### Functions #####
 ===============================================================================
 */

// Init and general functions

bool rn487x_init(void);
void rn487x_hwReset(void);
void rn487x_hwWakeUp(void);
void nr487x_hwSleep(void);
bool rn487x_setSerializedName(const char *newName);
bool rn487x_setDeviceName(const char *dName);
bool rn487x_reboot(void);
int8_t rn487x_getConnectionStatus(void);

// Modes

bool rn487x_dataMode(void);
bool rn487x_cmdMode(void);

// Advertisements

bool rn487x_setAdvPower(uint8_t value);
bool rn487x_setConnPower(uint8_t value);
bool rn487x_stopAdvertising(void);
bool rn487x_clearImmediateAdvertising(void);
bool rn487x_startImmediateAdvertising(uint8_t advType, const uint8_t *advData, size_t size);

// Send command

void rn487x_sendCommand(const char *cmd);

// Services

bool rn487x_deviceService_setManufName(const char *name);
bool rn487x_setServiceUUID(const char *uuid);
bool rn487x_clearAllServices(void);

// Characteristics

bool rn487x_setCharactUUID(ble_charact_t *bc, const char *uuid, uint8_t property, uint8_t octetLen);
bool rn487x_writeLocalCharact(ble_charact_t *bc, const uint8_t *value);
int8_t rn487x_readLocalCharact(ble_charact_t *bc, uint8_t *vbuff);
bool rn487x_buildCharacts(void);

// privates

// int uartBufferLen;
// char endStreamChar;
// bool connectionStatus;
// char btAddress[12];
// char peerAddress[6];
// char deviceName[MAX_DEVICE_NAME_LEN];
// uint8_t whiteListCnt;

#endif
