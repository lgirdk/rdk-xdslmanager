/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
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
/**********************************************************************
   Copyright [2014] [Cisco Systems, Inc.]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
**********************************************************************/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "xtm_apis.h"
#include "xtm_internal.h"
#include "plugin_main_apis.h"
#include "xdsl_hal.h"

/* ******************************************************************* */

//VLAN Agent
#define VLAN_DBUS_PATH                    "/com/cisco/spvtg/ccsp/vlanmanager"
#define VLAN_COMPONENT_NAME               "eRT.com.cisco.spvtg.ccsp.vlanmanager"
#define VLAN_ETH_LINK_TABLE_NAME          "Device.X_RDK_Ethernet.Link."
#define VLAN_ETH_NOE_PARAM_NAME           "Device.X_RDK_Ethernet.LinkNumberOfEntries"
#define VLAN_ETH_LINK_PARAM_ALIAS         "Device.X_RDK_Ethernet.Link.%d.Alias"
#define VLAN_ETH_LINK_PARAM_NAME          "Device.X_RDK_Ethernet.Link.%d.Name"
#define VLAN_ETH_LINK_PARAM_LOWERLAYERS   "Device.X_RDK_Ethernet.Link.%d.LowerLayers"
#define VLAN_ETH_LINK_PARAM_ENABLE        "Device.X_RDK_Ethernet.Link.%d.Enable"

#define XTM_LINK_ENABLE "Device.PTM.Link.%d.Enable"
#define XTM_LINK_STATUS "Device.PTM.Link.%d.Status"
#define XTM_LINK_STATS "Device.PTM.Link.%d.Stats"

#define DATAMODEL_PARAM_LENGTH 256

extern char                g_Subsystem[32];
extern ANSC_HANDLE         bus_handle;

static ANSC_STATUS CosaDmlXtmGetParamValues(char *pComponent, char *pBus, char *pParamName, char *pReturnVal);
static ANSC_STATUS CosaDmlXtmSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag);
static ANSC_STATUS CosaDmlXtmGetParamNames(char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize);
static ANSC_STATUS CosaDmlXtmGetLowerLayersInstanceInOtherAgent(XTM_NOTIFY_ENUM enNotifyAgent, char *pLowerLayers, INT *piInstanceNumber);

/* ******************************************************************* */

/* * SListPushEntryByInsNum() */
ANSC_STATUS
SListPushEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        PCONTEXT_LINK_OBJECT   pContext
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pContextEntry = (PCONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        AnscSListPushEntryAtBack(pListHead, &pContext->Linkage);
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pContextEntry = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pContext->InstanceNumber < pContextEntry->InstanceNumber )
            {
                AnscSListPushEntryByIndex(pListHead, &pContext->Linkage, ulIndex);

                return ANSC_STATUS_SUCCESS;
            }
        }

        AnscSListPushEntryAtBack(pListHead, &pContext->Linkage);
    }

    return ANSC_STATUS_SUCCESS;
}

/* * SListGetEntryByInsNum() */
PCONTEXT_LINK_OBJECT
SListGetEntryByInsNum
    (
        PSLIST_HEADER               pListHead,
        ULONG                       InstanceNumber
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pContextEntry = (PCONTEXT_LINK_OBJECT)NULL;
    PSINGLE_LINK_ENTRY              pSLinkEntry       = (PSINGLE_LINK_ENTRY       )NULL;
    ULONG                           ulIndex           = 0;

    if ( pListHead->Depth == 0 )
    {
        return NULL;
    }
    else
    {
        pSLinkEntry = AnscSListGetFirstEntry(pListHead);

        for ( ulIndex = 0; ulIndex < pListHead->Depth; ulIndex++ )
        {
            pContextEntry = ACCESS_CONTEXT_LINK_OBJECT(pSLinkEntry);
            pSLinkEntry       = AnscSListGetNextEntry(pSLinkEntry);

            if ( pContextEntry->InstanceNumber == InstanceNumber )
            {
                return pContextEntry;
            }
        }
    }

    return NULL;
}

/* Get data from the other component. */
static ANSC_STATUS CosaDmlXtmGetParamNames(char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterInfoStruct_t **retInfo;
    char *ParamName[1];
    int ret = 0,
        nval;

    ret = CcspBaseIf_getParameterNames(
        bus_handle,
        pComponent,
        pBus,
        pParamName,
        1,
        &nval,
        &retInfo);
    
    if (CCSP_SUCCESS == ret)
    {
        int iLoopCount;
        *pReturnSize = nval;
    
        for (iLoopCount = 0; iLoopCount < nval; iLoopCount++)
        {
            if (NULL != retInfo[iLoopCount]->parameterName)
            {
                snprintf(a2cReturnVal[iLoopCount], strlen(retInfo[iLoopCount]->parameterName) + 1, "%s", retInfo[iLoopCount]->parameterName);
            }
        }
    
        if (retInfo)
        {
            free_parameterInfoStruct_t(bus_handle, nval, retInfo);
        }
    
        return ANSC_STATUS_SUCCESS;
    }
    
    if (retInfo)
    {
        free_parameterInfoStruct_t(bus_handle, nval, retInfo);
    }
    
    return ANSC_STATUS_FAILURE;
}
/* *CosaDmlXtmGetParamValues() */
static ANSC_STATUS CosaDmlXtmGetParamValues(char *pComponent, char *pBus, char *pParamName, char *pReturnVal)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t **retVal;
    char *ParamName[1];
    int ret = 0,
        nval;

    //Assign address for get parameter name
    ParamName[0] = pParamName;
    
    ret = CcspBaseIf_getParameterValues(
        bus_handle,
        pComponent,
        pBus,
        ParamName,
        1,
        &nval,
        &retVal);
    
    //Copy the value
    if (CCSP_SUCCESS == ret)
    {
        if (NULL != retVal[0]->parameterValue)
        {
            memcpy(pReturnVal, retVal[0]->parameterValue, strlen(retVal[0]->parameterValue) + 1);
        }
        if (retVal)
        {
            free_parameterValStruct_t(bus_handle, nval, retVal);
        }
        return ANSC_STATUS_SUCCESS;
    }
    
    if (retVal)
    {
        free_parameterValStruct_t(bus_handle, nval, retVal);
    }
    
    return ANSC_STATUS_FAILURE;
}

/* Notification to the Other component. */
static ANSC_STATUS CosaDmlXtmSetParamValues(const char *pComponent, const char *pBus, const char *pParamName, const char *pParamVal, enum dataType_e type, unsigned int bCommitFlag)
{
    CCSP_MESSAGE_BUS_INFO *bus_info = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t param_val[1] = {0};
    char *faultParam = NULL;
    int ret = 0;

    param_val[0].parameterName = pParamName;
    param_val[0].parameterValue = pParamVal;
    param_val[0].type = type;

    ret = CcspBaseIf_setParameterValues(
        bus_handle,
        pComponent,
        pBus,
        0,
        0,
        param_val,
        1,
        bCommitFlag,
        &faultParam);
    
    if ((ret != CCSP_SUCCESS) && (faultParam != NULL))
    {
        CcspTraceError(("%s-%d Failed to set %s\n", __FUNCTION__, __LINE__, pParamName));
        bus_info->freefunc(faultParam);
        return ANSC_STATUS_FAILURE;
    }
    
    return ANSC_STATUS_SUCCESS;
}

/* * CosaDmlXtmGetLowerLayersInstanceInOtherAgent() */
static ANSC_STATUS CosaDmlXtmGetLowerLayersInstanceInOtherAgent(XTM_NOTIFY_ENUM enNotifyAgent, char *pLowerLayers, INT *piInstanceNumber)
{
    //Validate buffer
    if ((NULL == pLowerLayers) || (NULL == piInstanceNumber))
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise default value
    *piInstanceNumber = -1;
    
    switch (enNotifyAgent)
    {
        case NOTIFY_TO_VLAN_AGENT:
        {
           char acTmpReturnValue[256] = {0},
                a2cTmpTableParams[10][256] = {0};
           INT iLoopCount,
               iTotalNoofEntries;
        
            if (ANSC_STATUS_FAILURE == CosaDmlXtmGetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETH_NOE_PARAM_NAME, acTmpReturnValue))
            {
               CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
               return ANSC_STATUS_FAILURE;
            }

            //Total count
            iTotalNoofEntries = atoi(acTmpReturnValue);
       
            CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));
        
            if (0 >= iTotalNoofEntries)
            {
              return ANSC_STATUS_SUCCESS;
            }

            //Get table names
            iTotalNoofEntries = 0;
        
            if (ANSC_STATUS_FAILURE == CosaDmlXtmGetParamNames(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, VLAN_ETH_LINK_TABLE_NAME, a2cTmpTableParams, &iTotalNoofEntries))
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                return ANSC_STATUS_FAILURE;
            }

            //Traverse from loop
            for (iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++)
            {
                char acTmpQueryParam[256] = {0};
                
                //Query
                snprintf(acTmpQueryParam, sizeof(acTmpQueryParam), "%sLowerLayers", a2cTmpTableParams[iLoopCount]);
                memset(acTmpReturnValue, 0, sizeof(acTmpReturnValue));
                
                if (ANSC_STATUS_FAILURE == CosaDmlXtmGetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue))
                {
                   CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                   continue;
                }

                //Compare lowerlayers
                if (0 == strcmp(acTmpReturnValue, pLowerLayers))
                {
                    char tmpTableParam[256] = {0};
                    const char *last_two;
                
                    //Copy table param
                    snprintf(tmpTableParam, sizeof(tmpTableParam), "%s", a2cTmpTableParams[iLoopCount]);
                
                    //Get last two chareters from return value and cut the instance
                    last_two = &tmpTableParam[strlen(tmpTableParam) - 2];
                    *piInstanceNumber = atoi(last_two);
                    break;
                 }
             }
          }
          break; /* * NOTIFY_TO_XTM_MANAGER */
   
          default:
          {
               CcspTraceError(("%s Invalid Case\n", __FUNCTION__));
          }
          break; /* * default */
       }

   return ANSC_STATUS_SUCCESS;
}

/* Create and Enable Ethernet.Link. */
ANSC_STATUS DmlXtmCreateEthLink( PDML_XTM   pEntry )
{
    char *acSetParamName = NULL;
    char *acSetParamValue = NULL;
    INT  iVLANInstance = -1;
    
    //Validate buffer
    if ( NULL == pEntry )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    
    //Get Instance for corresponding lower layer
    CosaDmlXtmGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_VLAN_AGENT, pEntry->Path, &iVLANInstance );
    
    //Create VLAN Link.
    //Index is not present. so needs to create a PTM instance
    if (-1 == iVLANInstance)
    {
        char acTableName[128] = {0};
        INT iNewTableInstance = -1;
    
        sprintf(acTableName, "%s", VLAN_ETH_LINK_TABLE_NAME);
        if (CCSP_SUCCESS != CcspBaseIf_AddTblRow(
                                bus_handle,
                                VLAN_COMPONENT_NAME,
                                VLAN_DBUS_PATH,
                                0, /* session id */
                                acTableName,
                                &iNewTableInstance))
        {
            CcspTraceError(("%s Failed to add table %s\n", __FUNCTION__, acTableName));
            return ANSC_STATUS_FAILURE;
        }
    
        //Assign new instance
        iVLANInstance = iNewTableInstance;
    }

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    memset(acSetParamName, 0, DATAMODEL_PARAM_LENGTH);
    memset(acSetParamValue, 0, DATAMODEL_PARAM_LENGTH);

    CcspTraceInfo(("%s %d VLANAgent -> Device.Ethernet.Link Instance:%d\n", __FUNCTION__, __LINE__, iVLANInstance));

    //Set Alias
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, VLAN_ETH_LINK_PARAM_ALIAS, iVLANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", pEntry->Alias);
    CosaDmlXtmSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string,FALSE);
    
    //Set Name
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, VLAN_ETH_LINK_PARAM_NAME, iVLANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", pEntry->Name);
    CosaDmlXtmSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string,FALSE);

    //Set Lowerlayers
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, VLAN_ETH_LINK_PARAM_LOWERLAYERS, iVLANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", pEntry->Path);
    CosaDmlXtmSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string,FALSE);
    
    //Set Enable
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, VLAN_ETH_LINK_PARAM_ENABLE, iVLANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "true");
    CosaDmlXtmSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE);
 
    if(acSetParamName != NULL)
    {
       free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
       free(acSetParamValue);
    }
    
    CcspTraceInfo(("%s: Successfully created XTM VLAN link table for %s interface\n",XTM_MARKER_VLAN_CREATE,pEntry->Name));
    
    return ANSC_STATUS_SUCCESS;
}

/* Disable and delete Eth link. (Ethernet.Link.) */
ANSC_STATUS DmlXtmDeleteEthLink( char *pLowerLayer )
{
    char    *acSetParamName = NULL;
    char    *acSetParamValue = NULL;
    char    acTableName[128] = { 0 };
    INT     iVLANInstance = -1;

    //Validate buffer
    if ( NULL == pLowerLayer )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    
    //Get Instance for corresponding lower layer
    CosaDmlXtmGetLowerLayersInstanceInOtherAgent(NOTIFY_TO_VLAN_AGENT, pLowerLayer, &iVLANInstance);
    
    //Index is not present. so no need to do anything any ETH Link instance
    if ( -1 == iVLANInstance )
    { 
        CcspTraceError(("%s %d Device.Ethernet.Link Table instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }
    
    CcspTraceInfo(("%s %d VLANAgent -> Device.Ethernet.Link Instance:%d\n", __FUNCTION__, __LINE__, iVLANInstance));

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        return ANSC_STATUS_FAILURE;
    }

    memset(acSetParamName, 0, DATAMODEL_PARAM_LENGTH);
    memset(acSetParamValue, 0, DATAMODEL_PARAM_LENGTH);

    //Disable link.
    snprintf(acSetParamName, DATAMODEL_PARAM_LENGTH, VLAN_ETH_LINK_PARAM_ENABLE, iVLANInstance);
    snprintf(acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false");
    CosaDmlXtmSetParamValues(VLAN_COMPONENT_NAME, VLAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE);

    if(acSetParamName != NULL)
    {
        free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
        free(acSetParamValue);
    }

    //Delay - to set param.
    sleep(2);

    /*
     * Delete Device.Ethernet.Link. Instance.
     * VLANAgent will delete the vlan interface as part table deletion process.
     */
    sprintf(acTableName, "%s%d.", VLAN_ETH_LINK_TABLE_NAME, iVLANInstance);
    if (CCSP_SUCCESS != CcspBaseIf_DeleteTblRow(
                            bus_handle,
                            VLAN_COMPONENT_NAME,
                            VLAN_DBUS_PATH,
                            0, /* session id */
                            acTableName))
    {
        CcspTraceError(("%s Failed to delete table %s\n", __FUNCTION__, acTableName));
        return ANSC_STATUS_FAILURE;
    }
    
    CcspTraceInfo(("%s:Successfully deleted XTM VLAN link %s table\n",XTM_MARKER_VLAN_DELETE,acTableName));
    
    return ANSC_STATUS_SUCCESS;
}

/* * DmlGetXtmIfEnable */
ANSC_STATUS
DmlGetXtmIfEnable
    (
        BOOLEAN     *pbEnable
    )
{
    //Validate buffer
    if ( NULL == pbEnable )
    {
        CcspTraceError(("%s %d - Invalid buffer\n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    /**
     * Construct Full DML path.
     * Device.PTM.Link.1.Enable.
     */
    ULONG ulInstanceNumber = 0;
    hal_param_t req_param;
    memset(&req_param, 0, sizeof(req_param));

    PTMLink_GetEntry(NULL, 0, &ulInstanceNumber);
    snprintf(req_param.name, sizeof(req_param.name), XTM_LINK_ENABLE, ulInstanceNumber);
    if (ANSC_STATUS_SUCCESS != xtm_hal_getLinkInfoParam(&req_param))
    {
        CcspTraceError(("%s %d - Failed to get link enable status \n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    /**
     * Store the enable status.
     */
    *pbEnable = atoi(req_param.value);

    return ANSC_STATUS_SUCCESS;
}

/* * DmlSetXtmIfEnable */
ANSC_STATUS
DmlSetXtmIfEnable
    (
        BOOLEAN     bEnable
    )
{
    /**
     * Construct Full DML path.
     * Device.PTM.Link.1.Enable.
     */
    ULONG ulInstanceNumber = 0;
    hal_param_t req_param;
    memset(&req_param, 0, sizeof(req_param));
    PTMLink_GetEntry(NULL, 0, &ulInstanceNumber);
    snprintf(req_param.name, sizeof(req_param), XTM_LINK_ENABLE, ulInstanceNumber);
    req_param.type = PARAM_BOOLEAN;
    snprintf(req_param.value, sizeof(req_param.value), "%d", bEnable);

    if (ANSC_STATUS_SUCCESS != xtm_hal_setLinkInfoParam(&req_param))
    {
        CcspTraceError(("%s %d - Failed to set link enable\n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s:Successfully configured XTM interface\n",XTM_MARKER_VLAN_CFG_CHNG,bEnable));

    return ANSC_STATUS_SUCCESS;
}

/* * DmlGetXtmIfStatus() */
ANSC_STATUS
DmlGetXtmIfStatus
    (
        ANSC_HANDLE         hContext,
        PDML_XTM      pEntry          /* Identified by InstanceNumber */
    )
{
    ANSC_STATUS             returnStatus  = ANSC_STATUS_FAILURE;
    xtm_link_status_e linkStatus;
    hal_param_t req_param;
    memset(&req_param, 0, sizeof(req_param));
    if (pEntry != NULL)
    {
        if( pEntry->Enable) {
            /**
             * Construct Full DML path.
             * Device.PTM.Link.1.Status.
             */
            snprintf(req_param.name, sizeof(req_param.name), XTM_LINK_STATUS, pEntry->InstanceNumber);
            if (ANSC_STATUS_SUCCESS != xtm_hal_getLinkInfoParam(&req_param))
            {
                pEntry->Status = Error;
                CcspTraceError(("%s Failed to get link status\n",__FUNCTION__));
            }
            else {
                    /**
                     * Convert status message and returned.
                     */
                    if (strncmp(req_param.value, XTM_LINK_UP, strlen(XTM_LINK_UP)) == 0)
                    {
                        pEntry->Status = Up;
                    }
                    else if (strncmp(req_param.value, XTM_LINK_DOWN, strlen(XTM_LINK_DOWN)) == 0)
                    {
                        pEntry->Status = Down;
                    }
                    else if (strncmp(req_param.value, XTM_LINK_UNKNOWN, strlen(XTM_LINK_UNKNOWN)) == 0)
                    {
                        pEntry->Status = Unknown;
                    }
                    else if (strncmp(req_param.value, XTM_LINK_LOWERLAYER_DOWN, strlen(XTM_LINK_LOWERLAYER_DOWN)) == 0)
                    {
                        pEntry->Status = LowerLayerDown;
                    }
                    else
                    {
                        pEntry->Status = Error;
                    }
                returnStatus = ANSC_STATUS_SUCCESS;
            }
        }
    }

    return returnStatus;
}

ANSC_STATUS
DmlGetXtmIfStatistics
    (
        ANSC_HANDLE        hContext,
        PDML_XTM      pEntry          /* Identified by InstanceNumber */
    )
{
    int returnStatus  = ANSC_STATUS_SUCCESS;
    char cLinkStats[256] = {'\0'};
    if (pEntry != NULL) {
        if( pEntry->Enable) {
            /**
             * Construct Full DML path.
             * Device.PTM.Link.1.Stats.
             */
            snprintf(cLinkStats, sizeof(cLinkStats), XTM_LINK_STATS, pEntry->InstanceNumber);
            memset(&pEntry->Statistics, 0, sizeof(pEntry->Statistics));
            returnStatus = xtm_hal_getLinkStats(cLinkStats, &pEntry->Statistics);
            if (returnStatus != ANSC_STATUS_SUCCESS) {
                CcspTraceError(("%s %d Failed to get XTM stats\n",__FUNCTION__,__LINE__));
            }
        }
    }

    return returnStatus;
}

/* * DmlSetXtm() */
ANSC_STATUS
DmlSetXtm
    (
        ANSC_HANDLE         hContext,
        PDML_XTM      pEntry          /* Identified by InstanceNumber */
    )
{
    ANSC_STATUS             returnStatus  = ANSC_STATUS_SUCCESS;

    //TBD

    return returnStatus;
}

/* * DmlAddXtm() */
ANSC_STATUS
DmlAddXtm
    (
        ANSC_HANDLE        hContext,
        PDML_XTM      pEntry
    )
{
    ANSC_STATUS             returnStatus  = ANSC_STATUS_SUCCESS;

    if (!pEntry)
    {
        return ANSC_STATUS_FAILURE;
    }

    //Create actual interface
    returnStatus = DmlSetXtmIfEnable( pEntry->Enable );

    //Create Ethernet.Link
    returnStatus = DmlXtmCreateEthLink( pEntry );

    return returnStatus;
}

/* * DmlDelXtm() */
ANSC_STATUS
DmlDelXtm
    (
        ANSC_HANDLE        hContext,
        PDML_XTM      pEntry
    )
{
    ANSC_STATUS  returnStatus   = ANSC_STATUS_SUCCESS;

    if ( !pEntry )
    {
        return ANSC_STATUS_FAILURE;
    }
    else 
    {
        //Delete Ethernet.Link
        returnStatus = DmlXtmDeleteEthLink( pEntry->Path );
   
        //Remove actual intergace
        returnStatus = DmlSetXtmIfEnable( pEntry->Enable );
    }

    return returnStatus;
}


