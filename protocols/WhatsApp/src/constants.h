#if !defined(CONSTANTS_H)
#define CONSTANTS_H

// Version management
#define __VERSION_DWORD             PLUGIN_MAKE_VERSION(0, 0, 2, 0)
#define __PRODUCT_DWORD             PLUGIN_MAKE_VERSION(0, 9, 14, 0)
#define __VERSION_STRING            "0.0.2.0"
#define __PRODUCT_STRING            "0.9.14.0"
#define __VERSION_VS_FILE           0,0,2,
#define __VERSION_VS_PROD           0,9,14,0
#define __VERSION_VS_FILE_STRING    "0, 0, 2, 0"
#define __VERSION_VS_PROD_STRING    "0, 9, 14, 0"

#define PRODUCT_NAME _T("WhatsApp Protocol")

// Limits
#define WHATSAPP_GROUP_NAME_LIMIT   420

// Defaults
#define DEFAULT_MAP_STATUSES			0
#define DEFAULT_SYSTRAY_NOTIFY      0

#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE   1
#define DEFAULT_EVENT_FEEDS_ENABLE           1
#define DEFAULT_EVENT_OTHER_ENABLE           1
#define DEFAULT_EVENT_CLIENT_ENABLE          1
#define DEFAULT_EVENT_COLBACK                0x00ffffff
#define DEFAULT_EVENT_COLTEXT                0x00000000
#define DEFAULT_EVENT_TIMEOUT_TYPE           0
#define DEFAULT_EVENT_TIMEOUT                -1

// #TODO Move constants below to WhatsAPI++

// WhatsApp
#define WHATSAPP_LOGIN_SERVER "c.whatsapp.net"
#define ACCOUNT_USER_AGENT "WhatsApp/2.8.3 iPhone_OS/5.0.1 Device/Unknown_(iPhone4,1)"
#define ACCOUNT_URL_CODEREQUEST "https://r.whatsapp.net/v1/code.php"
#define ACCOUNT_URL_CODEREQUESTV2 "https://v.whatsapp.net/v2/code"
#define ACCOUNT_URL_REGISTERREQUEST "https://r.whatsapp.net/v1/register.php"
#define ACCOUNT_URL_REGISTERREQUESTV2 "https://v.whatsapp.net/v2/register"
#define ACCOUNT_URL_UPLOADREQUEST "https://mms.whatsapp.net/client/iphone/upload.php"
#define ACCOUNT_URL_EXISTSV2 "https://v.whatsapp.net/v2/exist"

// WhatsApp Nokia 302 S40
#define ACCOUNT_RESOURCE  "S40-2.3.53"
/*
#define ACCOUNT_USER_AGENT_REGISTRATION "WhatsApp/2.3.53 S40Version/14.26 Device/Nokia302"
#define ACCOUNT_TOKEN_PREFIX1 "PdA2DJyKoUrwLw1Bg6EIhzh502dF9noR9uFCllGk"
#define ACCOUNT_TOKEN_PREFIX2 "1354754753509"
*/
#define ACCOUNT_USER_AGENT_REGISTRATION "WhatsApp/2.9.4 WP7/7.10.8858 Device/HTC-HTC-H0002"
#define ACCOUNT_TOKEN_PREFIX1 "Od52pFozHNWF9XbTN5lrqDtnsiZGL2G3l9yw1GiQ"
#define ACCOUNT_TOKEN_PREFIX2 "21a31a2d9dbdc9a8ce324ef2df918064fd26e30a"

#define WHATSAPP_RECV_MESSAGE 1
#define WHATSAPP_SEND_MESSAGE 2

#define MAX_SILENT_INTERVAL 210

// Event flags
#define WHATSAPP_EVENT_CLIENT          0x10000000 // WhatsApp error or info message
#define WHATSAPP_EVENT_NOTIFICATION    0x40000000 // WhatsApp notification
#define WHATSAPP_EVENT_OTHER           0x80000000 // WhatsApp other event - friend requests/new messages

#define IS_CHAT 1

#define REG_STATE_REQ_CODE 1
#define REG_STATE_REG_CODE 2

#endif