/**
 ****************************************************************************************
 *
 * @file mac.h
 *
 * @brief MAC related definitions.
 *
 * Copyright (C) RivieraWaves 2011-2021
 *
 ****************************************************************************************
 */

#ifndef _MAC_H_
#define _MAC_H_

/**
 ****************************************************************************************
 * @defgroup MAC MAC
 * @ingroup MACSW
 * @brief  Common MAC types, defines and utils functions
 *
 * This module contains various types and definitions related to 802.11 standard and used
 * by all MACSW fimware modules.
 * - Types, definitions and functions that are only used internally
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
// standard includes
#include "export/common/co_int.h"
#include "export/common/co_bool.h"
#include "export/common/co_math.h"
#include "export/common/co_utils.h"
#include "compiler.h"
#include "export/mac/mac_types.h"

// for memcmp
#include <string.h>


/*
 * DEFINES
 ****************************************************************************************
 */
/// duration of a Time Unit in microseconds
#define TU_DURATION                     1024

/// Mask to test if it's a basic rate - BIT(7)
#define MAC_BASIC_RATE                  0x80

/// Mask for extracting/checking word alignment
#define WORD_ALIGN                      3

/// Maximum Number of CCK rates
#define N_CCK       8
/// Maximum Number of OFDM rates
#define N_OFDM      8
/// First OFDM rate index
#define FIRST_OFDM  N_CCK
/// First HT rate index
#define FIRST_HT    FIRST_OFDM + N_OFDM
/// First VHT rate index
#define FIRST_VHT   FIRST_HT + (8 * 2 * 2 * 4)
/// First HE SU rate index
#define FIRST_HE_SU FIRST_VHT + (10 * 4 * 2 * 8)
/// First HE MU rate index
#define FIRST_HE_MU FIRST_HE_SU + (12 * 4 * 3 * 8)
/// First HE ER rate index
#define FIRST_HE_ER FIRST_HE_MU + (12 * 6 * 3 * 8)
/// Total Number of rates
#define TOT_RATES   FIRST_HE_ER + (2 * 3 * 3 * 3 + 3) // change to support ER+DCM

/**
 ****************************************************************************************
 * Compare two MAC addresses.
 * The MAC addresses MUST be 16 bit aligned.
 * @param[in] addr1_ptr Pointer to the first MAC address.
 * @param[in] addr2_ptr Pointer to the second MAC address.
 * @return True if equal, false if not.
 ****************************************************************************************
 */
#define MAC_ADDR_CMP(addr1_ptr, addr2_ptr)                                              \
    ((*(((uint16_t*)(addr1_ptr)) + 0) == *(((uint16_t*)(addr2_ptr)) + 0)) &&            \
     (*(((uint16_t*)(addr1_ptr)) + 1) == *(((uint16_t*)(addr2_ptr)) + 1)) &&            \
     (*(((uint16_t*)(addr1_ptr)) + 2) == *(((uint16_t*)(addr2_ptr)) + 2)))

/**
 ****************************************************************************************
 * Compare two MAC addresses whose alignment is not known.
 * @param[in] __a1 Pointer to the first MAC address.
 * @param[in] __a2 Pointer to the second MAC address.
 * @return True if equal, false if not.
 ****************************************************************************************
 */
#define MAC_ADDR_CMP_PACKED(__a1, __a2)                                                 \
    (memcmp(__a1, __a2, MAC_ADDR_LEN) == 0)

/**
 ****************************************************************************************
 * Copy a MAC address.
 * The MAC addresses MUST be 16 bit aligned.
 * @param[in] addr1_ptr Pointer to the destination MAC address.
 * @param[in] addr2_ptr Pointer to the source MAC address.
 ****************************************************************************************
 */
#define MAC_ADDR_CPY(addr1_ptr, addr2_ptr)                                              \
    *(((uint16_t*)(addr1_ptr)) + 0) = *(((uint16_t*)(addr2_ptr)) + 0);                  \
    *(((uint16_t*)(addr1_ptr)) + 1) = *(((uint16_t*)(addr2_ptr)) + 1);                  \
    *(((uint16_t*)(addr1_ptr)) + 2) = *(((uint16_t*)(addr2_ptr)) + 2)

/**
 ****************************************************************************************
 * Extract MAC address from unaligned bitstream into struct mac_addr.
 * @param[in] mac_addr struct mac_addr to initialize
 * @param[in] addr_ptr Address of the MAC address in bitstream.
 ****************************************************************************************
 */
#if (CHAR_LEN == 1)
#define MAC_ADDR_EXTRACT(mac_addr, addr_ptr)                        \
    *(((uint8_t*)(mac_addr)) + 0) = *(((uint8_t*)(addr_ptr)) + 0);  \
    *(((uint8_t*)(mac_addr)) + 1) = *(((uint8_t*)(addr_ptr)) + 1);  \
    *(((uint8_t*)(mac_addr)) + 2) = *(((uint8_t*)(addr_ptr)) + 2);  \
    *(((uint8_t*)(mac_addr)) + 3) = *(((uint8_t*)(addr_ptr)) + 3);  \
    *(((uint8_t*)(mac_addr)) + 4) = *(((uint8_t*)(addr_ptr)) + 4);  \
    *(((uint8_t*)(mac_addr)) + 5) = *(((uint8_t*)(addr_ptr)) + 5)
#else
#define MAC_ADDR_EXTRACT(mac_addr, addr_ptr)  \
    memcpy(mac_addr, addr_ptr, MAC_ADDR_LEN)
#endif

/**
 ****************************************************************************************
 * Compare two SSID.
 * @param[in] ssid1_ptr Pointer to the first SSID structure.
 * @param[in] ssid2_ptr Pointer to the second SSID structure.
 * @return True if equal, false if not.
 ****************************************************************************************
 */
#define MAC_SSID_CMP(ssid1_ptr,ssid2_ptr)                                               \
    (((ssid1_ptr)->length == (ssid2_ptr)->length) &&                                    \
     (memcmp((&(ssid1_ptr)->array[0]), (&(ssid2_ptr)->array[0]), (ssid1_ptr)->length) == 0))

/**
 ****************************************************************************************
 * Check if MAC address is a group address: test the multicast bit.
 * @param[in] mac_addr_ptr Pointer to the MAC address to be checked.
 * @return 0 if unicast address, 1 otherwise
 ****************************************************************************************
 */
#define MAC_ADDR_GROUP(mac_addr_ptr) ((*((uint8_t *)(mac_addr_ptr))) & 1)

/// Maximum size of the frame body for frames that are internally carried.
#define MAC_FRAME_LEN 512

/// MAC Frame structure.
struct mac_frame
{
    /// Actual length of the frame.
    uint16_t length;
    /// Array containing the frame body.
    uint8_t array[MAC_FRAME_LEN];
};

/// Max number of default keys for given VIF
#define MAC_DEFAULT_KEY_COUNT 4
/// Max number of MFP key for given VIF
#define MAC_DEFAULT_MFP_KEY_COUNT 6

/// Structure containing the information about a key
struct key_info_tag
{
    /// Replay counters for RX
    uint64_t rx_pn[TID_MAX];
    /// Replay counter for TX
    uint64_t tx_pn;
    /// Union of MIC and MFP keys
    union
    {
        struct
        {
            /// TX MIC key
            uint32_t tx_key[2];
            /// RX MIC key
            uint32_t rx_key[2];
        } mic;
        struct
        {
            uint32_t key[4];
        } mfp;
    }u;
    /// Type of security currently used
    uint8_t cipher;
    /// Key index as specified by the authenticator/supplicant
    uint8_t key_idx;
    /// Index of the key in the HW memory
    uint8_t hw_key_idx;
    /// Flag indicating if the key is valid
    bool valid;
    /// Key cookie
    uint8_t cookie;
};

/// MAC HT operation element
struct mac_htoprnelmt
{
    /// Primary channel information
    uint8_t     prim_channel;
    /// HT operation information 1
    uint8_t     ht_oper_1;
    /// HT operation information 2
    uint16_t    ht_oper_2;
    /// HT operation information 3
    uint16_t    ht_oper_3;
    /// Basic MCS set
    uint8_t     mcs_rate[MAX_MCS_LEN];

};

/// MU EDCA Parameter Set Element
struct mac_mu_edca_param_set
{
    /// Per-AC MU EDCA parameters
    uint32_t        ac_param[AC_MAX];
    /// QoS information
    uint8_t         qos_info;
    /// Parameter set counter
    uint8_t         param_set_cnt;
};

/// EDCA Parameter Set Element
struct mac_edca_param_set
{
    /// Per-AC EDCA parameters
    uint32_t        ac_param[AC_MAX];
    /// QoS information
    uint8_t         qos_info;
    /// Admission Control Mandatory bitfield
    uint8_t         acm;
    /// Parameter set counter
    uint8_t         param_set_cnt;
};

/// UAPSD enabled on VO
#define MAC_QOS_INFO_STA_UAPSD_ENABLED_VO      CO_BIT(0)
/// UAPSD enabled on VI
#define MAC_QOS_INFO_STA_UAPSD_ENABLED_VI      CO_BIT(1)
/// UAPSD enabled on BK
#define MAC_QOS_INFO_STA_UAPSD_ENABLED_BK      CO_BIT(2)
/// UAPSD enabled on BE
#define MAC_QOS_INFO_STA_UAPSD_ENABLED_BE      CO_BIT(3)
/// UAPSD enabled on all access categories
#define MAC_QOS_INFO_STA_UAPSD_ENABLED_ALL     0x0F
/// UAPSD enabled in AP
#define MAC_QOS_INFO_AP_UAPSD_ENABLED          CO_BIT(7)

// Protection Status field (Bit indexes, Masks, Offsets)
/// Non-ERP stations are present
#define MAC_PROT_NONERP_PRESENT_BIT        CO_BIT(0)
/// ERP protection shall be used
#define MAC_PROT_USE_PROTECTION_BIT        CO_BIT(1)
/// Barker preamble protection shall be used
#define MAC_PROT_BARKER_PREAMB_BIT         CO_BIT(2)
/// ERP protection status mask
#define MAC_PROT_ERP_STATUS_MASK           (MAC_PROT_NONERP_PRESENT_BIT |                \
                                            MAC_PROT_USE_PROTECTION_BIT |                \
                                            MAC_PROT_BARKER_PREAMB_BIT)

/*
 * GLOBAL VARIABLES
 ****************************************************************************************
 */
/// Array converting a TID to its associated AC
extern const uint8_t mac_tid2ac[];

/// Array converting an AC to its corresponding bit in the QoS Information field
extern const uint8_t mac_ac2uapsd[];

/// Array converting an ACI to its corresponding AC. See Table 9-136 in 802.11-2016
extern const uint8_t mac_aci2ac[];

/// Array converting an AC to its corresponding ACI. See Table 9-136 in 802.11-2016
extern const uint8_t mac_ac2aci[];


/// Array converting a MAC HW rate id into its corresponding standard rate value
extern const uint8_t mac_id2rate[];

/// Constant value corresponding to the Broadcast MAC address
extern const struct mac_addr mac_addr_bcst;


/**
 ****************************************************************************************
 * Get the value of a bit field in the HE MAC capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] field  Bit field to be read
 * @return The value of the field
 ****************************************************************************************
 */
#define HE_MAC_CAPA_VAL_GET(he_cap, field)    co_val_get((he_cap)->mac_cap_info,         \
                                                         HE_MAC_CAPA_##field##_OFT,      \
                                                         HE_MAC_CAPA_##field##_WIDTH)

/**
 ****************************************************************************************
 * Set the value of a bit field in the HE MAC capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] field Bit field to be written
 * @param[in] val The value of the field
 ****************************************************************************************
 */
#define HE_MAC_CAPA_VAL_SET(he_cap, field, val) co_val_set((he_cap)->mac_cap_info,       \
                                                           HE_MAC_CAPA_##field##_OFT,    \
                                                           HE_MAC_CAPA_##field##_WIDTH,  \
                                                           HE_MAC_CAPA_##field##_##val)

/**
 ****************************************************************************************
 * Test if a bit in the HE MAC capability element is set.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be set
 * @return true if set, false otherwise
 ****************************************************************************************
 */
#define HE_MAC_CAPA_BIT_IS_SET(he_cap, bit)   co_bit_is_set((he_cap)->mac_cap_info,      \
                                                            HE_MAC_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Set a bit in the HE MAC capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be set
 ****************************************************************************************
 */
#define HE_MAC_CAPA_BIT_SET(he_cap, bit)      co_bit_set((he_cap)->mac_cap_info,         \
                                                          HE_MAC_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Clear a bit in the HE MAC capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be cleared
 ****************************************************************************************
 */
#define HE_MAC_CAPA_BIT_CLR(he_cap, bit)      co_bit_clr((he_cap)->mac_cap_info,         \
                                                         HE_MAC_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Get the value of a bit field in the HE PHY capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] field  Bit field to be read
 * @return The value of the field
 ****************************************************************************************
 */
#define HE_PHY_CAPA_VAL_GET(he_cap, field)    co_val_get((he_cap)->phy_cap_info,         \
                                                         HE_PHY_CAPA_##field##_OFT,      \
                                                         HE_PHY_CAPA_##field##_WIDTH)

/**
 ****************************************************************************************
 * Set the value of a bit field in the HE PHY capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] field Bit field to be written
 * @param[in] val The value of the field
 ****************************************************************************************
 */
#define HE_PHY_CAPA_VAL_SET(he_cap, field, val) co_val_set((he_cap)->phy_cap_info,       \
                                                           HE_PHY_CAPA_##field##_OFT,    \
                                                           HE_PHY_CAPA_##field##_WIDTH,  \
                                                           HE_PHY_CAPA_##field##_##val)

/**
 ****************************************************************************************
 * Test if a bit in the HE PHY capability element is set.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be set
 * @return true if set, false otherwise
 ****************************************************************************************
 */
#define HE_PHY_CAPA_BIT_IS_SET(he_cap, bit)   co_bit_is_set((he_cap)->phy_cap_info,      \
                                                            HE_PHY_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Set a bit in the HE PHY capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be set
 ****************************************************************************************
 */
#define HE_PHY_CAPA_BIT_SET(he_cap, bit)      co_bit_set((he_cap)->phy_cap_info,         \
                                                         HE_PHY_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Clear a bit in the HE PHY capability element.
 * @param[in] he_cap Pointer to the HE capability structure
 * @param[in] bit    Bit to be cleared
 ****************************************************************************************
 */
#define HE_PHY_CAPA_BIT_CLR(he_cap, bit)      co_bit_clr((he_cap)->phy_cap_info,         \
                                                         HE_PHY_CAPA_##bit##_POS)

/**
 ****************************************************************************************
 * Test if a bit in the extended capabilities element is set.
 * @param[in] ext_cap Pointer to the HE extended capabilities structure
 * @param[in] bit     Bit to test
 * @param[in] length  Length of the elements
 * @return true if set, false otherwise
 ****************************************************************************************
 */
#define EXT_CAPA_BIT_IS_SET(ext_cap, bit, length)   co_bit_is_set_var(ext_cap,      \
                                                            MAC_EXT_CAPA_##bit##_POS, length)

/**
 ****************************************************************************************
 * Set the value of a bit field in the FTM Parameters element.
 * @param[in] params Pointer to the FTM Parameters structure
 * @param[in] field Bit field to be written
 * @param[in] val The value of the field
 ****************************************************************************************
 */
#define FTM_PARAMS_VAL_SET(params, field, val) co_val_set((uint8_t *) params,   \
                                                          FTM_##field##_OFT,    \
                                                          FTM_##field##_WIDTH,  \
                                                          FTM_##field##_##val)

/**
 ****************************************************************************************
 * Set the numeric value of a bit field in the FTM Parameters element.
 * @param[in] params Pointer to the FTM Parameters structure
 * @param[in] field Bit field to be written
 * @param[in] val The numeric value
 ****************************************************************************************
 */
#define FTM_PARAMS_VAL_SET_NUM(params, field, val) co_val_set((uint8_t *) params,  \
                                                              FTM_##field##_OFT,   \
                                                              FTM_##field##_WIDTH, \
                                                              val)

/**
 ****************************************************************************************
 * Get the value of a bit field in the FTM Parameters element.
 * @param[in] params Pointer to the FTM Parameters structure
 * @param[in] field  Bit field to be read
 * @return The value of the field
 ****************************************************************************************
 */
#define FTM_PARAMS_VAL_GET(params, field) co_val_get((uint8_t *) params,      \
                                                     FTM_##field##_OFT,       \
                                                     FTM_##field##_WIDTH)

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Get the value of the nonTransmitted BSSID, based on the reference BSSID, the
 * nonTransmitted BSSID index and the Maximum BSSID indicator
 *
 * @param[in]  bssid_index nonTransmitted BSSID index
 * @param[in]  max_bssid_ind Maximum BSSID indicator
 * @param[in]  ref_bssid Pointer to the reference BSSID
 * @param[out] bssid nonTransmitted BSSID value
 *
 * @return false if the BSSID index or Max BSSID indicator are outside our supported range
 ****************************************************************************************
 */
__INLINE bool mac_nontxed_bssid_get(uint8_t bssid_index,
                                    uint8_t max_bssid_ind,
                                    struct mac_addr const *ref_bssid,
                                    struct mac_addr *bssid)
{
    uint16_t mask;

    // Check if parameters are valid
    if (!bssid_index || (max_bssid_ind > 8))
        return false;

    // Compute the nonTransmitted BSSID. Its (48-max_bss_ind) LSB shall be
    // equal to the reference BSSID (48-max_bss_ind) LSB, and its max_bss_ind MSB
    // are equal to: ((max_bss_ind reference BSSID LSB) + bssid_index) % (2^max_bss_ind)
    mask = (CO_BIT(max_bssid_ind) - 1) << 8;
    *bssid = *ref_bssid;
    bssid->array[2] &= ~mask;
    bssid->array[2] |= (ref_bssid->array[2] + (bssid_index << 8)) & mask;

    return true;
}

 /**
  ****************************************************************************************
  * @brief Get the value of the refernec BSSID, based on the nonTransmitted BSSID, the
  * nonTransmitted BSSID index and the Maximum BSSID indicator
  *
  * @param[in]  bssid_index nonTransmitted BSSID index
  * @param[in]  max_bssid_ind Maximum BSSID indicator
  * @param[in]  bssid nonTransmitted BSSID value
  * @param[out] ref_bssid Pointer to the reference BSSID
  ****************************************************************************************
  */
 __INLINE void mac_ref_bssid_get(uint8_t bssid_index,
                                 uint8_t max_bssid_ind,
                                 struct mac_addr const *bssid,
                                 struct mac_addr *ref_bssid)
{
    uint16_t mask;

    // Compute the reference BSSID. Its (48-max_bss_ind) LSB shall be
    // equal to the nonTransmitted BSSID (48-max_bss_ind) LSB, and its max_bss_ind MSB
    // are equal to: ((max_bss_ind nonTransmitted BSSID LSB) - bssid_index) % (2^max_bss_ind)
    mask = (CO_BIT(max_bssid_ind) - 1) << 8;
    *ref_bssid = *bssid;
    ref_bssid->array[2] &= ~mask;
    ref_bssid->array[2] |= (bssid->array[2] - (bssid_index << 8)) & mask;
}

/**
 ****************************************************************************************
 * @brief Compute the PAID/GID to be put in the THD for frames that need to be sent to
 * an AP or a Mesh peer.
 * @see 802.11ac-2013, table 9-5b for details about the computation.
 *
 * @param[in] mac_addr   Pointer to the BSSID of the AP or the MAC address of the mesh
 *                       peer
 *
 * @return The PAID/GID
 ****************************************************************************************
 */
uint32_t mac_paid_gid_sta_compute(struct mac_addr const *mac_addr);

/**
 ****************************************************************************************
 * @brief Compute the PAID/GID to be put in the THD for frames that need to be sent to
 * a STA connected to an AP or a TDLS peer.
 * @see 802.11ac-2013, table 9-5b for details about the computation.
 *
 * @param[in] mac_addr   Pointer to the BSSID of the AP
 * @param[in] aid        Association ID of the STA
 *
 * @return The PAID/GID
 ****************************************************************************************
 */
uint32_t mac_paid_gid_ap_compute(struct mac_addr const *mac_addr, uint16_t aid);

/**
 ****************************************************************************************
 * @brief Convert IEE802.11 AKM suite into enum mac_akm_suite
 *
 * @param[in] akm_suite Ful AKM suite
 * @return The corresponding enum value and -1 if the suite is unknown
 ****************************************************************************************
 */
enum mac_akm_suite mac_akm_suite_value(uint32_t akm_suite);

/**
 ****************************************************************************************
 * @brief Convert IEE802.11 Cipher suite into enum mac_cipher_suite
 *
 * @param[in] cipher_suite Full Cipher suite
 * @return The corresponding enum value and -1 if the suite is unknown
 ****************************************************************************************
 */
enum mac_cipher_suite mac_cipher_suite_value(uint32_t cipher_suite);

/**
 ****************************************************************************************
 * @brief Format dBm value for RCPI element
 *
 * @param[in] dbm  RCPI value in dBm
 * @return The dBm value formatted for RCPI element
 ****************************************************************************************
 */
uint8_t mac_rcpi_format(int8_t dbm);
/// @}

#endif // _MAC_H_
