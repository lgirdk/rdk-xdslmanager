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

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "xtm_apis.h"
#include "xtm_internal.h"
#include "plugin_main_apis.h"
#include "xdsl_hal.h"
#include "xdsl_event_queue.h"

/* ******************************************************************* */

#define PTM_LINK_ENABLE "Device.PTM.Link.%d.Enable"
#define PTM_LINK_STATUS "Device.PTM.Link.%d.Status"
#define PTM_LINK_STATS  "Device.PTM.Link.%d.Stats"

#define ATM_LINK_ENABLE "Device.ATM.Link.%d.Enable"
#define ATM_LINK_STATUS "Device.ATM.Link.%d.Status"
#define ATM_LINK_STATS  "Device.ATM.Link.%d.Stats"

#define DATAMODEL_PARAM_LENGTH 256

/* * DmlSetPtmIfEnable */
ANSC_STATUS DmlSetPtmIfEnable (PDML_PTM p_Ptm)
{
    //Validate PTM link
    if ( NULL == p_Ptm )
    {
        CcspTraceError(("%s %d - Invalid PTM link\n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }
    /**
     * Construct Full DML path.
     * Device.PTM.Link.1.Enable.
     */
    hal_param_t req_param;
    memset(&req_param, 0, sizeof(req_param));
    snprintf(req_param.name, sizeof(req_param), PTM_LINK_ENABLE, p_Ptm->InstanceNumber);
    req_param.type = PARAM_BOOLEAN;
    snprintf(req_param.value, sizeof(req_param.value), "%d", p_Ptm->Enable);

    if (ANSC_STATUS_SUCCESS != xtm_hal_setLinkInfoParam(&req_param))
    {
        CcspTraceError(("%s %d - Failed to set link enable\n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s:Successfully configured PTM interface\n",PTM_MARKER_VLAN_CFG_CHNG));

    return ANSC_STATUS_SUCCESS;
}

static xtm_link_status_e XtmStatusStrToEnum(char *status)
{
    if (strncmp(status, XTM_LINK_UP, strlen(XTM_LINK_UP)) == 0)
    {
	return Up;
    }
    else if (strncmp(status, XTM_LINK_DOWN, strlen(XTM_LINK_DOWN)) == 0)
    {
	return Down;
    }
    else if (strncmp(status, XTM_LINK_UNKNOWN, strlen(XTM_LINK_UNKNOWN)) == 0)
    {
	return Unknown;
    }
    else if (strncmp(status, XTM_LINK_LOWERLAYER_DOWN, strlen(XTM_LINK_LOWERLAYER_DOWN)) == 0)
    {
	return LowerLayerDown;
    }

    return Error;
}

/* * DmlGetPtmIfStatus() */
ANSC_STATUS DmlGetPtmIfStatus (ANSC_HANDLE hContext, PDML_PTM pEntry )
{
    ANSC_STATUS returnStatus = ANSC_STATUS_FAILURE;
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
            snprintf(req_param.name, sizeof(req_param.name), PTM_LINK_STATUS, pEntry->InstanceNumber);
            if (ANSC_STATUS_SUCCESS != xtm_hal_getLinkInfoParam(&req_param))
            {
                pEntry->Status = Error;
                CcspTraceError(("%s Failed to get link status\n",__FUNCTION__));
            }
            else {
		pEntry->Status = XtmStatusStrToEnum(req_param.value);
                returnStatus = ANSC_STATUS_SUCCESS;
            }
        }
    }

    return returnStatus;
}

ANSC_STATUS DmlGetPtmIfStatistics (ANSC_HANDLE hContext, PDML_PTM pEntry)
{
    int returnStatus  = ANSC_STATUS_SUCCESS;
    char cLinkStats[256] = {'\0'};
    if (pEntry != NULL) {
        if( pEntry->Enable) {
            /**
             * Construct Full DML path.
             * Device.PTM.Link.1.Stats.
             */
            snprintf(cLinkStats, sizeof(cLinkStats), PTM_LINK_STATS, pEntry->InstanceNumber);
            memset(&pEntry->Statistics, 0, sizeof(pEntry->Statistics));
            returnStatus = xtm_hal_getLinkStats(cLinkStats, &pEntry->Statistics);
            if (returnStatus != ANSC_STATUS_SUCCESS) {
                CcspTraceError(("%s %d Failed to get PTM stats\n",__FUNCTION__,__LINE__));
            }
        }
    }

    return returnStatus;
}

/* * DmlSetPtm() */
ANSC_STATUS DmlSetPtm (ANSC_HANDLE hContext, PDML_PTM pEntry)
{
    ANSC_STATUS             returnStatus  = ANSC_STATUS_SUCCESS;

    returnStatus = DmlSetPtmIfEnable( pEntry );

    return returnStatus;
}

int PtmLinkStatusCallback(int ptm_id, char *status)
{
    XDSLMSGQXtmStatusData MSGQXtmStatusData = {};

    MSGQXtmStatusData.link_id = ptm_id;
    MSGQXtmStatusData.is_atm = FALSE;
    MSGQXtmStatusData.Status = XtmStatusStrToEnum(status);
    
    DmlXtmLinkSendStatusToEventQueue(&MSGQXtmStatusData);
}

/* * DmlSetAtmIfEnable */
ANSC_STATUS DmlSetAtmIfEnable (PDML_ATM pAtm)
{
    if (ANSC_STATUS_SUCCESS != atm_hal_setLinkInfoParam(pAtm))
    {
        CcspTraceError(("%s %d - Failed to set link enable\n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("Successfully configured ATM interface\n"));

    return ANSC_STATUS_SUCCESS;
}

/* DmlStartAtmLoopbackDiagnostics */
ANSC_STATUS DmlStartAtmLoopbackDiagnostics (PDML_ATM_DIAG pDiag)
{
    if (ANSC_STATUS_SUCCESS != atm_hal_startAtmLoopbackDiagnostics(pDiag))
    {
        CcspTraceError(("%s %d - Failed to start ATM diagnostics \n",__FUNCTION__,__LINE__));
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

/* DmlAtmInit() */
ANSC_STATUS DmlAtmInit (ANSC_HANDLE hDml, PANSC_HANDLE phContext)
{
    PDATAMODEL_ATM  pMyObject = (PDATAMODEL_ATM)phContext;

    //ATM Diagnostics init
    DmlAtmDiagnosticsInit( pMyObject );

    return ANSC_STATUS_SUCCESS;
}

/*DmlAtmDiagnosticsInit */
ANSC_STATUS DmlAtmDiagnosticsInit (PANSC_HANDLE phContext)
{
    PDATAMODEL_ATM    pMyObject      = (PDATAMODEL_ATM)phContext;
    PDML_ATM_DIAG     pATMDiagTmp    = NULL;

    pATMDiagTmp = (PDML_ATM_DIAG) AnscAllocateMemory( sizeof(DML_ATM_DIAG) );

    //Return failure if allocation failiure
    if( NULL == pATMDiagTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    //Memset all memory
    memset( pATMDiagTmp, 0, sizeof(DML_ATM_DIAG) );

    //Initialize
    pATMDiagTmp->DiagnosticsState      = DIAG_STATE_NONE;
    pATMDiagTmp->NumberOfRepetitions   = 0;
    pATMDiagTmp->Timeout               = 0;
    pATMDiagTmp->SuccessCount          = 0;
    pATMDiagTmp->FailureCount          = 0;
    pATMDiagTmp->AverageResponseTime   = 0;
    pATMDiagTmp->MinimumResponseTime   = 0;
    pATMDiagTmp->MaximumResponseTime   = 0;

    //Assign the memory address to oringinal structure
    pMyObject->pATMDiag = pATMDiagTmp;

    return ANSC_STATUS_SUCCESS;
}

/* * DmlGetPtmIfStatus() */
ANSC_STATUS DmlGetAtmIfStatus (ANSC_HANDLE hContext, PDML_ATM pEntry)
{
    ANSC_STATUS returnStatus  = ANSC_STATUS_FAILURE;
    xtm_link_status_e linkStatus;
    hal_param_t req_param;
    memset(&req_param, 0, sizeof(req_param));
    if (pEntry != NULL)
    {
        if( pEntry->Enable)
        {
            /**
             * Construct Full DML path.
             * Device.ATM.Link.1.Status.
             */
            snprintf(req_param.name, sizeof(req_param.name), ATM_LINK_STATUS, pEntry->InstanceNumber);
            if (ANSC_STATUS_SUCCESS != xtm_hal_getLinkInfoParam(&req_param))
            {
                pEntry->Status = Error;
                CcspTraceError(("%s Failed to get link status\n",__FUNCTION__));
            }
            else 
            {
                pEntry->Status = XtmStatusStrToEnum(req_param.value);
                returnStatus = ANSC_STATUS_SUCCESS;
            }
        }
    }

    return returnStatus;
}

ANSC_STATUS DmlGetAtmIfStatistics (ANSC_HANDLE hContext, PDML_ATM pEntry)
{
    int returnStatus  = ANSC_STATUS_SUCCESS;
    char cLinkStats[256] = {'\0'};
    if (pEntry != NULL) {
        if( pEntry->Enable) {
            /**
             * Construct Full DML path.
             * Device.ATM.Link.1.Stats.
             */
            snprintf(cLinkStats, sizeof(cLinkStats), ATM_LINK_STATS, pEntry->InstanceNumber);
            memset(&pEntry->Statistics, 0, sizeof(pEntry->Statistics));
            returnStatus = atm_hal_getLinkStats(cLinkStats, &pEntry->Statistics);
            if (returnStatus != ANSC_STATUS_SUCCESS) {
                CcspTraceError(("%s %d Failed to get ATM stats\n",__FUNCTION__,__LINE__));
            }
        }
    }

    return returnStatus;
}

/* * DmlSetAtm() */
ANSC_STATUS DmlSetAtm (ANSC_HANDLE   hContext, PDML_ATM pEntry )
{
    ANSC_STATUS returnStatus = ANSC_STATUS_SUCCESS;

    returnStatus = DmlSetAtmIfEnable( pEntry );

    return returnStatus;
}

int AtmLinkStatusCallback(int atm_id, char *status)
{
    XDSLMSGQXtmStatusData MSGQXtmStatusData = {};

    MSGQXtmStatusData.link_id = atm_id;
    MSGQXtmStatusData.is_atm = TRUE;
    MSGQXtmStatusData.Status = XtmStatusStrToEnum(status);

    DmlXtmLinkSendStatusToEventQueue(&MSGQXtmStatusData);
}

void DmlPtmLinkSetEnable(PDML_PTM p_Ptm, BOOL bValue)
{
    p_Ptm->Enable = bValue;
}

void DmlPtmLinkSetStatus(PDML_PTM p_Ptm, xtm_link_status_e status)
{
    p_Ptm->Status = status;
}

void DmlAtmLinkSetEnable(PDML_ATM p_Atm, BOOL bValue)
{
    p_Atm->Enable = bValue;
}

void DmlAtmLinkSetStatus(PDML_ATM p_Atm, xtm_link_status_e status)
{
    p_Atm->Status = status;
}
