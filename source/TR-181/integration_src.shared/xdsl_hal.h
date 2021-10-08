/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _XDSL_JSON_APIS_H
#define _XDSL_JSON_APIS_H

#include "xdsl_hal.h"
#include "json_hal_client.h"
#include "xtm_apis.h"
#include "xdsl_apis.h"

#define GET_PARAMETER_METHOD "getParameters"
#define SET_PARAMETER_METHOD "setParameters"

#define XTM_LINK_UP "Up"
#define XTM_LINK_DOWN "Down"
#define XTM_LINK_UNKNOWN "Unknown"
#define XTM_LINK_ERROR "Error"
#define XTM_LINK_LOWERLAYER_DOWN "LowerLayerDown"

#define JSON_RPC_PARAM_ARR_INDEX 0

/** Status of the DSL physical link */
typedef enum _DslLinkStatus_t {
    LINK_UP = 1,
    LINK_INITIALIZING,
    LINK_ESTABLISHING,
    LINK_NOSIGNAL,
    LINK_DISABLED,
    LINK_ERROR
}DslLinkStatus_t;

enum dsl_xtse_bit {
        /* Octet 1 - ADSL */
        T1_413                                  = 1,
        ETSI_101_388                            = 2,
        G_992_1_POTS_NON_OVERLAPPED             = 3, /* Annex A */
        G_992_1_POTS_OVERLAPPED                 = 4, /* Annex A */
        G_992_1_ISDN_NON_OVERLAPPED             = 5, /* Annex B */
        G_992_1_ISDN_OVERLAPPED                 = 6, /* Annex B */
        G_992_1_TCM_ISDN_NON_OVERLAPPED         = 7, /* Annex C */
        G_992_1_TCM_ISDN_OVERLAPPED             = 8, /* Annex C */

        /* Octet 2 - Splitter-less ADSL, i.e. g.lite */
        G_992_2_POTS_NON_OVERLAPPED             = 9, /* Annex A */
        G_992_2_POTS_OVERLAPPED                 = 10, /* Annex B */
        G_992_2_TCM_ISDN_NON_OVERLAPPED         = 11, /* Annex C */
        G_992_2_TCM_ISDN_OVERLAPPED             = 12, /* Annex C */
        /* Bits 13 - 16 are reserved */

        /* Octet 3 - ADSL2 */
        /* Bits 17 - 18 are reserved */
        G_992_3_POTS_NON_OVERLAPPED             = 19, /* Annex A */
        G_992_3_POTS_OVERLAPPED                 = 20, /* Annex A */
        G_992_3_ISDN_NON_OVERLAPPED             = 21, /* Annex B */
        G_992_3_ISDN_OVERLAPPED                 = 22, /* Annex B */
        G_992_3_TCM_ISDN_NON_OVERLAPPED         = 23, /* Annex C */
        G_992_3_TCM_ISDN_OVERLAPPED             = 24, /* Annex C */

        /* Octet 4 - Splitter-less ADSL2 and ADSL2 */
        G_992_4_POTS_NON_OVERLAPPED             = 25, /* Annex A */
        G_992_4_POTS_OVERLAPPED                 = 26, /* Annex A */
        /* Bits 27 - 28 are reserved */
        G_992_3_ANNEX_I_NON_OVERLAPPED          = 29, /* All digital mode */
        G_992_3_ANNEX_I_OVERLAPPED              = 30, /* All digital mode */
        G_992_3_ANNEX_J_NON_OVERLAPPED          = 31, /* All digital mode */
        G_992_3_ANNEX_J_OVERLAPPED              = 32, /* All digital mode */


        /* Octet 5 - Splitter-less ADSL2 and ADSL2 */
        G_992_4_ANNEX_I_NON_OVERLAPPED          = 33, /* All digital mode */
        G_992_4_ANNEX_I_OVERLAPPED              = 34, /* All digital mode */
        G_992_3_POTS_MODE_1                     = 35, /* Annex L, non-overlapped, wide upstream */
        G_992_3_POTS_MODE_2                     = 36, /* Annex L, non-overlapped, narrow upstream */
        G_992_3_POTS_MODE_3                     = 37, /* Annex L, overlapped, wide upstream */
        G_992_3_POTS_MODE_4                     = 38, /* Annex L, overlapped, narrow upstream */
        G_992_3_EXT_POTS_NON_OVERLAPPED         = 39, /* Annex M */
        G_992_3_EXT_POTS_OVERLAPPED             = 40, /* Annex M */

        /* Octet 6 - ADSL2+ */
        G_992_5_POTS_NON_OVERLAPPED             = 41, /* Annex A */
        G_992_5_POTS_OVERLAPPED                 = 42, /* Annex A */
        G_992_5_ISDN_NON_OVERLAPPED             = 43, /* Annex B */
        G_992_5_ISDN_OVERLAPPED                 = 44, /* Annex B */
        G_992_5_TCM_ISDN_NON_OVERLAPPED         = 45, /* Annex C */
        G_992_5_TCM_ISDN_OVERLAPPED             = 46, /* Annex C */
        G_992_5_ANNEX_I_NON_OVERLAPPED          = 47, /* All digital mode */
        G_992_5_ANNEX_I_OVERLAPPED              = 48, /* All digital mode */

        /* Octet 7 - ADSL2+ */
        G_992_5_ANNEX_J_NON_OVERLAPPED          = 49, /* All digital mode */
        G_992_5_ANNEX_J_OVERLAPPED              = 50, /* All digital mode */
        G_992_5_EXT_POTS_NON_OVERLAPPED         = 51, /* Annex M */
        G_992_5_EXT_POTS_OVERLAPPED             = 52, /* Annex M */
        /* Bits 53 - 56 are reserved */

        /* Octet 8 - VDSL2 */
        G_993_2_NORTH_AMERICA                   = 57, /* Annex A */
        G_993_2_EUROPE                          = 58, /* Annex B */
        G_993_2_JAPAN                           = 59, /* Annex C */
        /* Bits 60 - 64 are reserved */
};

/**
 * This callback sends to upper layer when it receives link status from  DSL driver
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
typedef int (*dsl_link_status_callback)(char *ifname, DslLinkStatus_t dsl_status );

/**
 * This function initialize and start DSL driver
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_init( void );

/**
 * This function sets the function pointer which receives link status event from driver
 *
 * @param[in] link_status_cb - The function pointer which receives link status event from driver.
 *                             if NULL then need to deregister this callback
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_registerDslLinkStatusCallback(dsl_link_status_callback link_status_cb);

/**
 * This function gets the enable status of DSL line
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[out] enable status The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetLineEnable( hal_param_t *req_param );

/**
 * This function gets the StandardUsed field from DSL driver
 *
 * @param[in] line_index - The line number which starts with 0
 * @param[out] StandardUsed value as JSON object
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetLineStandardUsed( hal_param_t *req_param, int line_index);

/**
 * This function sets the enable/disble of DSL line
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[in] enable   - configure DSL line
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslSetLineEnable( hal_param_t *req_msg );

/**
 * This function sets the enable/disble for data gathering of DSL line
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[in] enable   - configure DSL line
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslSetLineEnableDataGathering( hal_param_t *req_msg );

/**
 * This function gets the number of DSL lines
 *
 * @return the number of DSL lines on success. Otherwise a negative value is returned.
 *
 */
int xdsl_hal_getTotalNoofLines(void);

/**
 * This function gets the number of DSL channels corresponding line
 *
 * @param[in] lineNo - The line number which starts with 0
 * @return the number of DSL channels on success. Otherwise a negative value is returned.
 *
 */
int xdsl_hal_getTotalNoOfChannels(int lineNo);

/**
 * This function gets the DSL line information
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[out] lineInfo - The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetLineInfo(int lineNo, PDML_XDSL_LINE pstLineInfo);

/*
 * This Function converts the LineEncoding string to corresponding Enum inerger.
 * @param[in] str - String returned by hal layer.
 * @param[out] enum - integer corresponding to string in enum XDSL_LINE_ENCODING_ENUM
*/
XDSL_LINE_ENCODING_ENUM LineEncoding_string_to_int(const char *str);

/**
 * This function gets the statistics counters of a DSL line
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[out] lineStats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetLineStats(int lineNo, PDML_XDSL_LINE_STATS pstLineInfo);

/**
 * This function gets the DSL channel information
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] channelInfo The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetChannelInfo(int lineNo, int channelNo, PDML_XDSL_CHANNEL pstChannelInfo);

/*
 * This Function converts the PowerManagementState string to corresponding Enum integer.
 * @param[in] str - String returned by hal layer.
 * @param[out] enum - integer corresponding to string in enum XDSL_LINE_POWER_MGMT_ENUM
*/
XDSL_LINE_POWER_MGMT_ENUM PowerManagementState_string_to_int(const char *str);

/**
 * This function gets the statistics counters of a DSL channel
 *
 * @param[in] lineNo - The line number which starts with 0
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] channelStats The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetChannelStats(int lineNo, int channelNo, PDML_XDSL_CHANNEL_STATS pstChannelStats);

/* ADSL Diagnostis */

/**
 * This function gets the Diagnostic of ADSL Line
 *
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] AdslLineTest The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
//int xdsl_dslGetDiagnosticAdslLineTest(int lineNo, DslDiagAdslLineTest_t *AdslLineTest);

/**
 * This function gets the Diagnostic Seltuer
 *
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] DiagSeltuer The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
//int xdsl_dslGetDiagnosticSeltuer(int lineNo, DslDiagSeltuer_t *DiagSeltuer);

/**
 * This function gets the Diagnostic Seltqln
 *
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] DiagSeltqln The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
//int xdsl_dslGetDiagnosticSeltqln(int lineNo, DslDiagSeltqln_t *DiagSeltqln);

/**
 * This function gets the Diagnostic Seltp
 *
 * @param[in] channelNo - The channel number which starts with 0
 * @param[out] DiagSeltp The output parameter to receive the data
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
//int xdsl_dslGetDiagnosticSeltp(int lineNo, DslDiagSeltp_t *DiagSeltp);

/**
* @brief API to set PTM (PTM) Link parameter. This API is called to enable/disable
* PTM interface.
*
* @param set_param    (OUT) - Pointer to hal_param_t contains the param name, type and value needs to set
* which is based on the param_type.
*
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS xtm_hal_setLinkInfoParam (hal_param_t *set_param);

/**
* @brief API to retrive PTM (PTM) Link information such as Enable, link Status
* and Mac address of PTM interface
*
* @param get_param    (OUT) - Pointer to hal_param_t contains the param name and type
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS xtm_hal_getLinkInfoParam (hal_param_t *get_param);

/**
* @brief Get PTM link statistics information
*
* @param PTM DML path. This string contins the full DML link path of PTM type (Eg: Device.PTM.Link.1.Stats)
* @param Structure pointer to hold statistics information
*
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS xtm_hal_getLinkStats(const CHAR *param_name, PDML_PTM_STATS link_stats);

/**
* @brief API to set ATM Link parameter. This API is called to enable/disable
* PTM interface.
*
* @param set_param    (OUT) - Pointer to hal_param_t contains the param name, type and value needs to set
* which is based on the param_type.
*
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS atm_hal_setLinkInfoParam (PDML_ATM link);

/**
* @brief Get ATM link statistics information
*
* @param PTM DML path. This string contins the full DML link path of ATM type (Eg: Device.ATM.Link.1.Stats)
* @param Structure pointer to hold statistics information
*
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS atm_hal_getLinkStats(const CHAR *param_name, PDML_ATM_STATS link_stats);


#endif /* _XDSL_JSON_APIS_H */
