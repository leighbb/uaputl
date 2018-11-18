/** @file  uapcmd.c
 *
 *  @brief This file contains the handling of command.
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
/****************************************************************************
Change log:
    03/01/08: Initial creation
****************************************************************************/

/****************************************************************************
        Header files
****************************************************************************/
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <getopt.h>
#include <netinet/in.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "uaputl.h"
#include "uapcmd.h"

extern struct option cmd_options[];

/****************************************************************************
        Local functions
****************************************************************************/
/**
 *  @brief Show usage information for the sys_cfg_ap_mac_address
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_ap_mac_address_usage(void)
{
    printf("\nUsage : sys_cfg_ap_mac_address [AP_MAC_ADDRESS]\n"
           "\nIf AP_MAC_ADDRESS is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_ssid command
 *
 *  $return         N/A
 */
void
print_sys_cfg_ssid_usage(void)
{
    printf("\nUsage : sys_cfg_ssid [SSID]\n"
           "\nIf SSID is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_beacon_period
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_beacon_period_usage(void)
{
    printf("\nUsage : sys_cfg_beacon_period [BEACON_PERIOD]\n"
           "\nIf BEACON_PERIOD is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_dtim_period
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_dtim_period_usage(void)
{
    printf("\nUsage : sys_cfg_dtim_period [DTIM_PERIOD]\n"
           "\nIf DTIM_PERIOD is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_channel
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_channel_usage(void)
{
    printf("\nUsage : sys_cfg_channel [CHANNEL] [MODE]\n"
           "\nIf CHANNEL is provided, a 'set' is performed, else a 'get' is performed."
           "\nIf MODE is provided, 0 for manual channel selection"
           "\nelse ACS (automatic channel selection) is performed\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_scan_channels
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_scan_channels_usage(void)
{
    printf("\nUsage : sys_cfg_scan_channels [CHANNELS]\n"
           "\nIf CHANNELS are provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_rates_ext command
 *
 *  $return         N/A
 */
void
print_sys_cfg_rates_ext_usage(void)
{
    printf
        ("\nUsage : sys_cfg_rates_ext [rates RATES] [mbrate RATE] [urate RATE]\n"
        "\nIf 'Rate' provided, a 'set' is performed else a 'get' is performed"
        "\nRATES is provided as a set of data rates, in unit of 500 kilobits"
        "\nA rate with MSB bit is basic rate, i.e 0x82 is basic rate.\n"
        "\nFollowing is the list of supported rates in units of 500 Kbps:"
        "\nDecimal: (2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108)"
        "\nHex: (0x02, 0x04, 0x0b, 0x16, 0x0C, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c)"
        "\nBasic rates: (0x82, 0x84, 0x8b, 0x96, 0x8C, 0x92, 0x98, 0xA4, 0xB0, 0xC8, 0xE0, 0xEc)\n"
        "\nRates 2, 4, 11 and 22 (in units of 500 Kbps) must be present in either of basic or"
        "\nnon-basic rates. If OFDM rates are enabled then 12, 24 and 48 (in units of 500 Kbps)"
        "\nmust be present in either basic or non-basic rates"
        "\nEach rate must be separated by a space."
        "\nrates followed by RATES for setting operational rates."
        "\nmbrate followed by RATE for setting multicast and broadcast rate."
        "\nurate followed by RATE for setting unicast rate.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_rates command
 *
 *  $return         N/A
 */
void
print_sys_cfg_rates_usage(void)
{
    printf("\nUsage : sys_cfg_rates [RATES]\n"
           "\n[RATES] is set of data rates in unit of 500 kbps and each rate can be"
           "\nentered in hexadecimal or decimal format. Rates must be separated by"
           "\nspace. Duplicate Rate  fields are not allowed"
           "\nA rate with MSB bit is basic rate, i.e 0x82 is basic rate."
           "\nFollowing is the list of supported rates in units of 500 Kbps:"
           "\nDecimal: (2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108)"
           "\nHex: (0x02, 0x04, 0x0b, 0x16, 0x0C, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c)"
           "\nBasic rates: (0x82, 0x84, 0x8b, 0x96, 0x8C, 0x92, 0x98, 0xA4, 0xB0, 0xC8, 0xE0, 0xEc)\n");
    return;
}

/**
 *   @brief Show usage information for the sys_cfg_tx_power
 *    command
 *
 *  $return         N/A
 */
void
print_sys_cfg_tx_power_usage(void)
{
    printf("\nUsage : sys_cfg_tx_power [TX_POWER]\n"
           "\nIf TX_POWER is provided, a 'set' is performed, else a 'get' is performed."
           "\nTX_POWER is represented in dBm.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_bcast_ssid_ctl
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_bcast_ssid_ctl_usage(void)
{
    printf("\nUsage : sys_cfg_bcast_ssid_ctl [0|1]\n"
           "\nOptions: 0     - Disable SSID broadcast"
           "\n         1     - Enable SSID broadcast"
           "\n         empty - Get current SSID broadcast setting\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_rsn_replay_prot
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_rsn_replay_prot_usage(void)
{
    printf("\nUsage : sys_cfg_rsn_replay_prot [0|1]\n"
           "\nOptions: 0     - Disable RSN replay protection"
           "\n         1     - Enable  RSN replay protection"
           "\n         empty - Get current RSN replay protection setting\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_preamble_ctl
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_preamble_ctl_usage(void)
{
    printf("\nUsage : sys_cfg_preamble_ctl\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_antenna_ctl
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_antenna_ctl_usage(void)
{
    printf("\nUsage : sys_cfg_antenna_ctl <ANTENNA> [MODE]\n"
           "\nOptions: ANTENNA : 0       - Rx antenna"
           "\n                   1       - Tx antenna"
           "\n         MODE    : 0       - Antenna A"
           "\n                   1       - Antenna B"
           "\n                   empty   - Get current antenna settings\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_rts_threshold
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_rts_threshold_usage(void)
{
    printf("\nUsage : sys_cfg_rts_threshold [RTS_THRESHOLD]\n"
           "\nIf RTS_THRESHOLD is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_frag_threshold
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_frag_threshold_usage(void)
{
    printf("\nUsage : sys_cfg_frag_threshold [FRAG_THRESHOLD]\n"
           "\nIf FRAG_THRESHOLD is provided, a 'set' is performed, else a 'get' is performed."
           "\nFragment threshold should between 256 and 2346.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_radio_ctl
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_radio_ctl_usage(void)
{
    printf("\nUsage : sys_cfg_radio_ctl [0|1]\n"
           "\nOptions: 0     - Turn radio on"
           "\n         1     - Turn radio off"
           "\n         empty - Get current radio setting\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_tx_data_rate
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_tx_data_rates_usage(void)
{
    printf("\nUsage : sys_cfg_tx_data_rate [TX_DATA_RATE]\n"
           "\nOptions: 0     - Auto rate"
           "\n         >0    - Set specified data rate"
           "\n         empty - Get current data rate"
           "\nFollowing is the list of supported rates in units of 500 Kbps"
           "\nDecimal: (2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108)"
           "\nHex: (0x02, 0x04, 0x0b, 0x16, 0x0C, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c)"
           "\nOnly zero or rates currently configured are allowed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_mcbc_data_rate
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_mcbc_data_rates_usage(void)
{
    printf("\nUsage : sys_cfg_mcbc_data_rate [MCBC_DATA_RATE]\n"
           "\nOptions: 0     - Auto rate"
           "\n         >0    - Set specified MCBC data rate"
           "\n         empty - Get current MCBC data rate"
           "\nFollowing is the list of supported rates in units of 500 Kbps"
           "\nDecimal: (2, 4, 11, 22, 12, 18, 24, 36, 48, 72, 96, 108)"
           "\nHex: (0x02, 0x04, 0x0b, 0x16, 0x0C, 0x12, 0x18, 0x24, 0x30, 0x48, 0x60, 0x6c)"
           "\nOnly zero or one of the basic rates currently configured are allowed.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_auth command
 *
 *  $return         N/A
 */
void
print_sys_cfg_auth_usage(void)
{
    printf("\nUsage : sys_cfg_auth [AUTHMODE]\n"
           "\nOptions: AUTHMODE :     0 - Open authentication"
           "\n                        1 - Shared key authentication"
           "\n         empty - Get current authenticaton mode\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_pkt_fwd_ctl command
 *
 *  $return         N/A
 */
void
print_sys_cfg_pkt_fwd_ctl_usage(void)
{
    printf("\nUsage : sys_cfg_pkt_fwd_ctl [0|1]\n"
           "\nOptions: 0     - Forward all packets to the host"
           "\n         1     - Firmware handles intra-BSS packets"
           "\n         empty - Get current packet forwarding setting\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_sta_ageout_timer
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_sta_ageout_timer_usage(void)
{
    printf("\nUsage : sys_cfg_sta_ageout_timer [STA_AGEOUT_TIMER]\n"
           "\nIf STA_AGEOUT_TIMER is provided, a 'set' is performed, else a 'get' is performed."
           "\nSTA_AGEOUT_TIMER is represented in units of 100 ms."
           "\nValue of 0 will mean that stations will never be aged out."
           "\nThe value should between 300 and 864000.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_protocol command
 *
 *  $return         N/A
 */
void
print_sys_cfg_protocol_usage(void)
{
    printf("\nUsage : sys_cfg_protocol [PROTOCOL]\n"
           "\nOptions: PROTOCOL:		1 - No RSN"
           "\n				2 - WEP Static"
           "\n				8 - WPA"
           "\n				32 - WPA2"
           "\n				40 - WPA2 Mixed"
           "\n				empty - Get current protocol\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_wep_key
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_wep_key_usage(void)
{
    printf("\nUsage : sys_cfg_wep_key "
           "[INDEX_0 IS_DEFAULT KEY_0] [INDEX_1 IS_DEFAULT KEY_1] [INDEX_2 IS_DEFAULT KEY_2] [INDEX_3 IS_DEFAULT KEY_3]\n"
           "[Index_0] [Index_1] [Index_2] [Index_3]\n"
           "\nOptions: INDEX_* :      0 - KeyIndex is 0"
           "\n                        1 - KeyIndex is 1"
           "\n                        2 - KeyIndex is 2"
           "\n                        3 - KeyIndex is 3"
           "\n         IS_DEFAULT :   0 - KeyIndex is not the default"
           "\n                        1 - KeyIndex is the default transmit key"
           "\n         KEY_* :        Key value"
           "\n         Index_*:       0 - Get key 0 setting"
           "\n                        1 - Get key 1 setting"
           "\n                        2 - Get key 2 setting"
           "\n                        3 - Get key 3 setting"
           "\n         empty - Get current WEP key settings\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_custom_ie
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_custom_ie_usage(void)
{
    printf("\nUsage : sys_cfg_custom_ie [INDEX] [MASK] [IEBuffer]"
           "\n         empty - Get all IE settings\n"
           "\n         INDEX:  0 - Get/Set IE index 0 setting"
           "\n                 1 - Get/Set IE index 1 setting"
           "\n                 2 - Get/Set IE index 2 setting"
           "\n                 3 - Get/Set IE index 3 setting"
           "\n         MASK :  Management subtype mask value as per bit defintions"
           "\n              :  Bit 0 - Association request."
           "\n              :  Bit 1 - Association response."
           "\n              :  Bit 2 - Reassociation request."
           "\n              :  Bit 3 - Reassociation response."
           "\n              :  Bit 4 - Probe request."
           "\n              :  Bit 5 - Probe response."
           "\n              :  Bit 8 - Beacon."
           "\n         MASK :  MASK = 0 to clear the mask and the IE buffer"
           "\n         IEBuffer :  IE Buffer in hex (max 256 bytes)\n\n");
    return;
}

 /* @brief Show usage information for the sys_cfg_cipher * command * * $return
    N/A */
void
print_sys_cfg_cipher_usage(void)
{
    printf("\nUsage : sys_cfg_cipher [PAIRWISE_CIPHER GROUP_CIPHER]\n"
           "\nOptions: PAIRWISE_CIPHER:  0 - NONE"
           "\n                           4 - TKIP"
           "\n                           8 - AES CCMP"
           "\n                           12 - AES CCMP + TKIP"
           "\n         GROUP_CIPHER :    0 - NONE"
           "\n                           4 - TKIP"
           "\n                           8 - AES CCMP"
           "\n         empty - Get current cipher settings\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_group_rekey_timer command
 *
 *  $return         N/A
 */
void
print_sys_cfg_group_rekey_timer_usage(void)
{
    printf("\nUsage : sys_cfg_group_rekey_timer [GROUP_REKEY_TIMER]\n"
           "\nOptions: GROUP_REKEY_TIME is represented in seconds"
           "\n         empty - Get current group re-key time\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_wpa_passphrase
 *   command
 *
 *  $return         N/A
 */
void
print_sys_cfg_wpa_passphrase_usage(void)
{
    printf("\nUsage : sys_cfg_wpa_passphrase [PASSPHRASE]\n"
           "\nIf PASSPHRASE is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/**
 *  @brief Show usage information for the sta_filter_table command
 *
 *  $return         N/A
 */
void
print_sta_filter_table_usage(void)
{
    printf("\nUsage : sta_filter_table <FILTERMODE> <MACADDRESS_LIST>\n"
           "\nOptions: FILTERMODE : 0 - Disable filter table"
           "\n                      1 - allow MAC addresses specified in the allowed list"
           "\n                      2 - block MAC addresses specified in the banned list"
           "\n         MACADDRESS_LIST is the list of MAC addresses to be acted upon. Each"
           "\n                      MAC address must be separated with a space. Maximum of"
           "\n                      16 MAC addresses are supported."
           "\n         empty - Get current mac filter settings\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_max_sta_num command
 *
 *  $return         N/A
 */
void
print_sys_cfg_max_sta_num_usage(void)
{
    printf("\nUsage : sys_cfg_max_sta_num [STA_NUM]\n"
           "\nIf STA_NUM is provided, a 'set' is performed, else a 'get' is performed."
           "\nSTA_NUM should not bigger than 8.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_retry_limit command
 *
 *  $return         N/A
 */
void
print_sys_cfg_retry_limit_usage(void)
{
    printf("\nUsage : sys_cfg_retry_limit [RETRY_LIMIT]\n"
           "\nIf RETRY_LIMIT is provided, a 'set' is performed, else a 'get' is performed."
           "\nRETRY_LIMIT should not bigger than 14.\n");
    return;
}

/**
 *  @brief Show usage information for the sys_cfg_retry_limit command
 *
 *  $return         N/A
 */
void
print_cfg_data_usage(void)
{
    printf("\nUsage : cfg_data <type> [*.conf]\n"
           "\n        type : 2 -- cal data"
           "\n        *.conf : file contain configuration data"
           "\n                 empty - get current configuration data\n");
    return;
}

/** 
 *  @brief  get configured operational rates.
 *
 *  @param  rates   Operational rates allowed are
 *                  stored at this pointer
 *  @return         number of basic rates allowed.
 *                  -1 if a failure 
 */
int
get_sys_cfg_rates(u8 * rates)
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_RATES *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int i = 0;
    int rate_cnt = 0;
    /* Initialize the command length */
    cmd_len =
        sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RATES) + MAX_DATA_RATES;
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return -1;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_RATES *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RATES_TLV_ID;
    cmd_buf->Action = ACTION_GET;
    tlv->Length = MAX_DATA_RATES;

    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RATES_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return -1;
        }

        /* copy response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            for (i = 0; i < tlv->Length; i++) {
                if (tlv->OperationalRates[i] != 0) {
                    rates[rate_cnt++] = tlv->OperationalRates[i];
                }
            }
        } else {
            printf("ERR:Could not get operational rates!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return rate_cnt;
}

/** 
 *  @brief check rate is valid or not.
 *
 *  @param  rate   rate for check
 *
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
is_tx_rate_valid(u8 rate)
{
    int rate_cnt = 0;
    int i;
    u8 rates[MAX_DATA_RATES];

    rate_cnt = get_sys_cfg_rates((u8 *) & rates);
    if (rate_cnt > 0) {
        for (i = 0; i < rate_cnt; i++) {
            if (rate == (rates[i] & ~BASIC_RATE_SET_BIT)) {
                return UAP_SUCCESS;
            }
        }
    }
    return UAP_FAILURE;
}

/** 
 *  @brief check mcbc rate is valid or not.
 *
 *  @param  rate   rate for check
 *
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
is_mcbc_rate_valid(u8 rate)
{
    int rate_cnt = 0;
    int i;
    u8 rates[MAX_DATA_RATES];

    rate_cnt = get_sys_cfg_rates((u8 *) & rates);
    if (rate_cnt > 0) {
        for (i = 0; i < rate_cnt; i++) {
            if (rates[i] & BASIC_RATE_SET_BIT) {
                if (rate == (rates[i] & ~BASIC_RATE_SET_BIT)) {
                    return UAP_SUCCESS;
                }
            }
        }
    }
    return UAP_FAILURE;
}

/****************************************************************************
        Global functions
****************************************************************************/
/** 
 *  @brief Creates a sys_cfg request for AP MAC address
 *   and sends to the driver
 *
 *  Usage: "sys_cfg_ap_mac_address [AP_MAC_ADDRESS]"
 *          if AP_MAC_ADDRESS is provided, a 'set' is performed,
 *          else a 'get' is performed.
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_ap_mac_address(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_AP_MAC_ADDRESS *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_ap_mac_address_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 1) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_ap_mac_address_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_AP_MAC_ADDRESS);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_AP_MAC_ADDRESS *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_AP_MAC_ADDRESS_TLV_ID;
    tlv->Length = ETH_ALEN;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        if ((ret = mac2raw(argv[0], tlv->ApMacAddr)) != UAP_SUCCESS) {
            printf("ERR: %s Address \n", ret == UAP_FAILURE ? "Invalid MAC" :
                   ret == UAP_RET_MAC_BROADCAST ? "Broadcast" : "Multicast");
            free(buffer);
            return;
        }
    }
    endian_convert_tlv_header_out(tlv);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_AP_MAC_ADDRESS_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("AP MAC address = ");
                print_mac(tlv->ApMacAddr);
                printf("\n");
            } else {
                printf("AP MAC address setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get AP MAC address!\n");
            } else {
                printf("ERR:Could not set AP MAC address!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }

    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for SSID
 *   and sends to the driver
 *
 *  Usage: "sys_cfg_ssid [SSID]"
 *          if SSID is provided, a 'set' is performed
 *          else a 'get' is performed 
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_ssid(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_SSID *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    u8 ssid[33];

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_ssid_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 1) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_ssid_usage();
        return;
    }

    if (argc == 0) {
        /* Initialize the command length */
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_SSID) +
            MAX_SSID_LENGTH;
    } else {
        if (strlen(argv[0]) > MAX_SSID_LENGTH) {
            printf("ERR:SSID too long.\n");
            return;
        }
        /* Initialize the command length */
        if (argv[0][1] == '"') {
            argv[0]++;
        }
        if (argv[0][strlen(argv[0])] == '"') {
            argv[0][strlen(argv[0])] = '\0';
        }
        if (!strlen(argv[0])) {
            printf("ERR:NULL SSID not allowed.\n");
            return;
        }
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_SSID) +
            strlen(argv[0]);
    }

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_SSID *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_SSID_TLV_ID;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        tlv->Length = MAX_SSID_LENGTH;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Length = strlen(argv[0]);
        memcpy(tlv->Ssid, argv[0], tlv->Length);
    }

    endian_convert_tlv_header_out(tlv);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    endian_convert_tlv_header_in(tlv);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_SSID_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                memset(ssid, 0, sizeof(ssid));
                memcpy(ssid, tlv->Ssid, tlv->Length);
                printf("SSID = %s\n", ssid);
            } else {
                printf("SSID setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get SSID!\n");
            } else {
                printf("ERR:Could not set SSID!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }

    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for beacon period
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_beacon_period [BEACON_PERIOD]"
 *           if BEACON_PERIOD is provided, a 'set' is performed
 *           else a 'get' is performed.
 *
 *           BEACON_PERIOD is represented in ms
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_beacon_period(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_BEACON_PERIOD *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_beacon_period_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && is_input_valid(BEACONPERIOD, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_beacon_period_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_BEACON_PERIOD);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_BEACON_PERIOD *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_BEACON_PERIOD_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->BeaconPeriod_ms = (u16) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->BeaconPeriod_ms = uap_cpu_to_le16(tlv->BeaconPeriod_ms);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->BeaconPeriod_ms = uap_le16_to_cpu(tlv->BeaconPeriod_ms);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_BEACON_PERIOD_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Beacon period = %d\n", tlv->BeaconPeriod_ms);
            } else {
                printf("Beacon period setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get beacon period!\n");
            } else {
                printf("ERR:Could not set beacon period!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for DTIM period
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_dtim_period [DTIM_PERIOD]"
 *           if DTIM_PERIOD is provided, a 'set' is performed
 *           else a 'get' is performed
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_dtim_period(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_DTIM_PERIOD *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_dtim_period_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(DTIMPERIOD, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_dtim_period_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_DTIM_PERIOD);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_DTIM_PERIOD *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_DTIM_PERIOD_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->DtimPeriod = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_DTIM_PERIOD_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("DTIM period = %d\n", tlv->DtimPeriod);
            } else {
                printf("DTIM period setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get DTIM period!\n");
            } else {
                printf("ERR:Could not set DTIM period!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for channel
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_channel [CHANNEL] [MODE]"
 *           if CHANNEL is provided, a 'set' is performed
 *           else a 'get' is performed
 *           if MODE is provided, a 'set' is performed with
 *           0 as manual channel selection
 *           1 as automatic channel selection.
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_channel(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_CHANNEL_CONFIG *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_channel_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && is_input_valid(CHANNEL, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_channel_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_CHANNEL_CONFIG);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_CHANNEL_CONFIG *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_CHANNELCONFIG_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        if (argc == 1) {
            tlv->ChanNumber = (u8) atoi(argv[0]);
            tlv->BandConfigType = 0;
        } else {
            tlv->BandConfigType = atoi(argv[1]) ? BAND_CONFIG_ACS_MODE : 0;
            tlv->ChanNumber = (u8) atoi(argv[0]);
        }
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_CHANNELCONFIG_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Mode    = %s\n",
                       (tlv->BandConfigType == 0) ? "Manual" : "ACS");
                printf("Channel = %d\n", tlv->ChanNumber);
            } else {
                printf("Channel setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get channel!\n");
            } else {
                printf("ERR:Could not set channel!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for channel list
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_scan_channels [CHANNELS]"
 *           if CHANNELS are provided, a 'set' is performed
 *           else a 'get' is performed
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_scan_channels(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_CHANNEL_LIST *tlv = NULL;
    CHANNEL_LIST *pChanList = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    int i;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_scan_channels_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && is_input_valid(SCANCHANNELS, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_scan_channels_usage();
        return;
    }

    /* Initialize the command length */
    if (argc == 0)
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_CHANNEL_LIST) +
            sizeof(CHANNEL_LIST) * MAX_CHANNELS;
    else
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_CHANNEL_LIST) +
            sizeof(CHANNEL_LIST) * argc;

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Can not allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* LOCATE HEADERS */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_CHANNEL_LIST *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_CHANNELLIST_TLV_ID;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        tlv->Length = sizeof(CHANNEL_LIST) * MAX_CHANNELS;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Length = sizeof(CHANNEL_LIST) * argc;
        pChanList = tlv->ChanList;
        for (i = 0; i < argc; i++) {
            pChanList->ChanNumber = (u8) atoi(argv[i]);
            pChanList->BandConfigType = 0;
            pChanList++;
        }
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_CHANNELLIST_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Channels List = ");
                if (tlv->Length % sizeof(CHANNEL_LIST)) {
                    printf("Error: Length mismatch\n");
                    free(buffer);
                    return;
                }
                pChanList = tlv->ChanList;
                for (i = 0; i < (tlv->Length / sizeof(CHANNEL_LIST)); i++) {
                    printf("%d ", pChanList->ChanNumber);
                    pChanList++;
                }
                printf("\n");
            } else {
                printf("Scan Channel List setting successful\n");
            }
        } else {
            printf("ERR:Could not %s scan channel list!\n",
                   argc ? "SET" : "GET");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief parser for sys_cfg_rates_ext input 
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @param output   stores indexes for "rates, mbrate and urate"
 *                  arguments
 *
 *                  e.g., 
 *
 *                  "rates 0x82 4 16 22 0x30 mbrate 2 urate 16"
 *                  
 *                  will have output array as
 *
 *                          start_index end_index
 *                  rates       0           5
 *                  mbrate      6           7
 *                  urate       8           9
 *
 *  @return         NA
 *
 */
void
parse_input(int argc, char **argv, int output[3][2])
{
    int i, j, k = 0;
    char *keywords[3] = { "rates", "mbrate", "urate" };

    for (i = 0; i < 3; i++)
        output[i][0] = -1;

    for (i = 0; i < argc; i++) {
        for (j = 0; j < 3; j++) {
            if (strcmp(argv[i], keywords[j]) == 0) {
                output[j][1] = output[j][0] = i;
                k = j;
                break;
            }
        }
        output[k][1] += 1;
    }
}

/**
 *  @brief Creates a sys_cfg request for setting data_rates, MCBC and
 *   TX data rates.
 *
 *   Usage: "sys_cfg_rates_ext  [RATES]" 
 *  
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_rates_ext(int argc, char *argv[])
{
    int i, j = 0;
    int opt;
    int rflag = 0, mflag = 0, uflag = 0;
    char *argv_rate[14];
    int argc_rate = 0;
    char *argv_mrate[1];
    char *argv_urate[1];
    int mrate_found = UAP_FAILURE;
    int urate_found = UAP_FAILURE;
    u8 *tlv_buf = NULL;
    TLVBUF_TX_DATA_RATE *tlv_urate = NULL;
    TLVBUF_MCBC_DATA_RATE *tlv_mrate = NULL;
    TLVBUF_RATES *tlv_rate = NULL;
    u8 *buffer = NULL;
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    int ret = UAP_FAILURE;
    u16 cmd_len;
    int output[3][2];

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_rates_ext_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc) {
        /* 
         * SET
         */
        parse_input(argc, argv, output);

        /* 
         * Rate
         */
        if ((output[0][0] != -1) && (output[0][1] > output[0][0])) {
            rflag = 1;
            i = 0;
            for (j = (output[0][0] + 1); j < output[0][1]; j++) {
                argv_rate[i] =
                    (char *) malloc(sizeof(char) * (strlen(argv[j]) + 1));
                strcpy(argv_rate[i], argv[j]);
                argc_rate = ++i;
            }
        }

        /* 
         * mrate
         */

        if ((output[1][0] != -1) && (output[1][1] > output[1][0])) {
            if ((output[1][1] - output[1][0]) != 2) {
                printf("\nERR: Invalid mrate input");
                print_sys_cfg_rates_ext_usage();
                goto done;
            }
            mflag = 1;
            argv_mrate[0] =
                (char *) malloc(sizeof(char) * (strlen(argv[j]) + 1));
            strcpy(argv_mrate[0], argv[output[1][0] + 1]);
        }

        /* 
         * urate
         */
        if ((output[2][0] != -1) && (output[2][1] > output[2][0])) {
            if ((output[2][1] - output[2][0]) != 2) {
                printf("\nERR: Invalid urate input");
                print_sys_cfg_rates_ext_usage();
                goto done;
            }
            uflag = 1;
            argv_urate[0] =
                (char *) malloc(sizeof(char) * (strlen(argv[j]) + 1));
            strcpy(argv_urate[0], argv[output[2][0] + 1]);
        }

        if (!rflag && !mflag & !uflag) {
            printf("ERR: Invalid input\n");
            print_sys_cfg_rates_ext_usage();
            goto done;
        }

        if (rflag && is_input_valid(RATE, argc_rate, argv_rate) != UAP_SUCCESS) {
            printf("ERR: Invalid RATE\n");
            print_sys_cfg_rates_ext_usage();
            goto done;
        }

        if (mflag && is_input_valid(MCBCDATARATE, 1, argv_mrate) != UAP_SUCCESS) {
            printf("ERR: Invalid MCBC RATE\n");
            print_sys_cfg_rates_ext_usage();
            goto done;
        }

        if (uflag && is_input_valid(TXDATARATE, 1, argv_urate) != UAP_SUCCESS) {
            printf("ERR: Invalid TX RATE\n");
            print_sys_cfg_rates_ext_usage();
            goto done;
        }

        if (!rflag && (mflag || uflag)) {
            /* 
             * Check mrate and urate wrt old Rates
             */
            if (mflag && A2HEXDECIMAL(argv_mrate[0]) &&
                is_mcbc_rate_valid(A2HEXDECIMAL(argv_mrate[0])) !=
                UAP_SUCCESS) {
                printf("ERR: invalid MCBC data rate.");
                print_sys_cfg_rates_ext_usage();
                goto done;
            }
            if (uflag && A2HEXDECIMAL(argv_urate[0]) &&
                is_tx_rate_valid(A2HEXDECIMAL(argv_urate[0])) != UAP_SUCCESS) {
                printf("ERR: invalid tx data rate.");
                print_sys_cfg_rates_ext_usage();
                goto done;
            }
        } else if (rflag && (mflag || uflag)) {
            /* 
             * Check mrate and urate wrt new Rates
             */
            for (i = 0; i < argc_rate; i++) {
                /* 
                 * MCBC rate must be from Basic rates
                 */
                if (mflag && !mrate_found &&
                    A2HEXDECIMAL(argv_rate[i]) & BASIC_RATE_SET_BIT) {
                    if (A2HEXDECIMAL(argv_mrate[0]) ==
                        (A2HEXDECIMAL(argv_rate[i]) & ~BASIC_RATE_SET_BIT)) {
                        mrate_found = UAP_SUCCESS;
                    }
                }
                if (uflag && !urate_found && (A2HEXDECIMAL(argv_urate[0]) ==
                                              ((A2HEXDECIMAL(argv_rate[i]) &
                                                ~BASIC_RATE_SET_BIT)))) {
                    urate_found = UAP_SUCCESS;
                }
            }

            if (mflag && A2HEXDECIMAL(argv_mrate[0]) &&
                !(mrate_found == UAP_SUCCESS)) {
                printf("ERR: mrate not valid\n");
                goto done;
            }

            if (uflag && A2HEXDECIMAL(argv_urate[0]) &&
                !(urate_found == UAP_SUCCESS)) {
                printf("ERR: urate not valid\n");
                goto done;
            }
        }
        /* post-parsing */
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE);
        if (rflag) {
            cmd_len += sizeof(TLVBUF_RATES) + argc_rate;
            cmd_len += sizeof(TLVBUF_MCBC_DATA_RATE);
            cmd_len += sizeof(TLVBUF_TX_DATA_RATE);
        } else {
            if (mflag)
                cmd_len += sizeof(TLVBUF_MCBC_DATA_RATE);
            if (uflag)
                cmd_len += sizeof(TLVBUF_TX_DATA_RATE);
        }
    } else {
        /* GET */
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE)
            + sizeof(TLVBUF_RATES) + MAX_RATES + sizeof(TLVBUF_MCBC_DATA_RATE)
            + sizeof(TLVBUF_TX_DATA_RATE);
    }

    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        goto done;
    }
    bzero((char *) buffer, cmd_len);

    /* Fill the command buffer */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->Action = argc ? ACTION_SET : ACTION_GET;
    tlv_buf = buffer + sizeof(APCMDBUF_SYS_CONFIGURE);
    /* Locate headers */
    if (rflag || (!argc)) {
        tlv_rate = (TLVBUF_RATES *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
        tlv_rate->Tag = MRVL_RATES_TLV_ID;
        tlv_rate->Length = argc ? argc_rate : MAX_RATES;
        for (i = 0; i < argc_rate; i++) {
            tlv_rate->OperationalRates[i] = (u8) A2HEXDECIMAL(argv_rate[i]);
        }
        tlv_buf += tlv_rate->Length + sizeof(TLVBUF_RATES);
        endian_convert_tlv_header_out(tlv_rate);
    }
    if (rflag || mflag || (!argc)) {
        tlv_mrate = (TLVBUF_MCBC_DATA_RATE *) tlv_buf;
        tlv_mrate->Tag = MRVL_MCBC_DATA_RATE_TLV_ID;
        tlv_mrate->Length = 2;
        tlv_mrate->MCBCdatarate = 0;
        if (mflag) {
            tlv_mrate->MCBCdatarate = (u16) A2HEXDECIMAL(argv_mrate[0])
                & ~BASIC_RATE_SET_BIT;
            tlv_mrate->MCBCdatarate = uap_cpu_to_le16(tlv_mrate->MCBCdatarate);
        }
        tlv_buf += sizeof(TLVBUF_MCBC_DATA_RATE);
        endian_convert_tlv_header_out(tlv_mrate);
    }
    if (rflag || uflag || (!argc)) {
        tlv_urate = (TLVBUF_TX_DATA_RATE *) tlv_buf;
        tlv_urate->Tag = MRVL_TX_DATA_RATE_TLV_ID;
        tlv_urate->Length = 2;
        tlv_urate->TxDataRate = 0;
        if (uflag) {
            tlv_urate->TxDataRate = (u16) A2HEXDECIMAL(argv_urate[0])
                & ~BASIC_RATE_SET_BIT;
            tlv_urate->TxDataRate = uap_cpu_to_le16(tlv_urate->TxDataRate);
        }
        endian_convert_tlv_header_out(tlv_urate);
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    tlv_buf = buffer + sizeof(APCMDBUF_SYS_CONFIGURE);

    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response! CmdCode=%x\n", cmd_buf->CmdCode);
            goto done;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc) {
                printf("Rates setting successful\n");
            } else {
                print_tlv((u8 *) tlv_buf,
                          cmd_buf->Size - sizeof(APCMDBUF_SYS_CONFIGURE) +
                          BUF_HEADER_SIZE);
            }
        } else {
            printf("ERR:Could not %s operational rates!\n",
                   argc ? "set" : "get");
            if (argc)
                printf
                    ("operational rates only allow to set before bss start.\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
  done:
    if (rflag) {
        for (i = 0; i < argc_rate; i++) {
            free(argv_rate[i]);
        }
    }
    if (mflag)
        free(argv_mrate[0]);
    if (uflag)
        free(argv_urate[0]);
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for data rates
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_rates  [RATES]"
 *
 *           RATES is provided as a set of data rates, in
 *           unit of 500 kilobits/s. 
 *           Maximum 12 rates can be provided.
 *
 *           if no RATE is provided, then it gets configured rates
 *
 *           Each rate must be separated by a space
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_rates(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_RATES *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int i = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_rates_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && is_input_valid(RATE, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_rates_usage();
        return;
    }
    if (argc == 0) {
        /* Initialize the command length */
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RATES) + MAX_RATES;
    } else {
        /* Initialize the command length */
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RATES) + argc;
    }
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_RATES *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RATES_TLV_ID;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        tlv->Length = MAX_RATES;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Length = argc;
        for (i = 0; i < tlv->Length; i++) {
            tlv->OperationalRates[i] = (u8) A2HEXDECIMAL(argv[i]);
        }
    }

    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RATES_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                print_rate(tlv);
            } else {
                printf("Rates setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get operational rates!\n");
            } else {
                printf("ERR:Could not set operational rates!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for Tx power
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_tx_power [TX_POWER]"
 *           if TX_POWER is provided, a 'set' is performed
 *           else a 'get' is performed.
 *
 *           TX_POWER is represented in dBm
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_tx_power(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_TX_POWER *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_tx_power_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && is_input_valid(TXPOWER, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_tx_power_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_TX_POWER);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_TX_POWER *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_TX_POWER_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        printf("Please check power calibration for board to see if this power\n"
               "setting is within calibrated range. Firmware may over-ride\n "
               "this setting if it is not within calibrated range, which can\n"
               "vary from board to board.\n");
        cmd_buf->Action = ACTION_SET;
        tlv->TxPower_dBm = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_TX_POWER_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Tx power = %d dBm\n", tlv->TxPower_dBm);
            } else {
                printf("Tx power setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get tx power!\n");
            } else {
                printf("ERR:Could not set tx power!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for SSID broadcast
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_bcast_ssid_ctl [0|1]"
 *
 *   Options: 0     - Disable SSID broadcast
 *            1     - Enable SSID broadcast
 *            empty - Get current SSID broadcast setting
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_bcast_ssid_ctl(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_BCAST_SSID_CTL *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_bcast_ssid_ctl_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && is_input_valid(BROADCASTSSID, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_bcast_ssid_ctl_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_BCAST_SSID_CTL);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_BCAST_SSID_CTL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_BCAST_SSID_CTL_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->BcastSsidCtl = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_BCAST_SSID_CTL_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("SSID broadcast is %s\n",
                       (tlv->BcastSsidCtl == 1) ? "enabled" : "disabled");
            } else {
                printf("SSID broadcast setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get SSID broadcast!\n");
            } else {
                printf("ERR:Could not set SSID broadcast!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for preamble settings
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_preamble_ctl"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_preamble_ctl(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_PREAMBLE_CTL *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_preamble_ctl_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_preamble_ctl_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PREAMBLE_CTL);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_PREAMBLE_CTL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_PREAMBLE_CTL_TLV_ID;
    tlv->Length = 1;
    cmd_buf->Action = ACTION_GET;
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_PREAMBLE_CTL_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS)
            printf("Preamble type is %s\n", (tlv->PreambleType == 0) ?
                   "auto" : ((tlv->PreambleType == 1) ? "short" : "long"));
        else
            printf("ERR:Could not get preamble type!\n");

    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for antenna configuration
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_antenna_ctl <ANTENNA> [MODE]"
 *
 *   Options: ANTENNA : 0 - Rx antenna
 *                      1 - Tx antenna
 *            MODE    : 0       - Antenna A
 *                      1       - Antenna B
 *                      empty   - Get current antenna settings
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_antenna_ctl(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_ANTENNA_CTL *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_antenna_ctl_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if ((argc == 0) || (argc > 2)) {
        printf("ERR:wrong arguments.\n");
        print_sys_cfg_antenna_ctl_usage();
        return;
    } else if (argc == 1) {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
            (atoi(argv[0]) > 1)) {
            printf
                ("ERR:Illegal ANTENNA parameter %s. Must be either '0' or '1'.\n",
                 argv[0]);
            print_sys_cfg_antenna_ctl_usage();
            return;
        }
    } else {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
            (atoi(argv[0]) > 1)) {
            printf
                ("ERR:Illegal ANTENNA parameter %s. Must be either '0' or '1'.\n",
                 argv[0]);
            print_sys_cfg_antenna_ctl_usage();
            return;
        }
        if ((ISDIGIT(argv[1]) == 0) || (atoi(argv[1]) < 0) ||
            (atoi(argv[1]) > 1)) {
            printf
                ("ERR:Illegal MODE parameter %s. Must be either '0' or '1'.\n",
                 argv[1]);
            print_sys_cfg_antenna_ctl_usage();
            return;
        }
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_ANTENNA_CTL);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_ANTENNA_CTL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_ANTENNA_CTL_TLV_ID;
    tlv->Length = 2;
    tlv->WhichAntenna = (u8) atoi(argv[0]);
    if (argc == 1) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->AntennaMode = (u8) atoi(argv[1]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_ANTENNA_CTL_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 1) {
                printf("%s antenna: %s\n", (tlv->WhichAntenna == 0) ?
                       "Rx" : "Tx", (tlv->AntennaMode == 0) ? "A" : "B");
            } else {
                printf("Antenna setting successful\n");
            }
        } else {
            if (argc == 1) {
                printf("ERR:Could not get antenna!\n");
            } else {
                printf("ERR:Could not set antenna!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for RTS threshold
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_rts_threshold [RTS_THRESHOLD]"
 *           if RTS_THRESHOLD is provided, a 'set' is performed
 *           else a 'get' is performed
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_rts_threshold(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_RTS_THRESHOLD *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_rts_threshold_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(RTSTHRESH, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_rts_threshold_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RTS_THRESHOLD);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_RTS_THRESHOLD *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RTS_THRESHOLD_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->RtsThreshold = (u16) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->RtsThreshold = uap_cpu_to_le16(tlv->RtsThreshold);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->RtsThreshold = uap_le16_to_cpu(tlv->RtsThreshold);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RTS_THRESHOLD_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("RTS threshold = %d\n", tlv->RtsThreshold);
            } else {
                printf("RTS threshold setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get RTS threshold!\n");
            } else {
                printf("ERR:Could not set RTS threshold!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for Fragmentation threshold
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_frag_threshold [FRAG_THRESHOLD]"
 *           if FRAG_THRESHOLD is provided, a 'set' is performed
 *           else a 'get' is performed
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_frag_threshold(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_FRAG_THRESHOLD *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_frag_threshold_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(FRAGTHRESH, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_frag_threshold_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_FRAG_THRESHOLD);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_FRAG_THRESHOLD *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_FRAG_THRESHOLD_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->FragThreshold = (u16) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->FragThreshold = uap_cpu_to_le16(tlv->FragThreshold);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->FragThreshold = uap_le16_to_cpu(tlv->FragThreshold);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_FRAG_THRESHOLD_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Fragmentation threshold = %d\n", tlv->FragThreshold);
            } else {
                printf("Fragmentation threshold setting successful\n");
            }
        } else {
            if (argc == 1) {
                printf("ERR:Could not get Fragmentation threshold!\n");
            } else {
                printf("ERR:Could not set Fragmentation threshold!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for radio settings
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_radio_ctl [0|1]"
 *
 *   Options: 0     - Turn radio on
 *            1     - Turn radio off
 *            empty - Get current radio setting
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_radio_ctl(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_RADIO_CTL *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_radio_ctl_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(RADIOCONTROL, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_radio_ctl_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RADIO_CTL);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_RADIO_CTL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RADIO_CTL_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->RadioCtl = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RADIO_CTL_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Radio is %s\n", (tlv->RadioCtl == 0) ? "on" : "off");
            } else {
                printf("Radio setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get radio status!\n");
            } else {
                printf("ERR:Could not set radio status!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for RSN replay protection
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_rsn_replay_prot [0|1]"
 *
 *   Options: 0     - Disable RSN replay protection
 *            1     - Enable  RSN replay protection
 *            empty - Get current RSN replay protection setting
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_rsn_replay_prot(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    tlvbuf_rsn_replay_prot *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_rsn_replay_prot_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && is_input_valid(RSNREPLAYPROT, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_rsn_replay_prot_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(tlvbuf_rsn_replay_prot);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (tlvbuf_rsn_replay_prot *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RSN_REPLAY_PROT_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->rsn_replay_prot = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RSN_REPLAY_PROT_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("RSN replay protection is %s\n",
                       (tlv->rsn_replay_prot == 1) ? "enabled" : "disabled");
            } else {
                printf("RSN replay protection setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get RSN replay protection !\n");
            } else {
                printf("ERR:Could not set RSN replay protection !\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for MCBC data rates
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_mcbc_data_rate [MCBC_DATA_RATE]"
 *
 *   Options: 0     - Auto rate
 *            >0    - Set specified MCBC data rate
 *            empty - Get current MCBC data rate
 *
 *           MCBC_DATA_RATE is represented in units of 500 kbps
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_mcbc_data_rate(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_MCBC_DATA_RATE *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_mcbc_data_rates_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc) {
        if (is_input_valid(MCBCDATARATE, argc, argv) != UAP_SUCCESS) {
            printf("ERR: Invalid input\n");
            print_sys_cfg_mcbc_data_rates_usage();
            return;
        }
        if ((A2HEXDECIMAL(argv[0]) != 0) &&
            (is_mcbc_rate_valid(A2HEXDECIMAL(argv[0])) != UAP_SUCCESS)) {
            printf("ERR: invalid MCBC data rate.");
            print_sys_cfg_mcbc_data_rates_usage();
            return;
        }
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_MCBC_DATA_RATE);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_MCBC_DATA_RATE *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_MCBC_DATA_RATE_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->MCBCdatarate = (u16) A2HEXDECIMAL(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->MCBCdatarate = uap_cpu_to_le16(tlv->MCBCdatarate);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->MCBCdatarate = uap_le16_to_cpu(tlv->MCBCdatarate);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_MCBC_DATA_RATE_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                if (tlv->MCBCdatarate == 0) {
                    printf("MCBC data rate is auto\n");
                } else {
                    printf("MCBC data rate = 0x%x\n", tlv->MCBCdatarate);
                }
            } else {
                printf("MCBC data rate setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get MCBC data rate!\n");
            } else {
                printf("ERR:Could not set MCBC data rate!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for Tx data rates
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_tx_data_rate [TX_DATA_RATE]"
 *
 *   Options: 0     - Auto rate
 *            >0    - Set specified data rate
 *            empty - Get current data rate
 *
 *           TX_DATA_RATE is represented in units of 500 kbps
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_tx_data_rate(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_TX_DATA_RATE *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_tx_data_rates_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc) {
        if (is_input_valid(TXDATARATE, argc, argv) != UAP_SUCCESS) {
            printf("ERR: Invalid input\n");
            print_sys_cfg_tx_data_rates_usage();
            return;
        } else if ((A2HEXDECIMAL(argv[0]) != 0) &&
                   (is_tx_rate_valid(A2HEXDECIMAL(argv[0])) != UAP_SUCCESS)) {
            printf("ERR: invalid tx data rate.");
            print_sys_cfg_tx_data_rates_usage();
            return;
        }
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_TX_DATA_RATE);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_TX_DATA_RATE *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_TX_DATA_RATE_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->TxDataRate = (u16) A2HEXDECIMAL(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->TxDataRate = uap_cpu_to_le16(tlv->TxDataRate);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->TxDataRate = uap_le16_to_cpu(tlv->TxDataRate);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_TX_DATA_RATE_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                if (tlv->TxDataRate == 0) {
                    printf("Tx data rate is auto\n");
                } else {
                    printf("Tx data rate = 0x%x\n", tlv->TxDataRate);
                }
            } else {
                printf("Tx data rate setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get tx data rate!\n");
            } else {
                printf("ERR:Could not set tx data rate!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for packet forwarding
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_pkt_fwd_ctl [0|1]"
 *
 *   Options: 0     - Forward all packets to the host
 *            1     - Firmware handles intra-BSS packets
 *            empty - Get current packet forwarding setting
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_pkt_fwd_ctl(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_PKT_FWD_CTL *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_pkt_fwd_ctl_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(PKTFWD, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_pkt_fwd_ctl_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PKT_FWD_CTL);
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_PKT_FWD_CTL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_PKT_FWD_CTL_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->PktFwdCtl = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_PKT_FWD_CTL_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("Firmware %s\n", (tlv->PktFwdCtl == 0) ?
                       "forwards all packets to the host" :
                       "handles intra-BSS packets");
            } else {
                printf("Packet control logic setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get packet control logic!\n");
            } else {
                printf("ERR:Could not set packet control logic!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for STA ageout timer
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_sta_ageout_timer [STA_AGEOUT_TIMER]"
 *           if STA_AGEOUT_TIMER is provided, a 'set' is performed
 *           else a 'get' is performed.
 *           The value should between 300 and 864000
 *
 *           STA_AGEOUT_TIMER is represented in units of 100 ms
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_sta_ageout_timer(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_STA_AGEOUT_TIMER *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_sta_ageout_timer_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(STAAGEOUTTIMER, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_sta_ageout_timer_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_STA_AGEOUT_TIMER);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_STA_AGEOUT_TIMER *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_STA_AGEOUT_TIMER_TLV_ID;
    tlv->Length = 4;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->StaAgeoutTimer_ms = (u32) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->StaAgeoutTimer_ms = uap_cpu_to_le32(tlv->StaAgeoutTimer_ms);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->StaAgeoutTimer_ms = uap_le32_to_cpu(tlv->StaAgeoutTimer_ms);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_STA_AGEOUT_TIMER_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("STA ageout timer value = %d\n",
                       (int) tlv->StaAgeoutTimer_ms);
            } else {
                printf("STA ageout timer setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get STA ageout timer!\n");
            } else {
                printf("ERR:Could not set STA ageout timer!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for authentication mode
 *   and sends to the driver
 *
 *   Usage: "Usage : sys_cfg_auth [AUTHMODE]"
 *
 *   Options: AUTHMODE :     0 - Open authentication
 *                           1 - Shared key authentication
 *            empty - Get current authentication mode                         
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_auth(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_AUTH_MODE *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_auth_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(AUTHMODE, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_auth_usage();
        return;
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_AUTH_MODE);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_AUTH_MODE *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_AUTH_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->AuthMode = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_AUTH_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                print_auth(tlv);
            } else {
                printf("authentication mode setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get authentication mode!\n");
            } else {
                printf("ERR:Could not set authentication mode!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for encryption protocol
 *   and sends to the driver
 *
 *   Usage: "Usage : sys_cfg_protocol [PROTOCOL]"
 *
 *   Options: PROTOCOL    	     Bit 0 - No RSN
 *		                     Bit 1 - WEP Static
 *		                     Bit 3 - WPA
 *	                             Bit 3 - WPA2
 *            empty - Get current protocol                         
 *           
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_protocol(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_PROTOCOL *tlv = NULL;
    TLVBUF_AKMP *akmp_tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_protocol_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && is_input_valid(PROTOCOL, argc, argv) != UAP_SUCCESS) {
        print_sys_cfg_protocol_usage();
        return;
    }
    /* Initialize the command length */
    if ((argc == 1) &&
        ((atoi(argv[0]) == PROTOCOL_NO_SECURITY) ||
         (atoi(argv[0]) == PROTOCOL_STATIC_WEP))) {
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PROTOCOL);
    } else
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PROTOCOL) +
            sizeof(TLVBUF_AKMP);
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_PROTOCOL *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
    akmp_tlv =
        (TLVBUF_AKMP *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE) +
                         sizeof(TLVBUF_PROTOCOL));
    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_PROTOCOL_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        akmp_tlv->Tag = MRVL_AKMP_TLV_ID;
        akmp_tlv->Length = 2;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Protocol = (u16) atoi(argv[0]);
        if (tlv->Protocol & (PROTOCOL_WPA | PROTOCOL_WPA2)) {
            akmp_tlv->Tag = MRVL_AKMP_TLV_ID;
            akmp_tlv->Length = 2;
            akmp_tlv->KeyMgmt = KEY_MGMT_PSK;
            akmp_tlv->KeyMgmt = uap_cpu_to_le16(akmp_tlv->KeyMgmt);
        }
    }
    endian_convert_tlv_header_out(tlv);
    if (cmd_len ==
        (sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PROTOCOL) +
         sizeof(TLVBUF_AKMP)))
        endian_convert_tlv_header_out(akmp_tlv);
    tlv->Protocol = uap_cpu_to_le16(tlv->Protocol);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    if (cmd_len ==
        (sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_PROTOCOL) +
         sizeof(TLVBUF_AKMP)))
        endian_convert_tlv_header_in(akmp_tlv);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                print_tlv((u8 *) tlv,
                          cmd_buf->Size - sizeof(APCMDBUF_SYS_CONFIGURE) +
                          BUF_HEADER_SIZE);
            } else {
                printf("protocol setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get protocol!\n");
            } else {
                printf("ERR:Could not set protocol!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for WEP keys settings
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_wep_key [INDEX_0 IS_DEFAULT KEY_0] [INDEX_1 IS_DEFAULT KEY_1] [INDEX_2 IS_DEFAULT KEY_2] [INDEX_3 IS_DEFAULT KEY_3]"
 *
 *   Options: INDEX_* :      0 - KeyIndex is 0
 *                           1 - KeyIndex is 1
 *                           2 - KeyIndex is 2
 *                           3 - KeyIndex is 3
 *            IS_DEFAUL :    0 - KeyIndex is not the default
 *                           1 - KeyIndex is the default transmit key
 *            KEY_* :        Key value
 *            empty - Get current WEP key settings
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_wep_key(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_WEP_KEY *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    int key_len = -1;
    int length = 0;
    int number_of_keys = 0;
    int i = 0;
    int keyindex = -1;
    int is_default = -1;
    char *key = NULL;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_wep_key_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 12) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_wep_key_usage();
        return;
    } else if ((argc > 1) && ((argc % 3) != 0)) {
        printf("ERR:Illegal number of parameters.\n");
        print_sys_cfg_wep_key_usage();
        return;
    } else if (argc > 1) {
        /* Find number of keys provided */
        number_of_keys = argc / 3;
        for (i = 0; i < number_of_keys; i++) {
            if ((ISDIGIT(argv[(3 * i)]) == 0) || (atoi(argv[(3 * i)]) < 0) ||
                (atoi(argv[(3 * i)]) > 3)) {
                printf
                    ("ERR:Illegal INDEX %s. Must be either '0', '1', '2', or '3'.\n",
                     argv[(3 * i)]);
                print_sys_cfg_wep_key_usage();
                return;
            }
            if ((ISDIGIT(argv[(3 * i) + 1]) == 0) ||
                (atoi(argv[(3 * i) + 1]) < 0) ||
                (atoi(argv[(3 * i) + 1]) > 1)) {
                printf
                    ("ERR:Illegal IS_DEFAULT %s. Must be either '0', or '1'.\n",
                     argv[(3 * i) + 1]);
                print_sys_cfg_wep_key_usage();
                return;
            }
            if ((strlen(argv[(3 * i) + 2]) != 5) &&
                (strlen(argv[(3 * i) + 2]) != 10)
                && (strlen(argv[(3 * i) + 2]) != 13) &&
                (strlen(argv[(3 * i) + 2]) != 26)) {
                printf("ERR:Incorrect KEY_%d length %d\n", atoi(argv[(3 * i)]),
                       strlen(argv[(3 * i) + 2]));
                print_sys_cfg_wep_key_usage();
                return;
            }
            if ((strlen(argv[(3 * i) + 2]) == 10) ||
                (strlen(argv[(3 * i) + 2]) == 26)) {
                if (UAP_FAILURE == ishexstring(argv[(3 * i) + 2])) {
                    printf
                        ("ERR:Only hex digits are allowed when key length is 10 or 26\n");
                    print_sys_cfg_wep_key_usage();
                    return;
                }
            }
        }
    } else if (argc == 1) {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
            (atoi(argv[0]) > 3)) {
            printf
                ("ERR:Illegal INDEX %s. Must be either '0', '1', '2', or '3'.\n",
                 argv[0]);
            print_sys_cfg_wep_key_usage();
            return;
        }
    }

    /* Initialize the command length */
    if (argc == 0 || argc == 1) {
        if (argc == 0)
            cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_HEADER);
        else
            cmd_len =
                sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_WEP_KEY) - 1;
        buf_len = MRVDRV_SIZE_OF_CMD_BUFFER;
    } else {
        buf_len = cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE);
    }
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(buf_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, buf_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    if (argc == 0 || argc == 1) {
        cmd_buf->Action = ACTION_GET;
        tlv = (TLVBUF_WEP_KEY *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
        tlv->Tag = MRVL_WEP_KEY_TLV_ID;
        if (argc == 0)
            tlv->Length = 0;
        else {
            tlv->Length = 1;
            tlv->KeyIndex = atoi(argv[0]);
        }
        endian_convert_tlv_header_out(tlv);
    } else {
        cmd_buf->Action = ACTION_SET;
    }
    /* Add key TLVs */
    for (i = 0; i < number_of_keys; i++) {
        keyindex = atoi(argv[(3 * i)]);
        is_default = atoi(argv[(3 * i) + 1]);
        key = argv[(3 * i) + 2];
        length = strlen(key);
        switch (length) {
        case 5:
        case 10:
            key_len = 5;
            break;
        case 13:
        case 26:
            key_len = 13;
            break;
        default:
            key_len = 0;
            break;
        }
        /* Adjust command buffer */
        buffer = realloc(buffer, (cmd_len + sizeof(TLVBUF_WEP_KEY) + key_len));
        if (!buffer) {
            printf("ERR:Cannot append WEP key configurations TLV!\n");
            return;
        }
        /* Locate headers */
        cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
        tlv = (TLVBUF_WEP_KEY *) (buffer + cmd_len);
        /* Adjust command length */
        cmd_len += (sizeof(TLVBUF_WEP_KEY) + key_len);
        /* Set TLV fields */
        tlv->Tag = MRVL_WEP_KEY_TLV_ID;
        tlv->Length = 2 + key_len;
        tlv->KeyIndex = (u8) keyindex;
        tlv->IsDefault = (u8) is_default;
        /* Check if string or raw */
        switch (length) {
        case 5:
        case 13:
            memcpy(tlv->Key, key, length);
            break;
        case 10:
        case 26:
            string2raw(key, tlv->Key);
            break;
        default:
            break;
        }
        endian_convert_tlv_header_out(tlv);
    }

    /* Update command length */
    cmd_buf->Size = cmd_len;
    if ((argc != 0) && (argc != 1))
        buf_len = cmd_len;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if ((argc != 0) && (argc != 1)) {
                printf("WEP key setting successful\n");
            } else {
                printf("query WEP key setting successful\n");
                tlv =
                    (TLVBUF_WEP_KEY *) (buffer +
                                        sizeof(APCMDBUF_SYS_CONFIGURE));
                print_tlv((u8 *) tlv,
                          cmd_buf->Size - sizeof(APCMDBUF_SYS_CONFIGURE) +
                          BUF_HEADER_SIZE);
            }
        } else {
            if ((argc != 0) && (argc != 1))
                printf("ERR:Could not set WEP keys!\n");
            else
                printf("ERR:Could not get WEP keys!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief Creates a sys_cfg request for custom IE settings
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_custom_ie [INDEX] [MASK] [IEBuffer]"
 *
 *   Options: INDEX :      0 - Get/Set IE index 0 setting
 *                         1 - Get/Set IE index 1 setting
 *                         2 - Get/Set IE index 2 setting
 *                         3 - Get/Set IE index 3 setting
 *            MASK  :      Management subtype mask value
 *            IEBuffer:      IE Buffer in hex
 *            empty - Get all IE settings
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_custom_ie(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    tlvbuf_custom_ie *tlv = NULL;
    custom_ie *ie_ptr = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    u16 buf_len;
    u16 mgmt_subtype_mask = 0;
    int ret = UAP_FAILURE;
    int ie_buf_len = 0, ie_len = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_custom_ie_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 3) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_custom_ie_usage();
        return;
    }

    /* Error checks and initialize the command length */
    if (argc >= 1) {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
            (atoi(argv[0]) > 3)) {
            printf
                ("ERR:Illegal index %s. Must be either '0', '1', '2', or '3'.\n",
                 argv[0]);
            print_sys_cfg_custom_ie_usage();
            return;
        }
    }
    switch (argc) {
    case 0:
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(tlvbuf_custom_ie);
        buf_len = MRVDRV_SIZE_OF_CMD_BUFFER;
        break;
    case 1:
        /* TLV header + ie_index */
        cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(tlvbuf_custom_ie) +
            sizeof(u16);
        buf_len = MRVDRV_SIZE_OF_CMD_BUFFER;
        break;
    case 2:
        if (UAP_FAILURE == ishexstring(argv[1]) || A2HEXDECIMAL(argv[1]) != 0) {
            printf("ERR: Mask value should be 0 to clear IEBuffers.\n");
            print_sys_cfg_custom_ie_usage();
            return;
        }
        buf_len = cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) +
            sizeof(tlvbuf_custom_ie) + sizeof(custom_ie);
        break;
    case 3:
        if (UAP_FAILURE == ishexstring(argv[1]) || A2HEXDECIMAL(argv[1]) == 0) {
            printf("ERR: Mask value should not be 0 to set IEBuffers.\n");
            print_sys_cfg_custom_ie_usage();
            return;
        }
        if (UAP_FAILURE == ishexstring(argv[2])) {
            printf("ERR:Only hex digits are allowed\n");
            print_sys_cfg_custom_ie_usage();
            return;
        }
        ie_buf_len = strlen(argv[2]);
        if (!strncasecmp("0x", argv[2], 2)) {
            ie_len = (ie_buf_len - 2 + 1) / 2;
            argv[2] += 2;
        } else
            ie_len = (ie_buf_len + 1) / 2;
        if (ie_len > MAX_IE_BUFFER_LEN) {
            printf("ERR:Incorrect IE length %d\n", ie_buf_len);
            print_sys_cfg_custom_ie_usage();
            return;
        }
        mgmt_subtype_mask = (u16) A2HEXDECIMAL(argv[1]);
        buf_len = cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) +
            sizeof(tlvbuf_custom_ie) + sizeof(custom_ie) + ie_len;
        break;
    }

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(buf_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    memset(buffer, 0, buf_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->Size = cmd_len;

    if (argc == 0 || argc == 1) {
        cmd_buf->Action = ACTION_GET;
        tlv = (tlvbuf_custom_ie *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
        tlv->Tag = MRVL_MGMT_IE_LIST_TLV_ID;
        if (argc == 0)
            tlv->Length = 0;
        else {
            tlv->Length = sizeof(u16);
            ie_ptr = (custom_ie *) (tlv->ie_data);
            ie_ptr->ie_index = (u16) uap_cpu_to_le16(atoi(argv[0]));
        }
        endian_convert_tlv_header_out(tlv);
    } else {
        cmd_buf->Action = ACTION_SET;
        /* Locate headers */
        tlv = (tlvbuf_custom_ie *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
        ie_ptr = (custom_ie *) (tlv->ie_data);
        /* Set TLV fields */
        tlv->Tag = MRVL_MGMT_IE_LIST_TLV_ID;
        tlv->Length = sizeof(custom_ie) + ie_len;
        ie_ptr->ie_index = uap_cpu_to_le16(atoi(argv[0]));
        ie_ptr->mgmt_subtype_mask = uap_cpu_to_le16(mgmt_subtype_mask);
        ie_ptr->ie_length = uap_cpu_to_le16(ie_len);
        if (argc == 3)
            string2raw(argv[2], ie_ptr->ie_buffer);
        endian_convert_tlv_header_out(tlv);
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if ((argc != 0) && (argc != 1)) {
                printf("custom IE setting successful\n");
            } else {
                printf("Querying custom IE successful\n");
                tlv =
                    (tlvbuf_custom_ie *) (buffer +
                                          sizeof(APCMDBUF_SYS_CONFIGURE));
                print_tlv((u8 *) tlv,
                          cmd_buf->Size - sizeof(APCMDBUF_SYS_CONFIGURE) +
                          BUF_HEADER_SIZE);
            }
        } else {
            if ((argc != 0) && (argc != 1))
                printf("ERR:Could not set custom IE elements!\n");
            else
                printf("ERR:Could not get custom IE elements!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for cipher configurations
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_security_cfg [PAIRWISE_CIPHER GROUP_CIPHER]"
 *
 *   Options: PAIRWISE_CIPHER : Bit 2 - TKIP
 *                              Bit 3 - AES CCMP
 *            GROUP_CIPHER :    Bit 2 - TKIP
 *                              Bit 3 - AES CCMP
 *            empty - Get current cipher settings
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_cipher(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_CIPHER *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_cipher_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if ((argc != 0) && (argc != 2)) {
        printf("ERR:wrong arguments.\n");
        print_sys_cfg_cipher_usage();
        return;
    }
    if (argc == 2) {
        if ((ISDIGIT(argv[0]) == 0) || (ISDIGIT(argv[1]) == 0)) {
            print_sys_cfg_cipher_usage();
            return;
        }
        if (atoi(argv[0]) & ~CIPHER_BITMAP) {
            printf("ERR:Illegal PAIRWISE_CIPHER parameter %s.\n", argv[0]);
            print_sys_cfg_cipher_usage();
            return;
        }
        if (atoi(argv[1]) & ~CIPHER_BITMAP) {
            printf("ERR:Illegal GROUP_CIPHER parameter %s.\n", argv[1]);
            print_sys_cfg_cipher_usage();
            return;
        }
        if (is_cipher_valid(atoi(argv[0]), atoi(argv[1])) != UAP_SUCCESS) {
            printf("ERR:Wrong group and pair cipher combination!\n");
            print_sys_cfg_cipher_usage();
            return;
        }
    }
    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_CIPHER);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_CIPHER *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_CIPHER_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->PairwiseCipher = (u8) atoi(argv[0]);
        tlv->GroupCipher = (u8) atoi(argv[1]);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_CIPHER_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                print_cipher(tlv);
            } else {
                printf("cipher setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get cipher parameters!\n");
            } else {
                printf("ERR:Could not set cipher parameters!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for group re-key timer
 *   and sends to the driver
 *
 *   Usage: "Usage : sys_cfg_group_rekey_timer [GROUP_REKEY_TIMER]"
 *
 *   Options: GROUP_REKEY_TIME is represented in seconds
 *            Get current group re-key timer                         
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_group_rekey_timer(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_GROUP_REKEY_TIMER *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_group_rekey_timer_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc && (is_input_valid(GROUPREKEYTIMER, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_group_rekey_timer_usage();
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_GROUP_REKEY_TIMER);
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv =
        (TLVBUF_GROUP_REKEY_TIMER *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_GRP_REKEY_TIME_TLV_ID;
    tlv->Length = 4;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->GroupRekeyTime_sec = (u32) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->GroupRekeyTime_sec = uap_cpu_to_le32(tlv->GroupRekeyTime_sec);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->GroupRekeyTime_sec = uap_le32_to_cpu(tlv->GroupRekeyTime_sec);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_GRP_REKEY_TIME_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                if (tlv->GroupRekeyTime_sec > 0)
                    printf("Group rekey time is %ld s\n",
                           tlv->GroupRekeyTime_sec);
                else
                    printf("Group rekey time is disabled\n");
            } else {
                printf("group re-key time setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get group re-key time!\n");
            } else {
                printf("ERR:Could not set group re-key time!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for WPA passphrase
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_wpa_passphrase [PASSPHRASE]"
 *           if PASSPHRASE is provided, a 'set' is performed
 *           else a 'get' is performed
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_wpa_passphrase(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_WPA_PASSPHRASE *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_wpa_passphrase_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc > 1) {
        printf("ERR:Too many arguments.\n");
        print_sys_cfg_wpa_passphrase_usage();
        return;
    }
    if ((argc == 1) && (strlen(argv[0]) > MAX_WPA_PASSPHRASE_LENGTH)) {
        printf("ERR:PASSPHRASE too long.\n");
        return;
    }
    if ((argc == 1) && (strlen(argv[0]) < MIN_WPA_PASSPHRASE_LENGTH)) {
        printf("ERR:PASSPHRASE too short.\n");
        return;
    }
    if ((argc == 1) && (strlen(argv[0]) == MAX_WPA_PASSPHRASE_LENGTH)) {
        if (UAP_FAILURE == ishexstring(argv[0])) {
            printf
                ("ERR:Only hex digits are allowed when passphrase's length is 64\n");
            return;
        }
    }
    /* Initialize the command length */
    if (argc == 0)
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_WPA_PASSPHRASE) +
            MAX_WPA_PASSPHRASE_LENGTH;
    else
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_WPA_PASSPHRASE) +
            strlen(argv[0]);
    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_WPA_PASSPHRASE *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_WPA_PASSPHRASE_TLV_ID;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        tlv->Length = MAX_WPA_PASSPHRASE_LENGTH;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Length = strlen(argv[0]);
        memcpy(tlv->Passphrase, argv[0], tlv->Length);
    }
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_WPA_PASSPHRASE_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                if (tlv->Length > 0)
                    printf("WPA passphrase = %s\n", tlv->Passphrase);
                else
                    printf("WPA passphrase: None\n");
            } else {
                printf("WPA passphrase setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get WPA passphrase!\n");
            } else {
                printf("ERR:Could not set WPA passphrase!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a STA filter request and sends to the driver
 *
 *   Usage: "sta_filter_table <FILTERMODE> <MACADDRESS_LIST>"
 *
 *   Options: FILTERMODE : 0 - Disable filter table
 *                         1 - Allow mac address specified in the allwed list
 *		           2 - Block MAC addresses specified in the  banned list
 *            MACADDRESS_LIST is the list of MAC addresses to be acted upon. Each
 *                         MAC address must be separated with a space. Maximum of
 *                         16 MAC addresses are supported.
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sta_filter_table(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_STA_MAC_ADDR_FILTER *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int i = 0;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sta_filter_table_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if (argc > (MAX_MAC_ONESHOT_FILTER + 1)) {
        printf("ERR:Too many arguments.\n");
        print_sta_filter_table_usage();
        return;
    }
    if (argc > 0) {
        if ((ISDIGIT(argv[0]) == 0) ||
            ((atoi(argv[0]) < 0) || (atoi(argv[0]) > 2))) {
            printf
                ("ERR:Illegal FILTERMODE parameter %s. Must be either '0', '1', or '2'.\n",
                 argv[1]);
            print_sta_filter_table_usage();
            return;
        }
        if ((atoi(argv[0]) != 0) && (argc == 1)) {
            printf("ERR:At least one mac is required.\n");
            print_sta_filter_table_usage();
            return;
        }
    }
    /* Initialize the command length */
    if (argc == 0) {
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) +
            sizeof(TLVBUF_STA_MAC_ADDR_FILTER) +
            (MAX_MAC_ONESHOT_FILTER * ETH_ALEN);
    } else {
        cmd_len =
            sizeof(APCMDBUF_SYS_CONFIGURE) +
            sizeof(TLVBUF_STA_MAC_ADDR_FILTER) + (argc - 1) * ETH_ALEN;
    }

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv =
        (TLVBUF_STA_MAC_ADDR_FILTER *) (buffer +
                                        sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_STA_MAC_ADDR_FILTER_TLV_ID;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
        tlv->Count = MAX_MAC_ONESHOT_FILTER;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->FilterMode = atoi(argv[0]);
        tlv->Count = argc - 1;
        for (i = 0; i < tlv->Count; i++) {
            if ((ret =
                 mac2raw(argv[i + 1],
                         &tlv->MacAddress[i * ETH_ALEN])) != UAP_SUCCESS) {
                printf("ERR: %s Address\n",
                       ret == UAP_FAILURE ? "Invalid MAC" : ret ==
                       UAP_RET_MAC_BROADCAST ? "Broadcast" : "Multicast");
                print_sta_filter_table_usage();
                free(buffer);
                return;
            }
        }
    }
    tlv->Length = tlv->Count * ETH_ALEN + 2;
    endian_convert_tlv_header_out(tlv);
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_STA_MAC_ADDR_FILTER_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                print_mac_filter(tlv);
            } else {
                printf("MAC address filter table setting successful!\n");
            }
        } else {
            if (argc == 0) {
                printf
                    ("ERR:Could not get MAC address filter table settings!\n");
            } else {
                printf
                    ("ERR:Could not set MAC address filter table settings!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for max station number
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_max_sta_num [STA_NUM]"
 *           if STA_NUM is provided, a 'set' is performed
 *           else a 'get' is performed.
 *
 *           STA_NUM should not bigger than 8
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_max_sta_num(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_MAX_STA_NUM *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_max_sta_num_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && (is_input_valid(MAXSTANUM, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_max_sta_num_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_MAX_STA_NUM);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_MAX_STA_NUM *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_MAX_STA_CNT_TLV_ID;
    tlv->Length = 2;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->Max_sta_num = (u16) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);
    tlv->Max_sta_num = uap_cpu_to_le16(tlv->Max_sta_num);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);
    tlv->Max_sta_num = uap_le16_to_cpu(tlv->Max_sta_num);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_MAX_STA_CNT_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("max station number = %d\n", tlv->Max_sta_num);
            } else {
                printf("max station number setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get max station number!\n");
            } else {
                printf("ERR:Could not set max station number!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief Creates a sys_cfg request for retry limit
 *   and sends to the driver
 *
 *   Usage: "sys_cfg_retry_limit [RETRY_LIMIT]"
 *           if RETRY_LIMIT is provided, a 'set' is performed
 *           else a 'get' is performed.
 *
 *           RETRY_LIMIT should not bigger than 14
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_cfg_retry_limit(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    TLVBUF_RETRY_LIMIT *tlv = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_cfg_max_sta_num_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc && (is_input_valid(RETRYLIMIT, argc, argv) != UAP_SUCCESS)) {
        print_sys_cfg_retry_limit_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(TLVBUF_RETRY_LIMIT);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
    tlv = (TLVBUF_RETRY_LIMIT *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    tlv->Tag = MRVL_RETRY_LIMIT_TLV_ID;
    tlv->Length = 1;
    if (argc == 0) {
        cmd_buf->Action = ACTION_GET;
    } else {
        cmd_buf->Action = ACTION_SET;
        tlv->retry_limit = (u8) atoi(argv[0]);
    }
    endian_convert_tlv_header_out(tlv);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
    endian_convert_tlv_header_in(tlv);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if ((cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) ||
            (tlv->Tag != MRVL_RETRY_LIMIT_TLV_ID)) {
            printf("ERR:Corrupted response! CmdCode=%x, Tlv->Tag=%x\n",
                   cmd_buf->CmdCode, tlv->Tag);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc == 0) {
                printf("retry limit = %d\n", tlv->retry_limit);
            } else {
                printf("retry limit setting successful\n");
            }
        } else {
            if (argc == 0) {
                printf("ERR:Could not get retry limit!\n");
            } else {
                printf("ERR:Could not set retry limit!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** 
 *  @brief convert string to integer
 * 
 *  @param ptr		A pointer to data buffer
 *  @param chr 		A pointer to return integer
 *  @return      	A pointer to next data field
 */
char *
convert2hex(char *ptr, u8 * chr)
{
    u8 val;

    for (val = 0; *ptr && isxdigit(*ptr); ptr++) {
        val = (val * 16) + hexc2bin(*ptr);
    }

    *chr = val;

    return ptr;
}

/** 
 *  @brief parse hex data
 *  @param fp 		A pointer to FILE stream
 *  @param dst		A pointer to receive hex data
 *  @return            	length of hex data
 */
int
fparse_for_hex(FILE * fp, u8 * dst)
{
    char *ptr;
    u8 *dptr;
    char buf[256];

    dptr = dst;
    while (fgets(buf, sizeof(buf), fp)) {
        ptr = buf;

        while (*ptr) {
            /* skip leading spaces */
            while (*ptr && (isspace(*ptr) || *ptr == '\t'))
                ptr++;

            /* skip blank lines and lines beginning with '#' */
            if (*ptr == '\0' || *ptr == '#')
                break;

            if (isxdigit(*ptr)) {
                ptr = convert2hex(ptr, dptr++);
            } else {
                /* Invalid character on data line */
                ptr++;
            }
        }
    }

    return (dptr - dst);
}

/** 
 *  @brief Creates a cfg_data request 
 *   and sends to the driver
 *
 *   Usage: "cfg_data <cfg_data.conf>"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_cfg_data(int argc, char *argv[])
{
    APCMDBUF_CFG_DATA *cmd_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    int opt;
    FILE *fp = NULL;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_cfg_data_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if ((argc == 0) || (argc > 2)) {
        printf("ERR:wrong arguments.\n");
        print_cfg_data_usage();
        return;
    } else {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) != 2)) {
            printf("ERR:Illegal type parameter %s. Must be '2'.\n", argv[0]);
            print_cfg_data_usage();
            return;
        }
    }
    buf_len = MRVDRV_SIZE_OF_CMD_BUFFER;
    buf = (u8 *) malloc(buf_len);
    memset(buf, 0, buf_len);
    cmd_buf = (APCMDBUF_CFG_DATA *) buf;
    if (buf == NULL) {
        printf("Error: allocate memory for hostcmd failed\n");
        return;
    }
    if (argc == 2) {
        /* Check if file exists */
        fp = fopen(argv[1], "r");
        if (fp == NULL) {
            printf("\nERR:Config file can not open %s.\n", argv[1]);
            free(buf);
            return;
        }
        cmd_buf->action = ACTION_SET;
        cmd_buf->data_len = fparse_for_hex(fp, cmd_buf->data);
        fclose(fp);
        if (cmd_buf->data_len > MAX_CFG_DATA_SIZE) {
            printf("ERR: Config file is too big %d\n", cmd_buf->data_len);
            free(buf);
            return;
        }
    } else {
        cmd_buf->action = ACTION_GET;
        cmd_buf->data_len = 0;
    }

    cmd_buf->action = uap_cpu_to_le16(cmd_buf->action);
    cmd_buf->type = atoi(argv[0]);
    cmd_buf->type = uap_cpu_to_le16(cmd_buf->type);
    cmd_buf->data_len = uap_cpu_to_le16(cmd_buf->data_len);

    /* Fill the command buffer */
    cmd_len = cmd_buf->data_len + sizeof(APCMDBUF_CFG_DATA);
    cmd_buf->CmdCode = HostCmd_CMD_CFG_DATA;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    /* Process response */
    if (ret == UAP_SUCCESS) {
        cmd_buf->action = uap_le16_to_cpu(cmd_buf->action);
        cmd_buf->data_len = uap_le16_to_cpu(cmd_buf->data_len);
        if (cmd_buf->action == ACTION_GET) {
            hexdump_data("cfg_data", cmd_buf->data, cmd_buf->data_len, ' ');
        } else
            printf("download cfg data successful\n");
    }
    if (buf)
        free(buf);
    return;
}
