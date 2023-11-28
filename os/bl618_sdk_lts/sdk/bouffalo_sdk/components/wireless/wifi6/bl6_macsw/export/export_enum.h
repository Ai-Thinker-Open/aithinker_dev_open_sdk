
enum
{
    #if NX_UMAC_PRESENT
    /// BROADCAST/GROUP DATA TX STA Index for first virtual AP.
    BROADCAST_STA_IDX_MIN = NX_REMOTE_STA_MAX,
    /// BROADCAST/GROUP DATA TX STA Index for last virtual AP.
    BROADCAST_STA_IDX_MAX = NX_REMOTE_STA_MAX + NX_VIRT_DEV_MAX - 1,
    /// Maximum number of supported STA
    STA_MAX,
    #else
    STA_MAX = NX_REMOTE_STA_MAX,
    #endif
    /// Invalid STA Index used for error checking.
    INVALID_STA_IDX = 0xFF
};

/// Messages that are logically related to the task.
enum ftm_msg_tag
{
    /// Request to start the FTM Measurements
    FTM_START_REQ = KE_FIRST_MSG(TASK_FTM),
    /// Confirmation of the FTM start.
    FTM_START_CFM,
    /// Message sent once the requested measurements have been done
    FTM_DONE_IND,
    /// Request Measurement to the peer STA
    FTM_MEASUREMENT_REQ,
    /// Timeout message for procedures requiring a response from the peer STA
    FTM_PEER_RSP_TIMEOUT_IND,
    /// Request for closing the session after last measurement
    FTM_CLOSE_SESSION_REQ,
    #if NX_FAKE_FTM_RSP
    FTM_AP_MEASUREMENT,
    #endif
};

enum apm_msg_tag
{
    /// Request to start the AP.
    APM_START_REQ = KE_FIRST_MSG(TASK_APM),
    /// Confirmation of the AP start.
    APM_START_CFM,
    /// Request to stop the AP.
    APM_STOP_REQ,
    /// Confirmation of the AP stop.
    APM_STOP_CFM,
    /// Request to start CAC
    APM_START_CAC_REQ,
    /// Confirmation of the CAC start
    APM_START_CAC_CFM,
    /// Request to stop CAC
    APM_STOP_CAC_REQ,
    /// Confirmation of the CAC stop
    APM_STOP_CAC_CFM,
    /// Request to Probe Client
    APM_PROBE_CLIENT_REQ,
    /// Confirmation of Probe Client
    APM_PROBE_CLIENT_CFM,
    /// Indication of Probe Client status
    APM_PROBE_CLIENT_IND,
};

enum
{
    /// Scan request from host.
    SCANU_START_REQ = KE_FIRST_MSG(TASK_SCANU),
    /// Scanning start Confirmation.
    SCANU_START_CFM,
    /// Join request
    SCANU_JOIN_REQ,
    /// Join confirmation.
    SCANU_JOIN_CFM,
    /// Scan result indication.
    SCANU_RESULT_IND,
    /// Get Scan result request.
    SCANU_GET_SCAN_RESULT_REQ,
    /// Scan result confirmation.
    SCANU_GET_SCAN_RESULT_CFM,
};

enum scan_msg_tag
{
    /// Scanning start Request.
    SCAN_START_REQ = KE_FIRST_MSG(TASK_SCAN),
    /// Scanning start Confirmation.
    SCAN_START_CFM,
    /// End of scanning indication.
    SCAN_DONE_IND,
    /// Cancel scan request
    SCAN_CANCEL_REQ,
    /// Cancel scan confirmation
    SCAN_CANCEL_CFM,

    /*
     * Section of internal SCAN messages. No SCAN API messages should be defined below this point
     */
    SCAN_PROBE_TIMER,

    SCAN_RAW_SEND_REQ,
    SCAN_RAW_SEND_CFM,

    /// MAX number of messages
    SCAN_MAX,
};

enum sm_msg_tag
{
    /// Request to connect to an AP
    SM_CONNECT_REQ = KE_FIRST_MSG(TASK_SM),
    /// Confirmation of connection
    SM_CONNECT_CFM,
    /// Indicates that the SM associated to the AP
    SM_CONNECT_IND,
    /// Request to disconnect
    SM_DISCONNECT_REQ,
    /// Confirmation of disconnection
    SM_DISCONNECT_CFM,
    /// Indicates that the SM disassociated the AP
    SM_DISCONNECT_IND,
    /// Request to start external authentication
    SM_EXTERNAL_AUTH_REQUIRED_IND,
    /// Response to external authentication request
    SM_EXTERNAL_AUTH_REQUIRED_RSP,
    /// Request to update assoc elements after FT over the air authentication
    SM_FT_AUTH_IND,
    /// Response to FT authentication with updated assoc elements
    SM_FT_AUTH_RSP,

    // Section of internal SM messages. No SM API messages should be defined below this point
    /// Timeout message for procedures requiring a response from peer
    SM_RSP_TIMEOUT_IND,
    SM_DEAUTH_TIMEOUT_IND,
};
enum
{
    /// Configuration request from host.
    ME_CONFIG_REQ = KE_FIRST_MSG(TASK_ME),
    /// Configuration confirmation.
    ME_CONFIG_CFM,
    /// Configuration request from host.
    ME_CHAN_CONFIG_REQ,
    /// Configuration confirmation.
    ME_CHAN_CONFIG_CFM,
    /// Set control port state for a station.
    ME_SET_CONTROL_PORT_REQ,
    /// Control port setting confirmation.
    ME_SET_CONTROL_PORT_CFM,
    /// TKIP MIC failure indication.
    ME_TKIP_MIC_FAILURE_IND,
    /// Add a station to the FW (AP mode)
    ME_STA_ADD_REQ,
    /// Confirmation of the STA addition
    ME_STA_ADD_CFM,
    /// Delete a station from the FW (AP mode)
    ME_STA_DEL_REQ,
    /// Confirmation of the STA deletion
    ME_STA_DEL_CFM,
    /// Indication of a TX RA/TID queue credit update
    ME_TX_CREDITS_UPDATE_IND,
    /// Request indicating to the FW that there is traffic buffered on host
    ME_TRAFFIC_IND_REQ,
    /// Confirmation that the @ref ME_TRAFFIC_IND_REQ has been executed
    ME_TRAFFIC_IND_CFM,
    /// Request RC statistics to a station
    ME_RC_STATS_REQ,
    /// RC statistics confirmation
    ME_RC_STATS_CFM,
    /// Request RC fixed rate
    ME_RC_SET_RATE_REQ,
    /// Configure monitor interface
    ME_CONFIG_MONITOR_REQ,
    /// Configure monitor interface response
    ME_CONFIG_MONITOR_CFM,
    /// Setting Power Save mode request from host
    ME_SET_PS_MODE_REQ,
    /// Set Power Save mode confirmation
    ME_SET_PS_MODE_CFM,

    /*
     * Section of internal ME messages. No ME API messages should be defined below this point
     */
    /// Internal request to indicate that a VIF needs to get the HW going to ACTIVE or IDLE
    ME_SET_ACTIVE_REQ,
    /// Confirmation that the switch to ACTIVE or IDLE has been executed
    ME_SET_ACTIVE_CFM,
    /// Internal request to indicate that a VIF desires to de-activate/activate the Power-Save mode
    ME_SET_PS_DISABLE_REQ,
    /// Confirmation that the PS state de-activate/activate has been executed
    ME_SET_PS_DISABLE_CFM,
    /// Indication that data path is flushed for a given station
    ME_DATA_PATH_FLUSHED_IND,
};

enum mm_msg_tag
{
    /// RESET Request.
    MM_RESET_REQ = KE_FIRST_MSG(TASK_MM),
    /// RESET Confirmation.
    MM_RESET_CFM,
    /// START Request.
    MM_START_REQ,
    /// START Confirmation.
    MM_START_CFM,
    /// Read Version Request.
    MM_VERSION_REQ,
    /// Read Version Confirmation.
    MM_VERSION_CFM,
    /// ADD INTERFACE Request.
    MM_ADD_IF_REQ,
    /// ADD INTERFACE Confirmation.
    MM_ADD_IF_CFM,
    /// REMOVE INTERFACE Request.
    MM_REMOVE_IF_REQ,
    /// REMOVE INTERFACE Confirmation.
    MM_REMOVE_IF_CFM,
    /// STA ADD Request.
    MM_STA_ADD_REQ,
    /// STA ADD Confirm.
    MM_STA_ADD_CFM,
    /// STA DEL Request.
    MM_STA_DEL_REQ,
    /// STA DEL Confirm.
    MM_STA_DEL_CFM,
    /// RX FILTER Configuration Request.
    MM_SET_FILTER_REQ,
    /// RX FILTER Configuration Confirmation.
    MM_SET_FILTER_CFM,
    /// CHANNEL Configuration Request.
    MM_SET_CHANNEL_REQ,
    /// CHANNEL Configuration Confirmation.
    MM_SET_CHANNEL_CFM,
    /// DTIM PERIOD Configuration Request.
    MM_SET_DTIM_REQ,
    /// DTIM PERIOD Configuration Confirmation.
    MM_SET_DTIM_CFM,
    /// BEACON INTERVAL Configuration Request.
    MM_SET_BEACON_INT_REQ,
    /// BEACON INTERVAL Configuration Confirmation.
    MM_SET_BEACON_INT_CFM,
    /// BASIC RATES Configuration Request.
    MM_SET_BASIC_RATES_REQ,
    /// BASIC RATES Configuration Confirmation.
    MM_SET_BASIC_RATES_CFM,
    /// BSSID Configuration Request.
    MM_SET_BSSID_REQ,
    /// BSSID Configuration Confirmation.
    MM_SET_BSSID_CFM,
    /// EDCA PARAMETERS Configuration Request.
    MM_SET_EDCA_REQ,
    /// EDCA PARAMETERS Configuration Confirmation.
    MM_SET_EDCA_CFM,
    /// ABGN MODE Configuration Request.
    MM_SET_MODE_REQ,
    /// ABGN MODE Configuration Confirmation.
    MM_SET_MODE_CFM,
    /// Request setting the VIF active state (i.e associated or AP started)
    MM_SET_VIF_STATE_REQ,
    /// Confirmation of the @ref MM_SET_VIF_STATE_REQ message.
    MM_SET_VIF_STATE_CFM,
    /// SLOT TIME PARAMETERS Configuration Request.
    MM_SET_SLOTTIME_REQ,
    /// SLOT TIME PARAMETERS Configuration Confirmation.
    MM_SET_SLOTTIME_CFM,
    /// IDLE mode change Request.
    MM_SET_IDLE_REQ,
    /// IDLE mode change Confirm.
    MM_SET_IDLE_CFM,
    /// KEY ADD Request.
    MM_KEY_ADD_REQ,
    /// KEY ADD Confirm.
    MM_KEY_ADD_CFM,
    /// KEY DEL Request.
    MM_KEY_DEL_REQ,
    /// KEY DEL Confirm.
    MM_KEY_DEL_CFM,
    /// Block Ack agreement info addition
    MM_BA_ADD_REQ,
    /// Block Ack agreement info addition confirmation
    MM_BA_ADD_CFM,
    /// Block Ack agreement info deletion
    MM_BA_DEL_REQ,
    /// Block Ack agreement info deletion confirmation
    MM_BA_DEL_CFM,
    /// Indication of the primary TBTT to the upper MAC. Upon the reception of this
    /// message the upper MAC has to push the beacon(s) to the beacon transmission queue.
    MM_PRIMARY_TBTT_IND,
    /// Indication of the secondary TBTT to the upper MAC. Upon the reception of this
    /// message the upper MAC has to push the beacon(s) to the beacon transmission queue.
    MM_SECONDARY_TBTT_IND,
    /// Request for changing the TX power
    MM_SET_POWER_REQ,
    /// Confirmation of the TX power change
    MM_SET_POWER_CFM,
    /// Request to the LMAC to trigger the embedded logic analyzer and forward the debug
    /// dump.
    MM_DBG_TRIGGER_REQ,
    /// Set Power Save mode
    MM_SET_PS_MODE_REQ,
    /// Set Power Save mode confirmation
    MM_SET_PS_MODE_CFM,
    /// Request to add a channel context
    MM_CHAN_CTXT_ADD_REQ,
    /// Confirmation of the channel context addition
    MM_CHAN_CTXT_ADD_CFM,
    /// Request to delete a channel context
    MM_CHAN_CTXT_DEL_REQ,
    /// Confirmation of the channel context deletion
    MM_CHAN_CTXT_DEL_CFM,
    /// Request to link a channel context to a VIF
    MM_CHAN_CTXT_LINK_REQ,
    /// Confirmation of the channel context link
    MM_CHAN_CTXT_LINK_CFM,
    /// Request to unlink a channel context from a VIF
    MM_CHAN_CTXT_UNLINK_REQ,
    /// Confirmation of the channel context unlink
    MM_CHAN_CTXT_UNLINK_CFM,
    /// Request to update a channel context
    MM_CHAN_CTXT_UPDATE_REQ,
    /// Confirmation of the channel context update
    MM_CHAN_CTXT_UPDATE_CFM,
    /// Request to schedule a channel context
    MM_CHAN_CTXT_SCHED_REQ,
    /// Confirmation of the channel context scheduling
    MM_CHAN_CTXT_SCHED_CFM,
    /// Request to change the beacon template in LMAC
    MM_BCN_CHANGE_REQ,
    /// Confirmation of the beacon change
    MM_BCN_CHANGE_CFM,
    /// Request to update the TIM in the beacon (i.e to indicate traffic bufferized at AP)
    MM_TIM_UPDATE_REQ,
    /// Confirmation of the TIM update
    MM_TIM_UPDATE_CFM,
    /// Connection loss indication
    MM_CONNECTION_LOSS_IND,
    /// Channel context switch indication to the upper layers
    MM_CHANNEL_SWITCH_IND,
    /// Channel context pre-switch indication to the upper layers
    MM_CHANNEL_PRE_SWITCH_IND,
    /// Request to remain on channel or cancel remain on channel
    MM_REMAIN_ON_CHANNEL_REQ,
    /// Confirmation of the (cancel) remain on channel request
    MM_REMAIN_ON_CHANNEL_CFM,
    /// Remain on channel expired indication
    MM_REMAIN_ON_CHANNEL_EXP_IND,
    /// Indication of a PS state change of a peer device
    MM_PS_CHANGE_IND,
    /// Indication that some buffered traffic should be sent to the peer device
    MM_TRAFFIC_REQ_IND,
    /// Request to modify the STA Power-save mode options
    MM_SET_PS_OPTIONS_REQ,
    /// Confirmation of the PS options setting
    MM_SET_PS_OPTIONS_CFM,
    /// Indication of PS state change for a P2P VIF
    MM_P2P_VIF_PS_CHANGE_IND,
    /// Indication that CSA counter has been updated
    MM_CSA_COUNTER_IND,
    /// Message containing channel information
    MM_CHANNEL_SURVEY_IND,
    /// Message containing Beamformer information
    MM_BFMER_ENABLE_REQ,
    /// Request to Start/Stop NOA - GO Only
    MM_SET_P2P_NOA_REQ,
    /// Request to Start/Stop Opportunistic PS - GO Only
    MM_SET_P2P_OPPPS_REQ,
    /// Start/Stop NOA Confirmation
    MM_SET_P2P_NOA_CFM,
    /// Start/Stop Opportunistic PS Confirmation
    MM_SET_P2P_OPPPS_CFM,
    /// P2P NoA Update Indication - GO Only
    MM_P2P_NOA_UPD_IND,
    /// Request to set RSSI threshold and RSSI hysteresis
    MM_CFG_RSSI_REQ,
    /// Indication that RSSI is below or above the threshold
    MM_RSSI_STATUS_IND,
    /// Indication that CSA is done
    MM_CSA_FINISH_IND,
    /// Indication that CSA is in prorgess (resp. done) and traffic must be stopped (resp. restarted)
    MM_CSA_TRAFFIC_IND,
    /// Request to update the group information of a station
    MM_MU_GROUP_UPDATE_REQ,
    /// Confirmation of the @ref MM_MU_GROUP_UPDATE_REQ message
    MM_MU_GROUP_UPDATE_CFM,
    /// Request to initialize the antenna diversity algorithm
    MM_ANT_DIV_INIT_REQ,
    /// Request to stop the antenna diversity algorithm
    MM_ANT_DIV_STOP_REQ,
    /// Request to update the antenna switch status
    MM_ANT_DIV_UPDATE_REQ,
    /// Request to switch the antenna connected to path_0
    MM_SWITCH_ANTENNA_REQ,
    /// Indication that a packet loss has occurred
    MM_PKTLOSS_IND,
    /// MU EDCA PARAMETERS Configuration Request.
    MM_SET_MU_EDCA_REQ,
    /// MU EDCA PARAMETERS Configuration Confirmation.
    MM_SET_MU_EDCA_CFM,
    /// UORA PARAMETERS Configuration Request.
    MM_SET_UORA_REQ,
    /// UORA PARAMETERS Configuration Confirmation.
    MM_SET_UORA_CFM,
    /// TXOP RTS THRESHOLD Configuration Request.
    MM_SET_TXOP_RTS_THRES_REQ,
    /// TXOP RTS THRESHOLD Configuration Confirmation.
    MM_SET_TXOP_RTS_THRES_CFM,
    /// HE BSS Color Configuration Request.
    MM_SET_BSS_COLOR_REQ,
    /// HE BSS Color Configuration Confirmation.
    MM_SET_BSS_COLOR_CFM,

    /*
     * Section of internal MM messages. No MM API messages should be defined below this point
     */
    /// Internal request to force the HW going to IDLE
    MM_FORCE_IDLE_REQ,
    /// Message indicating that the switch to the scan channel is done
    MM_SCAN_CHANNEL_START_IND,
    /// Message indicating that the scan can end early
    MM_SCAN_CHANNEL_END_EARLY,
    /// Message indicating that the scan on the channel is finished
    MM_SCAN_CHANNEL_END_IND,
    /// Internal request to move the AP TBTT by an offset
    MM_TBTT_MOVE_REQ,
    /// Message indicating that start to send raw packet
    MM_RAW_SEND_REQ,

    /// MAX number of messages
    MM_MAX,
};

enum mm_remain_on_channel_op
{
    MM_ROC_OP_START = 0,
    MM_ROC_OP_CANCEL,

    MM_ROC_OP_MAX,
};

/// AMSDU TX values
enum amsdu_tx
{
    /// AMSDU configured as recommended by peer
    AMSDU_TX_ADV,
    /// AMSDU Enabled
    AMSDU_TX_EN,
    /// AMSDU Disabled
    AMSDU_TX_DIS,
};

enum rx_status_bits
{
    /// The buffer can be forwarded to the networking stack
    RX_STAT_FORWARD = CO_BIT(0),
    /// A new buffer has to be allocated
    RX_STAT_ALLOC = CO_BIT(1),
    /// The buffer has to be deleted
    RX_STAT_DELETE = CO_BIT(2),
    /// The length of the buffer has to be updated
    RX_STAT_LEN_UPDATE = CO_BIT(3),
    /// The length in the Ethernet header has to be updated
    RX_STAT_ETH_LEN_UPDATE = CO_BIT(4),
    /// Simple copy
    RX_STAT_COPY = CO_BIT(5),
    /// Spurious frame (inform upper layer and discard)
    RX_STAT_SPURIOUS = CO_BIT(6),
    /// Frame for monitor interface
    RX_STAT_MONITOR = CO_BIT(7),
    /// unsupported frame
    RX_STAT_UF = CO_BIT(8),
};

enum mm_features
{
    /// Beaconing
    MM_FEAT_BCN_BIT         = 0,
    /// Radar Detection
    MM_FEAT_RADAR_BIT,
    /// Power Save
    MM_FEAT_PS_BIT,
    /// UAPSD
    MM_FEAT_UAPSD_BIT,
    /// A-MPDU
    MM_FEAT_AMPDU_BIT,
    /// A-MSDU
    MM_FEAT_AMSDU_BIT,
    /// P2P
    MM_FEAT_P2P_BIT,
    /// P2P Go
    MM_FEAT_P2P_GO_BIT,
    /// UMAC Present
    MM_FEAT_UMAC_BIT,
    /// VHT support
    MM_FEAT_VHT_BIT,
    /// Beamformee
    MM_FEAT_BFMEE_BIT,
    /// Beamformer
    MM_FEAT_BFMER_BIT,
    /// WAPI
    MM_FEAT_WAPI_BIT,
    /// MFP
    MM_FEAT_MFP_BIT,
    /// Mu-MIMO RX support
    MM_FEAT_MU_MIMO_RX_BIT,
    /// Mu-MIMO TX support
    MM_FEAT_MU_MIMO_TX_BIT,
    /// Wireless Mesh Networking
    MM_FEAT_MESH_BIT,
    /// TDLS support
    MM_FEAT_TDLS_BIT,
    /// Antenna Diversity support
    MM_FEAT_ANT_DIV_BIT,
    /// UF support
    MM_FEAT_UF_BIT,
    /// A-MSDU maximum size (bit0)
    MM_AMSDU_MAX_SIZE_BIT0,
    /// A-MSDU maximum size (bit1)
    MM_AMSDU_MAX_SIZE_BIT1,
    /// MON_DATA support
    MM_FEAT_MON_DATA_BIT,
    /// HE (802.11ax) support
    MM_FEAT_HE_BIT,
    /// TWT support
    MM_FEAT_TWT_BIT,
    /// FTM initiator support
    MM_FEAT_FTM_INIT_BIT,
    /// Fake FTM responder support
    MM_FEAT_FAKE_FTM_RSP_BIT
};

enum dbg_msg_tag
{
    /// Memory read request
    DBG_MEM_READ_REQ = KE_FIRST_MSG(TASK_DBG),
    /// Memory read confirm
    DBG_MEM_READ_CFM,
    /// Memory write request
    DBG_MEM_WRITE_REQ,
    /// Memory write confirm
    DBG_MEM_WRITE_CFM,
    /// Module filter request
    DBG_SET_MOD_FILTER_REQ,
    /// Module filter confirm
    DBG_SET_MOD_FILTER_CFM,
    /// Severity filter request
    DBG_SET_SEV_FILTER_REQ,
    /// Severity filter confirm
    DBG_SET_SEV_FILTER_CFM,
    /// Fatal error indication
    DBG_ERROR_IND,
    /// Request to get system statistics
    DBG_GET_SYS_STAT_REQ,
    /// COnfirmation of system statistics
    DBG_GET_SYS_STAT_CFM,


    /*
     * Section of internal DBG messages. No DBG API messages should be defined below this point
     */
    /// Timer allowing resetting the system statistics periodically to avoid
    /// wrap around of timer
    DBG_SYS_STAT_TIMER,
};

#if NX_POWERSAVE
enum
{
    /// Power-save off
    PS_MODE_OFF,
    /// Power-save on - Normal mode
    PS_MODE_ON,
    /// Power-save on - Dynamic mode
    PS_MODE_ON_DYN,
};
#endif

#define VIF_TO_BCMC_IDX(idx) (BROADCAST_STA_IDX_MIN + (idx))
