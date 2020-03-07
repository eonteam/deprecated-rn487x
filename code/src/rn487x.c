#include "rn487x.h"
#include "rn487x_const.h"

/** 
 ===============================================================================
            ##### Private macros #####
 ===============================================================================
*/
#ifdef RN487X_DEBUG
#define DEBUG_PRINTLN uart2_println
#define DEBUG_PRINT uart2_print
#else
#define DEBUG_PRINTLN(__X__)
#define DEBUG_PRINT(__X__)
#endif

#define _IS_HEXCOMMA(__x__) (((__x__) >= 'A' && (__x__) <= 'F') || ((__x__) >= '0' && (__x__) <= '9') || ((__x__) == ','))
#define _IS_ENDWORD(__x__) ((__x__) == 'E' || (__x__) == 'N' || (__x__) == 'D')
#define HANDLE_POS 33 // 32 from UUID, 1 from comma
#define PROPERTY_POS (HANDLE_POS + 5)

#define UART_BUFF_LEN 500
#define CMD_MODE 1
#define DATA_MODE 0

/** 
 ===============================================================================
            ##### Private variables #####
 ===============================================================================
*/
static char _uart_buffer[UART_BUFF_LEN] = {0};
static char _device_name[MAX_DEVICE_NAME_LEN] = {0};
static uint16_t _charact_handles[BLE_MAX_NUMBER_OF_CHARACTERISTICS] = {0};
static uint8_t _operation_mode = DATA_MODE;
static uint16_t _charact_id_cnt = 0;

/** 
 ===============================================================================
            ##### Private functions #####
 ===============================================================================
 */

// ------------------------------------------------------------
// Clear hardware input uart buffer
// ------------------------------------------------------------
static void _serialFlush(void)
{
  while (BLE_SERIAL_AVAILABLE() > 0)
  {
    BLE_SERIAL_READ();
  }
}

// ------------------------------------------------------------
// Clear private buffer
// ------------------------------------------------------------
static void _clearBuffer(void)
{
  memset(_uart_buffer, 0, UART_BUFF_LEN);
}

// ------------------------------------------------------------
// Reads a buffer until the carriage return
// ------------------------------------------------------------
static uint16_t _readUntilCR(uint16_t timeout)
{
  uint16_t i = 0;
  int c = 0;
  unsigned long previous;
  previous = millis();
  while (c != CR && i < UART_BUFF_LEN && (millis() - previous < timeout))
  {
    if (BLE_SERIAL_AVAILABLE())
    {
      c = BLE_SERIAL_READ();
      if (c == CR)
        return i;
      _uart_buffer[i] = c;
      i++;
    }
  }
  return i;
}

// ------------------------------------------------------------
// Get the current operation mode
// ------------------------------------------------------------
// static int _getOperationMode(void) {
//   return _operation_mode;
// }

// ------------------------------------------------------------
// Check if response is equals to the one specify
// ------------------------------------------------------------
static bool _expectResponse(const char *expected_response, uint16_t timeout)
{
  DEBUG_PRINT("  => expectedResponse: ");
  DEBUG_PRINTLN(expected_response);
  _clearBuffer();
  if (_readUntilCR(timeout) > 0)
  {
    DEBUG_PRINT("  => Received (");
    DEBUG_PRINT(_uart_buffer);
    DEBUG_PRINT(")");
    if (strstr(_uart_buffer, expected_response) != NULL)
    {
      DEBUG_PRINTLN(" found match!");
      return true;
    }
    DEBUG_PRINTLN(" not match...!");
    return false;
  }
  DEBUG_PRINTLN("  => TIMEOUT!");
  return false;
}

// ------------------------------------------------------------
// Convert decimal number to hex buffer
// ------------------------------------------------------------
static void _decToHex(uint16_t dec, uint8_t *hexBuff, uint8_t hexBuffLen)
{
  uint16_t quotient = dec;
  uint16_t remainder = 0;
  uint8_t i = 0;
  uint8_t j = hexBuffLen - 1;
  memset(hexBuff, 0, hexBuffLen);
  while (quotient != 0 && j >= 0)
  {
    remainder = quotient % 16;
    if (remainder < 10)
      hexBuff[j--] = 48 + remainder;
    else
      hexBuff[j--] = 55 + remainder;
    quotient = quotient / 16;
    i++;
  }
  for (j = 0; j < (hexBuffLen - i); j++)
  {
    hexBuff[j] = '0';
  }
}

// ------------------------------------------------------------
// Value string to number
// ------------------------------------------------------------
static uint16_t _valueStrToNum(const char *buff, uint8_t size)
{
  uint16_t val = 0;
  for (uint8_t i = 0; i < size; i++)
  {
    uint8_t byte = buff[i];
    if (byte >= '0' && byte <= '9')
      byte = byte - '0';
    else if (byte >= 'A' && byte <= 'F')
      byte = byte - 'A' + 10;
    val = (val << 4) | (byte & 0xF);
  }
  return val;
}

// ------------------------------------------------------------
// Hex digit to decimal digit
// ------------------------------------------------------------
static uint8_t _hexDigitToDec(char hexDigit)
{
  if ((hexDigit >= '0') && (hexDigit <= '9'))
  {
    return (hexDigit - '0');
  }
  if ((hexDigit >= 'A') && (hexDigit <= 'F'))
  {
    return (hexDigit - 'A' + 10);
  }
  return 0;
}

/** 
 ===============================================================================
            ##### Public functions #####
 ===============================================================================
 */

// ------------------------------------------------------------
// Send a command
// ------------------------------------------------------------
void rn487x_sendCommand(const char *cmd)
{
  DEBUG_PRINT(" => sendCommand: ");
  DEBUG_PRINTLN(cmd);

  BLE_SERIAL_PRINT(cmd);
  // This should be after print the command 'cause there are commands
  // that use _uart_buffer to be generated
  _clearBuffer();
  _serialFlush();
  BLE_SERIAL_PRINT("\r");
}

// ------------------------------------------------------------
// Send data
// ------------------------------------------------------------
void rn487x_sendData(char *data, uint16_t dataLen)
{
  for (uint8_t i = 0; i < dataLen; i++)
  {
    BLE_SERIAL_WRITE(data[i]);
  }
}

// ------------------------------------------------------------
// Hardware reset
// ------------------------------------------------------------
void rn487x_hwReset(void)
{
  DEBUG_PRINTLN("[info] hwReset");
  gpio_mode(RN487X_RESET_PIN, OUTPUT_PP, NOPULL, SPEED_HIGH);
  gpio_reset(RN487X_RESET_PIN);
  delay(5);
  gpio_set(RN487X_RESET_PIN);
  delay(500);
}

// ------------------------------------------------------------
// Hardware wake up (available only in RN4871)
// ------------------------------------------------------------
void rn487x_hwWakeUp(void)
{
#ifdef RN487X_WAKE_PIN
  DEBUG_PRINTLN("[info] wakeUp");
  gpio_mode(RN487X_WAKE_PIN, OUTPUT_PP, NOPULL, SPEED_HIGH);
  gpio_reset(RN487X_WAKE_PIN);
  delay(5);
#endif
}

// ------------------------------------------------------------
// Reboot the module
// ------------------------------------------------------------
bool rn487x_reboot(void)
{
  DEBUG_PRINTLN("[info] reboot");
  rn487x_sendCommand(REBOOT);
  if (_expectResponse(REBOOTING_RESP, RESET_CMD_TIMEOUT))
  {
    delay(RESET_CMD_TIMEOUT);
    return true;
  }
  return false;
}

// ------------------------------------------------------------
// Initialization
// ------------------------------------------------------------
bool rn487x_init(void)
{
  DEBUG_PRINTLN("[info] init");
  rn487x_hwReset();
  rn487x_hwWakeUp();
  _clearBuffer();
  _serialFlush();
  if (rn487x_reboot())
  {
    _operation_mode = DATA_MODE;
    return true;
  }
  if (rn487x_cmdMode())
  {
    if (rn487x_reboot())
    {
      _operation_mode = DATA_MODE;
      return true;
    }
  }
  return false;
}

// ------------------------------------------------------------
// Enter into command mode
// ------------------------------------------------------------
bool rn487x_cmdMode(void)
{
  DEBUG_PRINTLN("[info] commandMode");
  delay(DELAY_BEFORE_CMD);
  _serialFlush();
  _clearBuffer();
  BLE_SERIAL_PRINT(ENTER_CMD);
  if (_expectResponse(PROMPT, DEFAULT_CMD_TIMEOUT))
  {
    _operation_mode = CMD_MODE;
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Set Serialized Device Name
// ------------------------------------------------------------------
bool rn487x_setSerializedName(const char *newName)
{
  DEBUG_PRINT("[info] setSerializedName: ");
  DEBUG_PRINTLN(newName);

  uint8_t cmdLen = strlen(SET_SERIALIZED_NAME);
  uint8_t nameLen = strlen(newName);
  if (nameLen > MAX_SERIALIZED_NAME_LEN)
  {
    nameLen = MAX_SERIALIZED_NAME_LEN;
    DEBUG_PRINTLN("[warn] Too many characters, name truncated");
  }

  // Fill the device name without the last two bytes of the Bluetooth MAC address
  memset(_device_name, 0, nameLen);
  memcpy(_device_name, newName, nameLen);
  // Fill the buffer
  _clearBuffer();
  memcpy(_uart_buffer, SET_SERIALIZED_NAME, cmdLen);
  memcpy(&_uart_buffer[cmdLen], newName, nameLen);
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Set Device Name
// ------------------------------------------------------------------
bool rn487x_setDeviceName(const char *dName)
{
  DEBUG_PRINT("[info] setDeviceName: ");
  DEBUG_PRINTLN(dName);
  uint8_t cmdLen = strlen(SET_DEVICE_NAME);
  uint8_t nameLen = strlen(dName);
  if (nameLen > MAX_DEVICE_NAME_LEN)
  {
    nameLen = MAX_DEVICE_NAME_LEN;
    DEBUG_PRINTLN("[warn] Too many characters, name truncated");
  }

  memset(_device_name, 0, nameLen);
  memcpy(_device_name, dName, nameLen);
  _clearBuffer();
  memcpy(_uart_buffer, SET_DEVICE_NAME, cmdLen);
  memcpy(&_uart_buffer[cmdLen], dName, nameLen);
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ---------------------------------------------------------------------------------
// Get the current connection status
// ---------------------------------------------------------------------------------
// If the RN4870/71 is not connected, the output is none.
// If the RN4870/71 is connected, the buffer must contains the information:
// <Peer BT Address>,<Address Type>,<Connection Type>
// where <Peer BT Address> is the 6-byte hex address of the peer device;
//       <Address Type> is either 0 for public address or 1 for random address;
//       <Connection Type> specifies if the connection enables UART Transparent
//                         feature [1: enabled, 0: disabled]
// ---------------------------------------------------------------------------------
int8_t rn487x_getConnectionStatus(void)
{
  DEBUG_PRINTLN("[info] getConnectionStatus");

  rn487x_sendCommand(GET_CONNECTION_STATUS);
  _clearBuffer();
  if (_readUntilCR(DEFAULT_CMD_TIMEOUT) > 0)
  {
    // Check for the connection
    if (strstr(_uart_buffer, NONE_RESP) != NULL)
    {
      return 0; // Not connected
    }
    return 1; // Connected
  }
  DEBUG_PRINTLN("[warn] getConnectionStatus: Timeout without a response !");
  return -1;
}

/********************** Advertisements ******************************/

// ------------------------------------------------------------------
// Adjust the output power under advertisement [ Range from (0-5) ]
// ------------------------------------------------------------------
bool rn487x_setAdvPower(uint8_t value)
{
  DEBUG_PRINTLN("[info] setAdvPower");

  if (value > MAX_POWER_OUTPUT)
  {
    value = MAX_POWER_OUTPUT;
  }
  uint8_t len = strlen(SET_ADV_POWER);
  _clearBuffer();
  memcpy(_uart_buffer, SET_ADV_POWER, len);
  _uart_buffer[len] = value + '0'; // convert to a string
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Adjust the output power under connected state [ Range from (0-5) ]
// ------------------------------------------------------------------
bool rn487x_setConnPower(uint8_t value)
{
  DEBUG_PRINTLN("[info] setConnPower");

  if (value > MAX_POWER_OUTPUT)
  {
    value = MAX_POWER_OUTPUT;
  }
  uint8_t len = strlen(SET_CONN_POWER);
  memcpy(_uart_buffer, SET_CONN_POWER, len);
  _uart_buffer[len] = value + '0'; // convert to a string
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------
// Stops Advertisement
// ------------------------------------------------------------
bool rn487x_stopAdvertising(void)
{
  DEBUG_PRINTLN("[info] stopAdvertising");
  rn487x_sendCommand(STOP_ADV);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Clear the advertising structure Immediately
// ------------------------------------------------------------------
bool rn487x_clearImmediateAdvertising(void)
{
  DEBUG_PRINTLN("[info] clearImmediateAdvertising");

  rn487x_sendCommand(CLEAR_IMMEDIATE_ADV);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Start Advertising immediately
// ------------------------------------------------------------------
bool rn487x_startImmediateAdvertising(uint8_t advType, const uint8_t *advData, size_t size)
{
  DEBUG_PRINTLN("[info] startImmediateAdvertising");

  uint8_t cmdLen = strlen(START_IMMEDIATE_ADV);
  uint8_t hexBuff[2] = {0};
  _decToHex(advType, hexBuff, 2);

  _clearBuffer();
  memcpy(_uart_buffer, START_IMMEDIATE_ADV, cmdLen);
  _uart_buffer[cmdLen] = hexBuff[0];
  _uart_buffer[cmdLen + 1] = hexBuff[1];
  _uart_buffer[cmdLen + 2] = ',';
  for (uint8_t i = 0, j = 0; i < size; i++, j += 2)
  {
    _decToHex(advData[i], hexBuff, 2);
    _uart_buffer[cmdLen + 3 + j] = hexBuff[0];
    _uart_buffer[cmdLen + 3 + j + 1] = hexBuff[1];
  }
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

/**************************** Services *********************************/

// ----------------------------------------------------------------------
// Clears all settings of services and characteristics.
// A power cycle is required afterwards to make the changes effective.
// ----------------------------------------------------------------------
bool rn487x_clearAllServices(void)
{
  DEBUG_PRINTLN("[info] cleanAllServices");

  rn487x_sendCommand(CLEAR_ALL_SERVICES);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ------------------------------------------------------------------
// Set Manufacturer Name in Device Info Service
// ------------------------------------------------------------------
bool rn487x_deviceService_setManufName(const char *name)
{
  DEBUG_PRINT("[info] deviceService_setManufName: ");
  DEBUG_PRINTLN(name);
  uint8_t cmdLen = strlen(SET_MANUF_NAME);
  uint8_t nameLen = strlen(name);
  if (nameLen > MAX_SERIALIZED_NAME_LEN)
  {
    nameLen = MAX_SERIALIZED_NAME_LEN;
    DEBUG_PRINTLN("[warn] Too many characters, name truncated");
  }

  memset(_device_name, 0, nameLen);
  memcpy(_device_name, name, nameLen);
  _clearBuffer();
  memcpy(_uart_buffer, SET_MANUF_NAME, cmdLen);
  memcpy(&_uart_buffer[cmdLen], name, nameLen);
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
// Sets the UUID of the public (16-bit) or the private (128-bit) service.
// This method must be called before the setCharactUUID() method.
// ----------------------------------------------------------------------
bool rn487x_setServiceUUID(const char *uuid)
{
  DEBUG_PRINT("[info] serServiceUUID: ");
  DEBUG_PRINTLN(uuid);

  uint8_t cmdLen = strlen(DEFINE_SERVICE_UUID);
  uint8_t uuidLen = strlen(uuid);

  if (uuidLen == PRIVATE_SERVICE_LEN)
  {
    DEBUG_PRINTLN("[info] 128-bit length (private service)");
  }
  else if (uuidLen == PUBLIC_SERVICE_LEN)
  {
    DEBUG_PRINTLN("[info] 16-bit length (public service)");
  }
  else
  {
    DEBUG_PRINTLN("[error] UUID length is not correct");
    return false;
  }

  _clearBuffer();
  memcpy(_uart_buffer, DEFINE_SERVICE_UUID, cmdLen);
  memcpy(&_uart_buffer[cmdLen], uuid, uuidLen);
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

/************************ Characteristics ******************************/

// ----------------------------------------------------------------------
// Sets the characteritics UUID
// This method must be called after the setServiceUUID() method.
// ----------------------------------------------------------------------
bool rn487x_setCharactUUID(ble_charact_t *bc, const char *uuid, uint8_t property, uint8_t octetLen)
{
  DEBUG_PRINT("[info] setCharactUUID: ");
  DEBUG_PRINTLN(uuid);

  if (octetLen < 0x01)
  {
    octetLen = 0x01;
    DEBUG_PRINTLN("[warn] Octet Length is out of range (0x01-0x14)");
  }
  else if (octetLen > 0x14)
  {
    octetLen = 0x14;
    DEBUG_PRINTLN("[warn] Octet Length is out of range (0x01-0x14)");
  }

  uint8_t cmdLen = strlen(DEFINE_CHARACT_UUID);
  uint8_t uuidLen = strlen(uuid);

  if (uuidLen == PRIVATE_SERVICE_LEN)
  {
    DEBUG_PRINTLN("[info] 128-bit length (private characteristic)");
  }
  else if (uuidLen == PUBLIC_SERVICE_LEN)
  {
    DEBUG_PRINTLN("[info] 16-bit length (public characteristic)");
  }
  else
  {
    DEBUG_PRINTLN("[error] UUID length is not correct");
    return false;
  }

  uint8_t hexBuff[2] = {0};

  _clearBuffer();
  memcpy(_uart_buffer, DEFINE_CHARACT_UUID, cmdLen);
  memcpy(&_uart_buffer[cmdLen], uuid, uuidLen);
  _uart_buffer[cmdLen + uuidLen] = ',';
  // property
  _decToHex(property, hexBuff, 2);
  _uart_buffer[cmdLen + uuidLen + 1] = hexBuff[0];
  _uart_buffer[cmdLen + uuidLen + 2] = hexBuff[1];
  _uart_buffer[cmdLen + uuidLen + 3] = ',';
  // octetLen
  _decToHex(octetLen, hexBuff, 2);
  _uart_buffer[cmdLen + uuidLen + 4] = hexBuff[0];
  _uart_buffer[cmdLen + uuidLen + 5] = hexBuff[1];

  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    bc->index = _charact_id_cnt;
    bc->length = octetLen;
    _charact_id_cnt++;
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
// Write local characteristic value as server
// ----------------------------------------------------------------------
bool rn487x_writeLocalCharact(ble_charact_t *bc, const uint8_t *value)
{
  DEBUG_PRINTLN("[info] writeLocalCharacteristic");

  uint16_t handle = _charact_handles[bc->index];
  uint8_t cmdLen = strlen(WRITE_LOCAL_CHARACT);
  uint8_t hexBuff[4] = {0};
  _decToHex(handle, hexBuff, 4);

  _clearBuffer();
  memcpy(_uart_buffer, WRITE_LOCAL_CHARACT, cmdLen);
  _uart_buffer[cmdLen] = hexBuff[0];
  _uart_buffer[cmdLen + 1] = hexBuff[1];
  _uart_buffer[cmdLen + 2] = hexBuff[2];
  _uart_buffer[cmdLen + 3] = hexBuff[3];
  _uart_buffer[cmdLen + 4] = ',';
  for (uint8_t i = 0, j = 0; i < bc->length; i++, j += 2)
  {
    _decToHex(value[i], hexBuff, 2);
    _uart_buffer[cmdLen + 5 + j] = hexBuff[0];
    _uart_buffer[cmdLen + 5 + j + 1] = hexBuff[1];
  }
  rn487x_sendCommand(_uart_buffer);
  if (_expectResponse(AOK_RESP, DEFAULT_CMD_TIMEOUT))
  {
    return true;
  }
  return false;
}

// ----------------------------------------------------------------------
// Read local characteristic value as server
// ----------------------------------------------------------------------
int8_t rn487x_readLocalCharact(ble_charact_t *bc, uint8_t *vbuff)
{
  DEBUG_PRINTLN("[info] readLocalCharact");

  uint16_t handle = _charact_handles[bc->index];
  uint8_t cmdLen = strlen(READ_LOCAL_CHARACT);
  uint8_t hexBuff[4] = {0};
  _decToHex(handle, hexBuff, 4);

  _clearBuffer();
  memcpy(_uart_buffer, READ_LOCAL_CHARACT, cmdLen);
  _uart_buffer[cmdLen] = hexBuff[0];
  _uart_buffer[cmdLen + 1] = hexBuff[1];
  _uart_buffer[cmdLen + 2] = hexBuff[2];
  _uart_buffer[cmdLen + 3] = hexBuff[3];
  rn487x_sendCommand(_uart_buffer);

  uint8_t dataLen = _readUntilCR(DEFAULT_CMD_TIMEOUT);
  memset(vbuff, 0, bc->length);
  if (dataLen == 0)
  {
    DEBUG_PRINTLN("=> Error TIMEOUT");
    return -2;
  }
  if (dataLen == 3)
  {
    if (_uart_buffer[0] == 'N' && _uart_buffer[1] == '/' && _uart_buffer[2] == 'A')
    {
      DEBUG_PRINTLN(" => No data to show");
      return 0;
    }
    else if (_uart_buffer[0] == 'E' && _uart_buffer[1] == 'R' && _uart_buffer[2] == 'R')
    {
      DEBUG_PRINTLN(" => Error from the module");
      return -1;
    }
  }
  if (dataLen != (2 * bc->length))
  {
    DEBUG_PRINTLN(" => Error invalid len");
    return -1;
  }
  for (uint8_t i = 0, j = 0; i < bc->length; i++, j += 2)
  {
    uint8_t d1 = _hexDigitToDec(_uart_buffer[j]) & 0xF;
    uint8_t d2 = _hexDigitToDec(_uart_buffer[j + 1]) & 0xF;
    vbuff[i] = (d1 << 4) | d2;
  }
  return 1;
}

bool rn487x_buildCharacts(void)
{
  uint16_t i = 0;
  uint16_t j = 0;
  DEBUG_PRINTLN("[info] buildCharacts");

  rn487x_sendCommand(LIST_CHARACTERISTICS);
  uint32_t start = millis();
  _clearBuffer();
  while (millis() - start < LIST_CMD_TIMEOUT)
  {
    if (BLE_SERIAL_AVAILABLE())
    {
      char c = BLE_SERIAL_READ();
      if (_IS_HEXCOMMA(c) || _IS_ENDWORD(c))
      {
        _uart_buffer[i] = c;
        i++;
      }

      if (c == CR)
      {
        if (strstr(_uart_buffer, "END") != NULL)
        {
          return true;
        }
        if (i >= (PROPERTY_POS + 2))
        {
          uint8_t prop = _valueStrToNum(&_uart_buffer[PROPERTY_POS], 2);
          if ((prop & (BLE_PROPERTY_INDICATE | BLE_PROPERTY_NOTIFY)) == 0)
          {
            _charact_handles[j] = _valueStrToNum(&_uart_buffer[HANDLE_POS], 4);
            j++;
          }
          if (j > (BLE_MAX_NUMBER_OF_CHARACTERISTICS - 1))
          {
            DEBUG_PRINTLN("[error] Number of characteristics overflowed");
            return false;
          }
        }
        i = 0;
        _clearBuffer();
      }
    }
  }
  DEBUG_PRINTLN("");
  return true;
}