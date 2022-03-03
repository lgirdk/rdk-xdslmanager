/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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

/**********************************************************************

    module: xdsl_hal.c
    For CCSP Component:  dsl json apis

---------------------------------------------------------------

    Description:

    This sample implementation file gives the function call prototypes and
    structure definitions used for the RDK-Broadband DSL features

---------------------------------------------------------------

**********************************************************************/

/*****************************************************************************
* PROJECT-SPECIFIC INCLUDE FILES
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "xdsl_apis.h"
#include "xdsl_hal.h"
#include "xtm_internal.h"
#include "xdsl_internal.h"

#include <json-c/json.h>
#include "json_hal_common.h"
#include "json_hal_client.h"

#define XDSL_JSON_CONF_PATH "/etc/rdk/conf/xdsl_manager_conf.json"

#define XDSL_LINE_ENABLE "Device.DSL.Line.%d.Enable"
#define XDSL_LINE_INFO "Device.DSL.Line.%d."
#define XDSL_LINE_STANDARD_USED "Device.DSL.Line.%d.StandardUsed"
#define XDSL_LINE_STATS "Device.DSL.Line.%d.Stats."
#define XDSL_LINE_TESTPARAMS "Device.DSL.Line.%d.TestParams."
#define XDSL_LINE_LINKSTATUS "Device.DSL.Line.1.LinkStatus"
#define XDSL_LINE_PROFILE "Device.DSL.Line.1.AllowedProfiles"
#define XDSL_LINE_DATA_GATHERING_ENABLE "Device.DSL.Line.%d.EnableDataGathering"

#define XDSL_CHANNEL_INFO "Device.DSL.Channel.%d."
#define XDSL_CHANNEL_STATS "Device.DSL.Channel.%d.Stats."

#define XDSL_NLNM_INFO  "Device.DSL.X_RDK_NLNM."

#define ATM_LINK_ENABLE "Device.ATM.Link.%d.Enable"
#define ATM_LINK_INFO "Device.ATM.Link.%d."
#define ATM_LINK_NAME "Device.ATM.Link.%d.Name"
#define ATM_LINK_DESTINATIONADDRESS "Device.ATM.Link.%d.DestinationAddress"
#define ATM_LINK_ENCAPSULATION "Device.ATM.Link.%d.Encapsulation"
#define ATM_LINK_AAL "Device.ATM.Link.%d.AAL"
#define ATM_LINK_LINKTYPE "Device.ATM.Link.%d.LinkType"

#define ATM_LINK_LINKSTATUS "Device.ATM.Link.1.Status"
#define ATM_LINK_STATS "Device.ATM.Link.%d.Stats."

#define ATM_LINK_QOS "Device.ATM.Link.%d.QoS."
#define ATM_LINK_QOS_QOSCLASS "Device.ATM.Link.%d.QoS.QoSClass"
#define ATM_LINK_QOS_PEAKCELLRATE "Device.ATM.Link.%d.QoS.PeakCellRate"
#define ATM_LINK_QOS_MAXBURSTSIZE "Device.ATM.Link.%d.QoS.MaximumBurstSize"
#define ATM_LINK_QOS_SUSCELLRATE "Device.ATM.Link.%d.QoS.SustainableCellRate"

#define ATM_LINK_DIAGNOSTICS   "Device.ATM.Diagnostics.F5Loopback."
#define ATM_LINK_DIAGNOSTICS_INTERFACE   "Device.ATM.Diagnostics.F5Loopback.Interface"
#define ATM_LINK_DIAGNOSTICS_STATE       "Device.ATM.Diagnostics.F5Loopback.DiagnosticsState"
#define ATM_LINK_DIAGNOSTICS_REPETITIONS "Device.ATM.Diagnostics.F5Loopback.NumberOfRepetitions"
#define ATM_LINK_DIAGNOSTICS_TIMEOUT     "Device.ATM.Diagnostics.F5Loopback.Timeout"

#define RPC_GET_PARAMETERS_REQUEST "getParameters"
#define RPC_SET_PARAMETERS_REQUEST "setParameters"
#define JSON_RPC_FIELD_PARAMS "params"

#define XDSL_MAX_LINES 1
#define HAL_CONNECTION_RETRY_MAX_COUNT 10

#define XDSL_LOWER_LAYER_IFACE "dsl0"
#define XDSL_LINK_UP "Up"
#define XDSL_LINK_DOWN "NoSignal"
#define XDSL_LINK_INITIALIZING "Initializing"
#define XDSL_LINK_ESTABLISHING "EstablishingLink"

#define CHECK(expr)                                                \
    if (!(expr))                                                   \
    {                                                              \
        CcspTraceError(("%s - %d Invalid parameter error \n!!!")); \
        return RETURN_ERR;                                         \
    }

#define FREE_JSON_OBJECT(expr) \
    if(expr)                   \
    {                          \
        json_object_put(expr); \
    }                          \

#define NULL_TYPE 0
#define BITS 8
/***************************************************************************************
* GLOBAL SYMBOLS
****************************************************************************************/
extern PBACKEND_MANAGER_OBJECT g_pBEManager;

dsl_link_status_callback dsl_link_status_cb = NULL;
static int subscribe_dsl_link_event();
static int g_successful_retrains = -1;
static void *eventcb(const char *msg, const int len);
#if !defined  _DT_WAN_Manager_Enable_
static ANSC_STATUS configure_xdsl_driver();
#endif
static ANSC_STATUS xtse_get_bit_position(char *StandardUsed, int *bit_position, int *bit_range);
static ANSC_STATUS compare_with_standards_supported( char *standardsSupported, char *Xtse, int size);
static ANSC_STATUS xdsl_hal_setXtsUsed(char *standardUsed, char *xtsUsedBuf, int size);
static ANSC_STATUS xdsl_hal_setXtse(char *standardsSupported, char *xtseBuf, int size);
static ANSC_STATUS getDestinationAddress(char *Interface, char *DestinationAddress);

/**
 * @brief Utility API to create json request message to send to the interface manager
 *
 * @param request_type (IN)   - Type of request message (GET/SET)
 * @param param_name   (IN)   - String holds the parameter name. This contains the absolute DML path for the parameter.
 * @param type         (IN)   - Type of the parameter needs to be passed to lower layer.
 * @param param_val    (IN)   - String holds the value of the parameter needs to set. This string contains the data based on the type.
 * @return Status of the operation
 * @retval ANSC_STATUS_SUCCESS if successful
 * @retval ANSC_STATUS_FAILURE if any error is detected
 */
static json_object *create_json_request_message(eActionType request_type, const CHAR *param_name, eParamType type, CHAR *param_val);

/**
 * @brief Utility API to get the value from the response
 * json message.
 *
 * @param get_param    (OUT) - Pointer to hal_param_t contains the param name and type
 * @retval ANSC_STATUS_SUCCESS if successful
 * @retval ANSC_STATUS_FAILURE if any error is detected
 */
static ANSC_STATUS get_link_info(hal_param_t *get_param);

/**
 * @brief Utility API to get PTM link statistics information.
 *
 * @param reply_msg  (IN) - Json object contains the json response message
 * @param link_stats (OUT)- Pointer to structure to hold statistics data
 * @retval ANSC_STATUS_SUCCESS if successful
 * @retval ANSC_STATUS_FAILURE if any error is detected
 */
static ANSC_STATUS get_ptm_link_stats(const json_object *reply_msg, PDML_PTM_STATS link_stats);

/**
 * @brief Utility API to get ATM link statistics information.
 *
 * @param reply_msg  (IN) - Json object contains the json response message
 * @param link_stats (OUT)- Pointer to structure to hold statistics data
 * @retval ANSC_STATUS_SUCCESS if successful
 * @retval ANSC_STATUS_FAILURE if any error is detected
 */

static ANSC_STATUS get_atm_link_stats(const json_object *reply_msg, PDML_ATM_STATS link_stats);

/* xdsl_hal_init() */
int xdsl_hal_init( void )
{
    int rc = RETURN_OK;
    rc = json_hal_client_init(XDSL_JSON_CONF_PATH);
    if (rc != RETURN_OK)
    {
        CcspTraceError(("%s-%d Failed to initialize hal client. \n",__FUNCTION__,__LINE__));
        return RETURN_ERR;
    }

    rc = json_hal_client_run();
    if (rc != RETURN_OK)
    {
        CcspTraceError(("%s-%d Failed to start hal client. \n", __FUNCTION__,__LINE__));
        return RETURN_ERR;
    }

    /**
     * Make sure HAL client connected to server.
     * Here waits for 10seconds time to check whether connection established or not.
     * If connection not established within this time, returned error.
     */
    int retry_count = 0;
    int is_client_connected = 0;
    while (retry_count < HAL_CONNECTION_RETRY_MAX_COUNT)
    {
        if (!json_hal_is_client_connected())
        {
            sleep(1);
            retry_count++;
        }
        else
        {
            CcspTraceInfo(("%s-%d Hal-client connected to the hal server \n", __FUNCTION__, __LINE__));
            is_client_connected = TRUE;
            break;
        }
    }

    if (is_client_connected != TRUE)
    {
        CcspTraceInfo(("Failed to connect to the hal server. \n"));
        return RETURN_ERR;
    }

    /* Event subscription for dsl link to hal-server. */
    rc = subscribe_dsl_link_event();
    if (rc != RETURN_OK)
    {
        CcspTraceError(("Failed to subscribe DSL link event \n"));
    }

#if !defined  _DT_WAN_Manager_Enable_
    /**
     * Configure xDSL driver.
     */
    rc = configure_xdsl_driver();
    if (rc != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("Failed to configure xDSL driver \n"));
    }
#endif
    return rc;
}

/* xdsl_hal_registerDslLinkStatusCallback() */
int xdsl_hal_registerDslLinkStatusCallback(dsl_link_status_callback link_status_cb)
{
    //Register or deregister
    if( NULL != link_status_cb )
    {
        dsl_link_status_cb = link_status_cb;
    }
    else
    {
        dsl_link_status_cb = NULL;
    }
    return RETURN_OK;
}

/* xdsl_hal_getTotalNoofLines() */
int xdsl_hal_getTotalNoofLines( void )
{
    return XDSL_MAX_LINES;
}

/* xdsl_hal_dslGetLineEnable() */
int xdsl_hal_dslGetLineEnable( hal_param_t *req_param )
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jresponse_msg = NULL;
    hal_param_t resp_param;

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    memset(&resp_param, 0, sizeof(resp_param));
    if (json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, req_param) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jresponse_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jresponse_msg);
        return RETURN_ERR;
    }

    if(jresponse_msg == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    rc = json_hal_get_param(jresponse_msg, 0, GET_RESPONSE_MESSAGE, &resp_param);
    if (rc != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jresponse_msg);
        return RETURN_ERR;
    }

    if (strstr (resp_param.name, "Enable"))
    {
        strncpy(req_param->value, resp_param.value, sizeof(resp_param.value));
    }

    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jresponse_msg);

    return rc;
}

/* xdsl_hal_dslGetLineStandardUsed() */
int xdsl_hal_dslGetLineStandardUsed( hal_param_t *req_param, int line_index)
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jresponse_msg = NULL;
    hal_param_t resp_param;

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param->name, sizeof(req_param->name), XDSL_LINE_STANDARD_USED, line_index);

    memset(&resp_param, 0, sizeof(resp_param));
    if (json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, req_param) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jresponse_msg) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jresponse_msg);
        return RETURN_ERR;
    }

    if(jresponse_msg == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    rc = json_hal_get_param(jresponse_msg, 0, GET_RESPONSE_MESSAGE, &resp_param);
    if (rc != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jresponse_msg);
        return RETURN_ERR;
    }

    if (strstr (resp_param.name, "StandardUsed"))
    {
        strncpy(req_param->value, resp_param.value, sizeof(resp_param.value));
    }

    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jresponse_msg);

    return rc;
}
/* xdsl_hal_dslSetLineEnable() */
int xdsl_hal_dslSetLineEnable( hal_param_t *req_msg )
{
    int rc = RETURN_OK;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_bool status = FALSE;

    jmsg = json_hal_client_get_request_header(RPC_SET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, req_msg);
    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    CHECK(jreply_msg);

    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d Set request for [%s] is successful ", __FUNCTION__, __LINE__, req_msg->name));
            rc = RETURN_OK;
        }
        else
        {
            CcspTraceError(("%s - %d - Set request for [%s] is failed \n", __FUNCTION__, __LINE__, req_msg->name));
            rc = RETURN_ERR;
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
        rc = RETURN_ERR;
    }

    // Free json objects.
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

/* xdsl_hal_dslSetLineEnableDataGathering() */
int xdsl_hal_dslSetLineEnableDataGathering(hal_param_t *req_msg)
{
    int rc = RETURN_OK;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_bool status = FALSE;

    jmsg = json_hal_client_get_request_header(RPC_SET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    if( json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, req_msg) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        CcspTraceError(("[%s][%d] json_hal_add_param failed \n", __FUNCTION__, __LINE__));
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));
    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK )
    {
        CcspTraceInfo(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d Set request for [%s] is successful ", __FUNCTION__, __LINE__, req_msg->name));
            rc = RETURN_OK;
        }
        else
        {
            CcspTraceError(("%s - %d - Set request for [%s] is failed \n", __FUNCTION__, __LINE__, req_msg->name));
            rc = RETURN_ERR;
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
        rc = RETURN_ERR;
    }

    // Free json objects.
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

static ANSC_STATUS xtse_get_bit_position(char *StandardUsed, int *bit_position, int *bit_range)
{
   if(StandardUsed != NULL)
   {
          if(!strcmp(StandardUsed,"G.992.1_Annex_A"))
          {
              *bit_position = G_992_1_POTS_NON_OVERLAPPED;
          /* NOTE : More than one XTSE bits can be mapped to the same standard. So we need to filter out the repeated ones.
               * If the standard is "G.992.1_Annex_A" , there are two bits corresponding to this standard,
               G_992_1_POTS_NON_OVERLAPPED             = 3,  < Annex A >
                   G_992_1_POTS_OVERLAPPED                 = 4,  < Annex A >
           * Set both bits (G_992_1_POTS_NON_OVERLAPPED and G_992_1_POTS_OVERLAPPED)
              */
              *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.1_Annex_B"))
          {
              *bit_position = G_992_1_ISDN_NON_OVERLAPPED;
              *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.1_Annex_C"))
          {
             *bit_position = G_992_1_TCM_ISDN_NON_OVERLAPPED;
             *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"T1.413"))
          {
              *bit_position = T1_413;
              *bit_range = 1;
          }
          else if(!strcmp(StandardUsed,"T1.413i2"))
          {
              *bit_position = T1_413;
              *bit_range = 1;
          }
          else if(!strcmp(StandardUsed,"ETSI_101_388"))
          {
              *bit_position = ETSI_101_388;
              *bit_range = 1;
          }
          else if(!strcmp(StandardUsed,"G.992.2"))
          {
              *bit_position = G_992_2_POTS_NON_OVERLAPPED;
              *bit_range = 4;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_A"))
          {
              *bit_position = G_992_3_POTS_NON_OVERLAPPED;
              *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_B"))
          {
               *bit_position = G_992_3_ISDN_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_C"))
          {
               *bit_position = G_992_3_TCM_ISDN_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_I"))
          {
               *bit_position = G_992_3_ANNEX_I_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_J"))
          {
               *bit_position = G_992_3_ANNEX_J_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_L"))
          {
               *bit_position = G_992_3_POTS_MODE_1;
               *bit_range = 4;
          }
          else if(!strcmp(StandardUsed,"G.992.3_Annex_M"))
          {
               *bit_position = G_992_3_EXT_POTS_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.4"))
          {
               *bit_position = G_992_4_POTS_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_A"))
          {
               *bit_position = G_992_5_POTS_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_B"))
          {
               *bit_position = G_992_5_ISDN_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_C"))
          {
               *bit_position = G_992_5_TCM_ISDN_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_I"))
          {
               *bit_position = G_992_5_ANNEX_I_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_J"))
          {
               *bit_position = G_992_5_ANNEX_J_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.992.5_Annex_M"))
          {
               *bit_position = G_992_5_EXT_POTS_NON_OVERLAPPED;
               *bit_range = 2;
          }
          else if(!strcmp(StandardUsed,"G.993.1"))
          {
              return ANSC_STATUS_FAILURE;
          //To Do:  Find the bit position
              // *bit_position = 0;
              // *bit_range = 0;
          }
          else if(!strcmp(StandardUsed,"G.993.1_Annex_A"))
          {
              return ANSC_STATUS_FAILURE;
              //To Do:  Find the bit position
              // *bit_position = 0;
              // *bit_range = 0;
          }
          else if(!strcmp(StandardUsed,"G.993.2_Annex_A"))
          {
               *bit_position = G_993_2_NORTH_AMERICA;
               *bit_range = 1;
          }
          else if(!strcmp(StandardUsed,"G.993.2_Annex_B"))
          {
              *bit_position = G_993_2_EUROPE;
              *bit_range = 1;
          }
          else if(!strcmp(StandardUsed,"G.993.2_Annex_C"))
          {
               *bit_position = G_993_2_JAPAN;
               *bit_range = 1;
          }
          else
          {
           CcspTraceError(("%s : %s have no match with StandardsSupported\n", __FUNCTION__, StandardUsed));
               return ANSC_STATUS_FAILURE;
          }
          return ANSC_STATUS_SUCCESS;
     }
     else
     {
          return ANSC_STATUS_FAILURE;
     }
}

static ANSC_STATUS compare_with_standards_supported( char *standardsSupported, char *xtse, int size)
{
    unsigned int bit_position, bit_range, octet, shift_bit;
    unsigned int byte[8] = {0};
    unsigned int temp_byte = 0x01;
    unsigned int buf = 0;
    int i = 0;

    if(standardsSupported != NULL)
    {
        char *str;
        str = strdup(standardsSupported);
        int init_size = strlen(str);
        char delim[] = ", ";
        int i = 0;
        char *ptr = strtok(str, delim);

        while(ptr != NULL)
        {
            if(xtse_get_bit_position(ptr,&bit_position, &bit_range) == ANSC_STATUS_SUCCESS)
            {
                for(i=0; i< bit_range; i++)
                {
                    octet = (((bit_position + i) -1) / BITS);
                    shift_bit = (((bit_position + i) - 1)% BITS);
                    buf = (temp_byte << shift_bit);
                    byte[octet] = (byte[octet] | buf);

                    CcspTraceDebug(("bit_position :%d, octet : %d\n", bit_position, octet));
                    buf = 0;  //clear buf
                }
            }
            ptr = strtok(NULL, delim);
        }

        snprintf(xtse, size, "%02x%02x%02x%02x%02x%02x%02x%02x", byte[7],byte[6],byte[5],byte[4],byte[3],byte[2],byte[1],byte[0] );
        return ANSC_STATUS_SUCCESS;
    }
    else
    {
        return ANSC_STATUS_FAILURE;
    }
}

static ANSC_STATUS xdsl_hal_setXtsUsed(char *standardUsed, char *xtsUsedBuf, int size)
{
    unsigned int bit_position, bit_range, octet, shift_bit;
    unsigned int byte[8] = {0};
    unsigned int temp_byte = 0x01;
    unsigned int buf = 0;
    int i = 0;

    if(xtse_get_bit_position(standardUsed, &bit_position, &bit_range) == ANSC_STATUS_SUCCESS)
    {
       for(i=0; i < bit_range; i++)
       {
          octet = (((bit_position + i) - 1) / BITS);
          shift_bit = (((bit_position + i) - 1) % BITS);
          buf = (temp_byte << shift_bit);
          byte[octet] = (byte[octet] | buf);

          buf = 0; //clear buf
       }
       snprintf(xtsUsedBuf, size, "%02x%02x%02x%02x%02x%02x%02x%02x", byte[7],byte[6],byte[5],byte[4],byte[3],byte[2],byte[1],byte[0] );

       return ANSC_STATUS_SUCCESS;
    }
    else
    {
       return ANSC_STATUS_FAILURE;
    }
}

static ANSC_STATUS xdsl_hal_setXtse(char *standardsSupported, char *xtseBuf, int size)
{
    char tmpBuf[17] = {0};
    if(compare_with_standards_supported(standardsSupported, &tmpBuf, sizeof(tmpBuf)) == ANSC_STATUS_SUCCESS)
    {
       snprintf(xtseBuf, size, "%s", tmpBuf);
       return ANSC_STATUS_SUCCESS;
    }
    else
    {
       CcspTraceError(("Failed to retrieve XTSE\n"));
       return ANSC_STATUS_FAILURE;
    }
}

XDSL_LINE_ENCODING_ENUM LineEncoding_string_to_int(const char *str)
{
   if(strcmp(str, "DMT") == 0 ){
       return DML_LINE_ENCODING_DMT;
   }
   else if(strcmp(str, "CAP") == 0){
       return DML_LINE_ENCODING_CAP;
   }
   else if(strcmp(str, "2B1Q") == 0){
       return DML_LINE_ENCODING_2B1Q;
   }
   else if(strcmp(str, "43BT") == 0){
       return DML_LINE_ENCODING_43BT;
   }
   else if(strcmp(str, "PAM") == 0){
       return DML_LINE_ENCODING_PAM;
   }
   else if (strcmp(str, "QAM") == 0){
       return DML_LINE_ENCODING_QAM;
   }
   else{
	return 0;
  }
}

XDSL_LINE_POWER_MGMT_ENUM PowerManagementState_string_to_int(const char *str)
{
   if(strcmp(str, "L0") == 0 ){
       return PM_STATE_L0;
   }
    else if(strcmp(str, "L1") == 0 ){
       return PM_STATE_L1;
   }
    else if(strcmp(str, "L2") == 0 ){
       return PM_STATE_L2;
   }
    else if(strcmp(str, "L3") == 0 ){
       return PM_STATE_L3;
   }
    else if(strcmp(str, "L4") == 0 ){
       return PM_STATE_L4;
   }
}

/* xdsl_hal_dslGetLineInfo() */
int xdsl_hal_dslGetLineInfo(int lineNo, PDML_XDSL_LINE pstLineInfo)
{
    int rc = RETURN_OK;
    char xtseBuf[17]    = { 0 };
    char xtsUsedBuf[17] = { 0 };
    hal_param_t req_param;
    hal_param_t resp_param;

    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_object *jparams = NULL;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param.name, sizeof(req_param.name), XDSL_LINE_INFO, lineNo);
    if(json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));
    if (json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK )
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Response message = %s \n", json_object_to_json_string_ext(jreply_msg,JSON_C_TO_STRING_PRETTY)));

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL) {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        if (json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return RETURN_ERR;
        }
        if (strstr (resp_param.name, "Status")) {

            /**
             * LinkStatus
             */
            if (strstr(resp_param.name, "Link"))
            {
                if (strcmp(resp_param.value, "Up") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_Up;
                }
                else if (strcmp(resp_param.value, "Initializing") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_Initializing;
                }
                else if (strcmp(resp_param.value, "EstablishingLink") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_EstablishingLink;
                }
                else if (strcmp(resp_param.value, "NoSignal") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_NoSignal;
                }
                else if (strcmp(resp_param.value, "Disabled") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_Disabled;
                }
                else if (strcmp(resp_param.value, "Error") == 0)
                {
                    pstLineInfo->LinkStatus = XDSL_LINK_STATUS_Error;
                }
            }
            else //IfStatus
            {
                if (strcmp(resp_param.value, "Up") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_Up;
                }
                else if (strcmp(resp_param.value, "Down") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_Down;
                }
                else if (strcmp(resp_param.value, "Unknown") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_Unknown;
                }
                else if (strcmp(resp_param.value, "Dormant") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_Dormant;
                }
                else if (strcmp(resp_param.value, "NotPresent") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_NotPresent;
                }
                else if (strcmp(resp_param.value, "LowerLayerDown") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_LowerLayerDown;
                }
                else if (strcmp(resp_param.value, "Error") == 0)
                {
                    pstLineInfo->Status = XDSL_IF_STATUS_Error;
                }
            }
        }
        else if (strstr (resp_param.name, "LastChange")) {
            pstLineInfo->LastChange = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "PowerManagementState")) {
            pstLineInfo->PowerManagementState = PowerManagementState_string_to_int(resp_param.value);
        }
        else if (strstr (resp_param.name, "UpstreamMaxBitRate")) {
            pstLineInfo->UpstreamMaxBitRate= atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "DownstreamMaxBitRate")) {
            pstLineInfo->DownstreamMaxBitRate = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SuccessFailureCause")) {
            pstLineInfo->SuccessFailureCause = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "RXTHRSHds")) {
            pstLineInfo->RXTHRSHds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTRAMODEds")) {
            pstLineInfo->ACTRAMODEds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTRAMODEus")) {
            pstLineInfo->ACTRAMODEus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTINPROCds")) {
            pstLineInfo->ACTINPROCds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTINPROCus")) {
            pstLineInfo->ACTINPROCus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMROCds")) {
            pstLineInfo->SNRMROCds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMROCus")) {
            pstLineInfo->SNRMROCus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastStateTransmittedDownstream")) {
            pstLineInfo->LastStateTransmittedDownstream = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastStateTransmittedUpstream")) {
            pstLineInfo->LastStateTransmittedUpstream = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UPBOKLER")) {
            pstLineInfo->UPBOKLER = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UPBOKLE")) {
            pstLineInfo->UPBOKLE = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LIMITMASK")) {
            pstLineInfo->LIMITMASK = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "US0MASK")) {
            pstLineInfo->US0MASK = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UpstreamAttenuation")) {
            pstLineInfo->UpstreamAttenuation = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "DownstreamAttenuation")) {
            pstLineInfo->DownstreamAttenuation = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UpstreamNoiseMargin")) {
            pstLineInfo->UpstreamNoiseMargin = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "DownstreamNoiseMargin")) {
            pstLineInfo->DownstreamNoiseMargin = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UpstreamPower")) {
            pstLineInfo->UpstreamPower = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "DownstreamPower")) {
            pstLineInfo->DownstreamPower = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LineEncoding")) {
            pstLineInfo->LineEncoding = LineEncoding_string_to_int(resp_param.value);
        }
        else if (strstr (resp_param.name, "TRELLISds")) {
            pstLineInfo->TRELLISds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "TRELLISus")) {
            pstLineInfo->TRELLISus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTSNRMODEds")) {
            pstLineInfo->ACTSNRMODEds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTSNRMODEus")) {
            pstLineInfo->ACTSNRMODEus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTUALCE")) {
            pstLineInfo->ACTUALCE = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LineNumber")) {
            pstLineInfo->LineNumber = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INMIATOds")) {
            pstLineInfo->INMIATOds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INMIATSds")) {
            pstLineInfo->INMIATSds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INMCCds")) {
            pstLineInfo->INMCCds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INMINPEQMODEds")) {
            pstLineInfo->INMINPEQMODEds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "XTURANSIStd")) {
            pstLineInfo->XTURANSIStd = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "XTURANSIRev")) {
            pstLineInfo->XTURANSIRev = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "XTUCANSIStd")) {
            pstLineInfo->XTUCANSIStd = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "XTUCANSIRev")) {
            pstLineInfo->XTUCANSIRev = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentProfile")) {
            snprintf(pstLineInfo->CurrentProfile, sizeof(pstLineInfo->CurrentProfile), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "AllowedProfiles")) {
            snprintf(pstLineInfo->AllowedProfiles, sizeof(pstLineInfo->AllowedProfiles), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "FirmwareVersion")) {
            snprintf(pstLineInfo->FirmwareVersion, sizeof(pstLineInfo->FirmwareVersion), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "StandardUsed")) {
            snprintf(pstLineInfo->StandardUsed, sizeof(pstLineInfo->StandardUsed), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMpbus")) {
            snprintf(pstLineInfo->SNRMpbus, sizeof(pstLineInfo->SNRMpbus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMpbds")) {
            snprintf(pstLineInfo->SNRMpbds, sizeof(pstLineInfo->SNRMpbds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTURVendor")) {
            snprintf(pstLineInfo->XTURVendor, sizeof(pstLineInfo->XTURVendor), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTURCountry")) {
            snprintf(pstLineInfo->XTURCountry, sizeof(pstLineInfo->XTURCountry), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTUCVendor")) {
            snprintf(pstLineInfo->XTUCVendor, sizeof(pstLineInfo->XTUCVendor), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTUCCountry")) {
            snprintf(pstLineInfo->XTUCCountry, sizeof(pstLineInfo->XTUCCountry), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "UPBOKLEPb")) {
            snprintf(pstLineInfo->UPBOKLEPb, sizeof(pstLineInfo->UPBOKLEPb), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "UPBOKLERPb")) {
            snprintf(pstLineInfo->UPBOKLERPb, sizeof(pstLineInfo->UPBOKLERPb), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTSE")) {
            snprintf(pstLineInfo->XTSE, sizeof(pstLineInfo->XTSE), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "XTSUsed")) {
            snprintf(pstLineInfo->XTSUsed, sizeof(pstLineInfo->XTSUsed), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "MREFPSDds")) {
            snprintf(pstLineInfo->MREFPSDds, sizeof(pstLineInfo->MREFPSDds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "MREFPSDus")) {
            snprintf(pstLineInfo->MREFPSDus, sizeof(pstLineInfo->MREFPSDus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "VirtualNoisePSDds")) {
            snprintf(pstLineInfo->VirtualNoisePSDds, sizeof(pstLineInfo->VirtualNoisePSDds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "VirtualNoisePSDus")) {
            snprintf(pstLineInfo->VirtualNoisePSDus, sizeof(pstLineInfo->VirtualNoisePSDus), "%s", resp_param.value);
        }

    }

    if(strstr (pstLineInfo->XTSE, "0000000000000000"))
    {
       if(strlen(pstLineInfo->StandardsSupported) != 0)
       {
         if(xdsl_hal_setXtse(pstLineInfo->StandardsSupported, &xtseBuf, sizeof(xtseBuf)) == ANSC_STATUS_SUCCESS)
         {
            snprintf(pstLineInfo->XTSE, sizeof(pstLineInfo->XTSE), "%s", xtseBuf);
         }
       }
    }

    if(strstr (pstLineInfo->XTSUsed, "0000000000000000"))
    {
       if(strlen(pstLineInfo->StandardUsed) != 0)
       {
         if(xdsl_hal_setXtsUsed(pstLineInfo->StandardUsed, &xtsUsedBuf, sizeof(xtsUsedBuf)) == ANSC_STATUS_SUCCESS)
         {
            snprintf(pstLineInfo->XTSUsed, sizeof(pstLineInfo->XTSUsed), "%s", xtsUsedBuf);
         }
      }
    }

    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

static void *eventcb(const char *msg, const int len)
{
    json_object *msg_param = NULL;
    json_object *msg_param_val = NULL;
    char event_name[256] = {'\0'};
    char event_val[256] = {'\0'};

    if(msg == NULL) {
        return;
    }

    /* Parse message and find event received. */
    json_object *jobj = json_tokener_parse(msg);
    CHECK(jobj);

    if (json_object_object_get_ex(jobj, JSON_RPC_FIELD_PARAMS, &msg_param))
    {
        json_object *jsubs_param_array = json_object_array_get_idx(msg_param, 0);
        if (jsubs_param_array == NULL)
        {
            CcspTraceError(("Failed to get params data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }

        if (json_object_object_get_ex(jsubs_param_array, "name", &msg_param_val))
        {
            strncpy(event_name, json_object_get_string(msg_param_val), sizeof(event_name)- 1);
            CcspTraceInfo(("Event name = %s \n", event_name));
        }
        else
        {
            CcspTraceError(("Failed to get event name data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }

        if (json_object_object_get_ex(jsubs_param_array, "value", &msg_param_val))
        {
            strncpy(event_val, json_object_get_string(msg_param_val), sizeof(event_val) - 1);
            CcspTraceInfo(("Event value = %s \n", event_val));
        }
        else
        {
            CcspTraceError(("Failed to get event value data from subscription json message \n"));
            FREE_JSON_OBJECT(jobj);
            return;
        }
    }

    if (strncmp(event_name, XDSL_LINE_LINKSTATUS, sizeof(XDSL_LINE_LINKSTATUS)) == 0)
    {
        CcspTraceInfo(("Event got for %s and its value =%s \n", event_name, event_val));
        if (dsl_link_status_cb)
        {
            DslLinkStatus_t link_status;
            if (strncmp(event_val, XDSL_LINK_UP, strlen(XDSL_LINK_UP)) == 0 )
            {
                link_status = LINK_UP;
                g_successful_retrains = g_successful_retrains + 1;
            }
            else if (strncmp(event_val, XDSL_LINK_INITIALIZING, strlen(XDSL_LINK_INITIALIZING)) == 0  ||
                     strncmp(event_val, XDSL_LINK_ESTABLISHING, strlen(XDSL_LINK_ESTABLISHING)) == 0)
            {
                link_status = LINK_INITIALIZING ;
            }
            else if (strncmp(event_val, XDSL_LINK_DOWN, strlen(XDSL_LINK_DOWN)) == 0 ) 
            {
                link_status = LINK_DISABLED ;
            }
            else  
            {
                FREE_JSON_OBJECT(jobj);
                return;
            }
            CcspTraceInfo(("Notifying DSLManager for the link event \n"));
            dsl_link_status_cb(XDSL_LOWER_LAYER_IFACE, link_status);
        }
    }

    FREE_JSON_OBJECT(jobj);
}

static int subscribe_dsl_link_event()
{
    int rc = RETURN_ERR;
    rc = json_hal_client_subscribe_event(eventcb, XDSL_LINE_LINKSTATUS, "onChange");
    return rc;
}

/* * xdsl_hal_dslGetLineStats() */
int xdsl_hal_dslGetLineStats(int lineNo, PDML_XDSL_LINE_STATS pstLineStats)
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_object *jparams = NULL;

    hal_param_t req_param;
    hal_param_t resp_param;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param.name, sizeof(req_param.name), XDSL_LINE_STATS, lineNo);
    if( json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceInfo(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL) {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        if (json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return RETURN_ERR;
        }

        if (strstr (resp_param.name, "BytesSent")) {
            pstLineStats->BytesSent = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "BytesReceived")) {
            pstLineStats->BytesReceived = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "PacketsSent")) {
            if (strstr (resp_param.name, "DiscardPacketsSent")) {
                pstLineStats->DiscardPacketsSent = atol(resp_param.value);
            }
            else {
                pstLineStats->PacketsSent= atol(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "PacketsReceived")) {
            if (strstr (resp_param.name, "DiscardPacketsReceived")) {
                pstLineStats->DiscardPacketsReceived = atol(resp_param.value);
            }
            else {
                pstLineStats->PacketsReceived = atol(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "ErrorsSent")) {
            pstLineStats->ErrorsSent = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "ErrorsReceived")) {
            pstLineStats->ErrorsReceived = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "TotalStart")) {
            pstLineStats->TotalStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "ShowtimeStart")) {
            pstLineStats->ShowtimeStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtimeStart")) {
            pstLineStats->LastShowtimeStart = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHourStart")) {
            pstLineStats->QuarterHourStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDayStart")) {
            pstLineStats->CurrentDayStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.Total.ErroredSecs")) {
            pstLineStats->stTotal.ErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.Total.SeverelyErroredSecs")) {
            pstLineStats->stTotal.SeverelyErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.Showtime.ErroredSecs")) {
            pstLineStats->stShowTime.ErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.Showtime.SeverelyErroredSecs")) {
            pstLineStats->stShowTime.SeverelyErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.LastShowtime.ErroredSecs")) {
            pstLineStats->stLastShowTime.ErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.LastShowtime.SeverelyErroredSecs")) {
            pstLineStats->stLastShowTime.SeverelyErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.CurrentDay.ErroredSecs")) {
            pstLineStats->stCurrentDay.ErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.CurrentDay.SeverelyErroredSecs")) {
            pstLineStats->stCurrentDay.SeverelyErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "X_RDK_LinkRetrain")) {
            if (strstr (resp_param.name, "Stats.CurrentDay.X_RDK_LinkRetrain")) {
               pstLineStats->stCurrentDay.X_RDK_LinkRetrain = atoi(resp_param.value);
            }
            else {
               pstLineStats->stQuarterHour.X_RDK_LinkRetrain = atoi(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "Stats.CurrentDay.X_RDK_InitErrors")) {
            pstLineStats->stCurrentDay.X_RDK_InitErrors = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.CurrentDay.X_RDK_InitTimeouts")) {
            pstLineStats->stCurrentDay.X_RDK_InitTimeouts = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.CurrentDay.X_RDK_SuccessfulRetrains")) {
            pstLineStats->stCurrentDay.X_RDK_SuccessfulRetrains = g_successful_retrains;
        }
        else if (strstr (resp_param.name, "Stats.QuarterHour.ErroredSecs")) {
            pstLineStats->stQuarterHour.ErroredSecs = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "Stats.QuarterHour.SeverelyErroredSecs")) {
            pstLineStats->stQuarterHour.SeverelyErroredSecs = atoi(resp_param.value);
        }
    }

    // Free json objects
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

/* * xdsl_hal_dslGetLineTestParams() */
int xdsl_hal_dslGetLineTestParams(int lineNo, PDML_XDSL_LINE_TESTPARAMS pstLineTestParams)
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_object *jparams = NULL;

    hal_param_t req_param;
    hal_param_t resp_param;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);
    snprintf(req_param.name, sizeof(req_param.name), XDSL_LINE_TESTPARAMS, lineNo);
    if( json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK) {
        FREE_JSON_OBJECT(jmsg);
       return RETURN_ERR;
    }


    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceInfo(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL) {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        if (json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return RETURN_ERR;
        }

        if (strstr (resp_param.name, "HLOGGds")) {
            pstLineTestParams->HLOGGds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "HLOGGus")) {
            pstLineTestParams->HLOGGus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "HLOGpsds")) {
        snprintf(pstLineTestParams->HLOGpsds, sizeof(pstLineTestParams->HLOGpsds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "HLOGpsus")) {
        snprintf(pstLineTestParams->HLOGpsus, sizeof(pstLineTestParams->HLOGpsus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "HLOGMTds")) {
            pstLineTestParams->HLOGMTds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "HLOGMTus")) {
            pstLineTestParams->HLOGMTus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "QLNGds")) {
            pstLineTestParams->QLNGds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "QLNGus")) {
            pstLineTestParams->QLNGus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "QLNpsds")) {
        snprintf(pstLineTestParams->QLNpsds, sizeof(pstLineTestParams->QLNpsds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "QLNpsus")) {
        snprintf(pstLineTestParams->QLNpsus, sizeof(pstLineTestParams->QLNpsus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "QLNMTds")) {
            pstLineTestParams->QLNMTds = atoi(resp_param.value);
       }
        else if (strstr (resp_param.name, "QLNMTus")) {
            pstLineTestParams->QLNMTus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRGds")) {
            pstLineTestParams->SNRGds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRGus")) {
            pstLineTestParams->SNRGus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRpsds")) {
        snprintf(pstLineTestParams->SNRpsds, sizeof(pstLineTestParams->SNRpsds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRpsus")) {
        snprintf(pstLineTestParams->SNRpsus, sizeof(pstLineTestParams->SNRpsus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMTds")) {
            pstLineTestParams->SNRMTds = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "SNRMTus")) {
            pstLineTestParams->SNRMTus = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LATNds")) {
        snprintf(pstLineTestParams->LATNds, sizeof(pstLineTestParams->LATNds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "LATNus")) {
        snprintf(pstLineTestParams->LATNus, sizeof(pstLineTestParams->LATNus), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SATNds")) {
        snprintf(pstLineTestParams->SATNds, sizeof(pstLineTestParams->SATNds), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "SATNus")) {
        snprintf(pstLineTestParams->SATNus, sizeof(pstLineTestParams->SATNus), "%s", resp_param.value);
        }
    }

    // Free json objects
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

/* * xdsl_hal_getTotalNoOfChannels() */
int xdsl_hal_getTotalNoOfChannels(int lineNo)
{
    //This must be return from iface maneger or so needs to revisit
    return 2;
}

/* * xdsl_hal_dslGetChannelInfo() */
int xdsl_hal_dslGetChannelInfo(int lineNo, int channelNo, PDML_XDSL_CHANNEL pstChannelInfo)
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    hal_param_t req_param;
    hal_param_t resp_param;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_object *jparams = NULL;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param.name, sizeof(req_param.name), XDSL_CHANNEL_INFO, channelNo);
    if( json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        rc = json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param);
        if (rc != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return rc;
        }

        if (strstr (resp_param.name, "Status")) {
            if (strcmp(resp_param.value, "Up") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_Up;
            }
            else if (strcmp(resp_param.value, "Down") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_Down;
            }
            else if (strcmp(resp_param.value, "Unknown") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_Unknown;
            }
            else if (strcmp(resp_param.value, "Dormant") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_Dormant;
            }
            else if (strcmp(resp_param.value, "NotPresent") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_NotPresent;
            }
            else if (strcmp(resp_param.value, "LowerLayerDown") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_LowerLayerDown;
            }
            else if (strcmp(resp_param.value, "Error") == 0)
            {
                pstChannelInfo->Status = XDSL_IF_STATUS_Error;
            }
        }
        else if (strstr (resp_param.name, "LastChange")) {
            pstChannelInfo->LastChange = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LPATH")) {
            pstChannelInfo->LPATH = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INTLVDEPTH")) {
            pstChannelInfo->INTLVDEPTH = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INTLVBLOCK")) {
            pstChannelInfo->INTLVBLOCK = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ActualInterleavingDelay")) {
            pstChannelInfo->ActualInterleavingDelay = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTINP")) {
            pstChannelInfo->ACTINP = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "INPREPORT")) {
            pstChannelInfo->INPREPORT = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "NFEC")) {
            pstChannelInfo->NFEC = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "RFEC")) {
            pstChannelInfo->RFEC = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LSYMB")) {
            pstChannelInfo->LSYMB = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "UpstreamCurrRate")) {
            pstChannelInfo->UpstreamCurrRate = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "DownstreamCurrRate")) {
            pstChannelInfo->DownstreamCurrRate = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTNDR")) {
            pstChannelInfo->ACTNDR = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "ACTINPREIN")) {
            pstChannelInfo->ACTINPREIN = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "LinkEncapsulationSupported")) {
            snprintf(pstChannelInfo->LinkEncapsulationSupported, sizeof(pstChannelInfo->LinkEncapsulationSupported), "%s", resp_param.value);
        }
        else if (strstr (resp_param.name, "LinkEncapsulationUsed")) {
            snprintf(pstChannelInfo->LinkEncapsulationUsed, sizeof(pstChannelInfo->LinkEncapsulationUsed), "%s", resp_param.value);
        }
    }

    // Free json objects
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

/* * xdsl_hal_dslGetChannelStats() */
int xdsl_hal_dslGetChannelStats(int lineNo, int channelNo, PDML_XDSL_CHANNEL_STATS pstChannelStats)
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    json_object *jmsg = NULL;
    json_object *jparams = NULL;
    json_object *jreply_msg = NULL;

    hal_param_t req_param;
    hal_param_t resp_param;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param.name, sizeof(req_param.name), XDSL_CHANNEL_STATS, lineNo);
    if( json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK )
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL) {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL) {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        if (json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return RETURN_ERR;
        }

        if (strstr (resp_param.name, "BytesSent")) {
            pstChannelStats->BytesSent = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "BytesReceived")) {
            pstChannelStats->BytesReceived = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "PacketsSent")) {
            if (strstr (resp_param.name, "DiscardPacketsSent")) {
                pstChannelStats->DiscardPacketsSent = atol(resp_param.value);
            }
            else {
                pstChannelStats->PacketsSent= atol(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "PacketsReceived")) {
            if (strstr (resp_param.name, "DiscardPacketsReceived")) {
                pstChannelStats->DiscardPacketsReceived = atol(resp_param.value);
            }
            else {
                pstChannelStats->PacketsReceived = atol(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "ErrorsSent")) {
            pstChannelStats->ErrorsSent = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "ErrorsReceived")) {
            pstChannelStats->ErrorsReceived = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "TotalStart")) {
            pstChannelStats->TotalStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "ShowtimeStart")) {
            pstChannelStats->ShowtimeStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtimeStart")) {
            pstChannelStats->LastShowtimeStart = atoi(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHourStart")) {
            pstChannelStats->QuarterHourStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDayStart")) {
            pstChannelStats->CurrentDayStart = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTURFECErrors")) {
            pstChannelStats->stTotal.XTURFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTUCFECErrors")) {
            pstChannelStats->stTotal.XTUCFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTURHECErrors")) {
            pstChannelStats->stTotal.XTURHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTUCHECErrors")) {
            pstChannelStats->stTotal.XTUCHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTURCRCErrors")) {
            pstChannelStats->stTotal.XTURCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Total.XTUCCRCErrors")) {
            pstChannelStats->stTotal.XTUCCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTURFECErrors")) {
            pstChannelStats->stShowTime.XTURFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTUCFECErrors")) {
            pstChannelStats->stShowTime.XTUCFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTURHECErrors")) {
            pstChannelStats->stShowTime.XTURHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTUCHECErrors")) {
            pstChannelStats->stShowTime.XTUCHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTURCRCErrors")) {
            pstChannelStats->stShowTime.XTURCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "Showtime.XTUCCRCErrors")) {
            pstChannelStats->stShowTime.XTUCCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTURFECErrors")) {
            pstChannelStats->stLastShowTime.XTURFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTUCFECErrors")) {
            pstChannelStats->stLastShowTime.XTUCFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTURHECErrors")) {
            pstChannelStats->stLastShowTime.XTURHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTUCHECErrors")) {
            pstChannelStats->stLastShowTime.XTUCHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTURCRCErrors")) {
            pstChannelStats->stLastShowTime.XTURCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "LastShowtime.XTUCCRCErrors")) {
            pstChannelStats->stLastShowTime.XTUCCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTURFECErrors")) {
            pstChannelStats->stCurrentDay.XTURFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTUCFECErrors")) {
            pstChannelStats->stCurrentDay.XTUCFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTURHECErrors")) {
            pstChannelStats->stCurrentDay.XTURHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTUCHECErrors")) {
            pstChannelStats->stCurrentDay.XTUCHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTURCRCErrors")) {
            pstChannelStats->stCurrentDay.XTURCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.XTUCCRCErrors")) {
            pstChannelStats->stCurrentDay.XTUCCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "X_RDK_LinkRetrain")) {
            if (strstr (resp_param.name, "CurrentDay.X_RDK_LinkRetrain")) {
               pstChannelStats->stCurrentDay.X_RDK_LinkRetrain = atoi(resp_param.value);
            }
            else {
               pstChannelStats->stQuarterHour.X_RDK_LinkRetrain = atoi(resp_param.value);
            }
        }
        else if (strstr (resp_param.name, "CurrentDay.X_RDK_InitErrors")) {
            pstChannelStats->stCurrentDay.X_RDK_InitErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.X_RDK_InitTimeouts")) {
            pstChannelStats->stCurrentDay.X_RDK_InitTimeouts = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.X_RDK_SeverelyErroredSecs")) {
            pstChannelStats->stCurrentDay.X_RDK_SeverelyErroredSecs = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "CurrentDay.X_RDK_ErroredSecs")) {
            pstChannelStats->stCurrentDay.X_RDK_ErroredSecs = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTURFECErrors")) {
            pstChannelStats->stQuarterHour.XTURFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTUCFECErrors")) {
            pstChannelStats->stQuarterHour.XTUCFECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTURHECErrors")) {
            pstChannelStats->stQuarterHour.XTURHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTUCHECErrors")) {
            pstChannelStats->stQuarterHour.XTUCHECErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTURCRCErrors")) {
            pstChannelStats->stQuarterHour.XTURCRCErrors = atol(resp_param.value);
        }
        else if (strstr (resp_param.name, "QuarterHour.XTUCCRCErrors")) {
            pstChannelStats->stQuarterHour.XTUCCRCErrors = atol(resp_param.value);
        }
    }

    // Free json objects
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

int xdsl_hal_dslGetXRdk_Nlm( PDML_XDSL_X_RDK_NLNM pstNlmInfo )
{
    int rc = RETURN_OK;
    int total_param_count = 0;

    hal_param_t req_param;
    hal_param_t resp_param;

    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_object *jparams = NULL;

    memset(&req_param, 0, sizeof(req_param));
    memset(&resp_param, 0, sizeof(resp_param));

    jmsg = json_hal_client_get_request_header(RPC_GET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(req_param.name, sizeof(req_param.name), XDSL_NLNM_INFO);
    if( json_hal_add_param(jmsg, GET_REQUEST_MESSAGE, &req_param) != RETURN_OK)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    if(jreply_msg == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        return RETURN_ERR;
    }

    if (json_object_object_get_ex(jreply_msg, JSON_RPC_FIELD_PARAMS, &jparams))
    {
        total_param_count = json_object_array_length(jparams);
    }

    if(jparams == NULL)
    {
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }

    for (int index = 0; index < total_param_count; index++)
    {
        rc = json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &resp_param);
        if (rc != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return rc;
        }
        if (strstr (resp_param.name, "echotonoiseratio")) {
            pstNlmInfo->echotonoiseratio = strtol(resp_param.value, NULL, 10);
        }
    }
    // Free json objects
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}

#if !defined  _DT_WAN_Manager_Enable_
static ANSC_STATUS configure_xdsl_driver()
{
    int rc = RETURN_OK;
    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_bool status = FALSE;

    jmsg = json_hal_client_get_request_header(RPC_SET_PARAMETERS_REQUEST);
    CHECK(jmsg);
    hal_param_t req_msg;
    memset(&req_msg, 0,sizeof(req_msg));
    strncpy(req_msg.name, XDSL_LINE_PROFILE, sizeof(req_msg.name));
    req_msg.type = PARAM_STRING;
    snprintf(req_msg.value, sizeof(req_msg.value), "%s,%s,%s,%s", "8b", "12a", "17a", "35b");
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &req_msg);
    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));
    if (json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }
    CHECK(jreply_msg);

    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d configure request is successful ", __FUNCTION__, __LINE__));
            rc = RETURN_OK;
        }
        else
        {
            CcspTraceError(("%s - %d - Set request failed \n", __FUNCTION__, __LINE__));
            rc = RETURN_ERR;
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
        rc = RETURN_ERR;
    }

    // Free json objects.
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);

    return rc;
}
#endif

ANSC_STATUS xtm_hal_setLinkInfoParam(hal_param_t *set_param)
{
    CHECK(set_param != NULL);

    /**
     * XTM Link Enable/Disable
     */
    json_object *jreply_msg;
    json_object *jrequest;
    int rc = ANSC_STATUS_FAILURE;
    json_bool status = FALSE;

    jrequest = create_json_request_message(SET_REQUEST_MESSAGE, set_param->name, set_param->type, set_param->value);
    CHECK(jrequest != NULL);

    if (json_hal_client_send_and_get_reply(jrequest, &jreply_msg) == RETURN_ERR)
    {
        CcspTraceError(("%s - %d Failed to get reply for the json request \n", __FUNCTION__, __LINE__));
        return rc;
    }

    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d Set request for [%s] is successful ", __FUNCTION__, __LINE__, set_param->name));
            rc = ANSC_STATUS_SUCCESS;
        }
        else
        {
            CcspTraceError(("%s - %d - Set request for [%s] is failed \n", __FUNCTION__, __LINE__, set_param->name));
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
    }

    // Free json objects.
    if (jrequest)
    {
        json_object_put(jrequest);
        jrequest = NULL;
    }

    if (jreply_msg)
    {
        json_object_put(jreply_msg);
        jreply_msg = NULL;
    }
    return rc;
}

ANSC_STATUS xtm_hal_getLinkInfoParam(hal_param_t *get_param)
{

    CHECK(get_param != NULL);

    /**
     * XTM Link info mainly required the following params:
     * 1. Enable Status
     * 2. Link Status
     * 3. Mac Address
     */
    int rc = ANSC_STATUS_SUCCESS;

    rc = get_link_info(get_param);
    if (rc != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s - %d Failed to get PTM link info for param [%s]  \n", __FUNCTION__, __LINE__, get_param->name));
    }

    return rc;
}

ANSC_STATUS xtm_hal_getLinkStats(const CHAR *param_name, PDML_PTM_STATS link_stats)
{
    CHECK(param_name != NULL);
    CHECK(link_stats != NULL);

    ANSC_STATUS rc = ANSC_STATUS_SUCCESS;

    json_object *jreply_msg = NULL;
    json_object *jrequest = create_json_request_message(GET_REQUEST_MESSAGE, param_name, NULL_TYPE , NULL);
    CHECK(jrequest != NULL);

    if (json_hal_client_send_and_get_reply(jrequest, &jreply_msg) == RETURN_ERR)
    {
        CcspTraceError(("%s - %d Failed to get reply for the json request \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    rc = get_ptm_link_stats(jreply_msg, link_stats);
    if (rc != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s - %d Failed to get statistics data  \n", __FUNCTION__, __LINE__));
    }

        // Free json objects.
    if (jrequest)
    {
        json_object_put(jrequest);
        jrequest = NULL;
    }

    if (jreply_msg)
    {
        json_object_put(jreply_msg);
        jreply_msg = NULL;
    }
    return rc;
}

static ANSC_STATUS get_link_info(hal_param_t *get_param)
{
    CHECK(get_param != NULL);

    json_object *jreply_msg;
    json_object *jrequest;
    hal_param_t param;
    int rc = ANSC_STATUS_FAILURE;

    jrequest = create_json_request_message(GET_REQUEST_MESSAGE, get_param->name, NULL_TYPE , NULL);
    CHECK(jrequest != NULL);

    CcspTraceInfo(("%s - %d Json request message = %s \n", __FUNCTION__, __LINE__, json_object_to_json_string_ext(jrequest, JSON_C_TO_STRING_PRETTY)));

    if (json_hal_client_send_and_get_reply(jrequest, &jreply_msg) == RETURN_ERR)
    {
        CcspTraceError(("%s - %d Failed to get reply for the json request \n", __FUNCTION__, __LINE__));
        return rc;
    }

    CHECK(jreply_msg != NULL);
    CcspTraceInfo(("Got Json response \n = %s \n", json_object_to_json_string_ext(jreply_msg, JSON_C_TO_STRING_PRETTY)));

    if (json_hal_get_param(jreply_msg, JSON_RPC_PARAM_ARR_INDEX, GET_RESPONSE_MESSAGE, &param) != RETURN_OK)
    {
        CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
        return rc;
    }

    strncpy(get_param->value, param.value, sizeof(param.value));

    // Free json objects.
    if (jrequest)
    {
        json_object_put(jrequest);
        jrequest = NULL;
    }

    if (jreply_msg)
    {
        json_object_put(jreply_msg);
        jreply_msg = NULL;
    }
    return ANSC_STATUS_SUCCESS;
}

static ANSC_STATUS get_ptm_link_stats(const json_object *reply_msg, PDML_PTM_STATS link_stats)
{
    ANSC_STATUS rc = ANSC_STATUS_SUCCESS;
    int total_param_count = 0;

    total_param_count = json_hal_get_total_param_count(reply_msg);
    hal_param_t resp_param;

    /**
     * Traverse through each index and retrieve value.
     */
    for (int i = 0; i < total_param_count; ++i)
    {
        if (json_hal_get_param(reply_msg, i, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get the param from response message [index = %d] \n", __FUNCTION__, __LINE__, i));
            continue;
        }

        if (strstr(resp_param.name, "BytesSent"))
        {
            link_stats->BytesSent = atol(resp_param.value);
        }
        else if (strstr(resp_param.name, "BytesReceived"))
        {
            link_stats->BytesReceived = atol(resp_param.value);
        }
        else if (strstr(resp_param.name, "PacketsSent"))
        {
            if (strstr(resp_param.name, "Unicast"))
            {
                link_stats->UnicastPacketsSent = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Discard"))
            {
                link_stats->DiscardPacketsSent = atoi(resp_param.value);
            }
            else if (strstr(resp_param.name, "Multicast"))
            {
                link_stats->MulticastPacketsSent = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Broadcast"))
            {
                link_stats->BroadcastPacketsSent = atol(resp_param.value);
            }
            else
            {
                link_stats->PacketsSent = atol(resp_param.value);
            }
        }
        else if (strstr(resp_param.name, "PacketsReceived"))
        {
            if (strstr(resp_param.name, "Unicast"))
            {
                link_stats->UnicastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Discard"))
            {
                link_stats->DiscardPacketsReceived = atoi(resp_param.value);
            }
            else if (strstr(resp_param.name, "Multicast"))
            {
                link_stats->MulticastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Broadcast"))
            {
                link_stats->BroadcastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "UnknownProto"))
            {
                link_stats->UnknownProtoPacketsReceived = atoi(resp_param.value);
            }
            else
            {
                link_stats->PacketsReceived = atol(resp_param.value);
            }
        }
        else if (strstr(resp_param.name, "ErrorsSent"))
        {
            link_stats->ErrorsSent = atoi(resp_param.value);
        }
        else if (strstr(resp_param.name, "ErrorsReceived"))
        {
            link_stats->ErrorsReceived = atoi(resp_param.value);
        }
    }

    CcspTraceInfo(("%s - %d Statistics Information \n", __FUNCTION__, __LINE__));
    CcspTraceInfo(("BytesSent = %ld \n", link_stats->BytesSent));
    CcspTraceInfo(("BytesReceived = %ld \n", link_stats->BytesReceived));
    CcspTraceInfo(("PacketsSent = %ld \n", link_stats->PacketsSent));
    CcspTraceInfo(("PacketsReceived = %ld \n", link_stats->PacketsReceived));
    CcspTraceInfo(("ErrorsSent = %d \n", link_stats->ErrorsSent));
    CcspTraceInfo(("ErrorsReceived = %d \n", link_stats->ErrorsReceived));
    CcspTraceInfo(("UnicastPacketsSent = %ld \n", link_stats->UnicastPacketsSent));
    CcspTraceInfo(("UnicastPacketsReceived = %ld \n", link_stats->UnicastPacketsReceived));
    CcspTraceInfo(("DiscardPacketsReceived = %d \n", link_stats->DiscardPacketsReceived));
    CcspTraceInfo(("DiscardPacketsSent = %d \n", link_stats->DiscardPacketsSent));
    CcspTraceInfo(("DiscardPacketsReceived = %d \n", link_stats->DiscardPacketsReceived));
    CcspTraceInfo(("MulticastPacketsSent = %ld \n", link_stats->MulticastPacketsSent));
    CcspTraceInfo(("MulticastPacketsReceived = %ld \n", link_stats->MulticastPacketsReceived));
    CcspTraceInfo(("BroadcastPacketsSent = %ld \n", link_stats->BroadcastPacketsSent));
    CcspTraceInfo(("BroadcastPacketsReceived = %ld \n", link_stats->BroadcastPacketsReceived));
    CcspTraceInfo(("UnknownProtoPacketsReceived = %d \n", link_stats->UnknownProtoPacketsReceived));

    return ANSC_STATUS_SUCCESS;
}

static json_object *create_json_request_message(eActionType request_type, const CHAR *param_name, eParamType type, CHAR *param_val)
{
    CcspTraceInfo(("%s - %d [VAV] Enter \n", __FUNCTION__, __LINE__));

    json_object *jrequest = NULL;
    json_object *jparam_arr = NULL;
    json_object *jparam_obj = NULL;
    hal_param_t stParam;
    memset(&stParam, 0, sizeof(stParam));
    json_bool val;
    switch (request_type)
    {
    case SET_REQUEST_MESSAGE:
        /**
         * In case PTM, we have only Device.PTM.Link.1.Enable Set operation.
         * As of now handled it only here.
         */
        jrequest = json_hal_client_get_request_header(SET_PARAMETER_METHOD);
        strncpy(stParam.name, param_name, sizeof(stParam.name) - 1);
        stParam.type = type;
        switch (type)
        {
        case PARAM_BOOLEAN:
            val = atoi(param_val);
            if (val)
            {
                strncpy(stParam.value, "true", sizeof(stParam.value));
            }
            else
            {
                strncpy(stParam.value, "false", sizeof(stParam.value));
            }
            json_hal_add_param(jrequest, SET_REQUEST_MESSAGE, &stParam);
            break;
        }
        break;
    case GET_REQUEST_MESSAGE:
        jrequest = json_hal_client_get_request_header(GET_PARAMETER_METHOD);
        strncpy(stParam.name, param_name, sizeof(stParam.name) - 1);
        json_hal_add_param(jrequest, GET_REQUEST_MESSAGE, &stParam);
        break;
    }
    return jrequest;
}

ANSC_STATUS atm_hal_setLinkInfoParam(PDML_ATM config)
{
    int rc = RETURN_OK;
    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_bool status = FALSE;
    hal_param_t param;

    if (NULL == config)
    {
        CcspTraceError(("Error: Invalid arguement \n"));
        return RETURN_ERR;
    }

    jmsg = json_hal_client_get_request_header(RPC_SET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    snprintf(param.name, sizeof(param), ATM_LINK_ENABLE, config->InstanceNumber);
    if (config->Enable)
        snprintf(param.value, sizeof(param.value), "%s", "true");
    else
        snprintf(param.value, sizeof(param.value), "%s", "false");
    param.type = PARAM_BOOLEAN;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    if (config->Enable)
    {
        /* At this point json message has been set with atm enabled or disabled parameter 
           based on link establishment or link disconnection. Set remianing params 
           if this is an atm interface addition/enabled case */
#ifndef _SR300_PRODUCT_REQ_
        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_NAME, config->InstanceNumber);
        snprintf(param.value, sizeof(param.value), "%s", config->Name);
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);
#endif // _SR300_PRODUCT_REQ_

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_DESTINATIONADDRESS, config->InstanceNumber);
        snprintf(param.value, sizeof(param.value), "%s", config->DestinationAddress);
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_ENCAPSULATION, config->InstanceNumber);
        switch(config->Encapsulation)
        {
            case LLC:
                snprintf(param.value, sizeof(param.value), "%s", "LLC");
                break;
            case VCMUX:
                snprintf(param.value, sizeof(param.value), "%s", "VCMUX");
                break;
        }
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

#ifndef _SR300_PRODUCT_REQ_
        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_AAL, config->InstanceNumber);
        switch(config->AAL)
        {
            case AAL1:
                snprintf(param.value, sizeof(param.value), "%s", "AAL1");
                break;
            case AAL2:
                snprintf(param.value, sizeof(param.value), "%s", "AAL2");
                break;
            case AAL3:
                snprintf(param.value, sizeof(param.value), "%s", "AAL3");
                break;
            case AAL4:
                snprintf(param.value, sizeof(param.value), "%s", "AAL4");
                break;
            case AAL5:
                snprintf(param.value, sizeof(param.value), "%s", "AAL5");
                break;
        }
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);
#endif // _SR300_PRODUCT_REQ_

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_LINKTYPE, config->InstanceNumber);
        switch(config->LinkType)
        {
#ifdef _SR300_PRODUCT_REQ_
            case EOA:
                snprintf(param.value, sizeof(param.value), "%s", "EoA");
                break;
            case IPOA:
                snprintf(param.value, sizeof(param.value), "%s", "IPoA");
                break;
            case PPPOA:
                snprintf(param.value, sizeof(param.value), "%s", "PPPoA");
                break;
#else
            case EOA:
                snprintf(param.value, sizeof(param.value), "%s", "EOA");
                break;
            case IPOA:
                snprintf(param.value, sizeof(param.value), "%s", "IPOA");
                break;
            case PPPOA:
                snprintf(param.value, sizeof(param.value), "%s", "PPPOA");
                break;
#endif //_SR300_PRODUCT_REQ_
            case CIP:
                snprintf(param.value, sizeof(param.value), "%s", "CIP");
                break;
            case UNCONFIGURED:
                snprintf(param.value, sizeof(param.value), "%s", "UNCONFIGURED");
                break;
        }
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_QOS_QOSCLASS, config->InstanceNumber);
        switch(config->Qos.QoSClass)
        {
            case UBR:
                snprintf(param.value, sizeof(param.value), "%s", "UBR");
                break;
            case CBR:
                snprintf(param.value, sizeof(param.value), "%s", "CBR");
                break;
            case GFR:
                snprintf(param.value, sizeof(param.value), "%s", "GFR");
                break;
            case VBR_NRT:
                snprintf(param.value, sizeof(param.value), "%s", "VBR-nrt");
                break;
            case VBR_RT:
                snprintf(param.value, sizeof(param.value), "%s", "VBR-rt");
                break;
            case UBR_PLUS:
                snprintf(param.value, sizeof(param.value), "%s", "UBR+");
                break;
            case ABR:
                snprintf(param.value, sizeof(param.value), "%s", "ABR");
                break;
        }
        param.type = PARAM_STRING;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_QOS_PEAKCELLRATE, config->InstanceNumber);
        snprintf(param.value, sizeof(param.value), "%d", config->Qos.PeakCellRate);
        param.type = PARAM_UNSIGNED_INTEGER;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_QOS_MAXBURSTSIZE, config->InstanceNumber);
        snprintf(param.value, sizeof(param.value), "%d", config->Qos.MaximumBurstSize);
        param.type = PARAM_UNSIGNED_INTEGER;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

        memset(&param, 0, sizeof(param));
        snprintf(param.name, sizeof(param), ATM_LINK_QOS_SUSCELLRATE, config->InstanceNumber);
        snprintf(param.value, sizeof(param.value), "%d", config->Qos.SustainableCellRate);
        param.type = PARAM_UNSIGNED_INTEGER;
        json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);
    }
    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return RETURN_ERR;
    }
    CHECK(jreply_msg);
    
    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d Set request is successful ", __FUNCTION__, __LINE__));
            rc = RETURN_OK;
        }
        else
        {
            CcspTraceError(("%s - %d - Set request is failed \n", __FUNCTION__, __LINE__));
            rc = RETURN_ERR;
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
        rc = RETURN_ERR;
    }
    
    // Free json objects.
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);
    
    return rc;

}

static ANSC_STATUS getDestinationAddress(char *Interface, char *DestinationAddress)
{
    int                     rc             = RETURN_OK;
    PDATAMODEL_ATM          pMyObject      = (PDATAMODEL_ATM)g_pBEManager->hATM;
    PSINGLE_LINK_ENTRY      pSListEntry    = NULL;
    PCONTEXT_LINK_OBJECT    pCxtLink       = NULL;
    PDML_ATM                p_Atm          = NULL;

    if (Interface == NULL || DestinationAddress == NULL)
    {
        CcspTraceError(("Error: Invalid arguement \n"));
        return RETURN_ERR;
    }

    int index = 0;
    sscanf(Interface, "%*[^0-9]%d", &index);

    pSListEntry  = AnscSListGetEntryByIndex(&pMyObject->Q_AtmList, (index-1));
    if ( pSListEntry )
    {
        pCxtLink  = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
        p_Atm  = (PDML_ATM) pCxtLink->hContext;
        if (p_Atm)
        {
            strncpy(DestinationAddress, p_Atm->DestinationAddress, sizeof(DestinationAddress) - 1);
        }
        else
        {
            rc = RETURN_ERR;
        }
    }
    else
    {
        rc = RETURN_ERR;
    }

    return rc;
}

ANSC_STATUS atm_hal_startAtmLoopbackDiagnostics(PDML_ATM_DIAG pDiag)
{
    json_object *jmsg = NULL;
    json_object *jreply_msg = NULL;
    json_bool status = FALSE;
    hal_param_t param;
    int total_param_count = 0;
    char DestinationAddress[128] = {'\0'};
    int index = 0;
    
    if (NULL == pDiag)
    {
        CcspTraceError(("Error: Invalid arguement \n"));
        return ANSC_STATUS_FAILURE;
    }
    
    jmsg = json_hal_client_get_request_header(RPC_SET_PARAMETERS_REQUEST);
    CHECK(jmsg);

    memset(&param, 0, sizeof(param));
    snprintf(param.name, sizeof(param), ATM_LINK_DIAGNOSTICS_INTERFACE);
    snprintf(param.value, sizeof(param.value), "%s", pDiag->Interface);
    param.type = PARAM_STRING;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    /* Get the destination address of the given interface */
    if ( getDestinationAddress(pDiag->Interface, DestinationAddress) == RETURN_ERR)
    {
        CcspTraceError(("Error: getDestinationAddress failed \n"));
        FREE_JSON_OBJECT(jmsg);
        return ANSC_STATUS_FAILURE;
    }

    sscanf(pDiag->Interface, "%*[^0-9]%d", &index);

    memset(&param, 0, sizeof(param));
    snprintf(param.name, sizeof(param), ATM_LINK_DESTINATIONADDRESS, index);
    snprintf(param.value, sizeof(param.value), "%s", DestinationAddress);
    param.type = PARAM_STRING;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    memset(&param, 0, sizeof(param));
    snprintf(param.name, sizeof(param), ATM_LINK_DIAGNOSTICS_STATE);
    switch(pDiag->DiagnosticsState)
    {
        case DIAG_STATE_REQUESTED:
            snprintf(param.value, sizeof(param.value), "%s", "Requested");
            break;
    }
    param.type = PARAM_STRING;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    memset(&param, 0, sizeof(param));
    snprintf(param.name, sizeof(param), ATM_LINK_DIAGNOSTICS_REPETITIONS);
    snprintf(param.value, sizeof(param.value), "%d", pDiag->NumberOfRepetitions);
    param.type = PARAM_UNSIGNED_INTEGER;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    memset(&param, 0, sizeof(param));
    snprintf(param.name, sizeof(param), ATM_LINK_DIAGNOSTICS_TIMEOUT);
    snprintf(param.value, sizeof(param.value), "%d", pDiag->Timeout);
    param.type = PARAM_UNSIGNED_INTEGER;
    json_hal_add_param(jmsg, SET_REQUEST_MESSAGE, &param);

    CcspTraceInfo(("JSON Request message = %s \n", json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if( json_hal_client_send_and_get_reply(jmsg, &jreply_msg) != RETURN_OK)
    {
        CcspTraceError(("[%s][%d] RPC message failed \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return ANSC_STATUS_FAILURE;
    }
    CHECK(jreply_msg);

    if (json_hal_get_result_status(jreply_msg, &status) == RETURN_OK)
    {
        if (status)
        {
            CcspTraceInfo(("%s - %d Set request is successful ", __FUNCTION__, __LINE__));
        }
        else
        {
            CcspTraceError(("%s - %d - Set request is failed \n", __FUNCTION__, __LINE__));
        }
    }
    else
    {
        CcspTraceError(("%s - %d Failed to get result status from json response, something wrong happened!!! \n", __FUNCTION__, __LINE__));
        FREE_JSON_OBJECT(jmsg);
        FREE_JSON_OBJECT(jreply_msg);
        return ANSC_STATUS_FAILURE;
    }

    jmsg = create_json_request_message(GET_REQUEST_MESSAGE, ATM_LINK_DIAGNOSTICS, NULL_TYPE , NULL);
    CHECK(jmsg != NULL);

    CcspTraceInfo(("%s - %d Json request message = %s \n", __FUNCTION__, __LINE__, json_object_to_json_string_ext(jmsg, JSON_C_TO_STRING_PRETTY)));

    if (json_hal_client_send_and_get_reply(jmsg, &jreply_msg) == RETURN_ERR)
    {
        CcspTraceError(("%s - %d Failed to get reply for the json request \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CHECK(jreply_msg != NULL);
    CcspTraceInfo(("Got Json response \n = %s \n", json_object_to_json_string_ext(jreply_msg, JSON_C_TO_STRING_PRETTY)));

    total_param_count = json_hal_get_total_param_count(jreply_msg);

    for (int index = 0; index < total_param_count; index++)
    {
        if (json_hal_get_param(jreply_msg, index, GET_RESPONSE_MESSAGE, &param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get required params from the response message \n", __FUNCTION__, __LINE__));
            FREE_JSON_OBJECT(jmsg);
            FREE_JSON_OBJECT(jreply_msg);
            return ANSC_STATUS_FAILURE;
        }
 
        if (strstr (param.name, "DiagnosticsState"))
        {
            if (strstr (param.value, "None"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_NONE;
            }
            else if (strstr (param.value, "Requested"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_REQUESTED;
            }
            else if (strstr (param.value, "Canceled"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_CANCELED;
            }
            else if (strstr (param.value, "Complete"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_COMPLETE;
            }
            else if (strstr (param.value, "Error"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_ERROR;
            }
            else if (strstr (param.value, "Error_Internal"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_ERROR_INTERNAL;
            }
            else if (strstr (param.value, "Error_Other"))
            {
                pDiag->DiagnosticsState = DIAG_STATE_ERROR_OTHER;
            }
        }
        else if (strstr (param.name, "SuccessCount"))
        {
            pDiag->SuccessCount = strtol(&(param.value), NULL, 10);
        }
        else if (strstr (param.name, "FailureCount"))
        {
            pDiag->FailureCount = strtol(&(param.value), NULL, 10);
        }
        else if (strstr (param.name, "AverageResponseTime"))
        {
            pDiag->AverageResponseTime = strtol(&(param.value), NULL, 10);
        }
        else if (strstr (param.name, "MinimumResponseTime"))
        {
            pDiag->MinimumResponseTime = strtol(&(param.value), NULL, 10);
        }
        else if (strstr (param.name, "MaximumResponseTime"))
        {
            pDiag->MaximumResponseTime = strtol(&(param.value), NULL, 10);
        }
    }

    // Free json objects.
    FREE_JSON_OBJECT(jmsg);
    FREE_JSON_OBJECT(jreply_msg);
    
    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS atm_hal_getLinkStats(const CHAR *param_name, PDML_ATM_STATS link_stats)
{
    CHECK(param_name != NULL);
    CHECK(link_stats != NULL);

    ANSC_STATUS rc = ANSC_STATUS_SUCCESS;

    json_object *jreply_msg = NULL;
    json_object *jrequest = create_json_request_message(GET_REQUEST_MESSAGE, param_name, NULL_TYPE , NULL);
    CHECK(jrequest != NULL);

    if (json_hal_client_send_and_get_reply(jrequest, &jreply_msg) == RETURN_ERR)
    {
        CcspTraceError(("%s - %d Failed to get reply for the json request \n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    rc = get_atm_link_stats(jreply_msg, link_stats);
    if (rc != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s - %d Failed to get statistics data  \n", __FUNCTION__, __LINE__));
    }

        // Free json objects.
    if (jrequest)
    {
        json_object_put(jrequest);
        jrequest = NULL;
    }

    if (jreply_msg)
    {
        json_object_put(jreply_msg);
        jreply_msg = NULL;
    }
    return rc;
}

static ANSC_STATUS get_atm_link_stats(const json_object *reply_msg, PDML_ATM_STATS link_stats)
{
    ANSC_STATUS rc = ANSC_STATUS_SUCCESS;
    int total_param_count = 0;

    total_param_count = json_hal_get_total_param_count(reply_msg);
    hal_param_t resp_param;

    /**
     * Traverse through each index and retrieve value.
     */
    for (int i = 0; i < total_param_count; ++i)
    {
        if (json_hal_get_param(reply_msg, i, GET_RESPONSE_MESSAGE, &resp_param) != RETURN_OK)
        {
            CcspTraceError(("%s - %d Failed to get the param from response message [index = %d] \n", __FUNCTION__, __LINE__, i));
            continue;
        }

        if (strstr(resp_param.name, "BytesSent"))
        {
            link_stats->BytesSent = atol(resp_param.value);
        }
        else if (strstr(resp_param.name, "BytesReceived"))
        {
            link_stats->BytesReceived = atol(resp_param.value);
        }
        else if (strstr(resp_param.name, "PacketsSent"))
        {
            if (strstr(resp_param.name, "Unicast"))
            {
                link_stats->UnicastPacketsSent = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Discard"))
            {
                link_stats->DiscardPacketsSent = atoi(resp_param.value);
            }
            else if (strstr(resp_param.name, "Multicast"))
            {
                link_stats->MulticastPacketsSent = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Broadcast"))
            {
                link_stats->BroadcastPacketsSent = atol(resp_param.value);
            }
            else
            {
                link_stats->PacketsSent = atol(resp_param.value);
            }
        }
        else if (strstr(resp_param.name, "PacketsReceived"))
        {
            if (strstr(resp_param.name, "Unicast"))
            {
                link_stats->UnicastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Discard"))
            {
                link_stats->DiscardPacketsReceived = atoi(resp_param.value);
            }
            else if (strstr(resp_param.name, "Multicast"))
            {
                link_stats->MulticastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "Broadcast"))
            {
                link_stats->BroadcastPacketsReceived = atol(resp_param.value);
            }
            else if (strstr(resp_param.name, "UnknownProto"))
            {
                link_stats->UnknownProtoPacketsReceived = atoi(resp_param.value);
            }
            else
            {
                link_stats->PacketsReceived = atol(resp_param.value);
            }
        }
        else if (strstr(resp_param.name, "ErrorsSent"))
        {
            link_stats->ErrorsSent = atoi(resp_param.value);
        }
        else if (strstr(resp_param.name, "ErrorsReceived"))
        {
            link_stats->ErrorsReceived = atoi(resp_param.value);
        }
    }

    CcspTraceDebug(("%s - %d Statistics Information \n", __FUNCTION__, __LINE__));
    CcspTraceDebug(("BytesSent = %ld \n", link_stats->BytesSent));
    CcspTraceDebug(("BytesReceived = %ld \n", link_stats->BytesReceived));
    CcspTraceDebug(("PacketsSent = %ld \n", link_stats->PacketsSent));
    CcspTraceDebug(("PacketsReceived = %ld \n", link_stats->PacketsReceived));
    CcspTraceDebug(("ErrorsSent = %d \n", link_stats->ErrorsSent));
    CcspTraceDebug(("ErrorsReceived = %d \n", link_stats->ErrorsReceived));
    CcspTraceDebug(("UnicastPacketsSent = %ld \n", link_stats->UnicastPacketsSent));
    CcspTraceDebug(("UnicastPacketsReceived = %ld \n", link_stats->UnicastPacketsReceived));
    CcspTraceDebug(("DiscardPacketsReceived = %d \n", link_stats->DiscardPacketsReceived));
    CcspTraceDebug(("DiscardPacketsSent = %d \n", link_stats->DiscardPacketsSent));
    CcspTraceDebug(("DiscardPacketsReceived = %d \n", link_stats->DiscardPacketsReceived));
    CcspTraceDebug(("MulticastPacketsSent = %ld \n", link_stats->MulticastPacketsSent));
    CcspTraceDebug(("MulticastPacketsReceived = %ld \n", link_stats->MulticastPacketsReceived));
    CcspTraceDebug(("BroadcastPacketsSent = %ld \n", link_stats->BroadcastPacketsSent));
    CcspTraceDebug(("BroadcastPacketsReceived = %ld \n", link_stats->BroadcastPacketsReceived));
    CcspTraceDebug(("UnknownProtoPacketsReceived = %d \n", link_stats->UnknownProtoPacketsReceived));

    return ANSC_STATUS_SUCCESS;
}


