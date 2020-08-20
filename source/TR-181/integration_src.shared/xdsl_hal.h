/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 * @param[in] req_param - Pointer to hal_param_t structure contains param name
 * @param[out] Enable status filled into the req_param structure pointer
 *
 * @return 0 on success. Otherwise a negative value is returned
 */
int xdsl_hal_dslGetLineEnable( hal_param_t *req_param );

/**
 * This function sets the enable/disble of DSL line
 *
 * @param[in] req_msg - Pointer to hal_param_t structure contains param name,type and value
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
* @brief API to set XTM (PTM) Link parameter. This API is called to enable/disable
* XTM interface.
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
* @brief API to retrive XTM (PTM) Link information such as Enable, link Status
* and Mac address of XTM interface
*
* @param get_param    (OUT) - Pointer to hal_param_t contains the param name and type
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS xtm_hal_getLinkInfoParam (hal_param_t *get_param);

/**
* @brief Get XTM link statistics information
*
* @param XTM DML path. This string contins the full DML link path of XTM type (Eg: Device.PTM.Link.1.Stats)
* @param Structure pointer to hold statistics information
*
* @return The status of the operation
* @retval ANSC_STATUS_SUCCESS if successful
* @retval ANSC_STATUS_FAILURE if any error is detected
*/
ANSC_STATUS xtm_hal_getLinkStats(const CHAR *param_name, PDML_PTM_STATS link_stats);

#endif /* _XDSL_JSON_APIS_H */
