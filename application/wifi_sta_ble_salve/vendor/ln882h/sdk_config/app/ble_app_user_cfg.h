#ifndef _BLE_APP_USER_CONFIG_H_
#define _BLE_APP_USER_CONFIG_H_


#define BLE_DEFAULT_ROLE                BLE_ROLE_PERIPHERAL

#define BLE_DEFAULT_DEVICE_NAME         ("ln882h_combo")
#define BLE_DEV_NAME_MAX_LEN            (40)

//#define BLE_USE_STATIC_PUBLIC_ADDR
#define BLE_DEFAULT_PUBLIC_ADDR         {0x56, 0x34, 0x12, 0x01, 0xFF, 0x00}

#define BLE_CONFIG_AUTO_ADV             (1)
#define BLE_CONFIG_AUTO_SCAN            (0)

/* enable ble data encrypt */
#define BLE_CONFIG_SECURITY             (0)

/* support multiple connection */
//#define BLE_CONFIG_MULTI_CONNECT      (1)



/* user data trans Server  */
#define BLE_DATA_TRANS_SERVER           (1)

/* user data trans Client  */
#define BLE_DATA_TRANS_CLIENT           (0)


//TODO: support
/// Proximity Profile Monitor Role
//#define CFG_PRF_PXPM
/// Proximity Profile Reporter Role
//#define CFG_PRF_PXPR
///Find Me Profile Locator role
//#define CFG_PRF_FMPL
///Find Me Profile Target role
//#define CFG_PRF_FMPT
///Health Thermometer Profile Collector Role
//#define CFG_PRF_HTPC
///Health Thermometer Profile Thermometer Role
//#define CFG_PRF_HTPT
///Device Information Service Client Role
//#define CFG_PRF_DISC
///Device Information Service Server Role
//#define CFG_PRF_DISS
///Blood Pressure Profile Collector Role
//#define CFG_PRF_BLPC
///Blood Pressure Profile Sensor Role
//#define CFG_PRF_BLPS
///Time Profile Client Role
//#define CFG_PRF_TIPC
///Time Profile Server Role
//#define CFG_PRF_TIPS
///Heart Rate Profile Collector Role
//#define CFG_PRF_HRPC
///Heart Rate Profile Sensor Role
//#define CFG_PRF_HRPS
///Scan Parameter Profile Client Role
//#define CFG_PRF_SCPPC
///Scan Parameter Profile Server Role
//#define CFG_PRF_SCPPS
///Battery Service Client Role
//#define CFG_PRF_BASC
///Battery Service Server Role
//#define CFG_PRF_BASS
///HID Device Role
//#define CFG_PRF_HOGPD
///HID Boot Host Role
//#define CFG_PRF_HOGPBH
///HID Report Host Role
//#define CFG_PRF_HOGPRH
/// Glucose Profile Collector Role
//#define CFG_PRF_GLPC
/// Glucose Profile Sensor Role
//#define CFG_PRF_GLPS
/// Running Speed and Cadence Profile Collector Role
//#define CFG_PRF_RSCPC
/// Running Speed and Cadence Profile Server Role
//#define CFG_PRF_RSCPS
/// Cycling Speed and Cadence Profile Collector Role
//#define CFG_PRF_CSCPC
/// Cycling Speed and Cadence Profile Server Role
//#define CFG_PRF_CSCPS
/// Cycling Power Profile Collector Role
//#define CFG_PRF_CPPC
/// Cycling Power Profile Server Role
//#define CFG_PRF_CPPS
/// Location and Navigation Profile Collector Role
//#define CFG_PRF_LANC
/// Location and Navigation Profile Server Role
//#define CFG_PRF_LANS
/// Alert Notification Profile Client Role
//#define CFG_PRF_ANPC
/// Alert Notification Profile Server Role
//#define CFG_PRF_ANPS
/// Phone Alert Status Profile Client Role
//#define CFG_PRF_PASPC
/// Phone Alert Status Profile Server Role
//#define CFG_PRF_PASPS
/// Internet Protocol Support Profile Server Role
//#define CFG_PRF_IPSS
/// Internet Protocol Support Profile Client Role
//#define CFG_PRF_IPSC
/// Environmental Sensing Profile Server Role
//#define CFG_PRF_ENVS
/// Environmental Sensing Profile Client Role
//#define CFG_PRF_ENVC
/// Weight Scale Profile Server Role
//#define CFG_PRF_WSCS
/// Weight Scale Profile Client Role
//#define CFG_PRF_WSCC
/// Body Composition Server Role
//#define CFG_PRF_BCSS
/// Body Composition Client Role
//#define CFG_PRF_BCSC
/// User Data Service Server Role
//#define CFG_PRF_UDSS
/// User Data Service Client Role
//#define CFG_PRF_UDSC
/// Wireless Power Transfer Profile Server Role
//#define CFG_PRF_WPTS
/// Wireless Power Transfer Profile Client Role
//#define CFG_PRF_WPTC
/// Pulse Oximeter Profile Server Role
//#define CFG_PRF_PLXS
/// Pulse Oximete Profile Client Role
//#define CFG_PRF_PLXC
/// Continuous Glucose Monitoring Profile Server Role
//#define CFG_PRF_CGMS
/// Continuous Glucose Monitoring Profile Client Role
//#define CFG_PRF_CGMC
/// Throughput tester profile for debug usage
//#define CFG_PRF_THPP


#endif /* _BLE_APP_USER_CONFIG_H_ */
