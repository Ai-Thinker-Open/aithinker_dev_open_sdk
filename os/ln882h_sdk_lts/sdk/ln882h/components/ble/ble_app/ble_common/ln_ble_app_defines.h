#ifndef _LN_BLE_APP_DEFINES_H_
#define _LN_BLE_APP_DEFINES_H_

/// Random number length
#define LN_RAND_NB_LEN                  (8)
/// Key length
#define LN_GAP_KEY_LEN                  (16)
/// BD address length
#define LN_BD_ADDR_LEN                  (6)
/// ble channel map
#define LN_BLE_CHNL_MAP_LEN             (5)
/// invalid gatt handle
#define LN_ATT_INVALID_HANDLE           (0xFFFF)

typedef struct
{
    uint8_t addr[LN_BD_ADDR_LEN];
} ln_bd_addr_t;

typedef struct
{
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
    /// BD Address of device
    ln_bd_addr_t addr;
} ln_bdaddr_t;

/// Long Term Key information
/*@TRACE*/
typedef struct
{
    /// Long Term Key
    uint8_t ltk[LN_GAP_KEY_LEN];
    /// Encryption Diversifier
    uint16_t ediv;
    /// Random Number
    uint8_t randnb[LN_RAND_NB_LEN];
    /// Encryption key size (7 to 16)
    uint8_t key_size;
} ln_gap_ltk_t;

/// Device IRK used for resolvable random BD address generation (LSB first)
/*@TRACE*/
typedef struct
{
    /// Key value MSB -> LSB
    uint8_t key[LN_GAP_KEY_LEN];
} ln_gap_irk_t;


/// structure of a list element header
/*@TRACE*/
struct le_list_node
{
    /// Pointer to next co_list_hdr
    struct le_list_node *next;
};

/// simplify type name of list element header
typedef struct le_list_node le_list_node_t;

#endif
