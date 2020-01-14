// Author: M16946
// Date: 2017/01/13

/*
    (c) 2016 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.
    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

#ifndef __RN487x_CONST
#define __RN487x_CONST

// ------------------- Serial -------------------------
#define DELAY_BEFORE_CMD 100     // delay before the first '$' to enter into command mode
#define DEFAULT_CMD_TIMEOUT 1000 // default timeout
#define RESET_CMD_TIMEOUT 2000
#define LIST_CMD_TIMEOUT 3000
#define RN487X_DEFAULT_BAUDRATE 115200
#define CRLF "\r\n"
#define CR '\r'
#define LF '\n'

// ------------------- Commands -----------------------
#define CMD_CHAR '$'
#define ENTER_CMD "$$$"
#define EXIT_CMD "---\r"
#define ENTER_DATA (EXIT_CMD)
#define DATA_LAST_CHAR '\r'

// --- Set Commands
#define SET_BEACON_FEATURES "SC,"
#define BEACON_OFF "0"
#define BEACON_ON "1"
#define BEACON_ADV_ON "2"
#define FACTORY_RESET "SF,1"
#define SET_ADV_POWER "SGA,"
#define SET_CONN_POWER "SGC,"
#define MIN_POWER_OUTPUT (0u)
#define MAX_POWER_OUTPUT (5u)
#define SET_SERIALIZED_NAME "S-,"
#define MAX_SERIALIZED_NAME_LEN 15
#define SET_DEVICE_NAME "SN,"
#define SET_MANUF_NAME "SDN,"
#define MAX_DEVICE_NAME_LEN 20
#define SET_LOW_POWER_ON "SO,1"
#define SET_LOW_POWER_OFF "SO,0"
#define SET_DORMANT_MODE "O,0"
#define SET_SETTINGS "S:,"

#define SET_SUPPORTED_FEATURES "SR,"
// > Bitmap of supported features
#define NO_BEACON_SCAN_BMP 0x1000
#define NO_CONNECT_SCAN_BMP 0x0800
#define NO_DUPLICATE_SCAN_BMP 0x0400
#define PASSIVE_SCAN_BMP 0x0200
#define UART_TRANSP_NO_ACK_BMP 0x0100
#define MLDP_SUPPORT_BMP 0x0020

#define SET_DEFAULT_SERVICES "SS,"
// > Bitmap of services
#define NO_SERVICE 0x00
#define DEVICE_INFO_SERVICE 0x80
#define UART_TRANSP_SERVICE 0x40
#define BEACON_SERVICE 0x20
#define AIRPATCH_SERVICE 0x10

//-- Get Commands
#define GET_SETTINGS "G:,"
#define MAX_SETTINGS_LEN (32u)
#define GET_DEVICE_NAME "GN"
#define GET_CONNECTION_STATUS "GK"

//--- Action Commands
#define START_DEFAULT_ADV "A"
#define START_CUSTOM_ADV "A,"
#define STOP_ADV "Y"
#define CLEAR_IMMEDIATE_ADV "IA,Z"
#define CLEAR_PERMANENT_ADV "NA,Z"
#define CLEAR_IMMEDIATE_BEACON "IB,Z"
#define CLEAR_PERMANENT_BEACON "NB,Z"
#define START_IMMEDIATE_ADV "IA,"
#define START_PERMANENT_ADV "NA,"
#define START_IMMEDIATE_BEACON "IB,"
#define START_PERMANENT_BEACON "NB,"
#define START_DEFAULT_SCAN "F"
#define START_CUSTOM_SCAN "F,"
#define STOP_SCAN "X"
#define ADD_WHITE_LIST "JA,"
#define MAX_WHITE_LIST_SIZE (16u)
#define MAC_ADDRESS_LEN (12u)
#define PUBLIC_ADDRESS_TYPE "0"
#define PRIVATE_ADDRESS_TYPE "1"
#define ADD_BONDED_WHITE_LIST "JB"
#define CLEAR_WHITE_LIST "JC"
#define KILL_CONNECTION "K,1"
#define GET_RSSI_LEVEL "M"
#define REBOOT "R,1"
#define DISPLAY_FW_VERSION "V"

// --- List Commands
#define LIST_CHARACTERISTICS "LS"

// --- Service Definition
#define DEFINE_CHARACT_UUID "PC,"
#define DEFINE_SERVICE_UUID "PS,"
#define CLEAR_ALL_SERVICES "PZ"
#define PRIVATE_SERVICE_LEN 32 // 128-bit
#define PUBLIC_SERVICE_LEN 4   // 16-bit

// --- Characteristic Access
#define READ_REMOTE_CHARACT "CHR,"
#define WRITE_REMOTE_CHARACT "CHW,"
#define DISCOVER_REMOTE "CI" // start client role
#define READ_LOCAL_CHARACT "SHR,"
#define WRITE_LOCAL_CHARACT "SHW,"

// ------------------- Response -----------------------
#define PROMPT "CMD>" // exact prompt is "CMD> " (last char is a space)
#define PROMPT_END "END"

//-- Response
#define AOK_RESP "AOK"
#define ERR_RESP "Err"
#define FACTORY_RESET_RESP "Reboot after Factory Reset"
#define DEVICE_MODEL "RN"
#define REBOOTING_RESP "Rebooting"
#define NONE_RESP "none"
#define SCANNING_RESP "Scanning"

//-- Events
#define REBOOT_EVENT "%REBOOT%"

#endif