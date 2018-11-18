/** @file  uaputl.h
 *
 *  @brief Header file for uaputl application
 * 
 * Copyright (C) 2008-2009, Marvell International Ltd. 
 *
 * This software file (the "File") is distributed by Marvell International 
 * Ltd. under the terms of the GNU General Public License Version 2, June 1991 
 * (the "License").  You may use, redistribute and/or modify this File in 
 * accordance with the terms and conditions of the License, a copy of which 
 * is available along with the File in the gpl.txt file or by writing to 
 * the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 
 * 02111-1307 or on the worldwide web at http://www.gnu.org/licenses/gpl.txt.
 *
 * THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE 
 * ARE EXPRESSLY DISCLAIMED.  The License provides additional details about 
 * this warranty disclaimer.
 *
 */
/************************************************************************
Change log:
    03/01/08: Initial creation
************************************************************************/

#ifndef _UAP_H
#define _UAP_H

#if (BYTE_ORDER == LITTLE_ENDIAN)
#undef BIG_ENDIAN
#endif

/** 16 bits byte swap */
#define swap_byte_16(x) \
  ((u16)((((u16)(x) & 0x00ffU) << 8) | \
         (((u16)(x) & 0xff00U) >> 8)))

/** 32 bits byte swap */
#define swap_byte_32(x) \
  ((u32)((((u32)(x) & 0x000000ffUL) << 24) | \
         (((u32)(x) & 0x0000ff00UL) <<  8) | \
         (((u32)(x) & 0x00ff0000UL) >>  8) | \
         (((u32)(x) & 0xff000000UL) >> 24)))

/** 64 bits byte swap */
#define swap_byte_64(x) \
  ((u64)((u64)(((u64)(x) & 0x00000000000000ffULL) << 56) | \
         (u64)(((u64)(x) & 0x000000000000ff00ULL) << 40) | \
         (u64)(((u64)(x) & 0x0000000000ff0000ULL) << 24) | \
         (u64)(((u64)(x) & 0x00000000ff000000ULL) <<  8) | \
         (u64)(((u64)(x) & 0x000000ff00000000ULL) >>  8) | \
         (u64)(((u64)(x) & 0x0000ff0000000000ULL) >> 24) | \
         (u64)(((u64)(x) & 0x00ff000000000000ULL) >> 40) | \
         (u64)(((u64)(x) & 0xff00000000000000ULL) >> 56) ))

#ifdef BIG_ENDIAN
/** Convert from 16 bit little endian format to CPU format */
#define uap_le16_to_cpu(x) swap_byte_16(x)
/** Convert from 32 bit little endian format to CPU format */
#define uap_le32_to_cpu(x) swap_byte_32(x)
/** Convert from 64 bit little endian format to CPU format */
#define uap_le64_to_cpu(x) swap_byte_64(x)
/** Convert to 16 bit little endian format from CPU format */
#define uap_cpu_to_le16(x) swap_byte_16(x)
/** Convert to 32 bit little endian format from CPU format */
#define uap_cpu_to_le32(x) swap_byte_32(x)
/** Convert to 64 bit little endian format from CPU format */
#define uap_cpu_to_le64(x) swap_byte_64(x)

/** Convert APCMD header to little endian format from CPU format */
#define endian_convert_request_header(x);               \
    {                                                   \
        (x)->CmdCode = uap_cpu_to_le16((x)->CmdCode);   \
        (x)->Size = uap_cpu_to_le16((x)->Size);         \
        (x)->SeqNum = uap_cpu_to_le16((x)->SeqNum);     \
        (x)->Result = uap_cpu_to_le16((x)->Result);     \
    }

/** Convert APCMD header from little endian format to CPU format */
#define endian_convert_response_header(x);              \
    {                                                   \
        (x)->CmdCode = uap_le16_to_cpu((x)->CmdCode);   \
        (x)->Size = uap_le16_to_cpu((x)->Size);         \
        (x)->SeqNum = uap_le16_to_cpu((x)->SeqNum);     \
        (x)->Result = uap_le16_to_cpu((x)->Result);     \
    }

/** Convert TLV header to little endian format from CPU format */
#define endian_convert_tlv_header_out(x);           \
    {                                               \
        (x)->Tag = uap_cpu_to_le16((x)->Tag);       \
        (x)->Length = uap_cpu_to_le16((x)->Length); \
    }

/** Convert TLV header from little endian format to CPU format */
#define endian_convert_tlv_header_in(x);            \
    {                                               \
        (x)->Tag = uap_le16_to_cpu((x)->Tag);       \
        (x)->Length = uap_le16_to_cpu((x)->Length); \
    }

#else /* BIG_ENDIAN */
/** Do nothing */
#define uap_le16_to_cpu(x) x
/** Do nothing */
#define uap_le32_to_cpu(x) x
/** Do nothing */
#define uap_le64_to_cpu(x) x
/** Do nothing */
#define uap_cpu_to_le16(x) x
/** Do nothing */
#define uap_cpu_to_le32(x) x
/** Do nothing */
#define uap_cpu_to_le64(x) x

/** Do nothing */
#define endian_convert_request_header(x)
/** Do nothing */
#define endian_convert_response_header(x)
/** Do nothing */
#define endian_convert_tlv_header_out(x)
/** Do nothing */
#define endian_convert_tlv_header_in(x)
#endif /* BIG_ENDIAN */

/** uAP application version string */
#define UAP_VERSION         "1.12"
/** Host Command ioctl number */
#define UAPHOSTCMD          (SIOCDEVPRIVATE + 1)
/** Private command ID to Power Mode */
#define	UAP_POWER_MODE	    (SIOCDEVPRIVATE + 3)

/** Default device name */
#define DEFAULT_DEV_NAME    "uap0"

/** Success */
#define UAP_SUCCESS     1
/** Failure */
#define UAP_FAILURE     0
/** MAC BROADCAST */
#define UAP_RET_MAC_BROADCAST   0x1FF
/** MAC MULTICAST */
#define UAP_RET_MAC_MULTICAST   0x1FE

/** Command is successful */
#define CMD_SUCCESS     0
/** Command fails */
#define CMD_FAILURE     -1

/** BSS start error : Invalid parameters */
#define BSS_FAILURE_START_INVAL     -2
/** BSS start error : BSS already started */
#define BSS_FAILURE_START_REDUNDANT -3

/** BSS stop error : BSS already stopped */
#define BSS_FAILURE_STOP_REDUNDANT  -2
/** BSS stop error : No active BSS */
#define BSS_FAILURE_STOP_INVAL      -3

/** Maximum line length for config file */
#define MAX_LINE_LENGTH         240
/** Maximum command length */
#define MAX_CMD_LENGTH          100
/** Size of command buffer */
#define MRVDRV_SIZE_OF_CMD_BUFFER       (2 * 1024)
/** Maximum number of clients supported by AP */
#define MAX_NUM_CLIENTS         16
/** Maximum number of MAC addresses for one-shot filter modifications */
#define MAX_MAC_ONESHOT_FILTER  16
/** Maximum SSID length */
#define MAX_SSID_LENGTH         32
/** Maximum SSID length */
#define MIN_SSID_LENGTH         1
/** Maximum WPA passphrase length */
#define MAX_WPA_PASSPHRASE_LENGTH   64
/** Minimum WPA passphrase length */
#define MIN_WPA_PASSPHRASE_LENGTH   8
/** Maximum data rates */
#define MAX_DATA_RATES          14
/** Maximum length of lines in configuration file */
#define MAX_CONFIG_LINE         240
/** MSB bit is set if its a basic rate */
#define BASIC_RATE_SET_BIT     0x80
/** Maximum group key timer */
#define MAX_GRP_TIMER   86400
/** Maximum Retry Limit */
#define MAX_RETRY_LIMIT 	14

/** Maximum TX Power Limit */
#define MAX_TX_POWER    20
/** Minimum TX Power Limit */
#define MIN_TX_POWER    0

/** Maximum channels */
#define MAX_CHANNELS    14
/** Maximum RTS threshold */
#define MAX_RTS_THRESHOLD   2347

/** Maximum fragmentation threshold */
#define MAX_FRAG_THRESHOLD 2346
/** Minimum fragmentation threshold */
#define MIN_FRAG_THRESHOLD 256

/** Maximum stage out time */
#define MAX_STAGE_OUT_TIME  864000
/** Minimum stage out time */
#define MIN_STAGE_OUT_TIME  300

/** Maximum DTIM period */
#define MAX_DTIM_PERIOD 100

/** Maximum BEACON period */
#define MAX_BEACON_PERIOD 4000

/** Minimum BEACON period */
#define MIN_BEACON_PERIOD 50

/** Maximum IE buffer length */
#define MAX_IE_BUFFER_LEN 256

/** Maximum custom IE count */
#define MAX_CUSTOM_IE_COUNT 4

/** Maximum number of rates allowed at a time */
#define MAX_RATES               12

/** Default wait period in seconds */
#define DEFAULT_WAIT_TIME       3

#ifdef __GNUC__
/** Structure packing begins */
#define PACK_START
/** Structure packeing end */
#define PACK_END  __attribute__ ((packed))
#else
/** Structure packing begins */
#define PACK_START   __packed
/** Structure packeing end */
#define PACK_END
#endif

#ifndef ETH_ALEN
/** MAC address length */
#define ETH_ALEN    6
#endif

/** Action field value : get */
#define ACTION_GET  0
/** Action field value : set */
#define ACTION_SET  1
/**
 * Hex or Decimal to Integer 
 * @param   num string to convert into decimal or hex
 */
#define A2HEXDECIMAL(num)  \
    (strncasecmp("0x", (num), 2)?(unsigned int) strtoll((num),NULL,0):a2hex((num)))\

/**
 * Check of decimal or hex string
 * @param   num string
 */
#define IS_HEX_OR_DIGIT(num) \
    (strncasecmp("0x", (num), 2)?ISDIGIT((num)):ishexstring((num)))\

/** Find minimum value */
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

/** Character, 1 byte */
typedef char s8;
/** Unsigned character, 1 byte */
typedef unsigned char u8;

/** Short integer */
typedef signed short s16;
/** Unsigned short integer */
typedef unsigned short u16;

/** Long integer */
typedef signed long s32;
/** Unsigned long integer */
typedef unsigned long u32;

/** Valid Input Commands */
typedef enum
{
    RDEEPROM,
    SCANCHANNELS,
    TXPOWER,
    PROTOCOL,
    CHANNEL,
    RATE,
    BROADCASTSSID,
    RTSTHRESH,
    FRAGTHRESH,
    DTIMPERIOD,
    RADIOCONTROL,
    TXDATARATE,
    MCBCDATARATE,
    PKTFWD,
    STAAGEOUTTIMER,
    AUTHMODE,
    GROUPREKEYTIMER,
    MAXSTANUM,
    BEACONPERIOD,
    RETRYLIMIT,
    RSNREPLAYPROT,
    COEX_COMM_BITMAP,
    COEX_PROTECTION,
    COEX_SCO_ACL_FREQ,
    COEX_ACL_ENABLED,
    COEX_ACL_BT_TIME,
    COEX_ACL_WLAN_TIME,
} valid_inputs;

/** Message verbosity level */
enum
{ MSG_NONE, MSG_DEBUG, MSG_ALL };

/** oids_table */
typedef struct
{
    /** oid type */
    u16 type;
    /** oid len */
    u16 len;
    /** oid name */
    char *name;
} oids_table;

/** 4 byte header to store buf len*/
#define BUF_HEADER_SIZE	4

/** AP CMD header */
#define APCMDHEADER     /** Buf Size */				    \
			u32 BufSize;				    \
			/** CmdCode */				    \
			u16 CmdCode;     			    \
			/** Size */				    \
                        u16 Size;        		            \
			/** SeqNum */				    \
                        u16 SeqNum;      			    \
			/** Result */				    \
                        s16 Result

/** TLV header */
#define TLVHEADER       /** Tag */				    \
			u16 Tag;         		            \
			/** Length */				    \
                        u16 Length

/* TLV Definitions */

/** TLV buffer header*/
typedef PACK_START struct _TLVBUF_HEADER
{
    /** Header type */
    u16 Type;
    /** Header length */
    u16 Len;
    /** Data */
    u8 Data[0];
} PACK_END TLVBUF_HEADER;

/** Band config ACS mode */
#define BAND_CONFIG_ACS_MODE    0x40

/** TLV buffer : Channel Config */
typedef PACK_START struct _TLVBUF_CHANNEL_CONFIG
{
    /** Header */
    TLVHEADER;
    /** Band Configuration 
     *
     * [7-6] Channel Selection Mode; 00 manual, 01 ACS
     * [3-2] Channel Width; 00 20 MHz
     * [1-0] Band Info; 00 2.4 GHz
     */
    u8 BandConfigType;
    /** Channel number */
    u8 ChanNumber;
} PACK_END TLVBUF_CHANNEL_CONFIG;

/** Channel List Entry */
typedef PACK_START struct _CHANNEL_LIST
{
    /** Band Config */
    u8 BandConfigType;
    /** Channel Number */
    u8 ChanNumber;
    /** Reserved */
    u8 Reserved1;
    /** Reserved */
    u16 Reserved2;
    /** Reserved */
    u16 Reserved3;
} PACK_END CHANNEL_LIST;

/** TLV buffer : Channel List */
typedef PACK_START struct _TLVBUF_CHANNEL_LIST
{
    /** Header */
    TLVHEADER;
    /** Channel List */
    CHANNEL_LIST ChanList[0];
} PACK_END TLVBUF_CHANNEL_LIST;

/** TLV buffer : AP MAC address */
typedef PACK_START struct _TLVBUF_AP_MAC_ADDRESS
{
    /** Header */
    TLVHEADER;
    /** AP MAC address */
    u8 ApMacAddr[ETH_ALEN];
} PACK_END TLVBUF_AP_MAC_ADDRESS;

/** TLV buffer : SSID */
typedef PACK_START struct _TLVBUF_SSID
{
    /** Header */
    TLVHEADER;
    /** SSID */
    u8 Ssid[0];
} PACK_END TLVBUF_SSID;

/** TLV buffer : Beacon period */
typedef PACK_START struct _TLVBUF_BEACON_PERIOD
{
    /** Header */
    TLVHEADER;
    /** Beacon period */
    u16 BeaconPeriod_ms;
} PACK_END TLVBUF_BEACON_PERIOD;

/** TLV buffer : DTIM period */
typedef PACK_START struct _TLVBUF_DTIM_PERIOD
{
    /** Header */
    TLVHEADER;
    /** DTIM period */
    u8 DtimPeriod;
} PACK_END TLVBUF_DTIM_PERIOD;

/** TLV buffer : Channel */
typedef PACK_START struct _TLVBUF_PHYPARAMDSSET
{
    /** Header */
    TLVHEADER;
    /** Channel */
    u8 Channel;
} PACK_END TLVBUF_PHYPARAMDSSET;

/** TLV buffer : Operational rates */
typedef PACK_START struct _TLVBUF_RATES
{
    /** Header */
    TLVHEADER;
    /** Operational rates */
    u8 OperationalRates[0];
} PACK_END TLVBUF_RATES;

/** TLV buffer : Tx power */
typedef PACK_START struct _TLVBUF_TX_POWER
{
    /** Header */
    TLVHEADER;
    /** Tx power in dBm */
    u8 TxPower_dBm;
} PACK_END TLVBUF_TX_POWER;

/** TLV buffer : SSID broadcast control */
typedef PACK_START struct _TLVBUF_BCAST_SSID_CTL
{
    /** Header */
    TLVHEADER;
    /** SSID broadcast control flag */
    u8 BcastSsidCtl;
} PACK_END TLVBUF_BCAST_SSID_CTL;

/** TLV buffer : RSN replay protection */
typedef PACK_START struct _tlvbuf_rsn_replay_prot
{
    /** Header */
    TLVHEADER;
    /** RSN replay protection control flag */
    u8 rsn_replay_prot;
} PACK_END tlvbuf_rsn_replay_prot;

/** TLV buffer : Preamble control */
typedef PACK_START struct _TLVBUF_PREAMBLE_CTL
{
    /** Header */
    TLVHEADER;
    /** Preamble type */
    u8 PreambleType;
} PACK_END TLVBUF_PREAMBLE_CTL;

/** TLV buffer : Antenna control */
typedef PACK_START struct _TLVBUF_ANTENNA_CTL
{
    /** Header */
    TLVHEADER;
    /** Antenna type */
    u8 WhichAntenna;
    /** Antenna mode */
    u8 AntennaMode;
} PACK_END TLVBUF_ANTENNA_CTL;

/** TLV buffer : RTS threshold */
typedef PACK_START struct _TLVBUF_RTS_THRESHOLD
{
    /** Header */
    TLVHEADER;
    /** RTS threshold */
    u16 RtsThreshold;
} PACK_END TLVBUF_RTS_THRESHOLD;

/** TLV buffer : Radio control */
typedef PACK_START struct _TLVBUF_RADIO_CTL
{
    /** Header */
    TLVHEADER;
    /** Radio control flag */
    u8 RadioCtl;
} PACK_END TLVBUF_RADIO_CTL;

/** TLV buffer : Tx data rate */
typedef PACK_START struct _TLVBUF_TX_DATA_RATE
{
    /** Header */
    TLVHEADER;
    /** Tx data rate */
    u16 TxDataRate;
} PACK_END TLVBUF_TX_DATA_RATE;

/** TLV buffer : MCBC Data Rate */
typedef PACK_START struct _TLVBUF_MCBC_DATA_RATE
{
    /** Header */
    TLVHEADER;
    /** MCBC data rate */
    u16 MCBCdatarate;
} PACK_END TLVBUF_MCBC_DATA_RATE;

/** TLV buffer : Packet forward control */
typedef PACK_START struct _TLVBUF_PKT_FWD_CTL
{
    /** Header */
    TLVHEADER;
    /** Packet forwarding control flag */
    u8 PktFwdCtl;
} PACK_END TLVBUF_PKT_FWD_CTL;

/** TLV buffer : STA information */
typedef PACK_START struct _TLVBUF_STA_INFO
{
    /** Header */
    TLVHEADER;
    /** STA MAC address */
    u8 MacAddress[ETH_ALEN];
    /** Power mfg status */
    u8 PowerMfgStatus;
    /** RSSI */
    s8 Rssi;
} PACK_END TLVBUF_STA_INFO;

/** TLV buffer : STA MAC address filtering control */
typedef PACK_START struct _TLVBUF_STA_MAC_ADDR_FILTER
{
    /** Header */
    TLVHEADER;
    /** Filter mode */
    u8 FilterMode;
    /** Number of STA MACs */
    u8 Count;
    /** STA MAC addresses buffer */
    u8 MacAddress[0];
} PACK_END TLVBUF_STA_MAC_ADDR_FILTER;

/** TLV buffer : STA ageout timer */
typedef PACK_START struct _TLVBUF_STA_AGEOUT_TIMER
{
    /** Header */
    TLVHEADER;
    /** STA ageout timer in ms */
    u32 StaAgeoutTimer_ms;
} PACK_END TLVBUF_STA_AGEOUT_TIMER;

/** TLV buffer : max station number */
typedef PACK_START struct _TLVBUF_MAX_STA_NUM
{
    /** Header */
    TLVHEADER;
    /** max station number */
    u16 Max_sta_num;
} PACK_END TLVBUF_MAX_STA_NUM;

/** TLV buffer : retry limit */
typedef PACK_START struct _TLVBUF_RETRY_LIMIT
{
    /** Header */
    TLVHEADER;
    /** retry limit */
    u8 retry_limit;
} PACK_END TLVBUF_RETRY_LIMIT;

/* Bitmap for protocol to use */
/** No security */
#define PROTOCOL_NO_SECURITY         	1
/** Static WEP */
#define PROTOCOL_STATIC_WEP	        2
/** WPA */
#define PROTOCOL_WPA		        8
/** WPA2 */
#define PROTOCOL_WPA2          		32
/** WP2 Mixed */
#define PROTOCOL_WPA2_MIXED         	40

/* Bitmap for unicast/bcast cipher type */
/** None */
#define CIPHER_NONE                 0
/** WEP 40 */
#define CIPHER_WEP_40         		1
/** WEP 104 */
#define CIPHER_WEP_104	        	2
/** TKIP */
#define CIPHER_TKIP		        4
/** AES CCMP */
#define CIPHER_AES_CCMP        		8
/** valid cipher bitmap */
#define CIPHER_BITMAP			0x0c

/** TLV buffer : Authentication Mode */
typedef PACK_START struct _TLVBUF_AUTH_MODE
{
    /** Header */
    TLVHEADER;
    /** Authentication Mode */
    u8 AuthMode;
} PACK_END TLVBUF_AUTH_MODE;

/** TLV buffer : Security Protocol */
typedef PACK_START struct _TLVBUF_PROTOCOL
{
    /** Header */
    TLVHEADER;
    /** Security protocol */
    u16 Protocol;
} PACK_END TLVBUF_PROTOCOL;

/** TLV buffer : cipher */
typedef PACK_START struct _TLVBUF_CIPHER
{
    /** Header */
    TLVHEADER;
    /** Pairwise cipher */
    u8 PairwiseCipher;
    /** Group cipher */
    u8 GroupCipher;
} PACK_END TLVBUF_CIPHER;

/** TLV buffer : Group re-key time */
typedef PACK_START struct _TLVBUF_GROUP_REKEY_TIMER
{
    /** Header */
    TLVHEADER;
    /** Group rekey time in seconds */
    u32 GroupRekeyTime_sec;
} PACK_END TLVBUF_GROUP_REKEY_TIMER;

/** Key_mgmt_psk */
#define KEY_MGMT_NONE	0x04
/** Key_mgmt_none */
#define KEY_MGMT_PSK	0x02

/** TLV buffer : KeyMgmt */
typedef PACK_START struct _TLVBUF_AKMP
{
    /** Header */
    TLVHEADER;
    /** KeyMgmt */
    u16 KeyMgmt;
} PACK_END TLVBUF_AKMP;

/** TLV buffer : Single WEP key */
typedef PACK_START struct _TLVBUF_WEP_KEY
{
    /** Header */
    TLVHEADER;
    /** Key index */
    u8 KeyIndex;
    /** Default key flag */
    u8 IsDefault;
    /** Key */
    u8 Key[0];
} PACK_END TLVBUF_WEP_KEY;

/** custom IE */
typedef PACK_START struct _custom_ie
{
    /** IE Index */
    u16 ie_index;
    /** Mgmt Subtype Mask */
    u16 mgmt_subtype_mask;
    /** IE Length */
    u16 ie_length;
    /** IE buffer */
    u8 ie_buffer[0];
} PACK_END custom_ie;

/** TLV buffer : custom IE */
typedef PACK_START struct _tlvbuf_custom_ie
{
    /** Header */
    TLVHEADER;
    /** custom IE data */
    custom_ie ie_data[0];
} PACK_END tlvbuf_custom_ie;

/** TLV buffer : WPA passphrase */
typedef PACK_START struct _TLVBUF_WPA_PASSPHRASE
{
    /** Header */
    TLVHEADER;
    /** WPA passphrase */
    u8 Passphrase[0];
} PACK_END TLVBUF_WPA_PASSPHRASE;

/** TLV buffer : Fragmentation threshold */
typedef PACK_START struct _TLVBUF_FRAG_THRESHOLD
{
    /** Header */
    TLVHEADER;
    /** Fragmentation threshold */
    u16 FragThreshold;
} PACK_END TLVBUF_FRAG_THRESHOLD;

/* APCMD definitions */
/** APCMD buffer */
typedef PACK_START struct _APCMDBUF
{
    /** Header */
    APCMDHEADER;
}
PACK_END APCMDBUF;

/** APCMD header length */
#define APCMDHEADERLEN  (sizeof(APCMDBUF))

/** APCMD buffer : sys_info request */
typedef PACK_START struct _APCMDBUF_SYS_INFO_REQUEST
{
    /** Header */
    APCMDHEADER;
} PACK_END APCMDBUF_SYS_INFO_REQUEST;

/** APCMD buffer : sys_info response */
typedef PACK_START struct _APCMDBUF_SYS_INFO_RESPONSE
{
    /** Header */
    APCMDHEADER;
    /** System information buffer */
    u8 SysInfo[64];
} PACK_END APCMDBUF_SYS_INFO_RESPONSE;

/** APCMD buffer : sys_reset */
typedef PACK_START struct _APCMDBUF_SYS_RESET
{
    /** Header */
    APCMDHEADER;
} PACK_END APCMDBUF_SYS_RESET;

/** APCMD buffer : sys_configure */
typedef PACK_START struct _APCMDBUF_SYS_CONFIGURE
{
    /** Header */
    APCMDHEADER;
    /** Action : GET or SET */
    u16 Action;
} PACK_END APCMDBUF_SYS_CONFIGURE;

/** APCMD buffer : SNMP MIB */
typedef PACK_START struct _APCMDBUF_SNMP_MIB
{
    /** Header */
    APCMDHEADER;
    /** Action : GET or SET */
    u16 Action;
} PACK_END APCMDBUF_SNMP_MIB;
/** APCMD buffer : bss_start */
typedef PACK_START struct _APCMDBUF_BSS_START
{
    /** Header */
    APCMDHEADER;
} PACK_END APCMDBUF_BSS_START;

/** APCMD buffer : bss_stop */
typedef PACK_START struct _APCMDBUF_BSS_STOP
{
    /** Header */
    APCMDHEADER;
} PACK_END APCMDBUF_BSS_STOP;

/** APCMD buffer : sta_list request */
typedef PACK_START struct _APCMDBUF_STA_LIST_REQUEST
{
    /** Header */
    APCMDHEADER;
} PACK_END APCMDBUF_STA_LIST_REQUEST;

/** APCMD buffer : sta_list response */
typedef PACK_START struct _APCMDBUF_STA_LIST_RESPONSE
{
    /** Header */
    APCMDHEADER;
    /** Number of STAs */
    u16 StaCount;
    /** STA information TLVs */
    TLVBUF_STA_INFO StaList[0];
} PACK_END APCMDBUF_STA_LIST_RESPONSE;

/** APCMD buffer : sta_deauth */
typedef PACK_START struct _APCMDBUF_STA_DEAUTH
{
    /** Header */
    APCMDHEADER;
    /** STA MAC address to deauthenticate */
    u8 StaMacAddress[ETH_ALEN];
    /** Reason Code */
    u16 ReasonCode;
} PACK_END APCMDBUF_STA_DEAUTH;

/** TLV : BT Coex common configuration */
typedef PACK_START struct _tlvbuf_coex_common_cfg
{
    /** Header */
    TLVHEADER;
    /** Configuration bitmap */
    u32 config_bitmap;
    /** Reserved */
    u32 reserved[4];
} PACK_END tlvbuf_coex_common_cfg;

/** TLV : BT Coex SCO configuration */
typedef PACK_START struct _tlvbuf_coex_sco_cfg
{
    /** Header */
    TLVHEADER;
    /** Qtime protection */
    u16 protection_qtime[4];
    /** Rate protection */
    u16 protection_rate;
    /** ACL frequency */
    u16 acl_frequency;
    /** Reserved */
    u32 reserved[4];
} PACK_END tlvbuf_coex_sco_cfg;

/** TLV : BT Coex ACL configuration */
typedef PACK_START struct _tlvbuf_coex_acl_cfg
{
    /** Header */
    TLVHEADER;
    /** Enabled or not */
    u16 enabled;
    /** BT time */
    u16 bt_time;
    /** Wlan time */
    u16 wlan_time;
    /** Rate protection */
    u16 protection_rate;
    /** Reserved */
    u32 reserved[4];
} PACK_END tlvbuf_coex_acl_cfg;

/** TLV : BT Coex statistics */
typedef PACK_START struct _tlvbuf_coex_stats
{
    /** Header */
    TLVHEADER;
    /** Null not sent */
    u32 null_not_sent;
    /** Null queued */
    u32 null_queued;
    /** Null not queued */
    u32 null_not_queued;
    /** CF end queued */
    u32 cf_end_queued;
    /** CF end not queued */
    u32 cf_end_not_queued;
    /** Null allocation failures */
    u32 null_alloc_fail;
    /** CF end allocation failures */
    u32 cf_end_alloc_fail;
    /** Reserved */
    u32 reserved[8];
} PACK_END tlvbuf_coex_stats;

/** APCMD buffer : BT Coex API extension */
typedef PACK_START struct _apcmdbuf_coex_config
{
    /** Header */
    APCMDHEADER;
    /** Action : GET or SET */
    u16 action;
    /** Reserved for alignment */
    u16 coex_reserved;
    /** TLV buffer */
    u8 tlv_buffer[0];
} PACK_END apcmdbuf_coex_config;

/** Reg TYPE*/
enum reg_commands
{
    CMD_MAC = 0,
    CMD_BBP,
    CMD_RF
};

/** APCMD buffer: Regrdwr */
typedef PACK_START struct _APCMDBUF_REG_RDWR
{
   /** Header */
    APCMDHEADER;
   /** Read or Write */
    u16 Action;
   /** Register offset */
    u16 Offset;
   /** Value */
    u32 Value;
} PACK_END APCMDBUF_REG_RDWR;

/** sub-band type */
typedef PACK_START struct _IEEEtypes_SubbandSet
{
    u8 FirstChan;       /**< First channel */
    u8 NoOfChan;        /**< Number of channels */
    u8 MaxTxPwr;        /**< Maximum Tx power */
} PACK_END IEEEtypes_SubbandSet_t;

/** country code length  used for 802.11D */
#define COUNTRY_CODE_LEN    3

/** MAX domain SUB-BAND*/
#define MAX_SUB_BANDS 40

/** Max Multi Domain Entries for G */
#define MaxMultiDomainCapabilityEntryG 1

/** Max Multi Domain Entries for A */
#define MaxMultiDomainCapabilityEntryA 31

/** Country code and Sub-band */
typedef PACK_START struct domain_param
{
    TLVHEADER;
    u8 CountryCode[COUNTRY_CODE_LEN];           /**< Country code */
    IEEEtypes_SubbandSet_t Subband[0];          /**< Set of subbands */
} PACK_END domain_param_t;

/** HostCmd_CFG_80211D */
typedef PACK_START struct _APCMDBUF_CFG_80211D
{
    /** Header */
    APCMDHEADER;
    /** Action */
    u16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    /** Domain parameters */
    domain_param_t Domain;
} PACK_END APCMDBUF_CFG_80211D;

/** HostCmd_MEM_ACCESS */
typedef PACK_START struct _APCMDBUF_MEM_ACCESS
{
    /** Header */
    APCMDHEADER;
    /** Action */
    u16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    /** Reserved field */
    u16 Reserved;
    /** Address */
    u32 Address;
    /** Value */
    u32 Value;
} PACK_END APCMDBUF_MEM_ACCESS;

/** HostCmd_EEPROM_ACCESS */
typedef PACK_START struct _APCMDBUF_EEPROM_ACCESS
{
    /** Header */
    APCMDHEADER;
    /** Action */
    u16 Action;                 /* 0 = ACT_GET; */
    /** Reserved field */
    u16 Offset;                 /* Multiples of 4 */
    /** Address */
    u16 ByteCount;              /* Multiples of 4 */
    /** Value */
    u8 Value[1];
} PACK_END APCMDBUF_EEPROM_ACCESS;

/** Max EEPROM length */
#define MAX_EEPROM_LEN         20

/**subcmd id for glbal flag */
#define DEBUG_SUBCOMMAND_GMODE 		1
/**subcmd id for Majorid mask */
#define DEBUG_SUBCOMMAND_MAJOREVTMASK 	2
/**subcmd id to trigger a scan */
#define DEBUG_SUBCOMMAND_CHANNEL_SCAN   3

/** Channel scan entry for each channel */
typedef PACK_START struct _CHANNEL_SCAN_ENTRY_T
{
    /** Channel Number */
    u8 chan_num;
    /** Number of APs */
    u8 num_of_aps;
    /** CCA count */
    u32 CCA_count;
    /** Duration */
    u32 duration;
    /** Channel weight */
    u32 channel_weight;
} PACK_END CHANNEL_SCAN_ENTRY_T;

/** Channel scan entry */
typedef PACK_START struct _CHANNEL_SCAN_ENTRY
{
    /** Number of channels */
    u8 numChannels;
    /** Channel scan entry */
    CHANNEL_SCAN_ENTRY_T cst[0];
} PACK_END CHANNEL_SCAN_ENTRY;

/** debugConfig_t */
typedef PACK_START union
{
        /** used in all new debug commands */
    u32 value;
        /** used in DEBUG_SUBCOMMAND_GMODE */
    u8 globalDebugMode;
        /** used in DEBUG_SUBCOMMAND_MAJOREVTMASK */
    u32 debugMajorIdMask;
        /** used in DEBUG_SUBCOMMAND_CHANNEL_SCAN */
    CHANNEL_SCAN_ENTRY cs_entry;
} PACK_END debugConfig_t;

/** HostCmd_SYS_DEBUG */
typedef PACK_START struct _APCMDBUF_SYS_DEBUG
{
    /** Header */
    APCMDHEADER;
    /** Action */
    u16 Action;                 /* 0 = ACT_GET; 1 = ACT_SET; */
    /** Sub command */
    u32 subcmd;
    /** debug parameter */
    debugConfig_t debugConfig;
} PACK_END APCMDBUF_SYS_DEBUG;

/** HostCmd_CFG_DATA */
typedef PACK_START struct _APCMDBUF_CFG_DATA
{
    /** Header */
    APCMDHEADER;
    /** Action */
    u16 action;
    /** Type */
    u16 type;
    /** Data length */
    u16 data_len;
    /** Data */
    u8 data[0];
} PACK_END APCMDBUF_CFG_DATA;

/** Maximum size of set/get configurations */
#define MAX_CFG_DATA_SIZE		2000    /* less than
                                                   MRVDRV_SIZE_OF_CMD_BUFFER */

/** Host Command ID bit mask (bit 11:0) */
#define HostCmd_CMD_ID_MASK             0x0fff
/** APCMD response check */
#define APCMD_RESP_CHECK            0x8000

/* AP CMD IDs */
/** APCMD : sys_info */
#define APCMD_SYS_INFO              0x00ae
/** APCMD : sys_reset */
#define APCMD_SYS_RESET             0x00af
/** APCMD : sys_configure */
#define APCMD_SYS_CONFIGURE         0x00b0
/** APCMD : bss_start */
#define APCMD_BSS_START             0x00b1
/** APCMD : bss_stop */
#define APCMD_BSS_STOP              0x00b2
/** APCMD : sta_list */
#define APCMD_STA_LIST              0x00b3
/** APCMD : sta_deauth */
#define APCMD_STA_DEAUTH            0x00b5
/** SNMP MIB SET/GET */
#define HostCmd_SNMP_MIB            0x0016
/** Read/Write Mac register */
#define HostCmd_CMD_MAC_REG_ACCESS  0x0019
/** Read/Write BBP register */
#define HostCmd_CMD_BBP_REG_ACCESS  0x001a
/** Read/Write RF register */
#define HostCmd_CMD_RF_REG_ACCESS   0x001b
/** Host Command ID : EEPROM access */
#define HostCmd_EEPROM_ACCESS       0x0059
/** Host Command ID : Memory access */
#define HostCmd_CMD_MEM_ACCESS      0x0086
/** Host Command ID : 802.11D configuration */
#define HostCmd_CMD_802_11D_DOMAIN_INFO      0x005b
/** Host Command ID : Configuration data */
#define HostCmd_CMD_CFG_DATA        0x008f
/** Host Command ID:  SYS_DEBUG */
#define APCMD_SYS_DEBUG		    0x00db

/** Host Command ID:  ROBUST_COEX */
#define HostCmd_ROBUST_COEX              0x00e0

/** Oid for 802.11D enable/disable */
#define OID_80211D_ENABLE           0x0009

/* TLV IDs */
/** TLV : Base */
#define PROPRIETARY_TLV_BASE_ID         0x0100

/**TLV: Domain type */
#define TLV_TYPE_DOMAIN                 0x0007

/** TLV : SSID */
#define MRVL_SSID_TLV_ID                0x0000
/** TLV : Operational rates */
#define MRVL_RATES_TLV_ID         	0x0001
/** TLV : Channel */
#define MRVL_PHYPARAMDSSET_TLV_ID       0x0003
/** TLV type : Scan Channels list */
#define MRVL_CHANNELLIST_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 1)
/** TLV type : Authentication type */
#define MRVL_AUTH_TLV_ID              (PROPRIETARY_TLV_BASE_ID + 31)
/** TLV Id : Channel Config */
#define MRVL_CHANNELCONFIG_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 42)
/** TLV : AP MAC address */
#define MRVL_AP_MAC_ADDRESS_TLV_ID      (PROPRIETARY_TLV_BASE_ID + 43)
/** TLV : Beacon period */
#define MRVL_BEACON_PERIOD_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 44)
/** TLV : DTIM period */
#define MRVL_DTIM_PERIOD_TLV_ID         (PROPRIETARY_TLV_BASE_ID + 45)
/** TLV : Tx power */
#define MRVL_TX_POWER_TLV_ID            (PROPRIETARY_TLV_BASE_ID + 47)
/** TLV : SSID broadcast control */
#define MRVL_BCAST_SSID_CTL_TLV_ID      (PROPRIETARY_TLV_BASE_ID + 48)
/** TLV : Preamble control */
#define MRVL_PREAMBLE_CTL_TLV_ID        (PROPRIETARY_TLV_BASE_ID + 49)
/** TLV : Antenna control */
#define MRVL_ANTENNA_CTL_TLV_ID         (PROPRIETARY_TLV_BASE_ID + 50)
/** TLV : RTS threshold */
#define MRVL_RTS_THRESHOLD_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 51)
/** TLV : Radio control */
#define MRVL_RADIO_CTL_TLV_ID           (PROPRIETARY_TLV_BASE_ID + 52)
/** TLV : Tx data rate */
#define MRVL_TX_DATA_RATE_TLV_ID        (PROPRIETARY_TLV_BASE_ID + 53)
/** TLV : Packet forwarding control */
#define MRVL_PKT_FWD_CTL_TLV_ID         (PROPRIETARY_TLV_BASE_ID + 54)
/** TLV : STA information */
#define MRVL_STA_INFO_TLV_ID            (PROPRIETARY_TLV_BASE_ID + 55)
/** TLV : STA MAC address filter */
#define MRVL_STA_MAC_ADDR_FILTER_TLV_ID (PROPRIETARY_TLV_BASE_ID + 56)
/** TLV : STA ageout timer */
#define MRVL_STA_AGEOUT_TIMER_TLV_ID    (PROPRIETARY_TLV_BASE_ID + 57)
/** TLV : WEP keys */
#define MRVL_WEP_KEY_TLV_ID             (PROPRIETARY_TLV_BASE_ID + 59)
/** TLV : WPA passphrase */
#define MRVL_WPA_PASSPHRASE_TLV_ID      (PROPRIETARY_TLV_BASE_ID + 60)
/** TLV type : protocol TLV */
#define MRVL_PROTOCOL_TLV_ID            (PROPRIETARY_TLV_BASE_ID + 64)
/** TLV type : AKMP TLV */
#define MRVL_AKMP_TLV_ID               	(PROPRIETARY_TLV_BASE_ID + 65)
/** TLV type : Cipher TLV */
#define MRVL_CIPHER_TLV_ID             	(PROPRIETARY_TLV_BASE_ID + 66)
/** TLV : Fragment threshold */
#define MRVL_FRAG_THRESHOLD_TLV_ID      (PROPRIETARY_TLV_BASE_ID + 70)
/** TLV : Group rekey timer */
#define MRVL_GRP_REKEY_TIME_TLV_ID	(PROPRIETARY_TLV_BASE_ID + 71)
/**TLV: Max Station number */
#define MRVL_MAX_STA_CNT_TLV_ID 	(PROPRIETARY_TLV_BASE_ID + 85)
/**TLV: Retry limit */
#define MRVL_RETRY_LIMIT_TLV_ID 	(PROPRIETARY_TLV_BASE_ID + 93)
/**TLV: MCBC data rate */
#define MRVL_MCBC_DATA_RATE_TLV_ID        (PROPRIETARY_TLV_BASE_ID + 98)
/**TLV: RSN replay protection */
#define MRVL_RSN_REPLAY_PROT_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 100)
/** TLV: Management IE list */
#define MRVL_MGMT_IE_LIST_TLV_ID          (PROPRIETARY_TLV_BASE_ID + 105)
/** TLV : Coex common configuration */
#define MRVL_BT_COEX_COMMON_CFG_TLV_ID    (PROPRIETARY_TLV_BASE_ID + 108)
/** TLV : Coex SCO configuration */
#define MRVL_BT_COEX_SCO_CFG_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 109)
/** TLV : Coex ACL configuration */
#define MRVL_BT_COEX_ACL_CFG_TLV_ID       (PROPRIETARY_TLV_BASE_ID + 110)
/** TLV : Coex stats configuration */
#define MRVL_BT_COEX_STATS_TLV_ID         (PROPRIETARY_TLV_BASE_ID + 111)

/** sleep_param */
typedef struct _ps_sleep_param
{
    /** control bitmap */
    u32 ctrl_bitmap;
    /** minimum sleep period (micro second) */
    u32 min_sleep;
    /** maximum sleep period (micro second) */
    u32 max_sleep;
} ps_sleep_param;

/** inactivity sleep_param */
typedef struct _inact_sleep_param
{
    /** inactivity timeout (micro second) */
    u32 inactivity_to;
    /** miniumu awake period (micro second) */
    u32 min_awake;
    /** maximum awake period (micro second) */
    u32 max_awake;
} inact_sleep_param;

/** flag for ps mode */
#define PS_FLAG_PS_MODE                 1
/** flag for sleep param */
#define PS_FLAG_SLEEP_PARAM             2
/** flag for inactivity sleep param */
#define PS_FLAG_INACT_SLEEP_PARAM       4

/** Disable power mode */
#define PS_MODE_DISABLE                      0
/** Enable periodic dtim ps */
#define PS_MODE_PERIODIC_DTIM                1
/** Enable inactivity ps */
#define PS_MODE_INACTIVITY                   2

/** sleep parameter */
#define SLEEP_PARAMETER                     1
/** inactivity sleep parameter */
#define INACTIVITY_SLEEP_PARAMETER          2

/** sleep parameter : lower limit in micro-sec */
#define PS_SLEEP_PARAM_MIN                  5000
/** sleep parameter : upper limit in micro-sec */
#define PS_SLEEP_PARAM_MAX                  32000
/** power save awake period minimum value in micro-sec */
#define PS_AWAKE_PERIOD_MIN                 10

/** ps_mgmt */
typedef struct _ps_mgmt
{
    /** flags for valid field */
    u16 flags;
    /** power mode */
    u16 ps_mode;
    /** sleep param */
    ps_sleep_param sleep_param;
    /** inactivity sleep param */
    inact_sleep_param inact_param;
} ps_mgmt;

/** Function Prototype Declaration */
int mac2raw(char *mac, u8 * raw);
void print_mac(u8 * raw);
int uap_ioctl(u8 * cmd, u16 * size, u16 buf_size);
void print_auth(TLVBUF_AUTH_MODE * tlv);
void print_tlv(u8 * buf, u16 len);
void print_cipher(TLVBUF_CIPHER * tlv);
void print_rate(TLVBUF_RATES * tlv);
int string2raw(char *str, unsigned char *raw);
void print_mac_filter(TLVBUF_STA_MAC_ADDR_FILTER * tlv);
int ishexstring(void *hex);
inline int ISDIGIT(char *x);
unsigned int a2hex(char *s);
int fparse_for_hex(FILE * fp, u8 * dst);
int is_input_valid(valid_inputs cmd, int argc, char *argv[]);
int is_cipher_valid(int pairwisecipher, int groupcipher);
int get_sys_cfg_rates(u8 * rates);
int is_tx_rate_valid(u8 rate);
int is_mcbc_rate_valid(u8 rate);
void hexdump_data(char *prompt, void *p, int len, char delim);
unsigned char hexc2bin(char chr);
#endif /* _UAP_H */
