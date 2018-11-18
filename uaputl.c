/** @file  uaputl.c
 *
 *  @brief Program to send AP commands to the driver/firmware of the uAP
 *         driver.
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
#include <stdarg.h>
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
#include <linux/if.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "uaputl.h"
#include "uapcmd.h"

/****************************************************************************
        Definitions
****************************************************************************/
/** Default debug level */
int debug_level = MSG_NONE;

/** Enable or disable debug outputs */
#define DEBUG   1

/** Convert character to integer */
#define CHAR2INT(x) (((x) >= 'A') ? ((x) - 'A' + 10) : ((x) - '0'))

/****************************************************************************
        Global variables
****************************************************************************/
/** Device name */
static char dev_name[IFNAMSIZ + 1];
/** option for cmd */
struct option cmd_options[] = {
    {"help", 0, 0, 'h'},
    {0, 0, 0, 0}
};

/****************************************************************************
        Local functions
****************************************************************************/
/**
 *    @brief convert char to hex integer
 *   
 *    @param chr          char
 *    @return             hex integer
 */
unsigned char
hexc2bin(char chr)
{
    if (chr >= '0' && chr <= '9')
        chr -= '0';
    else if (chr >= 'A' && chr <= 'F')
        chr -= ('A' - 10);
    else if (chr >= 'a' && chr <= 'f')
        chr -= ('a' - 10);

    return chr;
}

/** 
 *  @brief check protocol is valid or not
 *
 *  @param protocol    	     protocol
 *
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
is_protocol_valid(int protocol)
{
    int ret = UAP_FAILURE;
    switch (protocol) {
    case PROTOCOL_NO_SECURITY:
    case PROTOCOL_STATIC_WEP:
    case PROTOCOL_WPA:
    case PROTOCOL_WPA2:
    case PROTOCOL_WPA2_MIXED:
        ret = UAP_SUCCESS;
        break;
    default:
        printf("ERR: Invalid Protocol: %d\n", protocol);
        break;
    }
    return ret;
}

/**
 *  @brief Function to check valid rate
 *
 *                  
 *  @param  rate    rate to verify
 *
 *  return 	    UAP_SUCCESS or UAP_FAILURE	
 **/
int
is_rate_valid(int rate)
{
    int ret = UAP_SUCCESS;
    switch (rate) {
    case 2:
    case 4:
    case 11:
    case 22:
    case 12:
    case 18:
    case 24:
    case 48:
    case 72:
    case 96:
    case 108:
    case 36:
        break;
    default:
        ret = UAP_FAILURE;
        break;
    }
    return ret;
}

/**
 *  @brief  detects duplicates rate in array of strings
 *          Note that 0x82 and 0x2 are same for rate
 *
 *  @param  argc    number of elements
 *  @param  argv    array of strings
 *  @return UAP_FAILURE or UAP_SUCCESS
 */
inline int
has_dup_rate(int argc, char *argv[])
{
    int i, j;
    /* Check for duplicate */
    for (i = 0; i < (argc - 1); i++) {
        for (j = i + 1; j < argc; j++) {
            if ((A2HEXDECIMAL(argv[i]) & ~BASIC_RATE_SET_BIT) ==
                (A2HEXDECIMAL(argv[j]) & ~BASIC_RATE_SET_BIT)) {
                return UAP_FAILURE;
            }
        }
    }
    return UAP_SUCCESS;
}

/**
 *  @brief Check for mandatory rates
 *
 *
 * 2, 4, 11, 22 must be present 
 *
 * 6 12 and 24 must be present for ofdm
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_FAILURE or UAP_SUCCESS
 *
 */
int
check_mandatory_rates(int argc, char **argv)
{
    int i;
    int tmp;
    u32 rate_bitmap = 0;
    int ofdm_enable = 0;
#define BITMAP_RATE_1M         0x01
#define BITMAP_RATE_2M         0x02
#define BITMAP_RATE_5_5M       0x04
#define BITMAP_RATE_11M        0x8
#define B_RATE_MANDATORY       0x0f
#define BITMAP_RATE_6M         0x10
#define BITMAP_RATE_12M        0x20
#define BITMAP_RATE_24M        0x40
#define G_RATE_MANDATORY        0x70
    for (i = 0; i < argc; i++) {
        tmp = (A2HEXDECIMAL(argv[i]) & ~BASIC_RATE_SET_BIT);
        switch (tmp) {
        case 2:
            rate_bitmap |= BITMAP_RATE_1M;
            break;
        case 4:
            rate_bitmap |= BITMAP_RATE_2M;
            break;
        case 11:
            rate_bitmap |= BITMAP_RATE_5_5M;
            break;
        case 22:
            rate_bitmap |= BITMAP_RATE_11M;
            break;
        case 12:
            ofdm_enable = 1;
            rate_bitmap |= BITMAP_RATE_6M;
            break;
        case 24:
            ofdm_enable = 1;
            rate_bitmap |= BITMAP_RATE_12M;
            break;
        case 48:
            ofdm_enable = 1;
            rate_bitmap |= BITMAP_RATE_24M;
            break;
        case 18:
        case 36:
        case 72:
        case 96:
        case 108:
            ofdm_enable = 1;
            break;
        }
    }
    if ((rate_bitmap & B_RATE_MANDATORY) != B_RATE_MANDATORY) {
        printf("Basic Rates 2, 4, 11 and 22 (500K units) \n"
               "must be present in basic or non-basic rates\n");
        return UAP_FAILURE;
    }
    if (ofdm_enable && ((rate_bitmap & G_RATE_MANDATORY) != G_RATE_MANDATORY)) {
        printf("OFDM Rates 12, 24 and 48 ( 500Kb units)\n"
               "must be present in basic or non-basic rates\n");
        return UAP_FAILURE;
    }
    return UAP_SUCCESS;
}

/**
 *  @brief  detects duplicates channel in array of strings
 *          
 *  @param  argc    number of elements
 *  @param  argv    array of strings
 *  @return UAP_FAILURE or UAP_SUCCESS
 */
inline int
has_dup_channel(int argc, char *argv[])
{
    int i, j;
    /* Check for duplicate */
    for (i = 0; i < (argc - 1); i++) {
        for (j = i + 1; j < argc; j++) {
            if (atoi(argv[i]) == atoi(argv[j])) {
                return UAP_FAILURE;
            }
        }
    }
    return UAP_SUCCESS;
}

/** 
 *    @brief convert string to hex integer
 *  
 *    @param s            A pointer string buffer
 *    @return             hex integer
 */
unsigned int
a2hex(char *s)
{
    unsigned int val = 0;
    if (!strncasecmp("0x", s, 2)) {
        s += 2;
    }
    while (*s && isxdigit(*s)) {
        val = (val << 4) + hexc2bin(*s++);
    }
    return val;
}

/** 
 *  @brief Dump hex data
 *
 *  @param prompt	A pointer prompt buffer
 *  @param p		A pointer to data buffer
 *  @param len		the len of data buffer
 *  @param delim	delim char
 *  @return            	None
 */
void
hexdump_data(char *prompt, void *p, int len, char delim)
{
    int i;
    unsigned char *s = p;

    if (prompt) {
        printf("%s: len=%d\n", prompt, (int) len);
    }
    for (i = 0; i < len; i++) {
        if (i != len - 1)
            printf("%02x%c", *s++, delim);
        else
            printf("%02x\n", *s);
        if ((i + 1) % 16 == 0)
            printf("\n");
    }
    printf("\n");
}

#if DEBUG
/** 
 * @brief           conditional printf
 *
 * @param level     severity level of the message
 * @param fmt       printf format string, followed by optional arguments
 */
void
uap_printf(int level, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (level <= debug_level) {
        vprintf(fmt, ap);
    }
    va_end(ap);
}

/** 
 *  @brief Dump hex data
 *
 *  @param prompt	A pointer prompt buffer
 *  @param p		A pointer to data buffer
 *  @param len		the len of data buffer
 *  @param delim	delim char
 *  @return            	None
 */
void
hexdump(char *prompt, void *p, int len, char delim)
{
    if (debug_level < MSG_ALL)
        return;
    hexdump_data(prompt, p, len, delim);
}
#endif

/** 
 *  @brief      Hex to number 
 *
 *  @param c    Hex value
 *  @return     Integer value or -1
 */
int
hex2num(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return -1;
}

/**
 *  @brief Show usage information for the sys_info command
 *
 *  $return         N/A
 */
void
print_sys_info_usage(void)
{
    printf("\nUsage : sys_info\n");
    return;
}

/**
 *  @brief Parse domain file for country information
 *
 *  @param country  Country name
 *  @param sub_bands band information 
 *  @return number of band/ UAP_FAILURE 
 */
u8
parse_domain_file(char *country, IEEEtypes_SubbandSet_t * sub_bands)
{
    FILE *fp;
    char str[64];
    char domain_name[40];
    int cflag = 0;
    int dflag = 0;
    int found = 0;
    int j = -1, reset_j = 0;
    u8 no_of_sub_band = 0;
    char third;
    char country2[3];

    fp = fopen("80211d_domain.conf", "r");
    if (fp == NULL) {
        printf("File opening Error\n");
        return UAP_FAILURE;
    }

    strncpy((char *) country2, country, 2);
    country2[2] = '\0';
    third = country[2];

    /** 
     * Search specific domain name
     */
    while (!feof(fp)) {
        fscanf(fp, "%s", str);
        if (cflag) {
            strcpy(domain_name, str);
            cflag = 0;
        }
        if (!strcmp(str, "COUNTRY:")) {
            /** store next string to domain_name */
            cflag = 1;
        }

        if (!strcmp(str, country2)) {
            /** Country is matched ;)*/
            if (third && !((third == 'I') || (third == 'O') || (third == ' ')))
                found = 0;
            else
                found = 1;
            break;
        }
    }

    if (!found) {
        printf("No match found for Country = %s in the 80211d_domain.conf \n",
               country);
        fclose(fp);
        found = 0;
        return UAP_FAILURE;
    }

    /**
     * Search domain specific information
     */
    while (!feof(fp)) {
        fscanf(fp, "%s", str);

        if (feof(fp) || (dflag && !strcmp(str, "DOMAIN:"))) {
            break;
        }

        if (dflag) {
            j++;
            if (strchr(str, ','))
                reset_j = 1;

            strcpy(str, strtok(str, ", "));

            if (str == NULL) {
                if (reset_j) {
                    j = -1;
                    reset_j = 0;
                }
                continue;
            }

            if (IS_HEX_OR_DIGIT(str) == UAP_FAILURE) {
                printf("ERR: Only Number values are allowed\n");
                fclose(fp);
                return UAP_FAILURE;
            }

            switch (j) {
            case 0:
                sub_bands[no_of_sub_band].FirstChan = (u8) A2HEXDECIMAL(str);
                break;
            case 1:
                sub_bands[no_of_sub_band].NoOfChan = (u8) A2HEXDECIMAL(str);
                break;
            case 2:
                sub_bands[no_of_sub_band++].MaxTxPwr = (u8) A2HEXDECIMAL(str);
                break;
            default:
                printf("ERR: Incorrect 80211d_domain.conf file\n");
                fclose(fp);
                return UAP_FAILURE;
            }

            if (reset_j) {
                j = -1;
                reset_j = 0;
            }
        }

        if (cflag && !strcmp(str, domain_name)) {
            /* Followed will be the band details */
            cflag = 0;
            dflag = 1;
        }
        if (!dflag && !strcmp(str, "DOMAIN:")) {
            cflag = 1;
        }
    }
    fclose(fp);
    return (no_of_sub_band);

}

/** 
 *
 *  @brief Set/Get SNMP MIB
 *
 *  @param action 0-GET 1-SET
 *  @param oid    oid
 *  @param size   size of oid value
 *  @param oid_buf  oid value
 *  @return UAP_FAILURE or UAP_SUCCESS
 *
 */
int
sg_snmp_mib(u16 action, u16 oid, u16 size, u8 * oid_buf)
{
    APCMDBUF_SNMP_MIB *cmd_buf = NULL;
    TLVBUF_HEADER *tlv = NULL;
    int ret = UAP_FAILURE;
    u8 *buf = NULL;
    u16 buf_len;
    u16 cmd_len;
    int i;

    buf_len = sizeof(APCMDBUF_SNMP_MIB) + sizeof(TLVBUF_HEADER) + size;
    buf = (u8 *) malloc(buf_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return ret;
    }
    bzero((char *) buf, buf_len);

    /* Locate Headers */
    cmd_buf = (APCMDBUF_SNMP_MIB *) buf;
    tlv = (TLVBUF_HEADER *) (buf + sizeof(APCMDBUF_SNMP_MIB));
    cmd_buf->Size = buf_len - BUF_HEADER_SIZE;
    cmd_buf->Result = 0;
    cmd_buf->SeqNum = 0;
    cmd_buf->CmdCode = HostCmd_SNMP_MIB;

    tlv->Type = uap_cpu_to_le16(oid);
    tlv->Len = uap_cpu_to_le16(size);
    for (i = 0; action && (i < size); i++) {
        tlv->Data[i] = oid_buf[i];
    }

    cmd_buf->Action = uap_cpu_to_le16(action);
    cmd_len = buf_len;
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (!action) {
                /** Reloacte the headers */
                tlv =
                    (TLVBUF_HEADER *) ((u8 *) cmd_buf +
                                       sizeof(APCMDBUF_SNMP_MIB));
                for (i = 0; i < MIN(uap_le16_to_cpu(tlv->Len), size); i++) {
                    oid_buf[i] = tlv->Data[i];
                }
            }
            ret = UAP_SUCCESS;
        } else {
            printf("ERR:Command Response incorrect!\n");
            ret = UAP_FAILURE;
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
    return ret;
}

/** 
 *  @brief Creates a sys_info request and sends to the driver
 *
 *  Usage: "sys_info"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_info(int argc, char *argv[])
{
    APCMDBUF_SYS_INFO_REQUEST *cmd_buf = NULL;
    APCMDBUF_SYS_INFO_RESPONSE *response_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_info_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_sys_info_usage();
        return;
    }

    buf_len =
        (sizeof(APCMDBUF_SYS_INFO_REQUEST) >=
         sizeof(APCMDBUF_SYS_INFO_RESPONSE)) ? sizeof(APCMDBUF_SYS_INFO_REQUEST)
        : sizeof(APCMDBUF_SYS_INFO_RESPONSE);

    /* alloc buf for command */
    buf = (u8 *) malloc(buf_len);

    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);

    /* Locate headers */
    cmd_len = sizeof(APCMDBUF_SYS_INFO_REQUEST);
    cmd_buf = (APCMDBUF_SYS_INFO_REQUEST *) buf;
    response_buf = (APCMDBUF_SYS_INFO_RESPONSE *) buf;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_INFO;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (response_buf->CmdCode != (APCMD_SYS_INFO | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buf);
            return;
        }
        /* Print response */
        if (response_buf->Result == CMD_SUCCESS) {
            printf("System information = %s\n", response_buf->SysInfo);
        } else {
            printf("ERR:Could not retrieve system information!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }

    free(buf);
    return;
}

/**
 *  @brief Show usage information for the powermode command
 *
 *  $return         N/A
 */
void
print_power_mode_usage(void)
{
    printf
        ("\nUsage : powermode [MODE] [SLEEP_PARAM=1 CTRL MIN_SLEEP MAX_SLEEP]");
    printf("\n                  [INACT_PARAM=2 INACTTO MIN_AWAKE MAX_AWAKE]");
    printf("\nOptions: MODE :     0 - disable power mode");
    printf("\n                    1 - periodic DTIM power save mode");
    printf("\n                    2 - inactivity based power save mode");
    printf("\n   SLEEP_PARAM:");
    printf
        ("\n        CTRL:  0 - disable CTS2Self protection frame Tx before PS");
    printf
        ("\n               1 - enable CTS2Self protection frame Tx before PS");
    printf("\n        MIN_SLEEP: Minimum sleep duration in microseconds");
    printf("\n        MAX_SLEEP: Maximum sleep duration in miroseconds");
    printf("\n   INACT_PARAM: (only for inactivity based power save mode)");
    printf("\n          INACTTO: Inactivity timeout in miroseconds");
    printf("\n        MIN_AWAKE: Minimum awake duration in microseconds");
    printf("\n        MAX_AWAKE: Maximum awake duration in microseconds");
    printf("\n          empty - get current power mode\n");
    return;
}

/** 
 *  @brief Set/get power mode 
 *
 *  @param pm      A pointer to ps_mgmt structure
 *  @return         N/A
 */
void
send_power_mode_ioctl(ps_mgmt * pm)
{
    struct ifreq ifr;
    s32 sockfd;

    /* Open socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERR:Cannot open socket\n");
        return;
    }
    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) pm;
    /* Perform ioctl */
    errno = 0;
    if (ioctl(sockfd, UAP_POWER_MODE, &ifr)) {
        perror("");
        printf("ERR:UAP_POWER_MODE is not supported by %s\n", dev_name);
        return;
    }
    switch (pm->ps_mode) {
    case 0:
        printf("power mode = Disabled\n");
        break;
    case 1:
        printf("power mode = Periodic DTIM PS\n");
        break;
    case 2:
        printf("power mode = Inactivity based PS \n");
        break;
    }
    if (pm->flags & PS_FLAG_SLEEP_PARAM) {
        printf("Sleep param:\n");
        printf("\tctrl_bitmap=%d\n", (int) pm->sleep_param.ctrl_bitmap);
        printf("\tmin_sleep=%d us\n", (int) pm->sleep_param.min_sleep);
        printf("\tmax_sleep=%d us\n", (int) pm->sleep_param.max_sleep);
    }
    if (pm->flags & PS_FLAG_INACT_SLEEP_PARAM) {
        printf("Inactivity sleep param:\n");
        printf("\tinactivity_to=%d us\n", (int) pm->inact_param.inactivity_to);
        printf("\tmin_awake=%d us\n", (int) pm->inact_param.min_awake);
        printf("\tmax_awake=%d us\n", (int) pm->inact_param.max_awake);
    }
    /* Close socket */
    close(sockfd);
    return;
}

/** 
 *  @brief Creates power mode request and send to driver
 *   and sends to the driver
 *
 *   Usage: "Usage : powermode [MODE]"
 *
 *   Options: MODE :     0 - disable power mode
 *                       1 - enable power mode
 *            		 2 - get current power mode                         
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_power_mode(int argc, char *argv[])
{
    int opt;
    ps_mgmt pm;
    int type = 0;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_power_mode_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    memset(&pm, 0, sizeof(ps_mgmt));
    /* Check arguments */
    if ((argc > 9) ||
        ((argc != 0) && (argc != 1) && (argc != 5) && (argc != 9))) {
        printf("ERR:wrong arguments.\n");
        print_power_mode_usage();
        return;
    }

    if (argc) {
        if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
            (atoi(argv[0]) > 2)) {
            printf
                ("ERR:Illegal power mode %s. Must be either '0' '1' or '2'.\n",
                 argv[0]);
            print_power_mode_usage();
            return;
        }
        pm.flags = PS_FLAG_PS_MODE;
        pm.ps_mode = atoi(argv[0]);
        if ((pm.ps_mode == PS_MODE_DISABLE) && (argc > 1)) {
            printf("ERR: Illegal parameter for disable power mode\n");
            print_power_mode_usage();
            return;
        }
        if ((pm.ps_mode != PS_MODE_INACTIVITY) && (argc > 5)) {
            printf("ERR: Illegal parameter\n");
            print_power_mode_usage();
            return;
        }
        if (argc >= 5) {
            if ((ISDIGIT(argv[1]) == 0) || (atoi(argv[1]) < 1) ||
                (atoi(argv[1]) > 2)) {
                printf
                    ("ERR:Illegal parameter type %s. Must be either '1' or '2'.\n",
                     argv[1]);
                print_power_mode_usage();
                return;
            }
            type = atoi(argv[1]);
            if ((type == INACTIVITY_SLEEP_PARAMETER) &&
                (pm.ps_mode != PS_MODE_INACTIVITY)) {
                printf
                    ("ERR: inactivity sleep parameter only valid for inactivity power save mode\n");
                print_power_mode_usage();
                return;
            }
            if (type == SLEEP_PARAMETER) {
                if ((ISDIGIT(argv[2]) == 0) || (atoi(argv[2]) < 0) ||
                    (atoi(argv[2]) > 1)) {
                    printf
                        ("ERR:Illegal ctrl bitmap = %s. Must be either '0' or '1'.\n",
                         argv[2]);
                    print_power_mode_usage();
                    return;
                }
                pm.flags |= PS_FLAG_SLEEP_PARAM;
                pm.sleep_param.ctrl_bitmap = atoi(argv[2]);
                if ((ISDIGIT(argv[3]) == 0) || (ISDIGIT(argv[4]) == 0)) {
                    printf("ERR:Illegal parameter\n");
                    print_power_mode_usage();
                    return;
                }
                pm.sleep_param.min_sleep = atoi(argv[3]);
                pm.sleep_param.max_sleep = atoi(argv[4]);
                if (pm.sleep_param.min_sleep > pm.sleep_param.max_sleep) {
                    printf
                        ("ERR: MIN_SLEEP value should be less than or equal to MAX_SLEEP\n");
                    return;
                }
                if (pm.sleep_param.min_sleep < PS_SLEEP_PARAM_MIN ||
                    ((pm.sleep_param.max_sleep > PS_SLEEP_PARAM_MAX) &&
                     pm.sleep_param.ctrl_bitmap)) {
                    printf
                        ("ERR: Incorrect value of sleep period. Please check README\n");
                    return;
                }
            } else {
                if ((ISDIGIT(argv[2]) == 0) || (ISDIGIT(argv[3]) == 0) ||
                    (ISDIGIT(argv[4]) == 0)) {
                    printf("ERR:Illegal parameter\n");
                    print_power_mode_usage();
                    return;
                }
                pm.flags |= PS_FLAG_INACT_SLEEP_PARAM;
                pm.inact_param.inactivity_to = atoi(argv[2]);
                pm.inact_param.min_awake = atoi(argv[3]);
                pm.inact_param.max_awake = atoi(argv[4]);
                if (pm.inact_param.min_awake > pm.inact_param.max_awake) {
                    printf
                        ("ERR: MIN_AWAKE value should be less than or equal to MAX_AWAKE\n");
                    return;
                }
                if (pm.inact_param.min_awake < PS_AWAKE_PERIOD_MIN) {
                    printf("ERR: Incorrect value of MIN_AWAKE period.\n");
                    return;
                }
            }
        }
        if (argc == 9) {
            if ((ISDIGIT(argv[5]) == 0) || (atoi(argv[5]) < 1) ||
                (atoi(argv[5]) > 2)) {
                printf
                    ("ERR:Illegal parameter type %s. Must be either '1' or '2'.\n",
                     argv[5]);
                print_power_mode_usage();
                return;
            }
            if (type == atoi(argv[5])) {
                printf("ERR: Duplicate parameter type %s.\n", argv[5]);
                print_power_mode_usage();
                return;
            }
            type = atoi(argv[5]);
            if (type == SLEEP_PARAMETER) {
                if ((ISDIGIT(argv[6]) == 0) || (atoi(argv[6]) < 0) ||
                    (atoi(argv[6]) > 1)) {
                    printf
                        ("ERR:Illegal ctrl bitmap = %s. Must be either '0' or '1'.\n",
                         argv[6]);
                    print_power_mode_usage();
                    return;
                }
                pm.flags |= PS_FLAG_SLEEP_PARAM;
                pm.sleep_param.ctrl_bitmap = atoi(argv[6]);
                if ((ISDIGIT(argv[7]) == 0) || (ISDIGIT(argv[8]) == 0)) {
                    printf("ERR:Illegal parameter\n");
                    print_power_mode_usage();
                    return;
                }
                pm.sleep_param.min_sleep = atoi(argv[7]);
                pm.sleep_param.max_sleep = atoi(argv[8]);
                if (pm.sleep_param.min_sleep > pm.sleep_param.max_sleep) {
                    printf
                        ("ERR: MIN_SLEEP value should be less than or equal to MAX_SLEEP\n");
                    return;
                }
                if (pm.sleep_param.min_sleep < PS_SLEEP_PARAM_MIN ||
                    ((pm.sleep_param.max_sleep > PS_SLEEP_PARAM_MAX) &&
                     pm.sleep_param.ctrl_bitmap)) {
                    printf
                        ("ERR: Incorrect value of sleep period. Please check README\n");
                    return;
                }
            } else {
                if ((ISDIGIT(argv[6]) == 0) || (ISDIGIT(argv[7]) == 0) ||
                    (ISDIGIT(argv[8]) == 0)) {
                    printf("ERR:Illegal parameter\n");
                    print_power_mode_usage();
                    return;
                }
                pm.flags |= PS_FLAG_INACT_SLEEP_PARAM;
                pm.inact_param.inactivity_to = atoi(argv[6]);
                pm.inact_param.min_awake = atoi(argv[7]);
                pm.inact_param.max_awake = atoi(argv[8]);
                if (pm.inact_param.min_awake > pm.inact_param.max_awake) {
                    printf
                        ("ERR: MIN_AWAKE value should be less than or equal to MAX_AWAKE\n");
                    return;
                }
                if (pm.inact_param.min_awake < PS_AWAKE_PERIOD_MIN) {
                    printf("ERR: Incorrect value of MIN_AWAKE period.\n");
                    return;
                }
            }
        }
    }
    send_power_mode_ioctl(&pm);
    return;
}

/**
 *  @brief Show usage information for the sys_reset command
 *
 *  $return         N/A
 */
void
print_sys_reset_usage(void)
{
    printf("\nUsage : sys_reset\n");
    return;
}

/** 
 *  @brief Creates a sys_reset request and sends to the driver
 *
 *  Usage: "sys_reset"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_reset(int argc, char *argv[])
{
    APCMDBUF_SYS_RESET *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;
    ps_mgmt pm;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_reset_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_sys_reset_usage();
        return;
    }

    memset(&pm, 0, sizeof(ps_mgmt));
    pm.flags = PS_FLAG_PS_MODE;
    pm.ps_mode = PS_MODE_DISABLE;
    send_power_mode_ioctl(&pm);

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_SYS_RESET);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_RESET *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_RESET;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_RESET | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("System reset successful!\n");
        } else {
            printf("ERR:Could not reset system!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief Show usage information for the bss_start command
 *
 *  $return         N/A
 */
void
print_bss_start_usage(void)
{
    printf("\nUsage : bss_start\n");
    return;
}

/** 
 *  @brief Creates a BSS start request and sends to the driver
 *
 *   Usage: "bss_start"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_bss_start(int argc, char *argv[])
{
    APCMDBUF_BSS_START *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_bss_start_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_bss_start_usage();
        return;
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_BSS_START);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_BSS_START *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_BSS_START;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_BSS_START | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("BSS started!\n");
        } else if (cmd_buf->Result == BSS_FAILURE_START_INVAL) {
            printf("ERR:Could not start BSS! Invalid BSS parameters.\n");
        } else if (cmd_buf->Result == BSS_FAILURE_START_REDUNDANT) {
            printf("ERR:Could not start BSS! BSS already started.\n");
        } else {
            printf("ERR:Could not start BSS!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief Show usage information for the bss_stop command
 *
 *  $return         N/A
 */
void
print_bss_stop_usage(void)
{
    printf("\nUsage : bss_stop\n");
    return;
}

/** 
 *  @brief Creates a BSS stop request and sends to the driver
 *
 *   Usage: "bss_stop"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_bss_stop(int argc, char *argv[])
{
    APCMDBUF_BSS_STOP *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_bss_stop_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;             /* Check arguments */

    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_bss_stop_usage();
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_BSS_STOP);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);

    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_BSS_STOP *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_BSS_STOP;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_BSS_STOP | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("BSS stopped!\n");
        } else if (cmd_buf->Result == BSS_FAILURE_STOP_REDUNDANT) {
            printf("ERR:Could not stop BSS! BSS already stopped.\n");
        } else if (cmd_buf->Result == BSS_FAILURE_STOP_INVAL) {
            printf("ERR:Could not stop BSS! No active BSS.\n");
        } else {
            printf("ERR:Could not stop BSS!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief Show usage information for the sta_list command
 *
 *  $return         N/A
 */
void
print_sta_list_usage(void)
{
    printf("\nUsage : sta_list\n");
    return;
}

/** 
 *  @brief Creates a STA list request and sends to the driver
 *
 *   Usage: "sta_list"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sta_list(int argc, char *argv[])
{
    APCMDBUF_STA_LIST_REQUEST *cmd_buf = NULL;
    APCMDBUF_STA_LIST_RESPONSE *response_buf = NULL;
    u8 *buf = NULL;
    u16 buf_len;
    TLVBUF_STA_INFO *tlv = NULL;
    u16 cmd_len;
    u16 response_len;
    int ret = UAP_FAILURE;
    int i = 0;
    int opt;
    int rssi = 0;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sta_list_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 0) {
        printf("ERR:Too many arguments.\n");
        print_sta_list_usage();
        return;
    }
    response_len =
        sizeof(APCMDBUF_STA_LIST_RESPONSE) +
        (MAX_NUM_CLIENTS * sizeof(TLVBUF_STA_INFO));
    if (response_len > sizeof(APCMDBUF_STA_LIST_REQUEST))
        buf_len = response_len;
    else
        buf_len = sizeof(APCMDBUF_STA_LIST_REQUEST);

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_STA_LIST_REQUEST);

    /* Initialize the command buffer */
    buf = (u8 *) malloc(buf_len);

    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_STA_LIST_REQUEST *) buf;
    response_buf = (APCMDBUF_STA_LIST_RESPONSE *) buf;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_STA_LIST;
    cmd_buf->Size = cmd_len;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    response_buf->StaCount = uap_le16_to_cpu(response_buf->StaCount);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (response_buf->CmdCode != (APCMD_STA_LIST | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buf);
            return;
        }

        /* Print response */
        if (response_buf->Result == CMD_SUCCESS) {
            printf("Number of STA = %d\n\n", response_buf->StaCount);
            for (i = 0; i < response_buf->StaCount; i++) {
                tlv = (TLVBUF_STA_INFO *) (&response_buf->StaList[i]);
                endian_convert_tlv_header_in(tlv);
                if (tlv) {
                    if (tlv->Tag != MRVL_STA_INFO_TLV_ID) {
                        printf("STA %d information corrupted.\n", i + 1);
                        continue;
                    }
                    printf("STA %d information:\n", i + 1);
                    printf("=====================\n");
                    printf("MAC Address: ");
                    print_mac(tlv->MacAddress);
                    printf("\nPower mfg status: %s\n",
                           (tlv->PowerMfgStatus ==
                            0) ? "active" : "power save");
                                        /** On some platform, s8 is same as unsigned char*/
                    rssi = (int) tlv->Rssi;
                    if (rssi > 0x7f)
                        rssi = -(256 - rssi);
                    printf("Rssi : %d dBm\n\n", rssi);
                } else {
                    printf("ERR:Unable to find information for STA %d\n\n",
                           i + 1);
                }
            }
        } else {
            printf("ERR:Could not get STA list!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
    return;
}

/**
 *  @brief Show usage information for the sta_deauth command
 *
 *  $return         N/A
 */
void
print_sta_deauth_usage(void)
{
    printf("\nUsage : sta_deauth <STA_MAC_ADDRESS> [REASON_CODE]\n");
    return;
}

/** 
 *  @brief Creates a STA deauth request and sends to the driver
 *
 *   Usage: "sta_deauth <STA_MAC_ADDRESS>"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sta_deauth(int argc, char *argv[])
{
    APCMDBUF_STA_DEAUTH *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sta_deauth_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc != 1 && argc != 2) {
        printf("ERR:wrong arguments! Must provide STA_MAC_ADDRESS.\n");
        printf("\t\t with optional REASON_CODE.\n");
        print_sta_deauth_usage();
        return;
    }

    /* Check Reason Code */
    if (argc == 2) {
        if (IS_HEX_OR_DIGIT(argv[1]) == UAP_FAILURE) {
            printf("ERR: Invalid input for reason code\n");
            print_sta_deauth_usage();
            return;
        }
    }

    /* Initialize the command length */
    cmd_len = sizeof(APCMDBUF_STA_DEAUTH);

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(cmd_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, cmd_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_STA_DEAUTH *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_STA_DEAUTH;
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    if ((ret = mac2raw(argv[0], cmd_buf->StaMacAddress)) != UAP_SUCCESS) {
        printf("ERR: %s Address\n", ret == UAP_FAILURE ? "Invalid MAC" :
               ret == UAP_RET_MAC_BROADCAST ? "Broadcast" : "Multicast");
        free(buffer);
        return;
    }
    if (argc == 2) {
        cmd_buf->ReasonCode = uap_cpu_to_le16((u16) A2HEXDECIMAL(argv[1]));
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_STA_DEAUTH | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buffer);
            return;
        }

        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("Deauthentication successful!\n");
        } else {
            printf("ERR:Deauthentication unsuccessful!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/**
 *  @brief Show usage information for the coex_config command
 *
 *  $return         N/A
 */
void
print_coex_config_usage(void)
{
    printf("\nUsage : coex_config [CONFIG_FILE]\n");
    printf
        ("\nIf CONFIG_FILE is provided, a 'set' is performed, else a 'get' is performed.\n");
    return;
}

/** 
 *  @brief Creates a coex_config request and sends to the driver
 *
 *  Usage: "Usage : coex_config [CONFIG_FILE]"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
void
apcmd_coex_config(int argc, char *argv[])
{
    apcmdbuf_coex_config *cmd_buf = NULL;
    tlvbuf_coex_common_cfg *coex_common_tlv;
    tlvbuf_coex_sco_cfg *coex_sco_tlv;
    tlvbuf_coex_acl_cfg *coex_acl_tlv;
    tlvbuf_coex_stats *coex_stats_tlv;
    u8 *buf = NULL;
    u16 cmd_len;
    int ret = UAP_FAILURE;
    int opt;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_coex_config_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 1) {
        printf("ERR:Too many arguments.\n");
        print_coex_config_usage();
        return;
    }
    if (argc == 1) {
        /* Read profile and send command to firmware */
        apcmd_coex_config_profile(argc, argv);
        return;
    }

    /* fixed command length */
    cmd_len = sizeof(apcmdbuf_coex_config) + sizeof(tlvbuf_coex_common_cfg)
        + sizeof(tlvbuf_coex_sco_cfg) + sizeof(tlvbuf_coex_acl_cfg)
        + sizeof(tlvbuf_coex_stats);
    /* alloc buf for command */
    buf = (u8 *) malloc(cmd_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, cmd_len);

    cmd_buf = (apcmdbuf_coex_config *) buf;

    coex_common_tlv = (tlvbuf_coex_common_cfg *) cmd_buf->tlv_buffer;
    coex_common_tlv->Tag = MRVL_BT_COEX_COMMON_CFG_TLV_ID;
    coex_common_tlv->Length =
        sizeof(tlvbuf_coex_common_cfg) - sizeof(TLVBUF_HEADER);
    endian_convert_tlv_header_out(coex_common_tlv);

    coex_sco_tlv = (tlvbuf_coex_sco_cfg *) (cmd_buf->tlv_buffer +
                                            sizeof(tlvbuf_coex_common_cfg));
    coex_sco_tlv->Tag = MRVL_BT_COEX_SCO_CFG_TLV_ID;
    coex_sco_tlv->Length = sizeof(tlvbuf_coex_sco_cfg) - sizeof(TLVBUF_HEADER);
    endian_convert_tlv_header_out(coex_sco_tlv);

    coex_acl_tlv = (tlvbuf_coex_acl_cfg *) (cmd_buf->tlv_buffer +
                                            sizeof(tlvbuf_coex_common_cfg) +
                                            sizeof(tlvbuf_coex_sco_cfg));
    coex_acl_tlv->Tag = MRVL_BT_COEX_ACL_CFG_TLV_ID;
    coex_acl_tlv->Length = sizeof(tlvbuf_coex_acl_cfg) - sizeof(TLVBUF_HEADER);
    endian_convert_tlv_header_out(coex_acl_tlv);

    coex_stats_tlv = (tlvbuf_coex_stats *) (cmd_buf->tlv_buffer +
                                            sizeof(tlvbuf_coex_common_cfg) +
                                            sizeof(tlvbuf_coex_sco_cfg)
                                            + sizeof(tlvbuf_coex_acl_cfg));
    coex_stats_tlv->Tag = MRVL_BT_COEX_STATS_TLV_ID;
    coex_stats_tlv->Length = sizeof(tlvbuf_coex_stats) - sizeof(TLVBUF_HEADER);
    endian_convert_tlv_header_out(coex_stats_tlv);

    /* Fill the command buffer */
    cmd_buf->CmdCode = HostCmd_ROBUST_COEX;
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->action = uap_cpu_to_le16(ACTION_GET);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (HostCmd_ROBUST_COEX | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buf);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("BT Coex settings:\n");
            print_tlv(buf + sizeof(apcmdbuf_coex_config),
                      cmd_buf->Size - sizeof(apcmdbuf_coex_config) +
                      BUF_HEADER_SIZE);
        } else {
            printf("ERR:Could not retrieve coex configuration.\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
    return;
}

/**
 *  @brief Show usage information for the sys_config command
 *
 *  $return         N/A
 */
void
print_sys_config_usage(void)
{
    printf("\nUsage : sys_config [CONFIG_FILE]\n");
    printf
        ("\nIf CONFIG_FILE is provided, a 'set' is performed, else a 'get' is performed.\n");
    printf("CONFIG_FILE is file contain all the Micro AP settings.\n");
    return;
}

/**
 *  @brief Show usage information for the rdeeprom command
 *
 *  $return         N/A
 */
void
print_apcmd_read_eeprom_usage(void)
{
    printf("\nUsage: rdeeprom <offset> <bytecount>\n");
    printf("    offset    : 0,4,8,..., multiple of 4\n");
    printf("    bytecount : 4-20, multiple of 4\n");
    return;
}

/**
 *  @brief Show protocol tlv 
 *
 *  @param tlv     Poniter to protocol tlv
 *  
 *  $return         N/A
 */
void
print_protocol(TLVBUF_PROTOCOL * tlv)
{
    switch (tlv->Protocol) {
    case 0:
    case PROTOCOL_NO_SECURITY:
        printf("PROTOCOL = No security\n");
        break;
    case PROTOCOL_STATIC_WEP:
        printf("PROTOCOL = Static WEP\n");
        break;
    case PROTOCOL_WPA:
        printf("PROTOCOL = WPA \n");
        break;
    case PROTOCOL_WPA2:
        printf("PROTOCOL = WPA2 \n");
        break;
    case PROTOCOL_WPA | PROTOCOL_WPA2:
        printf("PROTOCOL = WPA/WPA2 \n");
        break;
    default:
        printf("Unknown PROTOCOL: 0x%x \n", tlv->Protocol);
        break;
    }
}

/**
 *  @brief Show wep tlv 
 *
 *  @param tlv     Poniter to wep tlv
 *  
 *  $return         N/A
 */
void
print_wep_key(TLVBUF_WEP_KEY * tlv)
{
    int i;
    if (tlv->Length <= 2) {
        printf("wrong wep_key tlv: length=%d\n", tlv->Length);
        return;
    }
    printf("WEP KEY_%d = ", tlv->KeyIndex);
    for (i = 0; i < tlv->Length - 2; i++)
        printf("%02x ", tlv->Key[i]);
    if (tlv->IsDefault)
        printf("\nDefault WEP Key = %d\n", tlv->KeyIndex);
    else
        printf("\n");
}

/** 
 *  @brief Parses a command line
 *
 *  @param line     The line to parse
 *  @param args     Pointer to the argument buffer to be filled in
 *  @return         Number of arguments in the line or EOF
 */
static int
parse_line(char *line, char *args[])
{
    int arg_num = 0;
    int is_start = 0;
    int is_quote = 0;
    int length = 0;
    int i = 0;

    arg_num = 0;
    length = strlen(line);
    /* Process line */

    /* Find number of arguments */
    is_start = 0;
    is_quote = 0;
    for (i = 0; i < length; i++) {
        /* Ignore leading spaces */
        if (is_start == 0) {
            if (line[i] == ' ') {
                continue;
            } else if (line[i] == '\t') {
                continue;
            } else if (line[i] == '\n') {
                break;
            } else {
                is_start = 1;
                args[arg_num] = &line[i];
                arg_num++;
            }
        }
        if (is_start == 1) {
            /* Ignore comments */
            if (line[i] == '#') {
                line[i] = '\0';
                arg_num--;
                break;
            }
            /* Separate by '=' */
            if (line[i] == '=') {
                line[i] = '\0';
                is_start = 0;
                continue;
            }
            /* Separate by ',' */
            if (line[i] == ',') {
                line[i] = '\0';
                is_start = 0;
                continue;
            }
            /* Change ',' to ' ', but not inside quotes */
            if ((line[i] == ',') && (is_quote == 0)) {
                line[i] = ' ';
                continue;
            }
        }
        /* Remove newlines */
        if (line[i] == '\n') {
            line[i] = '\0';
        }
        /* Check for quotes */
        if (line[i] == '"') {
            is_quote = (is_quote == 1) ? 0 : 1;
            continue;
        }
        if (((line[i] == ' ') || (line[i] == '\t')) && (is_quote == 0)) {
            line[i] = '\0';
            is_start = 0;
            continue;
        }
    }
    return arg_num;
}

/** 
 *  @brief      Parse function for a configuration line  
 *
 *  @param s        Storage buffer for data
 *  @param size     Maximum size of data
 *  @param stream   File stream pointer
 *  @param line     Pointer to current line within the file
 *  @param _pos     Output string or NULL
 *  @return     String or NULL
 */
static char *
config_get_line(char *s, int size, FILE * stream, int *line, char **_pos)
{
    char *pos, *end, *sstart;
    while (fgets(s, size, stream)) {
        (*line)++;
        s[size - 1] = '\0';
        pos = s;
        /* Skip white space from the beginning of line. */
        while (*pos == ' ' || *pos == '\t' || *pos == '\r')
            pos++;
        /* Skip comment lines and empty lines */
        if (*pos == '#' || *pos == '\n' || *pos == '\0')
            continue;
        /* 
         * Remove # comments unless they are within a double quoted
         * string.
         */
        sstart = strchr(pos, '"');
        if (sstart)
            sstart = strrchr(sstart + 1, '"');
        if (!sstart)
            sstart = pos;
        end = strchr(sstart, '#');
        if (end)
            *end-- = '\0';
        else
            end = pos + strlen(pos) - 1;
        /* Remove trailing white space. */
        while (end > pos &&
               (*end == '\n' || *end == ' ' || *end == '\t' || *end == '\r'))
            *end-- = '\0';
        if (*pos == '\0')
            continue;
        if (_pos)
            *_pos = pos;
        return pos;
    }

    if (_pos)
        *_pos = NULL;
    return NULL;
}

/** 
 *  @brief Read the profile and sends to the driver
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
void
apcmd_coex_config_profile(int argc, char *argv[])
{
    FILE *config_file = NULL;
    char *line = NULL;
    int i, ret, index, li = 0;
    char *pos = NULL;
    int arg_num = 0;
    char *args[30];
    int is_coex_config = 0;
    int is_coex_common_config = 0;
    int is_coex_sco_config = 0;
    int is_coex_acl_config = 0;
    u8 *buf = NULL;
    apcmdbuf_coex_config *cmd_buf = NULL;
    tlvbuf_coex_common_cfg *coex_common_tlv;
    tlvbuf_coex_sco_cfg *coex_sco_tlv;
    tlvbuf_coex_acl_cfg *coex_acl_tlv;
    u16 acl_enabled = 0;
    u32 conf_bitmap = 0;
    u16 cmd_len = 0, tlv_len = 0;
    u16 sco_prot_qtime[4] = { 0, 0, 0, 0 }, sco_prot_rate = 0, sco_acl_freq = 0;
    u16 acl_bt_time = 0, acl_wlan_time = 0, acl_prot_rate = 0;

    /* Check if file exists */
    config_file = fopen(argv[0], "r");
    if (config_file == NULL) {
        printf("\nERR:Config file can not open.\n");
        return;
    }
    line = (char *) malloc(MAX_CONFIG_LINE);
    if (!line) {
        printf("ERR:Cannot allocate memory for line\n");
        goto done;
    }
    bzero(line, MAX_CONFIG_LINE);

    /* fixed command length */
    cmd_len = sizeof(apcmdbuf_coex_config) + sizeof(tlvbuf_coex_common_cfg)
        + sizeof(tlvbuf_coex_sco_cfg) + sizeof(tlvbuf_coex_acl_cfg);
    /* alloc buf for command */
    buf = (u8 *) malloc(cmd_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        goto done;
    }
    bzero((char *) buf, cmd_len);

    cmd_buf = (apcmdbuf_coex_config *) buf;

    /* Fill the command buffer */
    cmd_buf->CmdCode = HostCmd_ROBUST_COEX;
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->action = uap_cpu_to_le16(ACTION_SET);

    /* Parse file and process */
    while (config_get_line(line, MAX_CONFIG_LINE, config_file, &li, &pos)) {
#if DEBUG
        uap_printf(MSG_DEBUG, "DBG:Received config line (%d) = %s\n", li, line);
#endif
        arg_num = parse_line(line, args);
#if DEBUG
        uap_printf(MSG_DEBUG, "DBG:Number of arguments = %d\n", arg_num);
        for (i = 0; i < arg_num; i++) {
            uap_printf(MSG_DEBUG, "\tDBG:Argument %d. %s\n", i + 1, args[i]);
        }
#endif
        /* Check for end of Coex configurations */
        if (is_coex_acl_config == 1) {
            if (strcmp(args[0], "}") == 0) {
                coex_acl_tlv =
                    (tlvbuf_coex_acl_cfg *) (cmd_buf->tlv_buffer + tlv_len);
                coex_acl_tlv->Tag = MRVL_BT_COEX_ACL_CFG_TLV_ID;
                coex_acl_tlv->Length =
                    sizeof(tlvbuf_coex_acl_cfg) - sizeof(TLVBUF_HEADER);
                endian_convert_tlv_header_out(coex_acl_tlv);
                coex_acl_tlv->enabled = uap_cpu_to_le16(acl_enabled);
                coex_acl_tlv->bt_time = uap_cpu_to_le16(acl_bt_time);
                coex_acl_tlv->wlan_time = uap_cpu_to_le16(acl_wlan_time);
                coex_acl_tlv->protection_rate = uap_cpu_to_le16(acl_prot_rate);
                tlv_len += sizeof(tlvbuf_coex_acl_cfg);
                is_coex_acl_config = 0;
            }
        } else if (is_coex_sco_config == 1) {
            if (strcmp(args[0], "}") == 0) {
                coex_sco_tlv =
                    (tlvbuf_coex_sco_cfg *) (cmd_buf->tlv_buffer + tlv_len);
                coex_sco_tlv->Tag = MRVL_BT_COEX_SCO_CFG_TLV_ID;
                coex_sco_tlv->Length =
                    sizeof(tlvbuf_coex_sco_cfg) - sizeof(TLVBUF_HEADER);
                endian_convert_tlv_header_out(coex_sco_tlv);
                for (i = 0; i < 4; i++)
                    coex_sco_tlv->protection_qtime[i] =
                        uap_cpu_to_le16(sco_prot_qtime[i]);
                coex_sco_tlv->protection_rate = uap_cpu_to_le16(sco_prot_rate);
                coex_sco_tlv->acl_frequency = uap_cpu_to_le16(sco_acl_freq);
                tlv_len += sizeof(tlvbuf_coex_sco_cfg);
                is_coex_sco_config = 0;
            }
        } else if (is_coex_common_config == 1) {
            if (strcmp(args[0], "}") == 0) {
                coex_common_tlv =
                    (tlvbuf_coex_common_cfg *) (cmd_buf->tlv_buffer + tlv_len);
                coex_common_tlv->Tag = MRVL_BT_COEX_COMMON_CFG_TLV_ID;
                coex_common_tlv->Length =
                    sizeof(tlvbuf_coex_common_cfg) - sizeof(TLVBUF_HEADER);
                endian_convert_tlv_header_out(coex_common_tlv);
                coex_common_tlv->config_bitmap = uap_cpu_to_le32(conf_bitmap);
                tlv_len += sizeof(tlvbuf_coex_common_cfg);
                is_coex_common_config = 0;
            }
        } else if (is_coex_config == 1) {
            if (strcmp(args[0], "}") == 0)
                is_coex_config = 0;
        }
        if (strcmp(args[0], "coex_config") == 0) {
            is_coex_config = 1;
        } else if (strcmp(args[0], "common_config") == 0) {
            is_coex_common_config = 1;
        } else if (strcmp(args[0], "sco_config") == 0) {
            is_coex_sco_config = 1;
        } else if (strcmp(args[0], "acl_config") == 0) {
            is_coex_acl_config = 1;
        }
        if ((strcmp(args[0], "bitmap") == 0) && is_coex_common_config) {
            if (is_input_valid(COEX_COMM_BITMAP, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            conf_bitmap = (u32) A2HEXDECIMAL(args[1]);
        } else if ((strncmp(args[0], "protectionFromQTime", 19) == 0) &&
                   is_coex_sco_config) {
            index = atoi(args[0] + strlen("protectionFromQTime"));
            if (index < 0 || index > 3) {
                printf("ERR:Incorrect index %d.\n", index);
                goto done;
            }
            if (is_input_valid(COEX_PROTECTION, arg_num, args) != UAP_SUCCESS) {
                goto done;
            }
            sco_prot_qtime[index] = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "scoProtectionFromRate") == 0) &&
                   is_coex_sco_config) {
            if (is_input_valid(COEX_PROTECTION, arg_num, args) != UAP_SUCCESS) {
                goto done;
            }
            sco_prot_rate = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "aclFrequency") == 0) && is_coex_sco_config) {
            if (is_input_valid(COEX_SCO_ACL_FREQ, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            sco_acl_freq = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "enabled") == 0) && is_coex_acl_config) {
            if (is_input_valid(COEX_ACL_ENABLED, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            acl_enabled = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "btTime") == 0) && is_coex_acl_config) {
            if (is_input_valid(COEX_ACL_BT_TIME, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            acl_bt_time = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "wlanTime") == 0) && is_coex_acl_config) {
            if (is_input_valid(COEX_ACL_WLAN_TIME, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            acl_wlan_time = (u16) atoi(args[1]);
        } else if ((strcmp(args[0], "aclProtectionFromRate") == 0) &&
                   is_coex_acl_config) {
            if (is_input_valid(COEX_PROTECTION, arg_num, args) != UAP_SUCCESS) {
                goto done;
            }
            acl_prot_rate = (u16) atoi(args[1]);
        }
    }
    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (HostCmd_ROBUST_COEX | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            goto done;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("BT Coex settings sucessfully set.\n");
        } else {
            printf("ERR:Could not set coex configuration.\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
  done:
    fclose(config_file);
    if (buf)
        free(buf);
    if (line)
        free(line);
}

/** 
 *  @brief Read the profile and sends to the driver
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
void
apcmd_sys_config_profile(int argc, char *argv[])
{
    FILE *config_file = NULL;
    char *line = NULL;
    int li = 0;
    char *pos = NULL;
    int arg_num = 0;
    char *args[30];
    int i;
    int is_ap_config = 0;
    int is_custom_ie_config = 0;
    int is_ap_mac_filter = 0;
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len = 0;
    u16 tlv_len = 0;
    u16 ie_len = 0;
    u16 ie_buf_len = 0;
    u16 mask_ie_index = 0;
    int keyindex = -1;
    int pairwisecipher = -1;
    int groupcipher = -1;
    TLVBUF_STA_MAC_ADDR_FILTER *filter_tlv = NULL;
    tlvbuf_custom_ie *custom_ie_tlv_head = NULL;
    tlvbuf_custom_ie *custom_ie_tlv = NULL;
    custom_ie *custom_ie_ptr = NULL;
    int custom_ie_tlv_len = 0;
    int custom_mask_count = 0;
    int custom_buf_count = 0;
    int filter_mac_count = -1;
    int tx_data_rate = -1;
    int mcbc_data_rate = -1;
    u8 rate[MAX_RATES];
    int found = 0;
    char country_80211d[4];
    u8 state_80211d;
    int flag_80211d = 0;

    memset(rate, 0, MAX_RATES);
    /* Check if file exists */
    config_file = fopen(argv[0], "r");
    if (config_file == NULL) {
        printf("\nERR:Config file can not open.\n");
        return;
    }
    line = (char *) malloc(MAX_CONFIG_LINE);
    if (!line) {
        printf("ERR:Cannot allocate memory for line\n");
        goto done;
    }
    bzero(line, MAX_CONFIG_LINE);

    /* Parse file and process */
    while (config_get_line(line, MAX_CONFIG_LINE, config_file, &li, &pos)) {
#if DEBUG
        uap_printf(MSG_DEBUG, "DBG:Received config line (%d) = %s\n", li, line);
#endif
        arg_num = parse_line(line, args);
#if DEBUG
        uap_printf(MSG_DEBUG, "DBG:Number of arguments = %d\n", arg_num);
        for (i = 0; i < arg_num; i++) {
            uap_printf(MSG_DEBUG, "\tDBG:Argument %d. %s\n", i + 1, args[i]);
        }
#endif
        /* Check for end of AP configurations */
        if (is_ap_config == 1) {
            if (strcmp(args[0], "}") == 0) {
                is_ap_config = 0;
                if (tx_data_rate != -1) {
                    if ((!rate[0]) && (tx_data_rate) &&
                        (is_tx_rate_valid((u8) tx_data_rate) != UAP_SUCCESS)) {
                        printf("ERR: Invalid Tx Data Rate \n");
                        goto done;
                    }
                    if (rate[0] && tx_data_rate) {
                        for (i = 0; rate[i] != 0; i++) {
                            if ((rate[i] & ~BASIC_RATE_SET_BIT) == tx_data_rate) {
                                found = 1;
                                break;
                            }
                        }
                        if (!found) {
                            printf("ERR: Invalid Tx Data Rate \n");
                            goto done;
                        }
                    }

                    /* Append a new TLV */
                    TLVBUF_TX_DATA_RATE *tlv = NULL;
                    tlv_len = sizeof(TLVBUF_TX_DATA_RATE);
                    buffer = realloc(buffer, cmd_len + tlv_len);
                    if (!buffer) {
                        printf("ERR:Cannot append tx data rate TLV!\n");
                        goto done;
                    }
                    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                    tlv = (TLVBUF_TX_DATA_RATE *) (buffer + cmd_len);
                    cmd_len += tlv_len;
                    /* Set TLV fields */
                    tlv->Tag = MRVL_TX_DATA_RATE_TLV_ID;
                    tlv->Length = 2;
                    tlv->TxDataRate = tx_data_rate;
                    endian_convert_tlv_header_out(tlv);
                    tlv->TxDataRate = uap_cpu_to_le16(tlv->TxDataRate);
                }
                if (mcbc_data_rate != -1) {
                    if ((!rate[0]) && (mcbc_data_rate) &&
                        (is_mcbc_rate_valid((u8) mcbc_data_rate) !=
                         UAP_SUCCESS)) {
                        printf("ERR: Invalid Tx Data Rate \n");
                        goto done;
                    }
                    if (rate[0] && mcbc_data_rate) {
                        for (i = 0; rate[i] != 0; i++) {
                            if (rate[i] & BASIC_RATE_SET_BIT) {
                                if ((rate[i] & ~BASIC_RATE_SET_BIT) ==
                                    mcbc_data_rate) {
                                    found = 1;
                                    break;
                                }
                            }
                        }
                        if (!found) {
                            printf("ERR: Invalid MCBC Data Rate \n");
                            goto done;
                        }
                    }

                    /* Append a new TLV */
                    TLVBUF_MCBC_DATA_RATE *tlv = NULL;
                    tlv_len = sizeof(TLVBUF_MCBC_DATA_RATE);
                    buffer = realloc(buffer, cmd_len + tlv_len);
                    if (!buffer) {
                        printf("ERR:Cannot append tx data rate TLV!\n");
                        goto done;
                    }
                    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                    tlv = (TLVBUF_MCBC_DATA_RATE *) (buffer + cmd_len);
                    cmd_len += tlv_len;
                    /* Set TLV fields */
                    tlv->Tag = MRVL_MCBC_DATA_RATE_TLV_ID;
                    tlv->Length = 2;
                    tlv->MCBCdatarate = mcbc_data_rate;
                    endian_convert_tlv_header_out(tlv);
                    tlv->MCBCdatarate = uap_cpu_to_le16(tlv->MCBCdatarate);
                }

                if ((pairwisecipher >= 0) && (groupcipher >= 0)) {
                    if (is_cipher_valid(pairwisecipher, groupcipher) !=
                        UAP_SUCCESS) {
                        printf
                            ("ERR:Wrong group and pair cipher combination!\n");
                        goto done;
                    }
                    TLVBUF_CIPHER *tlv = NULL;
                    /* Append a new TLV */
                    tlv_len = sizeof(TLVBUF_CIPHER);
                    buffer = realloc(buffer, cmd_len + tlv_len);
                    if (!buffer) {
                        printf("ERR:Cannot append cipher TLV!\n");
                        goto done;
                    }
                    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                    tlv = (TLVBUF_CIPHER *) (buffer + cmd_len);
                    bzero((char *) tlv, tlv_len);
                    cmd_len += tlv_len;
                    /* Set TLV fields */
                    tlv->Tag = MRVL_CIPHER_TLV_ID;
                    tlv->Length = 2;
                    tlv->PairwiseCipher = pairwisecipher;
                    tlv->GroupCipher = groupcipher;
                    endian_convert_tlv_header_out(tlv);
                }
                cmd_buf->Size = cmd_len;
                /* Send collective command */
                uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
                cmd_len = 0;
                if (buffer) {
                    free(buffer);
                    buffer = NULL;
                }
                continue;
            }
        }

        /* Check for beginning of AP configurations */
        if (strcmp(args[0], "ap_config") == 0) {
            is_ap_config = 1;
            cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE);
            if (buffer) {
                free(buffer);
                buffer = NULL;
            }
            buffer = (u8 *) malloc(cmd_len);
            if (!buffer) {
                printf("ERR:Cannot allocate memory!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
            cmd_buf->Size = cmd_len;
            cmd_buf->SeqNum = 0;
            cmd_buf->Result = 0;
            cmd_buf->Action = ACTION_SET;
            continue;
        }

        /* Check for end of AP MAC address filter configurations */
        if (is_ap_mac_filter == 1) {
            if (strcmp(args[0], "}") == 0) {
                is_ap_mac_filter = 0;
                if (filter_tlv->Count != filter_mac_count) {
                    printf
                        ("ERR:Number of MAC address provided does not match 'Count'\n");
                    goto done;
                }
                if (filter_tlv->FilterMode && (filter_tlv->Count == 0)) {
                    printf
                        ("ERR:Filter list can not be empty for %s Filter mode\n",
                         (filter_tlv->FilterMode == 1) ? "'Allow'" : "'Block'");
                    goto done;
                }
                filter_tlv->Length = (filter_tlv->Count * ETH_ALEN) + 2;
                cmd_len -=
                    (MAX_MAC_ONESHOT_FILTER - filter_mac_count) * ETH_ALEN;
                cmd_buf->Size = cmd_len;
                endian_convert_tlv_header_out(filter_tlv);
                uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
                cmd_len = 0;
                if (buffer) {
                    free(buffer);
                    buffer = NULL;
                }
                continue;
            }
        }

        /* Check for end of custom IE configurations */
        if (is_custom_ie_config == 1) {
            if (strcmp(args[0], "}") == 0) {
                if (custom_mask_count != custom_buf_count) {
                    printf
                        ("ERR:custom IE mask count and buffer count does not match\n");
                    goto done;
                }
                is_custom_ie_config = 0;
                custom_ie_tlv_head->Length = custom_ie_tlv_len;
                cmd_len -=
                    (MAX_IE_BUFFER_LEN * MAX_CUSTOM_IE_COUNT) -
                    custom_ie_tlv_len;
                cmd_len -= sizeof(custom_ie) * MAX_CUSTOM_IE_COUNT;
                cmd_buf->Size = cmd_len;
                endian_convert_tlv_header_out(custom_ie_tlv_head);
                uap_ioctl((u8 *) cmd_buf, &cmd_len, cmd_len);
                cmd_len = 0;
                if (buffer) {
                    free(buffer);
                    buffer = NULL;
                }
                continue;
            }
        }

        if (flag_80211d && (strcmp(args[0], "11d_enable") == 0)) {
            if (IS_HEX_OR_DIGIT(args[1]) == UAP_FAILURE) {
                printf("ERR: valid input for state are 0 or 1\n");
                goto done;
            }
            state_80211d = (u8) A2HEXDECIMAL(args[1]);

            if ((state_80211d != 0) && (state_80211d != 1)) {
                printf("ERR: valid input for state are 0 or 1 \n");
                goto done;
            }
            if (sg_snmp_mib
                (ACTION_SET, OID_80211D_ENABLE, sizeof(state_80211d),
                 &state_80211d)
                == UAP_FAILURE) {
                goto done;
            }
        }

        if (strcmp(args[0], "country") == 0) {
            APCMDBUF_CFG_80211D *cmd_buf = NULL;
            IEEEtypes_SubbandSet_t sub_bands[MAX_SUB_BANDS];
            u8 no_of_sub_band = 0;
            u16 buf_len;
            u16 cmdlen;
            u8 *buf = NULL;

            if ((strlen(args[1]) > 3) || (strlen(args[1]) < 0)) {
                printf("In-correct country input\n");
                goto done;
            }
            strcpy(country_80211d, args[1]);
            for (i = 0; i < strlen(country_80211d); i++) {
                if ((country_80211d[i] < 'A') || (country_80211d[i] > 'z')) {
                    printf("Invalid Country Code\n");
                    goto done;
                }
                if (country_80211d[i] > 'Z')
                    country_80211d[i] = country_80211d[i] - 'a' + 'A';
            }
            no_of_sub_band = parse_domain_file(country_80211d, sub_bands);
            if (no_of_sub_band == UAP_FAILURE) {
                printf("Parsing Failed\n");
                goto done;
            }
            buf_len = sizeof(APCMDBUF_CFG_80211D);
            buf_len += no_of_sub_band * sizeof(IEEEtypes_SubbandSet_t);
            buf = (u8 *) malloc(buf_len);
            if (!buf) {
                printf("ERR:Cannot allocate buffer from command!\n");
                goto done;
            }
            bzero((char *) buf, buf_len);
            cmd_buf = (APCMDBUF_CFG_80211D *) buf;
            cmdlen = buf_len;
            cmd_buf->Size = cmdlen - BUF_HEADER_SIZE;
            cmd_buf->Result = 0;
            cmd_buf->SeqNum = 0;
            cmd_buf->Action = uap_cpu_to_le16(ACTION_SET);
            cmd_buf->CmdCode = HostCmd_CMD_802_11D_DOMAIN_INFO;
            cmd_buf->Domain.Tag = uap_cpu_to_le16(TLV_TYPE_DOMAIN);
            cmd_buf->Domain.Length = uap_cpu_to_le16(sizeof(domain_param_t)
                                                     - BUF_HEADER_SIZE
                                                     +
                                                     (no_of_sub_band *
                                                      sizeof
                                                      (IEEEtypes_SubbandSet_t)));

            memset(cmd_buf->Domain.CountryCode, ' ',
                   sizeof(cmd_buf->Domain.CountryCode));
            memcpy(cmd_buf->Domain.CountryCode, country_80211d,
                   strlen(country_80211d));
            memcpy(cmd_buf->Domain.Subband, sub_bands,
                   no_of_sub_band * sizeof(IEEEtypes_SubbandSet_t));

            /* Send the command */
            uap_ioctl((u8 *) cmd_buf, &cmdlen, buf_len);
            if (buf)
                free(buf);
        }

        /* Check for beginning of AP MAC address filter configurations */
        if (strcmp(args[0], "ap_mac_filter") == 0) {
            is_ap_mac_filter = 1;
            cmd_len =
                sizeof(APCMDBUF_SYS_CONFIGURE) +
                sizeof(TLVBUF_STA_MAC_ADDR_FILTER) +
                (MAX_MAC_ONESHOT_FILTER * ETH_ALEN);
            if (buffer) {
                free(buffer);
                buffer = NULL;
            }
            buffer = (u8 *) malloc(cmd_len);
            if (!buffer) {
                printf("ERR:Cannot allocate memory!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
            cmd_buf->Size = cmd_len;
            cmd_buf->SeqNum = 0;
            cmd_buf->Result = 0;
            cmd_buf->Action = ACTION_SET;
            filter_tlv =
                (TLVBUF_STA_MAC_ADDR_FILTER *) (buffer +
                                                sizeof(APCMDBUF_SYS_CONFIGURE));
            filter_tlv->Tag = MRVL_STA_MAC_ADDR_FILTER_TLV_ID;
            filter_tlv->Length = 2;
            filter_tlv->Count = 0;
            filter_mac_count = 0;
            continue;
        }

        /* Check for beginning of custom IE configurations */
        if (strcmp(args[0], "custom_ie_config") == 0) {
            is_custom_ie_config = 1;
            cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE) + sizeof(tlvbuf_custom_ie)
                +
                ((MAX_IE_BUFFER_LEN + sizeof(custom_ie)) * MAX_CUSTOM_IE_COUNT);
            if (buffer) {
                free(buffer);
                buffer = NULL;
            }
            buffer = (u8 *) malloc(cmd_len);
            if (!buffer) {
                printf("ERR:Cannot allocate memory!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
            cmd_buf->Size = cmd_len;
            cmd_buf->SeqNum = 0;
            cmd_buf->Result = 0;
            cmd_buf->Action = ACTION_SET;
            custom_ie_tlv =
                (tlvbuf_custom_ie *) (buffer + sizeof(APCMDBUF_SYS_CONFIGURE));
            custom_ie_ptr = (custom_ie *) (custom_ie_tlv->ie_data);
            custom_ie_tlv_head = custom_ie_tlv;
            custom_ie_tlv_head->Tag = MRVL_MGMT_IE_LIST_TLV_ID;
            continue;
        }

        if ((strcmp(args[0], "FilterMode") == 0) && is_ap_mac_filter) {
            if ((ISDIGIT(args[1]) == 0) || (atoi(args[1]) < 0) ||
                (atoi(args[1]) > 2)) {
                printf
                    ("ERR:Illegal FilterMode paramter %d. Must be either '0', '1', or '2'.\n",
                     atoi(args[1]));
                goto done;
            }
            filter_tlv->FilterMode = atoi(args[1]);
            continue;
        }
        if ((strcmp(args[0], "Count") == 0) && is_ap_mac_filter) {
            filter_tlv->Count = atoi(args[1]);
            if ((ISDIGIT(args[1]) == 0) ||
                (filter_tlv->Count > MAX_MAC_ONESHOT_FILTER)) {
                printf("ERR: Illegal Count parameter.\n");
                goto done;
            }
        }
        if ((strncmp(args[0], "mac_", 4) == 0) && is_ap_mac_filter) {
            if (filter_mac_count < MAX_MAC_ONESHOT_FILTER) {
                if (mac2raw
                    (args[1],
                     &filter_tlv->MacAddress[filter_mac_count * ETH_ALEN]) !=
                    UAP_SUCCESS) {
                    printf("ERR: Invalid MAC address %s \n", args[1]);
                    goto done;
                }
                filter_mac_count++;
            } else {
                printf
                    ("ERR: Filter table can not have more than %d MAC addresses\n",
                     MAX_MAC_ONESHOT_FILTER);
                goto done;
            }
        }

        /* custom ie configuration parameters */
        if ((strncmp(args[0], "MgmtSubtypeMask_", 16) == 0) &&
            is_custom_ie_config) {
            if (UAP_FAILURE == ishexstring(args[1])) {
                printf("ERR:Illegal MgmtSubtypeMask %s.\n", args[1]);
                goto done;
            }
            mask_ie_index = (u16) atoi(args[0] + strlen("MgmtSubtypeMask_"));
            if (mask_ie_index > 3) {
                printf("ERR:Incorrect index %d.\n", mask_ie_index);
                goto done;
            }
            custom_ie_ptr->ie_index = uap_cpu_to_le16(mask_ie_index);
            custom_ie_ptr->mgmt_subtype_mask = (u16) A2HEXDECIMAL(args[1]);
            custom_ie_ptr->mgmt_subtype_mask = uap_cpu_to_le16
                (custom_ie_ptr->mgmt_subtype_mask);
            custom_mask_count++;
            continue;
        }
        if ((strncmp(args[0], "IEBuffer_", 9) == 0) && is_custom_ie_config) {
            if (UAP_FAILURE == ishexstring(args[1])) {
                printf("ERR:Only hex digits are allowed\n");
                goto done;
            }
            ie_buf_len = strlen(args[1]);
            if (!strncasecmp("0x", args[1], 2)) {
                ie_len = (ie_buf_len - 2 + 1) / 2;
                args[1] += 2;
            } else
                ie_len = (ie_buf_len + 1) / 2;

            if (ie_len > MAX_IE_BUFFER_LEN) {
                printf("ERR:Incorrect IE length %d\n", ie_buf_len);
                goto done;
            }

            custom_ie_ptr->ie_index = (u16) atoi(args[0] + strlen("IEBuffer_"));
            if (custom_ie_ptr->ie_index != mask_ie_index) {
                printf("ERR:IE buffer%d should follow MgmtSubtypeMask%d\n",
                       mask_ie_index, mask_ie_index);
                goto done;
            }
            custom_ie_ptr->ie_index = uap_cpu_to_le16(custom_ie_ptr->ie_index);
            string2raw(args[1], custom_ie_ptr->ie_buffer);
            custom_ie_ptr->ie_length = uap_cpu_to_le16(ie_len);
            custom_ie_tlv_len += sizeof(custom_ie) + ie_len;
            custom_ie_tlv = (tlvbuf_custom_ie *) ((u8 *) custom_ie_tlv
                                                  + sizeof(custom_ie) + ie_len);
            custom_ie_ptr = (custom_ie *) (custom_ie_tlv->ie_data);
            custom_buf_count++;
            continue;
        }
        if (strcmp(args[0], "SSID") == 0) {
            if (arg_num == 1) {
                printf("ERR:SSID field is blank!\n");
                goto done;
            } else {
                TLVBUF_SSID *tlv = NULL;
                if (args[1][0] == '"') {
                    args[1]++;
                }
                if (args[1][strlen(args[1]) - 1] == '"') {
                    args[1][strlen(args[1]) - 1] = '\0';
                }
                if ((strlen(args[1]) > MAX_SSID_LENGTH) ||
                    (strlen(args[1]) == 0)) {
                    printf("ERR:SSID length out of range (%d to %d).\n",
                           MIN_SSID_LENGTH, MAX_SSID_LENGTH);
                    goto done;
                }
                /* Append a new TLV */
                tlv_len = sizeof(TLVBUF_SSID) + strlen(args[1]);
                buffer = realloc(buffer, cmd_len + tlv_len);
                if (!buffer) {
                    printf("ERR:Cannot realloc SSID TLV!\n");
                    goto done;
                }
                cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                tlv = (TLVBUF_SSID *) (buffer + cmd_len);
                cmd_len += tlv_len;
                /* Set TLV fields */
                tlv->Tag = MRVL_SSID_TLV_ID;
                tlv->Length = strlen(args[1]);
                memcpy(tlv->Ssid, args[1], tlv->Length);
                endian_convert_tlv_header_out(tlv);
            }
        }
        if (strcmp(args[0], "BeaconPeriod") == 0) {
            if (is_input_valid(BEACONPERIOD, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_BEACON_PERIOD *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_BEACON_PERIOD);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot realloc beacon period TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_BEACON_PERIOD *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_BEACON_PERIOD_TLV_ID;
            tlv->Length = 2;
            tlv->BeaconPeriod_ms = (u16) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->BeaconPeriod_ms = uap_cpu_to_le16(tlv->BeaconPeriod_ms);
        }
        if (strcmp(args[0], "ChanList") == 0) {
            if (is_input_valid(SCANCHANNELS, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }

            TLVBUF_CHANNEL_LIST *tlv = NULL;
            CHANNEL_LIST *pChanList = NULL;
            /* Append a new TLV */
            tlv_len =
                sizeof(TLVBUF_CHANNEL_LIST) +
                ((arg_num - 1) * sizeof(CHANNEL_LIST));
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append channel list TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_CHANNEL_LIST *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_CHANNELLIST_TLV_ID;
            tlv->Length = sizeof(CHANNEL_LIST) * (arg_num - 1);
            pChanList = (CHANNEL_LIST *) tlv->ChanList;
            for (i = 0; i < (arg_num - 1); i++) {
                pChanList->ChanNumber = (u8) atoi(args[i + 1]);
                pChanList->BandConfigType = 0;
                pChanList++;
            }
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "Channel") == 0) {
            if (is_input_valid(CHANNEL, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_CHANNEL_CONFIG *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_CHANNEL_CONFIG);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append channel TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_CHANNEL_CONFIG *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_CHANNELCONFIG_TLV_ID;
            tlv->Length = 2;
            tlv->ChanNumber = (u8) atoi(args[1]);
            if ((arg_num - 1) == 2)
                tlv->BandConfigType = atoi(args[2]) ? BAND_CONFIG_ACS_MODE : 0;
            else
                tlv->BandConfigType = 0;
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "AP_MAC") == 0) {
            int ret;
            TLVBUF_AP_MAC_ADDRESS *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_AP_MAC_ADDRESS);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append channel TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_AP_MAC_ADDRESS *) (buffer + cmd_len);
            cmd_len += tlv_len;
            cmd_buf->Action = ACTION_SET;
            tlv->Tag = MRVL_AP_MAC_ADDRESS_TLV_ID;
            tlv->Length = ETH_ALEN;
            if ((ret = mac2raw(args[1], tlv->ApMacAddr)) != UAP_SUCCESS) {
                printf("ERR: %s Address \n",
                       ret == UAP_FAILURE ? "Invalid MAC" : ret ==
                       UAP_RET_MAC_BROADCAST ? "Broadcast" : "Multicast");
                goto done;
            }
            endian_convert_tlv_header_out(tlv);
        }

        if (strcmp(args[0], "RxAntenna") == 0) {
            if ((ISDIGIT(args[1]) != UAP_SUCCESS) || (atoi(args[1]) < 0) ||
                (atoi(args[1]) > 1)) {
                printf("ERR: Invalid Antenna value\n");
                goto done;
            }
            TLVBUF_ANTENNA_CTL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_ANTENNA_CTL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append channel TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_ANTENNA_CTL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            cmd_buf->Action = ACTION_SET;
            tlv->Tag = MRVL_ANTENNA_CTL_TLV_ID;
            tlv->Length = 2;
            tlv->WhichAntenna = 0;
            tlv->AntennaMode = atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }

        if (strcmp(args[0], "TxAntenna") == 0) {
            if ((ISDIGIT(args[1]) != UAP_SUCCESS) || (atoi(args[1]) < 0) ||
                (atoi(args[1]) > 1)) {
                printf("ERR: Invalid Antenna value\n");
                goto done;
            }
            TLVBUF_ANTENNA_CTL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_ANTENNA_CTL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append channel TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_ANTENNA_CTL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            cmd_buf->Action = ACTION_SET;
            tlv->Tag = MRVL_ANTENNA_CTL_TLV_ID;
            tlv->Length = 2;
            tlv->WhichAntenna = 1;
            tlv->AntennaMode = atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "Rate") == 0) {
            if (is_input_valid(RATE, arg_num - 1, args + 1) != UAP_SUCCESS) {
                printf("ERR: Invalid Rate input\n");
                goto done;
            }
            TLVBUF_RATES *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_RATES) + arg_num - 1;
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append rates TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_RATES *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_RATES_TLV_ID;
            tlv->Length = arg_num - 1;
            for (i = 0; i < tlv->Length; i++) {
                rate[i] = tlv->OperationalRates[i] =
                    (u8) A2HEXDECIMAL(args[i + 1]);
            }
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "TxPowerLevel") == 0) {
            if (is_input_valid(TXPOWER, arg_num - 1, args + 1) != UAP_SUCCESS) {
                printf("ERR:Invalid TxPowerLevel \n");
                goto done;
            } else {
                TLVBUF_TX_POWER *tlv = NULL;
                /* Append a new TLV */
                tlv_len = sizeof(TLVBUF_TX_POWER);
                buffer = realloc(buffer, cmd_len + tlv_len);
                if (!buffer) {
                    printf("ERR:Cannot append tx power level TLV!\n");
                    goto done;
                }
                cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                tlv = (TLVBUF_TX_POWER *) (buffer + cmd_len);
                cmd_len += tlv_len;
                /* Set TLV fields */
                tlv->Tag = MRVL_TX_POWER_TLV_ID;
                tlv->Length = 1;
                tlv->TxPower_dBm = (u8) atoi(args[1]);
                endian_convert_tlv_header_out(tlv);
            }
        }
        if (strcmp(args[0], "BroadcastSSID") == 0) {
            if (is_input_valid(BROADCASTSSID, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_BCAST_SSID_CTL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_BCAST_SSID_CTL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append SSID broadcast control TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_BCAST_SSID_CTL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_BCAST_SSID_CTL_TLV_ID;
            tlv->Length = 1;
            tlv->BcastSsidCtl = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "RTSThreshold") == 0) {
            if (is_input_valid(RTSTHRESH, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_RTS_THRESHOLD *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_RTS_THRESHOLD);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append RTS threshold TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_RTS_THRESHOLD *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_RTS_THRESHOLD_TLV_ID;
            tlv->Length = 2;
            tlv->RtsThreshold = (u16) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->RtsThreshold = uap_cpu_to_le16(tlv->RtsThreshold);
        }
        if (strcmp(args[0], "FragThreshold") == 0) {
            if (is_input_valid(FRAGTHRESH, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_FRAG_THRESHOLD *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_FRAG_THRESHOLD);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append Fragmentation threshold TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_FRAG_THRESHOLD *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_FRAG_THRESHOLD_TLV_ID;
            tlv->Length = 2;
            tlv->FragThreshold = (u16) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->FragThreshold = uap_cpu_to_le16(tlv->FragThreshold);
        }
        if (strcmp(args[0], "DTIMPeriod") == 0) {
            if (is_input_valid(DTIMPERIOD, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_DTIM_PERIOD *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_DTIM_PERIOD);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append DTIM period TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_DTIM_PERIOD *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_DTIM_PERIOD_TLV_ID;
            tlv->Length = 1;
            tlv->DtimPeriod = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "RadioControl") == 0) {
            if (is_input_valid(RADIOCONTROL, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_RADIO_CTL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_RADIO_CTL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append radio control TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_RADIO_CTL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_RADIO_CTL_TLV_ID;
            tlv->Length = 1;
            tlv->RadioCtl = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "RSNReplayProtection") == 0) {
            if (is_input_valid(RSNREPLAYPROT, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            tlvbuf_rsn_replay_prot *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(tlvbuf_rsn_replay_prot);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append RSN replay protection TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (tlvbuf_rsn_replay_prot *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_RSN_REPLAY_PROT_TLV_ID;
            tlv->Length = 1;
            tlv->rsn_replay_prot = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "TxDataRate") == 0) {
            if (is_input_valid(TXDATARATE, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            tx_data_rate = (u16) A2HEXDECIMAL(args[1]);
        }
        if (strcmp(args[0], "MCBCdataRate") == 0) {
            if (is_input_valid(MCBCDATARATE, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            mcbc_data_rate = (u16) A2HEXDECIMAL(args[1]);
        }
        if (strcmp(args[0], "PktFwdCtl") == 0) {
            if (is_input_valid(PKTFWD, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_PKT_FWD_CTL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_PKT_FWD_CTL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append packet forwarding control TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_PKT_FWD_CTL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_PKT_FWD_CTL_TLV_ID;
            tlv->Length = 1;
            tlv->PktFwdCtl = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "StaAgeoutTimer") == 0) {
            if (is_input_valid(STAAGEOUTTIMER, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_STA_AGEOUT_TIMER *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_STA_AGEOUT_TIMER);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append STA ageout timer TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_STA_AGEOUT_TIMER *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_STA_AGEOUT_TIMER_TLV_ID;
            tlv->Length = 4;
            tlv->StaAgeoutTimer_ms = (u32) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->StaAgeoutTimer_ms = uap_cpu_to_le32(tlv->StaAgeoutTimer_ms);
        }
        if (strcmp(args[0], "AuthMode") == 0) {
            if (is_input_valid(AUTHMODE, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_AUTH_MODE *tlv = NULL;
            if ((atoi(args[1]) < 0) || (atoi(args[1]) > 1)) {
                printf
                    ("ERR:Illegal AuthMode parameter. Must be either '0' or '1'.\n");
                goto done;
            }
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_AUTH_MODE);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append auth mode TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_AUTH_MODE *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_AUTH_TLV_ID;
            tlv->Length = 1;
            tlv->AuthMode = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
        if (strcmp(args[0], "KeyIndex") == 0) {
            if (arg_num == 1) {
                printf("KeyIndex is blank!\n");
                goto done;
            } else {
                if (ISDIGIT(args[1]) == 0) {
                    printf
                        ("ERR:Illegal KeyIndex parameter. Must be either '0', '1', '2', or '3'.\n");
                    goto done;
                }
                keyindex = atoi(args[1]);
                if ((keyindex < 0) || (keyindex > 3)) {
                    printf
                        ("ERR:Illegal KeyIndex parameter. Must be either '0', '1', '2', or '3'.\n");
                    goto done;
                }
            }
        }
        if (strncmp(args[0], "Key_", 4) == 0) {
            if (arg_num == 1) {
                printf("ERR:%s is blank!\n", args[0]);
                goto done;
            } else {
                TLVBUF_WEP_KEY *tlv = NULL;
                int key_len = 0;
                if (args[1][0] == '"') {
                    if ((strlen(args[1]) != 2) && (strlen(args[1]) != 7) &&
                        (strlen(args[1]) != 15)) {
                        printf("ERR:Wrong key length!\n");
                        goto done;
                    }
                    key_len = strlen(args[1]) - 2;
                } else {
                    if ((strlen(args[1]) != 0) && (strlen(args[1]) != 10) &&
                        (strlen(args[1]) != 26)) {
                        printf("ERR:Wrong key length!\n");
                        goto done;
                    }
                    if (UAP_FAILURE == ishexstring(args[1])) {
                        printf
                            ("ERR:Only hex digits are allowed when key length is 10 or 26\n");
                        goto done;
                    }
                    key_len = strlen(args[1]) / 2;
                }
                /* Append a new TLV */
                tlv_len = sizeof(TLVBUF_WEP_KEY) + key_len;
                buffer = realloc(buffer, cmd_len + tlv_len);
                if (!buffer) {
                    printf("ERR:Cannot append WEP key configurations TLV!\n");
                    goto done;
                }
                cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                tlv = (TLVBUF_WEP_KEY *) (buffer + cmd_len);
                cmd_len += tlv_len;
                /* Set TLV fields */
                tlv->Tag = MRVL_WEP_KEY_TLV_ID;
                tlv->Length = key_len + 2;
                if (strcmp(args[0], "Key_0") == 0) {
                    tlv->KeyIndex = 0;
                } else if (strcmp(args[0], "Key_1") == 0) {
                    tlv->KeyIndex = 1;
                } else if (strcmp(args[0], "Key_2") == 0) {
                    tlv->KeyIndex = 2;
                } else if (strcmp(args[0], "Key_3") == 0) {
                    tlv->KeyIndex = 3;
                }
                if (keyindex == tlv->KeyIndex) {
                    tlv->IsDefault = 1;
                } else {
                    tlv->IsDefault = 0;
                }
                if (args[1][0] == '"') {
                    memcpy(tlv->Key, &args[1][1], strlen(args[1]) - 2);
                } else {
                    string2raw(args[1], tlv->Key);
                }
                endian_convert_tlv_header_out(tlv);
            }
        }
        if (strcmp(args[0], "PSK") == 0) {
            if (arg_num == 1) {
                printf("ERR:PSK is blank!\n");
                goto done;
            } else {
                TLVBUF_WPA_PASSPHRASE *tlv = NULL;
                if (args[1][0] == '"') {
                    args[1]++;
                }
                if (args[1][strlen(args[1]) - 1] == '"') {
                    args[1][strlen(args[1]) - 1] = '\0';
                }
                tlv_len = sizeof(TLVBUF_WPA_PASSPHRASE) + strlen(args[1]);
                if (strlen(args[1]) > MAX_WPA_PASSPHRASE_LENGTH) {
                    printf("ERR:PSK too long.\n");
                    goto done;
                }
                if (strlen(args[1]) < MIN_WPA_PASSPHRASE_LENGTH) {
                    printf("ERR:PSK too short.\n");
                    goto done;
                }
                if (strlen(args[1]) == MAX_WPA_PASSPHRASE_LENGTH) {
                    if (UAP_FAILURE == ishexstring(args[1])) {
                        printf
                            ("ERR:Only hex digits are allowed when passphrase's length is 64\n");
                        goto done;
                    }
                }
                /* Append a new TLV */
                buffer = realloc(buffer, cmd_len + tlv_len);
                if (!buffer) {
                    printf("ERR:Cannot append WPA passphrase TLV!\n");
                    goto done;
                }
                cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                tlv = (TLVBUF_WPA_PASSPHRASE *) (buffer + cmd_len);
                cmd_len += tlv_len;
                /* Set TLV fields */
                tlv->Tag = MRVL_WPA_PASSPHRASE_TLV_ID;
                tlv->Length = strlen(args[1]);
                memcpy(tlv->Passphrase, args[1], tlv->Length);
                endian_convert_tlv_header_out(tlv);
            }
        }
        if (strcmp(args[0], "Protocol") == 0) {
            if (is_input_valid(PROTOCOL, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_PROTOCOL *tlv = NULL;
            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_PROTOCOL);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append protocol TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_PROTOCOL *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_PROTOCOL_TLV_ID;
            tlv->Length = 2;
            tlv->Protocol = (u16) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->Protocol = uap_cpu_to_le16(tlv->Protocol);
            if (atoi(args[1]) & (PROTOCOL_WPA | PROTOCOL_WPA2)) {
                TLVBUF_AKMP *tlv = NULL;
                /* Append a new TLV */
                tlv_len = sizeof(TLVBUF_AKMP);
                buffer = realloc(buffer, cmd_len + tlv_len);
                if (!buffer) {
                    printf("ERR:Cannot append AKMP TLV!\n");
                    goto done;
                }
                cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
                tlv = (TLVBUF_AKMP *) (buffer + cmd_len);
                cmd_len += tlv_len;
                /* Set TLV fields */
                tlv->Tag = MRVL_AKMP_TLV_ID;
                tlv->Length = 2;
                tlv->KeyMgmt = KEY_MGMT_PSK;
                endian_convert_tlv_header_out(tlv);
                tlv->KeyMgmt = uap_cpu_to_le16(tlv->KeyMgmt);
            }
        }
        if (strcmp(args[0], "PairwiseCipher") == 0) {
            if (arg_num == 1) {
                printf("ERR:PairwiseCipher is blank!\n");
                goto done;
            } else {
                if (ISDIGIT(args[1]) == 0) {
                    printf
                        ("ERR:Illegal PairwiseCipher parameter. Must be either bit '2' or '3'.\n");
                    goto done;
                }
                pairwisecipher = atoi(args[1]);
                if (pairwisecipher & ~CIPHER_BITMAP) {
                    printf
                        ("ERR:Illegal PairwiseCipher parameter. Must be either bit '2' or '3'.\n");
                    goto done;
                }
            }
        }
        if (strcmp(args[0], "GroupCipher") == 0) {
            if (arg_num == 1) {
                printf("ERR:GroupCipher is blank!\n");
                goto done;
            } else {
                if (ISDIGIT(args[1]) == 0) {
                    printf
                        ("ERR:Illegal GroupCipher parameter. Must be either bit '2' or '3'.\n");
                    goto done;
                }
                groupcipher = atoi(args[1]);
                if (groupcipher & ~CIPHER_BITMAP) {
                    printf
                        ("ERR:Illegal GroupCipher parameter. Must be either bit '2' or '3'.\n");
                    goto done;
                }
            }
        }
        if (strcmp(args[0], "GroupRekeyTime") == 0) {
            if (is_input_valid(GROUPREKEYTIMER, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_GROUP_REKEY_TIMER *tlv = NULL;

            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_GROUP_REKEY_TIMER);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot append protocol TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_GROUP_REKEY_TIMER *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_GRP_REKEY_TIME_TLV_ID;
            tlv->Length = 4;
            tlv->GroupRekeyTime_sec = (u32) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->GroupRekeyTime_sec = uap_cpu_to_le32(tlv->GroupRekeyTime_sec);
        }
        if (strcmp(args[0], "MaxStaNum") == 0) {
            if (is_input_valid(MAXSTANUM, arg_num - 1, args + 1) != UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_MAX_STA_NUM *tlv = NULL;

            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_MAX_STA_NUM);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot realloc max station number TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_MAX_STA_NUM *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_MAX_STA_CNT_TLV_ID;
            tlv->Length = 2;
            tlv->Max_sta_num = (u16) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
            tlv->Max_sta_num = uap_cpu_to_le16(tlv->Max_sta_num);
        }
        if (strcmp(args[0], "Retrylimit") == 0) {
            if (is_input_valid(RETRYLIMIT, arg_num - 1, args + 1) !=
                UAP_SUCCESS) {
                goto done;
            }
            TLVBUF_RETRY_LIMIT *tlv = NULL;

            /* Append a new TLV */
            tlv_len = sizeof(TLVBUF_RETRY_LIMIT);
            buffer = realloc(buffer, cmd_len + tlv_len);
            if (!buffer) {
                printf("ERR:Cannot realloc retry limit TLV!\n");
                goto done;
            }
            cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buffer;
            tlv = (TLVBUF_RETRY_LIMIT *) (buffer + cmd_len);
            cmd_len += tlv_len;
            /* Set TLV fields */
            tlv->Tag = MRVL_RETRY_LIMIT_TLV_ID;
            tlv->Length = 1;
            tlv->retry_limit = (u8) atoi(args[1]);
            endian_convert_tlv_header_out(tlv);
        }
#if DEBUG
        if (cmd_len != 0) {
            hexdump("Command Buffer", (void *) cmd_buf, cmd_len, ' ');
        }
#endif
    }
  done:
    fclose(config_file);
    if (buffer)
        free(buffer);
    if (line)
        free(line);
}

/**
 *  @brief Show usage information for the cfg_80211d command
 *
 *  $return         N/A
 */
void
print_apcmd_cfg_80211d_usage(void)
{
    printf("\nUsage: cfg_80211d <state 0/1> <country Country_code> \n");
    return;
}

/**
 *  @brief Show usage information for the uap_stats command
 *
 *  $return         N/A
 */
void
print_apcmd_uap_stats(void)
{
    printf("Usage: uap_stats \n");
    return;
}

/**
 *  SNMP MIB OIDs Table
 */
static oids_table snmp_oids[] = {
    {0x0b, 4, "dot11LocalTKIPMICFailures"},
    {0x0c, 4, "dot11CCMPDecryptErrors"},
    {0x0d, 4, "dot11WEPUndecryptableCount"},
    {0x0e, 4, "dot11WEPICVErrorCount"},
    {0x0f, 4, "dot11DecryptFailureCount"},
    {0x12, 4, "dot11FailedCount"},
    {0x13, 4, "dot11RetryCount"},
    {0x14, 4, "dot11MultipleRetryCount"},
    {0x15, 4, "dot11FrameDuplicateCount"},
    {0x16, 4, "dot11RTSSuccessCount"},
    {0x17, 4, "dot11RTSFailureCount"},
    {0x18, 4, "dot11ACKFailureCount"},
    {0x19, 4, "dot11ReceivedFragmentCount"},
    {0x1a, 4, "dot11MulticastReceivedFrameCount"},
    {0x1b, 4, "dot11FCSErrorCount"},
    {0x1c, 4, "dot11TransmittedFrameCount"},
    {0x1d, 4, "dot11RSNATKIPCounterMeasuresInvoked"},
    {0x1e, 4, "dot11RSNA4WayHandshakeFailures"},
    {0x1f, 4, "dot11MulticastTransmittedFrameCount"}
};

/** 
 *  @brief Get uAP stats
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  
 *  @return         NA
 */
void
apcmd_uap_stats(int argc, char *argv[])
{
    u8 no_of_oids = sizeof(snmp_oids) / sizeof(snmp_oids[0]);
    u16 i, j;
    int size;
    APCMDBUF_SNMP_MIB *cmd_buf = NULL;
    u8 *buf = NULL;
    TLVBUF_HEADER *tlv = NULL;
    u16 cmd_len = 0;
    u16 buf_len = 0;
    u8 ret;
    int opt;
    u16 oid_size;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_apcmd_uap_stats();
            return;
        }
    }

    argc -= optind;
    argv += optind;
    if (argc) {
        printf("Error: Invalid Input\n");
        print_apcmd_uap_stats();
        return;
    }

    /**  Command Header */
    buf_len += sizeof(APCMDBUF_SNMP_MIB);

    for (i = 0; i < no_of_oids; i++) {
        /** 
         * size of Oid + Oid_value + Oid_size 
         */
        buf_len += snmp_oids[i].len + sizeof(TLVBUF_HEADER);
    }
    buf = (u8 *) malloc(buf_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);

    /* Locate Headers */
    cmd_buf = (APCMDBUF_SNMP_MIB *) buf;
    cmd_buf->Size = buf_len - BUF_HEADER_SIZE;
    cmd_buf->Result = 0;
    cmd_buf->SeqNum = 0;
    cmd_buf->CmdCode = HostCmd_SNMP_MIB;
    cmd_buf->Action = uap_cpu_to_le16(ACTION_GET);

    tlv = (TLVBUF_HEADER *) ((u8 *) cmd_buf + sizeof(APCMDBUF_SNMP_MIB));
    /* Add oid, oid_size and oid_value for each OID */
    for (i = 0; i < no_of_oids; i++) {
        /** Copy Index as Oid */
        tlv->Type = uap_cpu_to_le16(snmp_oids[i].type);
        /** Copy its size */
        tlv->Len = uap_cpu_to_le16(snmp_oids[i].len);
        /** Next TLV */
        tlv = (TLVBUF_HEADER *) & (tlv->Data[snmp_oids[i].len]);
    }
    cmd_len = buf_len;
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            tlv =
                (TLVBUF_HEADER *) ((u8 *) cmd_buf + sizeof(APCMDBUF_SNMP_MIB));

            size =
                cmd_buf->Size - (sizeof(APCMDBUF_SNMP_MIB) - BUF_HEADER_SIZE);

            while (size >= sizeof(TLVBUF_HEADER)) {
                tlv->Type = uap_le16_to_cpu(tlv->Type);
                for (i = 0; i < no_of_oids; i++) {
                    if (snmp_oids[i].type == tlv->Type) {
                        printf("%s: ", snmp_oids[i].name);
                        break;
                    }
                }
                oid_size = uap_le16_to_cpu(tlv->Len);
                switch (oid_size) {
                case 1:
                    printf("%d", (unsigned int) tlv->Data[0]);
                    break;
                case 2:
                    printf("%d",
                           (unsigned int) uap_le16_to_cpu(*(u16 *) tlv->Data));
                    break;
                case 4:
                    printf("%ld",
                           (unsigned long) uap_le32_to_cpu(*(u32 *) tlv->Data));
                    break;
                default:
                    for (j = 0; j < oid_size; j++) {
                        printf("%d ", (u8) tlv->Data[i]);
                    }
                    break;
                }
                /** Next TLV */
                tlv = (TLVBUF_HEADER *) & (tlv->Data[oid_size]);
                size -= (sizeof(TLVBUF_HEADER) + oid_size);
                size = (size > 0) ? size : 0;
                printf("\n");
            }

        } else {
            printf("ERR:Command Response incorrect!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
}

/**
 *  @brief parser for sys_cfg_80211d input 
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @param output   stores indexes for "state, country"
 *                  arguments
 *
 *  @return         NA
 *
 */
void
parse_input_80211d(int argc, char **argv, int output[2][2])
{
    int i, j, k = 0;
    char *keywords[2] = { "state", "country" };

    for (i = 0; i < 2; i++)
        output[i][0] = -1;

    for (i = 0; i < argc; i++) {
        for (j = 0; j < 2; j++) {
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
 *  @brief Set/Get 802.11D country information 
 *
 *  Usage: cfg_80211d state country_code 
 *  
 *  State 0 or 1
 *  
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_cfg_80211d(int argc, char *argv[])
{
    APCMDBUF_CFG_80211D *cmd_buf = NULL;
    IEEEtypes_SubbandSet_t *subband = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int output[2][2];
    int ret = UAP_FAILURE;
    int opt;
    int i, j;
    u8 state = 0;
    char country[4] = { ' ', ' ', 0, 0 };
    u8 sflag = 0, cflag = 0;
    IEEEtypes_SubbandSet_t sub_bands[MAX_SUB_BANDS];
    u8 no_of_sub_band = 0;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_apcmd_cfg_80211d_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    if (argc) {
        /** SET */
        parse_input_80211d(argc, argv, output);

        /** state */
        if ((output[0][0] != -1) && (output[0][1] > output[0][0])) {
            if ((output[0][1] - output[0][0]) != 2) {
                printf("ERR: Invalid state inputs\n");
                print_apcmd_cfg_80211d_usage();
                return;
            }

            if (IS_HEX_OR_DIGIT(argv[output[0][0] + 1]) == UAP_FAILURE) {
                printf("ERR: valid input for state are 0 or 1\n");
                print_apcmd_cfg_80211d_usage();
                return;
            }
            state = (u8) A2HEXDECIMAL(argv[output[0][0] + 1]);

            if ((state != 0) && (state != 1)) {
                printf("ERR: valid input for state are 0 or 1 \n");
                print_apcmd_cfg_80211d_usage();
                return;
            }
            sflag = 1;
        }

        /** country */
        if ((output[1][0] != -1) && (output[1][1] > output[1][0])) {
            if ((output[1][1] - output[1][0]) > 2) {
                printf("ERR: Invalid country inputs\n");
                print_apcmd_cfg_80211d_usage();
                return;
            }
            if ((strlen(argv[output[1][0] + 1]) > 3) ||
                (strlen(argv[output[1][0] + 1]) < 0)) {
                print_apcmd_cfg_80211d_usage();
                return;
            }

            strcpy(country, argv[output[1][0] + 1]);

            for (i = 0; i < strlen(country); i++) {
                if ((country[i] < 'A') || (country[i] > 'z')) {
                    printf("Invalid Country Code\n");
                    print_apcmd_cfg_80211d_usage();
                    return;
                }
                if (country[i] > 'Z')
                    country[i] = country[i] - 'a' + 'A';
            }

            cflag = 1;

           /** Get domain information from the file */
            no_of_sub_band = parse_domain_file(country, sub_bands);
            if (no_of_sub_band == UAP_FAILURE) {
                printf("Parsing Failed\n");
                return;
            }
        }
    }

    if (argc && !cflag && !sflag) {
        printf("ERR: Invalid input\n");
        print_apcmd_cfg_80211d_usage();
        return;
    }

    if (sflag && !cflag) {
        /**
         * Update MIB only and return
         */
        if (sg_snmp_mib(ACTION_SET, OID_80211D_ENABLE, sizeof(state), &state) ==
            UAP_SUCCESS) {
            printf("802.11d %sd \n", state ? "enable" : "disable");
        }
        return;
    }

    buf_len = sizeof(APCMDBUF_CFG_80211D);

    if (cflag) {
        buf_len += no_of_sub_band * sizeof(IEEEtypes_SubbandSet_t);
    } else { /** Get */
        buf_len += MAX_SUB_BANDS * sizeof(IEEEtypes_SubbandSet_t);
    }

    buf = (u8 *) malloc(buf_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);
    /* Locate headers */
    cmd_buf = (APCMDBUF_CFG_80211D *) buf;
    cmd_len = argc ? buf_len :
                 /** set */
        (sizeof(APCMDBUF_CFG_80211D) - sizeof(domain_param_t)); /** Get */

    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->Result = 0;
    cmd_buf->SeqNum = 0;
    cmd_buf->Action = argc ? ACTION_SET : ACTION_GET;
    cmd_buf->Action = uap_cpu_to_le16(cmd_buf->Action);
    cmd_buf->CmdCode = HostCmd_CMD_802_11D_DOMAIN_INFO;

    if (cflag) {
        cmd_buf->Domain.Tag = uap_cpu_to_le16(TLV_TYPE_DOMAIN);
        cmd_buf->Domain.Length = uap_cpu_to_le16(sizeof(domain_param_t)
                                                 - BUF_HEADER_SIZE
                                                 +
                                                 (no_of_sub_band *
                                                  sizeof
                                                  (IEEEtypes_SubbandSet_t)));

        memset(cmd_buf->Domain.CountryCode, ' ',
               sizeof(cmd_buf->Domain.CountryCode));
        memcpy(cmd_buf->Domain.CountryCode, country, strlen(country));
        memcpy(cmd_buf->Domain.Subband, sub_bands,
               no_of_sub_band * sizeof(IEEEtypes_SubbandSet_t));
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (argc) {
                printf("Set executed successfully\n");
                if (sflag) {
                    if (sg_snmp_mib
                        (ACTION_SET, OID_80211D_ENABLE, sizeof(state),
                         &state) == UAP_SUCCESS) {
                        printf("802.11d %sd \n", state ? "enable" : "disable");
                    }
                }
            } else {
                j = uap_le16_to_cpu(cmd_buf->Domain.Length);
                if (sg_snmp_mib
                    (ACTION_GET, OID_80211D_ENABLE, sizeof(state), &state)
                    == UAP_SUCCESS) {
                    printf("State = %sd\n", state ? "enable" : "disable");
                }

                if (!(cmd_buf->Domain.CountryCode[0] |
                      cmd_buf->Domain.CountryCode[1] |
                      cmd_buf->Domain.CountryCode[2])) {
                    printf("Dot11d = country code is not set.\n");
                } else {
                    printf("Country string = %c%c%c",
                           cmd_buf->Domain.CountryCode[0],
                           cmd_buf->Domain.CountryCode[1],
                           cmd_buf->Domain.CountryCode[2]);
                    j -= sizeof(cmd_buf->Domain.CountryCode);
                    subband =
                        (IEEEtypes_SubbandSet_t *) cmd_buf->Domain.Subband;
                    printf("\nSub-band info=");
                    // printf("\n\t\t(First Channel, Number of Channels,
                    // Maximum TX Power) \n"); 
                    printf("\t(1st, #chan, MAX-power) \n");
                    for (i = 0; i < (j / 3); i++) {
                        printf("\t\t(%d, \t%d, \t%d dbm)\n", subband->FirstChan,
                               subband->NoOfChan, subband->MaxTxPwr);
                        subband++;
                    }
                }
            }
        } else {
            printf("ERR:Command Response incorrect!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
    return;
}

/** 
 *  @brief Creates a sys_config request and sends to the driver
 *
 *  Usage: "Usage : sys_config [CONFIG_FILE]"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
void
apcmd_sys_config(int argc, char *argv[])
{
    APCMDBUF_SYS_CONFIGURE *cmd_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    int opt;
    char **argv_dummy;
    ps_mgmt pm;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_config_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (argc > 1) {
        printf("ERR:Too many arguments.\n");
        print_sys_config_usage();
        return;
    }
    if (argc == 1) {
        /* Read profile and send command to firmware */
        apcmd_sys_config_profile(argc, argv);
        return;
    }

        /** Query AP's setting */
    buf_len = MRVDRV_SIZE_OF_CMD_BUFFER;

    /* alloc buf for command */
    buf = (u8 *) malloc(buf_len);

    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);

    /* Locate headers */
    cmd_len = sizeof(APCMDBUF_SYS_CONFIGURE);
    cmd_buf = (APCMDBUF_SYS_CONFIGURE *) buf;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_CONFIGURE;
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_CONFIGURE | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response!\n");
            free(buf);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("AP settings:\n");
            print_tlv(buf + sizeof(APCMDBUF_SYS_CONFIGURE),
                      cmd_buf->Size - sizeof(APCMDBUF_SYS_CONFIGURE) +
                      BUF_HEADER_SIZE);
            printf("\n802.11D setting:\n");
            apcmd_cfg_80211d(1, argv_dummy);
        } else {
            printf("ERR:Could not retrieve system configure\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    free(buf);
    memset(&pm, 0, sizeof(ps_mgmt));
    send_power_mode_ioctl(&pm);
    return;
}

/** 
 *  @brief Send read/write command along with register details to the driver
 *  @param reg      reg type
 *  @param offset   pointer to register offset string
 *  @param strvalue pointer to value string
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
apcmd_regrdwr_process(int reg, s8 * offset, s8 * strvalue)
{
    APCMDBUF_REG_RDWR *cmd_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    s8 *whichreg;

    buf_len = sizeof(APCMDBUF_REG_RDWR);

    /* alloc buf for command */
    buf = (u8 *) malloc(buf_len);

    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return UAP_FAILURE;
    }
    bzero((char *) buf, buf_len);

    /* Locate headers */
    cmd_len = sizeof(APCMDBUF_REG_RDWR);
    cmd_buf = (APCMDBUF_REG_RDWR *) buf;

    /* Fill the command buffer */
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;

    switch (reg) {
    case CMD_MAC:
        whichreg = "MAC";
        cmd_buf->CmdCode = HostCmd_CMD_MAC_REG_ACCESS;
        break;
    case CMD_BBP:
        whichreg = "BBP";
        cmd_buf->CmdCode = HostCmd_CMD_BBP_REG_ACCESS;
        break;
    case CMD_RF:
        cmd_buf->CmdCode = HostCmd_CMD_RF_REG_ACCESS;
        whichreg = "RF";
        break;
    default:
        printf("Invalid register set specified.\n");
        free(buf);
        return UAP_FAILURE;
    }
    if (strvalue) {
        cmd_buf->Action = 1;    // WRITE
    } else {
        cmd_buf->Action = 0;    // READ
    }
    cmd_buf->Action = uap_cpu_to_le16(cmd_buf->Action);
    cmd_buf->Offset = A2HEXDECIMAL(offset);
    cmd_buf->Offset = uap_cpu_to_le16(cmd_buf->Offset);
    if (strvalue) {
        cmd_buf->Value = A2HEXDECIMAL(strvalue);
        cmd_buf->Value = uap_cpu_to_le32(cmd_buf->Value);
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("Successfully executed the command\n");
            printf("%s[0x%04hx] = 0x%08lx\n",
                   whichreg, uap_le16_to_cpu(cmd_buf->Offset),
                   uap_le32_to_cpu(cmd_buf->Value));
        } else {
            printf("ERR:Command sending failed!\n");
            free(buf);
            return UAP_FAILURE;
        }
    } else {
        printf("ERR:Command sending failed!\n");
        free(buf);
        return UAP_FAILURE;
    }

    free(buf);
    return UAP_SUCCESS;
}

/**
 *  @brief Send read command for EEPROM 
 *
 *  Usage: "Usage : rdeeprom <offset> <byteCount>"
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_read_eeprom(int argc, char *argv[])
{
    APCMDBUF_EEPROM_ACCESS *cmd_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    u16 byteCount, offset;
    int ret = UAP_FAILURE;
    int opt;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_apcmd_read_eeprom_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if (!argc || (argc && is_input_valid(RDEEPROM, argc, argv) != UAP_SUCCESS)) {
        print_apcmd_read_eeprom_usage();
        return;
    }
    offset = A2HEXDECIMAL(argv[0]);
    byteCount = A2HEXDECIMAL(argv[1]);

    buf_len = sizeof(APCMDBUF_EEPROM_ACCESS) + MAX_EEPROM_LEN;
    buf = (u8 *) malloc(buf_len);
    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_EEPROM_ACCESS *) buf;
    cmd_len = sizeof(APCMDBUF_EEPROM_ACCESS);

    cmd_buf->Size = sizeof(APCMDBUF_EEPROM_ACCESS) - BUF_HEADER_SIZE;
    cmd_buf->Result = 0;
    cmd_buf->SeqNum = 0;
    cmd_buf->Action = 0;

    cmd_buf->CmdCode = HostCmd_EEPROM_ACCESS;
    cmd_buf->Offset = uap_cpu_to_le16(offset);
    cmd_buf->ByteCount = uap_cpu_to_le16(byteCount);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("Successfully executed the command\n");
            byteCount = uap_le16_to_cpu(cmd_buf->ByteCount);
            offset = uap_le16_to_cpu(cmd_buf->Offset);
            hexdump_data("EEPROM", (void *) cmd_buf->Value, byteCount, ' ');
        } else {
            printf("ERR:Command Response incorrect!\n");
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }

    free(buf);
    return;
}

/**
 *  @brief Show usage information for the regrdwr command
 *  command
 *
 *  $return         N/A
 */
void
print_regrdwr_usage(void)
{
    printf("\nUsage : uaputl.exe regrdwr <TYPE> <OFFSET> [value]\n");
    printf("\nTYPE Options: 0     - read/write MAC register");
    printf("\n              1     - read/write BBP register");
    printf("\n              2     - read/write RF register");
    printf("\n");
    return;

}

/** 
 *  @brief Provides interface to perform read/write operations on regsiters
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_regrdwr(int argc, char *argv[])
{
    int opt;
    s32 reg;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_regrdwr_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if ((argc < 2) || (argc > 3)) {
        printf("ERR:wrong arguments.\n");
        print_regrdwr_usage();
        return;
    }
    if ((atoi(argv[0]) != 0) && (atoi(argv[0]) != 1) && (atoi(argv[0]) != 2)) {
        printf("ERR:Illegal register type %s. Must be either '0','1' or '2'.\n",
               argv[0]);
        print_regrdwr_usage();
        return;
    }
    reg = atoi(argv[0]);
    apcmd_regrdwr_process(reg, argv[1], argc > 2 ? argv[2] : NULL);
    return;
}

/**
 *    @brief Show usage information for the memaccess command
 *    command
 *    
 *    $return         N/A
 */
void
print_memaccess_usage(void)
{
    printf("\nUsage : uaputl.exe memaccess <ADDRESS> [value]\n");
    printf("\nRead/Write memory location");
    printf("\nADDRESS: Address of the memory location to be read/written");
    printf("\nValue  : Value to be written at that address\n");
    return;
}

/** 
 *  @brief Provides interface to perform read/write memory location
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_memaccess(int argc, char *argv[])
{
    int opt;
    APCMDBUF_MEM_ACCESS *cmd_buf = NULL;
    u8 *buf = NULL;
    u16 cmd_len;
    u16 buf_len;
    int ret = UAP_FAILURE;
    s8 *address = NULL;
    s8 *value = NULL;

    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_memaccess_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;

    /* Check arguments */
    if ((argc < 1) || (argc > 2)) {
        printf("ERR:wrong arguments.\n");
        print_memaccess_usage();
        return;
    }

    address = argv[0];
    if (argc == 2)
        value = argv[1];

    buf_len = sizeof(APCMDBUF_MEM_ACCESS);

    /* alloc buf for command */
    buf = (u8 *) malloc(buf_len);

    if (!buf) {
        printf("ERR:Cannot allocate buffer from command!\n");
        return;
    }
    bzero((char *) buf, buf_len);
    /* Locate headers */
    cmd_len = sizeof(APCMDBUF_MEM_ACCESS);
    cmd_buf = (APCMDBUF_MEM_ACCESS *) buf;

    /* Fill the command buffer */
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->CmdCode = HostCmd_CMD_MEM_ACCESS;

    if (value)
        cmd_buf->Action = 1;    // WRITE
    else
        cmd_buf->Action = 0;    // READ

    cmd_buf->Action = uap_cpu_to_le16(cmd_buf->Action);
    cmd_buf->Address = A2HEXDECIMAL(address);
    cmd_buf->Address = uap_cpu_to_le32(cmd_buf->Address);

    if (value) {
        cmd_buf->Value = A2HEXDECIMAL(value);
        cmd_buf->Value = uap_cpu_to_le32(cmd_buf->Value);
    }

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        if (cmd_buf->Result == CMD_SUCCESS) {
            printf("Successfully executed the command\n");
            printf("[0x%04lx] = 0x%08lx\n",
                   uap_le32_to_cpu(cmd_buf->Address),
                   uap_le32_to_cpu(cmd_buf->Value));
        } else {
            printf("ERR:Command sending failed!\n");
            free(buf);
            return;
        }
    } else {
        printf("ERR:Command sending failed!\n");
        free(buf);
        return;
    }
    free(buf);
    return;
}

/**
 *    @brief Show usage information for sys_debug command
 *    command
 *    
 *    $return         N/A
 */
void
print_sys_debug_usage(void)
{
    printf("\nUsage : uaputl.exe sys_debug <subcmd> [parameter]\n");
    printf("\nSet/Get debug parameter");
    printf("\nsubcmd: used to set/get debug parameters or set user scan");
    printf("\nparameter  :  parameters for specific subcmd");
    printf("\n		If no [parameter] are given, it return");
    printf("\n		debug parameters for selected subcmd");
    printf("\n\n");
    return;
}

/** 
 *  @brief Creates a sys_debug request and sends to the driver
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         N/A
 */
void
apcmd_sys_debug(int argc, char *argv[])
{
    APCMDBUF_SYS_DEBUG *cmd_buf = NULL;
    u8 *buffer = NULL;
    u16 cmd_len;
    u16 buf_len;
    u32 subcmd = 0;
    int ret = UAP_FAILURE;
    int opt;
    s8 *value = NULL;
    u32 parameter = 0;
    while ((opt = getopt_long(argc, argv, "+", cmd_options, NULL)) != -1) {
        switch (opt) {
        default:
            print_sys_debug_usage();
            return;
        }
    }
    argc -= optind;
    argv += optind;
    /* Check arguments */
    if ((argc == 0) || (argc > 2)) {
        printf("ERR:wrong arguments.\n");
        print_sys_debug_usage();
        return;
    } else {
        if (argc == 2) {
            value = argv[1];
            parameter = A2HEXDECIMAL(value);
        }
    }
    subcmd = atoi(argv[0]);
    /* Initialize the command length */
    if (subcmd == DEBUG_SUBCOMMAND_CHANNEL_SCAN) {
        buf_len =
            sizeof(APCMDBUF_SYS_DEBUG) +
            MAX_CHANNELS * sizeof(CHANNEL_SCAN_ENTRY_T);
        cmd_len = sizeof(APCMDBUF_SYS_DEBUG) - sizeof(debugConfig_t);
    } else {
        cmd_len = sizeof(APCMDBUF_SYS_DEBUG);
        buf_len = cmd_len;
    }

    /* Initialize the command buffer */
    buffer = (u8 *) malloc(buf_len);
    if (!buffer) {
        printf("ERR:Cannot allocate buffer for command!\n");
        return;
    }
    bzero((char *) buffer, buf_len);

    /* Locate headers */
    cmd_buf = (APCMDBUF_SYS_DEBUG *) buffer;

    /* Fill the command buffer */
    cmd_buf->CmdCode = APCMD_SYS_DEBUG;
    cmd_buf->Size = cmd_len - BUF_HEADER_SIZE;
    cmd_buf->SeqNum = 0;
    cmd_buf->Result = 0;
    cmd_buf->subcmd = subcmd;
    if (subcmd == DEBUG_SUBCOMMAND_CHANNEL_SCAN) {
        cmd_buf->Action = ACTION_SET;
    } else {
        if (argc == 1) {
            cmd_buf->Action = ACTION_GET;
        } else {
            cmd_buf->Action = ACTION_SET;
            if (subcmd == DEBUG_SUBCOMMAND_GMODE)
                cmd_buf->debugConfig.globalDebugMode = (u8) parameter;
            else if (subcmd == DEBUG_SUBCOMMAND_MAJOREVTMASK) {
                cmd_buf->debugConfig.debugMajorIdMask = parameter;
                cmd_buf->debugConfig.debugMajorIdMask =
                    uap_cpu_to_le32(cmd_buf->debugConfig.debugMajorIdMask);
            } else {
                cmd_buf->debugConfig.value = uap_cpu_to_le32(parameter);
            }
        }
    }
    cmd_buf->Action = uap_cpu_to_le16(cmd_buf->Action);
    cmd_buf->subcmd = uap_cpu_to_le32(cmd_buf->subcmd);

    /* Send the command */
    ret = uap_ioctl((u8 *) cmd_buf, &cmd_len, buf_len);

    /* Process response */
    if (ret == UAP_SUCCESS) {
        /* Verify response */
        if (cmd_buf->CmdCode != (APCMD_SYS_DEBUG | APCMD_RESP_CHECK)) {
            printf("ERR:Corrupted response! CmdCode=%x\n", cmd_buf->CmdCode);
            free(buffer);
            return;
        }
        /* Print response */
        if (cmd_buf->Result == CMD_SUCCESS) {
            if (subcmd == DEBUG_SUBCOMMAND_CHANNEL_SCAN) {
                int i = 0;
                CHANNEL_SCAN_ENTRY_T *cst = NULL;
                if (cmd_buf->Size <
                    (sizeof(APCMDBUF_SYS_DEBUG) - BUF_HEADER_SIZE)) {
                    printf
                        ("ERR: Invalid command response size, cmd_buf->Size = %x\n",
                         cmd_buf->Size);
                    free(buffer);
                    return;
                }
                for (i = 0; i < cmd_buf->debugConfig.cs_entry.numChannels; i++) {
                    if (i == 0) {
                        printf
                            ("\n------------------------------------------------------");
                        printf("\nChan\tNumAPs\tCCA_Count\tDuration\tWeight");
                        printf
                            ("\n------------------------------------------------------");
                        cst = cmd_buf->debugConfig.cs_entry.cst;
                    }
                    printf("\n%d\t%d\t%ld\t\t%ld\t\t%ld", cst->chan_num,
                           cst->num_of_aps,
                           cst->CCA_count, cst->duration, cst->channel_weight);
                    cst++;
                }
                printf
                    ("\n------------------------------------------------------\n");
            } else {
                if (argc == 1) {
                    if (subcmd == DEBUG_SUBCOMMAND_GMODE) {
                        printf("globalDebugmode=%d\n",
                               cmd_buf->debugConfig.globalDebugMode);
                    } else if (subcmd == DEBUG_SUBCOMMAND_MAJOREVTMASK) {
                        printf("MajorId mask=0x%08lx\n",
                               uap_le32_to_cpu(cmd_buf->debugConfig.
                                               debugMajorIdMask));
                    } else {
                        printf("Value = %ld\n",
                               uap_le32_to_cpu(cmd_buf->debugConfig.value));
                    }
                } else {
                    printf("set debug parameter successful\n");
                }
            }
        } else {
            if (argc == 1) {
                printf("ERR:Could not get debug parameter!\n");
            } else {
                printf("ERR:Could not set debug parameter!\n");
            }
        }
    } else {
        printf("ERR:Command sending failed!\n");
    }
    if (buffer)
        free(buffer);
    return;
}

/** structure of command table*/
typedef struct
{
    /** command name */
    char *cmd;
    /** command function pointer */
    void (*func) (int argc, char *argv[]);
    /**command usuage */
    char *help;
} command_table;

/** ap command table */
static command_table ap_command[] = {
    {"sys_config", apcmd_sys_config, "\tSet/get uAP's profile"},
    {"sys_info", apcmd_sys_info, "\tDisplay system info"},
    {"sys_reset", apcmd_sys_reset, "\tReset uAP"},
    {"bss_start", apcmd_bss_start, "\tStart the BSS"},
    {"bss_stop", apcmd_bss_stop, "\tStop the BSS"},
    {"sta_deauth", apcmd_sta_deauth, "\tDeauth client"},
    {"sta_list", apcmd_sta_list, "\tDisplay list of clients"},
    {"sys_cfg_ap_mac_address", apcmd_sys_cfg_ap_mac_address,
     "Set/get uAP mac address"},
    {"sys_cfg_ssid", apcmd_sys_cfg_ssid, "\tSet/get uAP ssid"},
    {"sys_cfg_beacon_period", apcmd_sys_cfg_beacon_period,
     "Set/get uAP beacon period"},
    {"sys_cfg_dtim_period", apcmd_sys_cfg_dtim_period,
     "Set/get uAP dtim period"},
    {"sys_cfg_channel", apcmd_sys_cfg_channel, "\tSet/get uAP radio channel"},
    {"sys_cfg_scan_channels", apcmd_sys_cfg_scan_channels,
     "Set/get uAP radio channel list"},
    {"sys_cfg_rates", apcmd_sys_cfg_rates, "\tSet/get uAP rates"},
    {"sys_cfg_rates_ext", apcmd_sys_cfg_rates_ext,
     "Set/get uAP rates (extended)"},
    {"sys_cfg_tx_power", apcmd_sys_cfg_tx_power, "Set/get uAP tx power"},
    {"sys_cfg_bcast_ssid_ctl", apcmd_sys_cfg_bcast_ssid_ctl,
     "Set/get uAP broadcast ssid"},
    {"sys_cfg_preamble_ctl", apcmd_sys_cfg_preamble_ctl, "Get uAP preamble"},
    {"sys_cfg_antenna_ctl", apcmd_sys_cfg_antenna_ctl,
     "Set/get uAP tx/rx antenna"},
    {"sys_cfg_rts_threshold", apcmd_sys_cfg_rts_threshold,
     "Set/get uAP rts threshold"},
    {"sys_cfg_frag_threshold", apcmd_sys_cfg_frag_threshold,
     "Set/get uAP frag threshold"},
    {"sys_cfg_radio_ctl", apcmd_sys_cfg_radio_ctl, "Set/get uAP radio on/off"},
    {"sys_cfg_tx_data_rate", apcmd_sys_cfg_tx_data_rate, "Set/get uAP tx rate"},
    {"sys_cfg_mcbc_data_rate", apcmd_sys_cfg_mcbc_data_rate,
     "Set/get uAP MCBC rate"},
    {"sys_cfg_rsn_replay_prot", apcmd_sys_cfg_rsn_replay_prot,
     "Set/get RSN replay protection"},
    {"sys_cfg_pkt_fwd_ctl", apcmd_sys_cfg_pkt_fwd_ctl,
     "Set/get uAP packet forwarding"},
    {"sys_cfg_sta_ageout_timer", apcmd_sys_cfg_sta_ageout_timer,
     "Set/get station ageout timer"},
    {"sys_cfg_auth", apcmd_sys_cfg_auth, "\tSet/get uAP authentication mode"},
    {"sys_cfg_protocol", apcmd_sys_cfg_protocol,
     "Set/get uAP security protocol"},
    {"sys_cfg_wep_key", apcmd_sys_cfg_wep_key, "\tSet/get uAP wep key"},
    {"sys_cfg_cipher", apcmd_sys_cfg_cipher, "\tSet/get uAP WPA/WPA cipher"},
    {"sys_cfg_wpa_passphrase", apcmd_sys_cfg_wpa_passphrase,
     "Set/get uAP WPA or WPA2 passphrase"},
    {"sys_cfg_group_rekey_timer", apcmd_sys_cfg_group_rekey_timer,
     "Set/get uAP group re-key time"},
    {"sys_cfg_max_sta_num", apcmd_sys_cfg_max_sta_num,
     "Set/get uAP max station number"},
    {"sys_cfg_retry_limit", apcmd_sys_cfg_retry_limit,
     "Set/get uAP retry limit number"},
    {"sys_cfg_custom_ie", apcmd_sys_cfg_custom_ie,
     "\tSet/get custom IE configuration"},
    {"sta_filter_table", apcmd_sta_filter_table, "Set/get uAP mac filter"},
    {"regrdwr", apcmd_regrdwr, "\t\tRead/Write register command"},
    {"memaccess", apcmd_memaccess, "\tRead/Write to a memory address command"},
    {"rdeeprom", apcmd_read_eeprom, "\tRead EEPROM "},
    {"cfg_data", apcmd_cfg_data,
     "\tGet/Set configuration file from/to firmware"},
    {"sys_debug", apcmd_sys_debug, "\tSet/Get debug parameter"},
    {"sys_cfg_80211d", apcmd_cfg_80211d, "\tSet/Get 802.11D info"},
    {"uap_stats", apcmd_uap_stats, "\tGet uAP stats"},
    {"powermode", apcmd_power_mode, "\tSet/get uAP power mode"},
    {"coex_config", apcmd_coex_config, "\tSet/get uAP BT coex configuration"},
    {NULL, NULL, 0}
};

/** 
 *  @brief Prints usage information of uaputl
 *
 *  @return          N/A
 */
static void
print_tool_usage(void)
{
    int i;
    printf("uaputl.exe - uAP utility ver %s\n", UAP_VERSION);
    printf("Usage:\n"
           "\tuaputl.exe [options] <command> [command parameters]\n");
    printf("Options:\n"
           "\t--help\tDisplay help\n"
           "\t-v\tDisplay version\n"
           "\t-i <interface>\n" "\t-d <debug_level=0|1|2>\n");
    printf("Commands:\n");
    for (i = 0; ap_command[i].cmd; i++)
        printf("\t%-4s\t\t%s\n", ap_command[i].cmd, ap_command[i].help);
    printf("\n"
           "For more information on the usage of each command use:\n"
           "\tuaputl.exe <command> --help\n");
}

/****************************************************************************
        Global functions
****************************************************************************/
/** option parameter*/
static struct option ap_options[] = {
    {"help", 0, NULL, 'h'},
    {"interface", 1, NULL, 'i'},
    {"debug", 1, NULL, 'd'},
    {"version", 0, NULL, 'v'},
    {NULL, 0, NULL, '\0'}
};

/**
 *    @brief isdigit for String.
 *   
 *    @param x            char string
 *    @return             UAP_FAILURE for non-digit.
 *                        UAP_SUCCESS for digit
 */
inline int
ISDIGIT(char *x)
{
    int i;
    for (i = 0; i < strlen(x); i++)
        if (isdigit(x[i]) == 0)
            return UAP_FAILURE;
    return UAP_SUCCESS;
}

/** 
 *  @brief Checkes a particular input for validatation.
 *
 *  @param cmd      Type of input
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
is_input_valid(valid_inputs cmd, int argc, char *argv[])
{
    int i;
    int ret = UAP_SUCCESS;
    if (argc == 0)
        return UAP_FAILURE;
    switch (cmd) {
    case RDEEPROM:
        if (argc != 2) {
            printf(" ERR: Argument count mismatch\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (ISDIGIT(argv[1]) == 0) ||
                (A2HEXDECIMAL(argv[0]) & 0x03) ||
                (A2HEXDECIMAL(argv[0]) < 0) ||
                (A2HEXDECIMAL(argv[1]) & 0x03) ||
                (A2HEXDECIMAL(argv[1]) < 4) || (A2HEXDECIMAL(argv[1]) > 20)) {
                printf(" ERR: Invalid inputs for Read EEPROM\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case SCANCHANNELS:
        if (argc > MAX_CHANNELS) {
            printf("ERR: Invalid List of Channels\n");
            ret = UAP_FAILURE;
        } else {
            for (i = 0; i < argc; i++) {
                if ((ISDIGIT(argv[i]) == 0) || (atoi(argv[i]) < 1) ||
                    (atoi(argv[i]) > MAX_CHANNELS)) {
                    printf("ERR: Channel must be in the range of 1 to %d\n",
                           MAX_CHANNELS);
                    ret = UAP_FAILURE;
                    break;
                }
            }
            if ((ret != UAP_FAILURE) &&
                (has_dup_channel(argc, argv) != UAP_SUCCESS)) {
                printf("ERR: Duplicate channel values entered\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case TXPOWER:
        if ((argc > 1) || (ISDIGIT(argv[0]) == 0)) {
            printf("ERR:Invalid Transmit power\n");
            ret = UAP_FAILURE;
        } else {
            if ((atoi(argv[0]) < MIN_TX_POWER) ||
                (atoi(argv[0]) > MAX_TX_POWER)) {
                printf("ERR: TX Powar must be in the rage of %d to %d. \n",
                       MIN_TX_POWER, MAX_TX_POWER);
                ret = UAP_FAILURE;
            }
        }
        break;
    case PROTOCOL:
        if ((argc > 1) || (ISDIGIT(argv[0]) == 0)) {
            printf("ERR:Invalid Protocol\n");
            ret = UAP_FAILURE;
        } else
            ret = is_protocol_valid(atoi(argv[0]));
        break;
    case CHANNEL:
        if ((argc != 1) && (argc != 2)) {
            printf("ERR: Incorrect arguments for channel.\n");
            ret = UAP_FAILURE;
        } else {
            if (argc == 2) {
                if ((ISDIGIT(argv[1]) == 0) || (atoi(argv[1]) < 0) ||
                    (atoi(argv[1]) > 1)) {
                    printf("ERR: MODE must be either 0 or 1\n");
                    ret = UAP_FAILURE;
                }
                if ((atoi(argv[1]) == 1) && (atoi(argv[0]) != 0)) {
                    printf("ERR: Channel must be 0 for ACS; MODE = 1.\n");
                    ret = UAP_FAILURE;
                }
            }
            if ((argc == 1) || (atoi(argv[1]) == 0)) {
                if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 1) ||
                    (atoi(argv[0]) > MAX_CHANNELS)) {
                    printf("ERR: Channel must be in the range of 1 to %d\n",
                           MAX_CHANNELS);
                    ret = UAP_FAILURE;
                }
            }
        }
        break;
    case RATE:
        if (argc > MAX_RATES) {
            printf("ERR: Incorrect number of RATES arguments.\n");
            ret = UAP_FAILURE;
        } else {
            for (i = 0; i < argc; i++) {
                if ((IS_HEX_OR_DIGIT(argv[i]) == UAP_FAILURE) ||
                    (is_rate_valid(A2HEXDECIMAL(argv[i]) & ~BASIC_RATE_SET_BIT)
                     != UAP_SUCCESS)) {
                    printf("ERR:Unsupported rate.\n");
                    ret = UAP_FAILURE;
                    break;
                }
            }
            if ((ret != UAP_FAILURE) &&
                (has_dup_rate(argc, argv) != UAP_SUCCESS)) {
                printf("ERR: Duplicate rate values entered\n");
                ret = UAP_FAILURE;
            }
            if (check_mandatory_rates(argc, argv) != UAP_SUCCESS) {
                ret = UAP_FAILURE;
            }
        }
        break;
    case BROADCASTSSID:
        if (argc != 1) {
            printf("ERR:wrong BROADCASTSSID arguments.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) ||
                ((atoi(argv[0]) != 0) && (atoi(argv[0]) != 1))) {
                printf
                    ("ERR:Illegal parameter %s for BROADCASTSSID. Must be either '0' or '1'.\n",
                     argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case RTSTHRESH:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for RTSTHRESHOLD\n");
            ret = UAP_FAILURE;
        } else if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                   (atoi(argv[0]) > MAX_RTS_THRESHOLD)) {
            printf
                ("ERR:Illegal RTSTHRESHOLD %s. The value must between 0 and %d\n",
                 argv[0], MAX_RTS_THRESHOLD);
            ret = UAP_FAILURE;
        }
        break;
    case FRAGTHRESH:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for FRAGTHRESH\n");
            ret = UAP_FAILURE;
        } else if ((ISDIGIT(argv[0]) == 0) ||
                   (atoi(argv[0]) < MIN_FRAG_THRESHOLD) ||
                   (atoi(argv[0]) > MAX_FRAG_THRESHOLD)) {
            printf
                ("ERR:Illegal FRAGTHRESH %s. The value must between %d and %d\n",
                 argv[0], MIN_FRAG_THRESHOLD, MAX_FRAG_THRESHOLD);
            ret = UAP_FAILURE;
        }
        break;
    case DTIMPERIOD:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for DTIMPERIOD\n");
            ret = UAP_FAILURE;
        } else if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 1) ||
                   (atoi(argv[0]) > MAX_DTIM_PERIOD)) {
            printf("ERR: DTIMPERIOD Value must be in range of 1 to %d\n",
                   MAX_DTIM_PERIOD);
            ret = UAP_FAILURE;
        }
        break;
    case RADIOCONTROL:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for RADIOCONTROL\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                (atoi(argv[0]) > 1)) {
                printf
                    ("ERR:Illegal RADIOCONTROL parameter %s. Must be either '0' or '1'.\n",
                     argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case RSNREPLAYPROT:
        if (argc != 1) {
            printf("ERR:wrong RSNREPLAYPROT arguments.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) ||
                ((atoi(argv[0]) != 0) && (atoi(argv[0]) != 1))) {
                printf
                    ("ERR:Illegal parameter %s for RSNREPLAYPROT. Must be either '0' or '1'.\n",
                     argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case MCBCDATARATE:
    case TXDATARATE:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for DATARATE\n");
            ret = UAP_FAILURE;
        } else {
            if (IS_HEX_OR_DIGIT(argv[0]) == UAP_FAILURE) {
                printf("ERR: invalid data rate\n");
                ret = UAP_FAILURE;
            } else if ((A2HEXDECIMAL(argv[0]) != 0) &&
                       (is_rate_valid
                        (A2HEXDECIMAL(argv[0]) & ~BASIC_RATE_SET_BIT) !=
                        UAP_SUCCESS)) {
                printf("ERR: invalid data rate\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case PKTFWD:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for PKTFWD.\n");
            ret = UAP_FAILURE;
        } else if ((ISDIGIT(argv[0]) == 0) ||
                   ((atoi(argv[0]) != 0) && (atoi(argv[0]) != 1))) {
            printf
                ("ERR:Illegal PKTFWD parameter %s. Must be either '0' or '1'.\n",
                 argv[0]);
            ret = UAP_FAILURE;
        }
        break;
    case STAAGEOUTTIMER:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for STAAGEOUTTIMER.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || ((atoi(argv[0]) != 0) &&
                                            ((atoi(argv[0]) <
                                              MIN_STAGE_OUT_TIME) ||
                                             (atoi(argv[0]) >
                                              MAX_STAGE_OUT_TIME)))) {
                printf
                    ("ERR:Illegal STAAGEOUTTIMER %s. Must be between %d and %d.\n",
                     argv[0], MIN_STAGE_OUT_TIME, MAX_STAGE_OUT_TIME);
                ret = UAP_FAILURE;
            }
        }
        break;
    case AUTHMODE:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for AUTHMODE\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                (atoi(argv[0]) > 1)) {
                printf
                    ("ERR:Illegal AUTHMODE parameter %s. Must be either '0', or '1'.\n",
                     argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case GROUPREKEYTIMER:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for GROUPREKEYTIMER.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                (atoi(argv[0]) > MAX_GRP_TIMER)) {
                printf("ERR: GROUPREKEYTIMER range is [0:%d] (0 for disable)\n",
                       MAX_GRP_TIMER);
                ret = UAP_FAILURE;
            }
        }
        break;
    case MAXSTANUM:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for MAXSTANUM\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) > 8) ||
                (atoi(argv[0]) < 0)) {
                printf("ERR:STA_NUM must be in the range of [0:8] %s.\n",
                       argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case BEACONPERIOD:
        if (argc != 1) {
            printf("ERR:Incorrect number of argument for BEACONPERIOD.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < MIN_BEACON_PERIOD)
                || (atoi(argv[0]) > MAX_BEACON_PERIOD)) {
                printf("ERR: BEACONPERIOD must be in range of %d to %d.\n",
                       MIN_BEACON_PERIOD, MAX_BEACON_PERIOD);
                ret = UAP_FAILURE;
            }
        }
        break;
    case RETRYLIMIT:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for RETRY LIMIT\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) > MAX_RETRY_LIMIT) ||
                (atoi(argv[0]) < 0)) {
                printf
                    ("ERR:RETRY_LIMIT must be in the range of [0:%d]. The  input was %s.\n",
                     MAX_RETRY_LIMIT, argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    case COEX_COMM_BITMAP:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for Bitmap.\n");
            ret = UAP_FAILURE;
        } else {
            /* Only bit 0 is supported now, hence check for 1 or 0 */
            if ((IS_HEX_OR_DIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                (atoi(argv[0]) > 1)) {
                printf("ERR: Bitmap must have value of 1 or 0.\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case COEX_SCO_ACL_FREQ:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for aclFrequency.\n");
            ret = UAP_FAILURE;
        } else {
            if (ISDIGIT(argv[0]) == 0) {
                printf("ERR: Incorrect value for aclFrequency.\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case COEX_ACL_ENABLED:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for (acl) enabled.\n");
            ret = UAP_FAILURE;
        } else {
            if ((ISDIGIT(argv[0]) == 0) || (atoi(argv[0]) < 0) ||
                (atoi(argv[0]) > 1)) {
                printf("ERR: (acl) enabled must have value of 1 or 0.\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case COEX_ACL_BT_TIME:
    case COEX_ACL_WLAN_TIME:
        if (argc != 1) {
            printf("ERR:Incorrect number of arguments for bt/wlan time.\n");
            ret = UAP_FAILURE;
        } else {
            if (ISDIGIT(argv[0]) == 0) {
                printf("ERR: Incorrect value for bt/wlan time.\n");
                ret = UAP_FAILURE;
            }
        }
        break;
    case COEX_PROTECTION:
        if (argc != 2) {
            printf("ERR:Incorrect number of arguments for %s.\n", argv[0]);
            ret = UAP_FAILURE;
        } else {
            if (ISDIGIT(argv[1]) == 0) {
                printf("ERR: Incorrect value for %s.\n", argv[0]);
                ret = UAP_FAILURE;
            }
        }
        break;
    default:
        ret = UAP_FAILURE;
        break;
    }
    return ret;
}

/** 
 *  @brief Converts colon separated MAC address to hex value
 *
 *  @param mac      A pointer to the colon separated MAC string
 *  @param raw      A pointer to the hex data buffer
 *  @return         UAP_SUCCESS or UAP_FAILURE
 *                  UAP_RET_MAC_BROADCAST  - if broadcast mac
 *                  UAP_RET_MAC_MULTICAST - if multicast mac
 */
int
mac2raw(char *mac, u8 * raw)
{
    unsigned int temp_raw[ETH_ALEN];
    int num_tokens = 0;
    int i;
    if (strlen(mac) != ((2 * ETH_ALEN) + (ETH_ALEN - 1))) {
        return UAP_FAILURE;
    }
    num_tokens = sscanf(mac, "%2x:%2x:%2x:%2x:%2x:%2x",
                        temp_raw + 0, temp_raw + 1, temp_raw + 2, temp_raw + 3,
                        temp_raw + 4, temp_raw + 5);
    if (num_tokens != ETH_ALEN) {
        return UAP_FAILURE;
    }
    for (i = 0; i < num_tokens; i++)
        raw[i] = (u8) temp_raw[i];

    if (memcmp(raw, "\xff\xff\xff\xff\xff\xff", ETH_ALEN) == 0) {
        return UAP_RET_MAC_BROADCAST;
    } else if (raw[0] & 0x01) {
        return UAP_RET_MAC_MULTICAST;
    }
    return UAP_SUCCESS;
}

/** 
 *  @brief Converts a string to hex value
 *
 *  @param str      A pointer to the string
 *  @param raw      A pointer to the raw data buffer
 *  @return         Number of bytes read
 */
int
string2raw(char *str, unsigned char *raw)
{
    int len = (strlen(str) + 1) / 2;

    do {
        if (!isxdigit(*str)) {
            return -1;
        }
        *str = toupper(*str);
        *raw = CHAR2INT(*str) << 4;
        ++str;
        *str = toupper(*str);
        if (*str == '\0')
            break;
        *raw |= CHAR2INT(*str);
        ++raw;
    } while (*++str != '\0');
    return len;
}

/** 
 *  @brief Prints a MAC address in colon separated form from hex data
 *
 *  @param raw      A pointer to the hex data buffer
 *  @return         N/A
 */
void
print_mac(u8 * raw)
{
    printf("%02x:%02x:%02x:%02x:%02x:%02x", (unsigned int) raw[0],
           (unsigned int) raw[1], (unsigned int) raw[2], (unsigned int) raw[3],
           (unsigned int) raw[4], (unsigned int) raw[5]);
    return;
}

/** 
 *  @brief 		check hex string
 *  
 *  @param hex		A pointer to hex string
 *  @return      	UAP_SUCCESS or UAP_FAILURE
 */
int
ishexstring(void *hex)
{
    int i, a;
    char *p = hex;
    int len = strlen(p);
    if (!strncasecmp("0x", p, 2)) {
        p += 2;
        len -= 2;
    }
    for (i = 0; i < len; i++) {
        a = hex2num(*p);
        if (a < 0)
            return UAP_FAILURE;
        p++;
    }
    return UAP_SUCCESS;
}

/**
 *  @brief Show auth tlv 
 *
 *  @param tlv     Poniter to auth tlv
 *  
 *  $return         N/A
 */
void
print_auth(TLVBUF_AUTH_MODE * tlv)
{
    switch (tlv->AuthMode) {
    case 0:
        printf("AUTHMODE = Open authentication\n");
        break;
    case 1:
        printf("AUTHMODE = Shared key authentication\n");
        break;
    case 2:
        printf("AUTHMODE = Auto (open and shared key)\n");
        break;
    default:
        printf("ERR: Invalid authmode=%d\n", tlv->AuthMode);
        break;
    }
}

/**
 *
 *  @brief Show cipher tlv 
 *
 *  @param tlv     Poniter to cipher tlv
 *  
 *  $return         N/A
 */
void
print_cipher(TLVBUF_CIPHER * tlv)
{
    switch (tlv->PairwiseCipher) {
    case CIPHER_TKIP:
        printf("PairwiseCipher = TKIP\n");
        break;
    case CIPHER_AES_CCMP:
        printf("PairwiseCipher = AES CCMP\n");
        break;
    case CIPHER_TKIP | CIPHER_AES_CCMP:
        printf("PairwiseCipher = TKIP + AES CCMP\n");
        break;
    case CIPHER_NONE:
        printf("PairwiseCipher =  None\n");
        break;
    default:
        printf("Unknown Pairwise cipher 0x%x\n", tlv->PairwiseCipher);
        break;
    }
    switch (tlv->GroupCipher) {
    case CIPHER_TKIP:
        printf("GroupCipher = TKIP\n");
        break;
    case CIPHER_AES_CCMP:
        printf("GroupCipher = AES CCMP\n");
        break;
    case CIPHER_NONE:
        printf("GroupCipher = None\n");
        break;
    default:
        printf("Unknown Group cipher 0x%x\n", tlv->GroupCipher);
        break;
    }
}

/**
 *  @brief Show mac filter tlv 
 *
 *  @param tlv     Poniter to filter tlv
 *  
 *  $return         N/A
 */
void
print_mac_filter(TLVBUF_STA_MAC_ADDR_FILTER * tlv)
{
    int i;
    switch (tlv->FilterMode) {
    case 0:
        printf("Filter Mode = Filter table is disabled\n");
        return;
    case 1:
        printf
            ("Filter Mode = Allow mac address specified in the allwed list\n");
        break;
    case 2:
        printf
            ("Filter Mode = Block MAC addresses specified in the  banned list\n");
        break;
    }
    for (i = 0; i < tlv->Count; i++) {
        printf("MAC_%d = ", i);
        print_mac(&tlv->MacAddress[i * ETH_ALEN]);
        printf("\n");
    }
}

/**
 *  @brief Show rate tlv 
 *
 *  @param tlv      Poniter to rate tlv
 *  
 *  $return         N/A
 */
void
print_rate(TLVBUF_RATES * tlv)
{
    int flag = 0;
    int i;
    printf("Basic Rates =");
    for (i = 0; i < tlv->Length; i++) {
        if (tlv->OperationalRates[i] > (BASIC_RATE_SET_BIT - 1)) {
            flag = flag ? : 1;
            printf(" 0x%x", tlv->OperationalRates[i]);
        }
    }
    printf("%s\nNon-Basic Rates =", flag ? "" : " ( none ) ");
    for (flag = 0, i = 0; i < tlv->Length; i++) {
        if (tlv->OperationalRates[i] < BASIC_RATE_SET_BIT) {
            flag = flag ? : 1;
            printf(" 0x%x", tlv->OperationalRates[i]);
        }
    }
    printf("%s\n", flag ? "" : " ( none ) ");
}

/**
 *  @brief Show all the tlv in the buf
 *
 *  @param buf     Poniter to tlv buffer
 *  @param len     tlv buffer len
 *  
 *  $return         N/A
 */
void
print_tlv(u8 * buf, u16 len)
{
    TLVBUF_HEADER *pCurrentTlv = (TLVBUF_HEADER *) buf;
    int tlvBufLeft = len;
    u16 tlvType;
    u16 tlvLen;
    u16 custom_ie_len;
    u8 ssid[33];
    int i = 0;
    TLVBUF_AP_MAC_ADDRESS *mac_tlv;
    TLVBUF_SSID *ssid_tlv;
    TLVBUF_BEACON_PERIOD *beacon_tlv;
    TLVBUF_DTIM_PERIOD *dtim_tlv;
    TLVBUF_RATES *rates_tlv;
    TLVBUF_TX_POWER *txpower_tlv;
    TLVBUF_BCAST_SSID_CTL *bcast_tlv;
    TLVBUF_PREAMBLE_CTL *preamble_tlv;
    TLVBUF_ANTENNA_CTL *antenna_tlv;
    TLVBUF_RTS_THRESHOLD *rts_tlv;
    TLVBUF_RADIO_CTL *radio_tlv;
    TLVBUF_TX_DATA_RATE *txrate_tlv;
    TLVBUF_MCBC_DATA_RATE *mcbcrate_tlv;
    TLVBUF_PKT_FWD_CTL *pkt_fwd_tlv;
    TLVBUF_STA_AGEOUT_TIMER *ageout_tlv;
    TLVBUF_AUTH_MODE *auth_tlv;
    TLVBUF_PROTOCOL *proto_tlv;
    TLVBUF_AKMP *akmp_tlv;
    TLVBUF_CIPHER *cipher_tlv;
    TLVBUF_GROUP_REKEY_TIMER *rekey_tlv;
    TLVBUF_WPA_PASSPHRASE *psk_tlv;
    TLVBUF_WEP_KEY *wep_tlv;
    TLVBUF_FRAG_THRESHOLD *frag_tlv;
    TLVBUF_STA_MAC_ADDR_FILTER *filter_tlv;
    TLVBUF_MAX_STA_NUM *max_sta_tlv;
    TLVBUF_RETRY_LIMIT *retry_limit_tlv;
    TLVBUF_CHANNEL_CONFIG *channel_tlv;
    TLVBUF_CHANNEL_LIST *chnlist_tlv;
    tlvbuf_custom_ie *custom_ie_tlv;
    custom_ie *custom_ie_ptr;
    tlvbuf_coex_common_cfg *coex_common_tlv;
    tlvbuf_coex_sco_cfg *coex_sco_tlv;
    tlvbuf_coex_acl_cfg *coex_acl_tlv;
    tlvbuf_coex_stats *coex_stats_tlv;
    CHANNEL_LIST *pChanList;
#ifdef DEBUG
    uap_printf(MSG_DEBUG, "tlv total len=%d\n", len);
#endif
    while (tlvBufLeft >= (int) sizeof(TLVBUF_HEADER)) {
        tlvType = uap_le16_to_cpu(pCurrentTlv->Type);
        tlvLen = uap_le16_to_cpu(pCurrentTlv->Len);
        if ((sizeof(TLVBUF_HEADER) + tlvLen) > tlvBufLeft) {
            printf("wrong tlv: tlvLen=%d, tlvBufLeft=%d\n", tlvLen, tlvBufLeft);
            break;
        }
        switch (tlvType) {
        case MRVL_AP_MAC_ADDRESS_TLV_ID:
            mac_tlv = (TLVBUF_AP_MAC_ADDRESS *) pCurrentTlv;
            printf("AP MAC address = ");
            print_mac(mac_tlv->ApMacAddr);
            printf("\n");
            break;
        case MRVL_SSID_TLV_ID:
            memset(ssid, 0, sizeof(ssid));
            ssid_tlv = (TLVBUF_SSID *) pCurrentTlv;
            memcpy(ssid, ssid_tlv->Ssid, ssid_tlv->Length);
            printf("SSID = %s\n", ssid);
            break;
        case MRVL_BEACON_PERIOD_TLV_ID:
            beacon_tlv = (TLVBUF_BEACON_PERIOD *) pCurrentTlv;
            beacon_tlv->BeaconPeriod_ms =
                uap_le16_to_cpu(beacon_tlv->BeaconPeriod_ms);
            printf("Beacon period = %d\n", beacon_tlv->BeaconPeriod_ms);
            break;
        case MRVL_DTIM_PERIOD_TLV_ID:
            dtim_tlv = (TLVBUF_DTIM_PERIOD *) pCurrentTlv;
            printf("DTIM period = %d\n", dtim_tlv->DtimPeriod);
            break;
        case MRVL_CHANNELCONFIG_TLV_ID:
            channel_tlv = (TLVBUF_CHANNEL_CONFIG *) pCurrentTlv;
            printf("Channel = %d\n", channel_tlv->ChanNumber);
            printf("Channel Select Mode = %s\n",
                   (channel_tlv->BandConfigType == 0) ? "Manual" : "ACS");
            break;
        case MRVL_CHANNELLIST_TLV_ID:
            chnlist_tlv = (TLVBUF_CHANNEL_LIST *) pCurrentTlv;
            printf("Channels List = ");
            pChanList = (CHANNEL_LIST *) & (chnlist_tlv->ChanList);
            if (chnlist_tlv->Length % sizeof(CHANNEL_LIST)) {
                break;
            }
            for (i = 0; i < (chnlist_tlv->Length / sizeof(CHANNEL_LIST)); i++) {
                printf("%d ", pChanList->ChanNumber);
                pChanList++;
            }
            printf("\n");
            break;
        case MRVL_RATES_TLV_ID:
            rates_tlv = (TLVBUF_RATES *) pCurrentTlv;
            print_rate(rates_tlv);
            break;
        case MRVL_TX_POWER_TLV_ID:
            txpower_tlv = (TLVBUF_TX_POWER *) pCurrentTlv;
            printf("Tx power = %d dBm\n", txpower_tlv->TxPower_dBm);
            break;
        case MRVL_BCAST_SSID_CTL_TLV_ID:
            bcast_tlv = (TLVBUF_BCAST_SSID_CTL *) pCurrentTlv;
            printf("SSID broadcast = %s\n",
                   (bcast_tlv->BcastSsidCtl == 1) ? "enabled" : "disabled");
            break;
        case MRVL_PREAMBLE_CTL_TLV_ID:
            preamble_tlv = (TLVBUF_PREAMBLE_CTL *) pCurrentTlv;
            printf("Preamble type = %s\n", (preamble_tlv->PreambleType == 0) ?
                   "auto" : ((preamble_tlv->PreambleType == 1) ? "short" :
                             "long"));
            break;
        case MRVL_ANTENNA_CTL_TLV_ID:
            antenna_tlv = (TLVBUF_ANTENNA_CTL *) pCurrentTlv;
            printf("%s antenna = %s\n", (antenna_tlv->WhichAntenna == 0) ?
                   "Rx" : "Tx", (antenna_tlv->AntennaMode == 0) ? "A" : "B");
            break;
        case MRVL_RTS_THRESHOLD_TLV_ID:
            rts_tlv = (TLVBUF_RTS_THRESHOLD *) pCurrentTlv;
            rts_tlv->RtsThreshold = uap_le16_to_cpu(rts_tlv->RtsThreshold);
            printf("RTS threshold = %d\n", rts_tlv->RtsThreshold);
            break;
        case MRVL_FRAG_THRESHOLD_TLV_ID:
            frag_tlv = (TLVBUF_FRAG_THRESHOLD *) pCurrentTlv;
            frag_tlv->FragThreshold = uap_le16_to_cpu(frag_tlv->FragThreshold);
            printf("Fragmentation threshold = %d\n", frag_tlv->FragThreshold);
            break;
        case MRVL_RADIO_CTL_TLV_ID:
            radio_tlv = (TLVBUF_RADIO_CTL *) pCurrentTlv;
            printf("Radio = %s\n", (radio_tlv->RadioCtl == 0) ? "on" : "off");
            break;
        case MRVL_TX_DATA_RATE_TLV_ID:
            txrate_tlv = (TLVBUF_TX_DATA_RATE *) pCurrentTlv;
            txrate_tlv->TxDataRate = uap_le16_to_cpu(txrate_tlv->TxDataRate);
            if (txrate_tlv->TxDataRate == 0)
                printf("Tx data rate = auto\n");
            else
                printf("Tx data rate = 0x%x\n", txrate_tlv->TxDataRate);
            break;
        case MRVL_MCBC_DATA_RATE_TLV_ID:
            mcbcrate_tlv = (TLVBUF_MCBC_DATA_RATE *) pCurrentTlv;
            mcbcrate_tlv->MCBCdatarate =
                uap_le16_to_cpu(mcbcrate_tlv->MCBCdatarate);
            if (mcbcrate_tlv->MCBCdatarate == 0)
                printf("MCBC data rate = auto\n");
            else
                printf("MCBC data rate = 0x%x\n", mcbcrate_tlv->MCBCdatarate);
            break;
        case MRVL_PKT_FWD_CTL_TLV_ID:
            pkt_fwd_tlv = (TLVBUF_PKT_FWD_CTL *) pCurrentTlv;
            printf("Firmware = %s\n", (pkt_fwd_tlv->PktFwdCtl == 0) ?
                   "forwards all packets to the host" :
                   "handles intra-BSS packets");
            break;
        case MRVL_STA_AGEOUT_TIMER_TLV_ID:
            ageout_tlv = (TLVBUF_STA_AGEOUT_TIMER *) pCurrentTlv;
            ageout_tlv->StaAgeoutTimer_ms =
                uap_le32_to_cpu(ageout_tlv->StaAgeoutTimer_ms);
            printf("STA ageout timer = %d\n",
                   (int) ageout_tlv->StaAgeoutTimer_ms);
            break;
        case MRVL_AUTH_TLV_ID:
            auth_tlv = (TLVBUF_AUTH_MODE *) pCurrentTlv;
            print_auth(auth_tlv);
            break;
        case MRVL_PROTOCOL_TLV_ID:
            proto_tlv = (TLVBUF_PROTOCOL *) pCurrentTlv;
            proto_tlv->Protocol = uap_le16_to_cpu(proto_tlv->Protocol);
            print_protocol(proto_tlv);
            break;
        case MRVL_AKMP_TLV_ID:
            akmp_tlv = (TLVBUF_AKMP *) pCurrentTlv;
            if (uap_le16_to_cpu(akmp_tlv->KeyMgmt) == KEY_MGMT_PSK)
                printf("KeyMgmt = PSK\n");
            else
                printf("KeyMgmt = NONE\n");
            break;
        case MRVL_CIPHER_TLV_ID:
            cipher_tlv = (TLVBUF_CIPHER *) pCurrentTlv;
            print_cipher(cipher_tlv);
            break;
        case MRVL_GRP_REKEY_TIME_TLV_ID:
            rekey_tlv = (TLVBUF_GROUP_REKEY_TIMER *) pCurrentTlv;
            if (rekey_tlv->GroupRekeyTime_sec == 0)
                printf("Group re-key time = disabled\n");
            else
                printf("Group re-key time = %ld second\n",
                       uap_le32_to_cpu(rekey_tlv->GroupRekeyTime_sec));
            break;
        case MRVL_WPA_PASSPHRASE_TLV_ID:
            psk_tlv = (TLVBUF_WPA_PASSPHRASE *) pCurrentTlv;
            if (psk_tlv->Length > 0) {
                printf("WPA passphrase = ");
                for (i = 0; i < psk_tlv->Length; i++)
                    printf("%c", psk_tlv->Passphrase[i]);
                printf("\n");
            } else
                printf("WPA passphrase = None\n");
            break;
        case MRVL_WEP_KEY_TLV_ID:
            wep_tlv = (TLVBUF_WEP_KEY *) pCurrentTlv;
            print_wep_key(wep_tlv);
            break;
        case MRVL_STA_MAC_ADDR_FILTER_TLV_ID:
            filter_tlv = (TLVBUF_STA_MAC_ADDR_FILTER *) pCurrentTlv;
            print_mac_filter(filter_tlv);
            break;
        case MRVL_MAX_STA_CNT_TLV_ID:
            max_sta_tlv = (TLVBUF_MAX_STA_NUM *) pCurrentTlv;
            printf("Max Station Number = %d\n", max_sta_tlv->Max_sta_num);
            break;
        case MRVL_RETRY_LIMIT_TLV_ID:
            retry_limit_tlv = (TLVBUF_RETRY_LIMIT *) pCurrentTlv;
            printf("Retry Limit = %d\n", retry_limit_tlv->retry_limit);
            break;
        case MRVL_MGMT_IE_LIST_TLV_ID:
            custom_ie_tlv = (tlvbuf_custom_ie *) pCurrentTlv;
            custom_ie_len = tlvLen;
            custom_ie_ptr = (custom_ie *) (custom_ie_tlv->ie_data);
            while (custom_ie_len >= sizeof(custom_ie)) {
                printf("Index [%d]\n",
                       uap_le16_to_cpu(custom_ie_ptr->ie_index));
                printf("Management Subtype Mask = 0x%02x\n",
                       uap_le16_to_cpu(custom_ie_ptr->mgmt_subtype_mask));
                hexdump_data("IE Buffer", (void *) custom_ie_ptr->ie_buffer,
                             uap_le16_to_cpu(custom_ie_ptr->ie_length), ' ');
                custom_ie_len -=
                    sizeof(custom_ie) +
                    uap_le16_to_cpu(custom_ie_ptr->ie_length);
                custom_ie_ptr =
                    (custom_ie *) ((u8 *) custom_ie_ptr + sizeof(custom_ie) +
                                   uap_le16_to_cpu(custom_ie_ptr->ie_length));
            }
            break;
        case MRVL_BT_COEX_COMMON_CFG_TLV_ID:
            printf("Coex common configuration:\n");
            coex_common_tlv = (tlvbuf_coex_common_cfg *) pCurrentTlv;
            printf("\tConfig Bitmap = 0x%02lx\n",
                   uap_le32_to_cpu(coex_common_tlv->config_bitmap));
            break;

        case MRVL_BT_COEX_SCO_CFG_TLV_ID:
            printf("Coex sco configuration:\n");
            coex_sco_tlv = (tlvbuf_coex_sco_cfg *) pCurrentTlv;
            for (i = 0; i < 4; i++)
                printf("\tQtime protection [%d] = %d usecs\n", i,
                       uap_le16_to_cpu(coex_sco_tlv->protection_qtime[i]));
            printf("\tProtection frame rate = %d\n",
                   uap_le16_to_cpu(coex_sco_tlv->protection_rate));
            printf("\tACL frequency = %d\n",
                   uap_le16_to_cpu(coex_sco_tlv->acl_frequency));
            break;

        case MRVL_BT_COEX_ACL_CFG_TLV_ID:
            printf("Coex acl configuration: ");
            coex_acl_tlv = (tlvbuf_coex_acl_cfg *) pCurrentTlv;
            coex_acl_tlv->enabled = uap_le16_to_cpu(coex_acl_tlv->enabled);
            printf("%s\n", (coex_acl_tlv->enabled) ? "enabled" : "disabled");
            if (coex_acl_tlv->enabled) {
                printf("\tBT time = %d usecs\n",
                       uap_le16_to_cpu(coex_acl_tlv->bt_time));
                printf("\tWLan time = %d usecs\n",
                       uap_le16_to_cpu(coex_acl_tlv->wlan_time));
                printf("\tProtection frame rate = %d\n",
                       uap_le16_to_cpu(coex_acl_tlv->protection_rate));
            }
            break;

        case MRVL_BT_COEX_STATS_TLV_ID:
            printf("Coex statistics: \n");
            coex_stats_tlv = (tlvbuf_coex_stats *) pCurrentTlv;
            printf("\tNull not sent = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->null_not_sent));
            printf("\tNull queued = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->null_queued));
            printf("\tNull not queued = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->null_not_queued));
            printf("\tCF End queued = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->cf_end_queued));
            printf("\tCF End not queued = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->cf_end_not_queued));
            printf("\tNull allocation failures = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->null_alloc_fail));
            printf("\tCF End allocation failures = %ld\n",
                   uap_le32_to_cpu(coex_stats_tlv->cf_end_alloc_fail));
            break;
        default:
            break;
        }
        tlvBufLeft -= (sizeof(TLVBUF_HEADER) + tlvLen);
        pCurrentTlv = (TLVBUF_HEADER *) (pCurrentTlv->Data + tlvLen);
    }
    return;
}

/** 
 *  @brief Performs the ioctl operation to send the command to
 *  the driver.
 *
 *  @param cmd        	 Pointer to the command buffer
 *  @param size          Pointer to the command size. This value is
 *                       overwritten by the function with the size of the
 *                       received response.
 *  @param buf_size 	 Size of the allocated command buffer
 *  @return              UAP_SUCCESS or UAP_FAILURE
 */
int
uap_ioctl(u8 * cmd, u16 * size, u16 buf_size)
{
    struct ifreq ifr;
    APCMDBUF *header = NULL;
    s32 sockfd;

    if (buf_size < *size) {
        printf("buf_size should not less than cmd buffer size\n");
        return UAP_FAILURE;
    }

    /* Open socket */
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("ERR:Cannot open socket\n");
        return UAP_FAILURE;
    }
    *(u32 *) cmd = buf_size - BUF_HEADER_SIZE;

    /* Initialize the ifr structure */
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_ifrn.ifrn_name, dev_name, strlen(dev_name));
    ifr.ifr_ifru.ifru_data = (void *) cmd;
    header = (APCMDBUF *) cmd;
    header->Size = *size - BUF_HEADER_SIZE;
    if (header->CmdCode == APCMD_SYS_CONFIGURE) {
        APCMDBUF_SYS_CONFIGURE *sys_cfg;
        sys_cfg = (APCMDBUF_SYS_CONFIGURE *) cmd;
        sys_cfg->Action = uap_cpu_to_le16(sys_cfg->Action);
    }
    endian_convert_request_header(header);
#if DEBUG
    /* Dump request buffer */
    hexdump("Request buffer", (void *) cmd, *size, ' ');
#endif
    /* Perform ioctl */
    errno = 0;
    if (ioctl(sockfd, UAPHOSTCMD, &ifr)) {
        perror("");
        printf("ERR:UAPHOSTCMD is not supported by %s\n", dev_name);
        close(sockfd);
        return UAP_FAILURE;
    }
    endian_convert_response_header(header);
    header->CmdCode &= HostCmd_CMD_ID_MASK;
    header->CmdCode |= APCMD_RESP_CHECK;
    *size = header->Size;

    /* Validate response size */
    if (*size > (buf_size - BUF_HEADER_SIZE)) {
        printf
            ("ERR:Response size (%d) greater than buffer size (%d)! Aborting!\n",
             *size, buf_size);
        close(sockfd);
        return UAP_FAILURE;
    }
#if DEBUG
    /* Dump respond buffer */
    hexdump("Respond buffer", (void *) header, header->Size + BUF_HEADER_SIZE,
            ' ');
#endif

    /* Close socket */
    close(sockfd);
    return UAP_SUCCESS;
}

/** 
 *  @brief check cipher is valid or not
 *
 *  @param pairwisecipher    pairwise cipher
 *  @param groupcipher       group cipher
 *  @return         UAP_SUCCESS or UAP_FAILURE
 */
int
is_cipher_valid(int pairwisecipher, int groupcipher)
{
    if ((pairwisecipher == CIPHER_NONE) && (groupcipher == CIPHER_NONE))
        return UAP_SUCCESS;
    if ((pairwisecipher == CIPHER_TKIP) && (groupcipher == CIPHER_TKIP))
        return UAP_SUCCESS;
    if ((pairwisecipher == CIPHER_AES_CCMP) && (groupcipher == CIPHER_AES_CCMP))
        return UAP_SUCCESS;
    if ((pairwisecipher == CIPHER_BITMAP) && (groupcipher == CIPHER_TKIP))
        return UAP_SUCCESS;
    return UAP_FAILURE;
}

/** 
 *  @brief The main function
 *
 *  @param argc     Number of arguments
 *  @param argv     Pointer to the arguments
 *  @return         0 or 1
 */
int
main(int argc, char *argv[])
{
    int opt, i;
    memset(dev_name, 0, sizeof(dev_name));
    strcpy(dev_name, DEFAULT_DEV_NAME);

    /* parse arguments */
    while ((opt = getopt_long(argc, argv, "+hi:d:v", ap_options, NULL)) != -1) {
        switch (opt) {
        case 'i':
            if (strlen(optarg) < IFNAMSIZ) {
                memset(dev_name, 0, sizeof(dev_name));
                strncpy(dev_name, optarg, strlen(optarg));
            }
            printf("dev_name:%s\n", dev_name);
            break;
        case 'v':
            printf("uaputl.exe - uAP utility ver %s\n", UAP_VERSION);
            exit(0);
        case 'd':
            debug_level = strtoul(optarg, NULL, 10);
            uap_printf(MSG_DEBUG, "debug_level=%x\n", debug_level);
            break;
        case 'h':
        default:
            print_tool_usage();
            exit(0);
        }
    }

    argc -= optind;
    argv += optind;
    optind = 0;

    if (argc < 1) {
        print_tool_usage();
        exit(1);
    }

    /* process command */
    for (i = 0; ap_command[i].cmd; i++) {
        if (strncmp(ap_command[i].cmd, argv[0], strlen(ap_command[i].cmd)))
            continue;
        if (strlen(ap_command[i].cmd) != strlen(argv[0]))
            continue;
        ap_command[i].func(argc, argv);
        break;
    }
    if (!ap_command[i].cmd) {
        printf("ERR: %s is not supported\n", argv[0]);
        exit(1);
    }
    return 0;
}
