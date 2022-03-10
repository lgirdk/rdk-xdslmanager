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

/**************************************************************************

    module: xdsl_apis.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

    -------------------------------------------------------------------

    environment:

        platform independent

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        23/10/2019    initial revision.

**************************************************************************/

//!!!  This code assumes that all data structures are the SAME in middle-layer APIs and HAL layer APIs
//!!!  So it uses casting from one to the other

//Standard library
#include <stdlib.h>
#include <errno.h>
#include <sysevent/sysevent.h>

//Specific includes
#include "xdsl_apis.h"
#include "xdsl_hal.h"
#include "xdsl_internal.h"
#include "xdsl_report.h"
#include "xdsl_event_queue.h"


#define DATAMODEL_PARAM_LENGTH 256

//WAN Agent
#define WAN_DBUS_PATH                     "/com/cisco/spvtg/ccsp/wanmanager"
#define WAN_COMPONENT_NAME                "eRT.com.cisco.spvtg.ccsp.wanmanager"
#define WAN_NOE_PARAM_NAME                "Device.X_RDK_WanManager.CPEInterfaceNumberOfEntries"
#define WAN_LINK_STATUS_PARAM_NAME        "Device.X_RDK_WanManager.CPEInterface.%d.LinkStatus"
#define WAN_BASE_INTERFACE_PARAM_NAME     "Device.X_RDK_WanManager.CPEInterface.%d.BaseInterface"

//XDSL
#define XDSL_LINE_ENABLE "Device.DSL.Line.%d.Enable"
#define XDSL_LINE_DATA_GATHERING_ENABLE "Device.DSL.Line.%d.EnableDataGathering"

/* *********************************************************************** */
//Global Declaration

extern ANSC_HANDLE                   bus_handle;
extern char                          g_Subsystem[32];
extern PBACKEND_MANAGER_OBJECT  g_pBEManager;
extern char * XdslReportStatusEnable;
extern char * XdslReportStatusDfltReportingPeriod;
extern char * XdslReportStatusReportingPeriod;

static ANSC_STATUS DmlXdslGetLineStaticInfo( INT LineIndex, PDML_XDSL_LINE pstLineInfo );
static ANSC_STATUS DmlXdslGetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal );
static ANSC_STATUS DmlXdslSetParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit );
static ANSC_STATUS DmlXdslGetParamNames( char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize );
static ANSC_STATUS DmlXdslGetChannelStaticInfo( INT LineIndex, INT ChannelIndex, PDML_XDSL_CHANNEL pstChannelInfo );
static void DmlXdslStatusStrToEnum(char *status, DML_XDSL_IF_STATUS *ifStatus);
void DmlXdslLineLinkStatusCallback( int line_id, char *status );
static ANSC_STATUS DmlConfigurePTMLink( PDML_XDSL_LINE pLine , BOOL bEnable );
static ANSC_STATUS DmlConfigureATMLink( PDML_XDSL_LINE pLine , BOOL bEnable );
static ANSC_STATUS DmlUpdateXdslStandardUsed( PDML_XDSL_LINE pLine );

int sysevent_fd = -1;
token_t sysevent_token;

/* *********************************************************************** */

int DmlXdslSyseventInit()
{
    char sysevent_ip[] = "127.0.0.1";
    char sysevent_name[] = "dslmgr";
    sysevent_fd =  sysevent_open(sysevent_ip, SE_SERVER_WELL_KNOWN_PORT, SE_VERSION, sysevent_name, &sysevent_token);
    if (sysevent_fd < 0)
        return -1;
    return 0;
}

/* DmlXdslInit() */
ANSC_STATUS
DmlXdslInit
    (
        ANSC_HANDLE                 hDml,
        PANSC_HANDLE                phContext
    )
{
    PDATAMODEL_XDSL      pMyObject    = (PDATAMODEL_XDSL)phContext;

    //dsl HAL init
    if ( RETURN_OK != xdsl_hal_init())
    {
        CcspTraceError(("%s Failed to init dsl HAL\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //DSL Line init
    DmlXdslLineInit( pMyObject );

    //DSL Channel init
    DmlXdslChannelInit( pMyObject );

// Initialize sysevent daemon
    if (DmlXdslSyseventInit() < 0)
    {
        return ANSC_STATUS_FAILURE;
    }

    //DSL Diagnostics init
    DmlXdslDiagnosticsInit( pMyObject );

    //Trigger Event Handler thread
    DmlXdslTriggerEventHandlerThread();

    //Register linkstatus callback
    xdsl_hal_registerDslLinkStatusCallback(DmlXdslLineLinkStatusCallback);

    //DSL Diagnostics init
    DmlXdslReportInit( pMyObject );

    //DSL  XRDKNLM Init
    DmlXdslXRdkNlmInit( pMyObject );

    return ANSC_STATUS_SUCCESS;
}

/*DmlXdslLineInit */
ANSC_STATUS
DmlXdslLineInit
    (
        PANSC_HANDLE                phContext
    )
{
    PDATAMODEL_XDSL      pMyObject    = (PDATAMODEL_XDSL)phContext;
    PDML_XDSL_LINE       pXDSLLineTmp  = NULL;
    INT                      iTotalLines  = 0,
                             iLoopCount   = 0;

    iTotalLines = DmlXdslGetTotalNoofLines( );
    pXDSLLineTmp = (PDML_XDSL_LINE) AnscAllocateMemory( sizeof(DML_XDSL_LINE) * iTotalLines );

    //Return failure if allocation failiure
    if( NULL == pXDSLLineTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    pMyObject->ulTotalNoofDSLLines = iTotalLines;

    //Memset all memory
    memset( pXDSLLineTmp, 0, ( sizeof(DML_XDSL_LINE) * iTotalLines ) );

    //Fill line static information and initialize default values
    for( iLoopCount = 0; iLoopCount < iTotalLines; iLoopCount++ )
    {
	pthread_mutex_init(&pXDSLLineTmp[iLoopCount].mDataMutex, NULL);
        pXDSLLineTmp[iLoopCount].LinkStatus         = XDSL_LINK_STATUS_Disabled;
        pXDSLLineTmp[iLoopCount].Status             = XDSL_IF_STATUS_Down;
        pXDSLLineTmp[iLoopCount].ulInstanceNumber   = iLoopCount + 1;

        DmlXdslGetLineStaticInfo( iLoopCount, &pXDSLLineTmp[iLoopCount] );
    }

    //Assign the memory address to oringinal structure
    pMyObject->pXDSLLine    = pXDSLLineTmp;

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslGetTotalNoofLines() */
INT DmlXdslGetTotalNoofLines( VOID )
{
    //Get total no of lines from HAL
    return ( xdsl_hal_getTotalNoofLines() );
}

/* DmlXdslGetLineStaticInfo() */
static ANSC_STATUS DmlXdslGetLineStaticInfo( INT LineIndex, PDML_XDSL_LINE pstLineInfo )
{
    //Fill default or static information for the line index

    //Get Lowerlayers
    snprintf( pstLineInfo->LowerLayers, sizeof(pstLineInfo->LowerLayers), "Device.DSL.Line.%d", LineIndex + 1 );

    //Get Name
    snprintf( pstLineInfo->Name, sizeof(pstLineInfo->Name), "dsl%d", LineIndex );

    //Get Alias
    snprintf( pstLineInfo->Alias, sizeof(pstLineInfo->Alias), "dsl%d", LineIndex );

    //As of now hardcoded
    snprintf( pstLineInfo->StandardsSupported, sizeof(pstLineInfo->StandardsSupported), "%s", "G.992.1_Annex_A, G.992.1_Annex_B, G.992.1_Annex_C, T1.413, G.992.2, G.992.3_Annex_A, G.992.3_Annex_B, G.992.3_Annex_C, G.993.1, G.993.1_Annex_A, G.993.2_Annex_B, G.993.2_Annex_C" );

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslGetLineCfg() */
ANSC_STATUS DmlXdslGetLineCfg( INT LineIndex, PDML_XDSL_LINE pstLineInfo )
{
    char            tmpBuf[128]    = { 0 };
    hal_param_t     req_param;

    if( NULL == pstLineInfo )
    {
    CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    memset(&req_param, 0, sizeof(req_param));
    snprintf(req_param.name, sizeof(req_param.name), XDSL_LINE_ENABLE, LineIndex);
    if ( RETURN_OK != xdsl_hal_dslGetLineEnable( &req_param ) )
    {
        CcspTraceError(("%s Failed to get line enable value\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    pstLineInfo->Enable = atoi(req_param.value);

    //Get line full information
    if ( RETURN_OK != xdsl_hal_dslGetLineInfo( LineIndex, pstLineInfo ) )
    {
         CcspTraceError(("%s Failed to get line info value\n", __FUNCTION__));
         return ANSC_STATUS_FAILURE;
    }

    //Initialize statistics
    memset( &pstLineInfo->stLineStats, 0, sizeof(DML_XDSL_LINE_STATS ));

    //Collect Line Statistics
    if ( RETURN_OK != xdsl_hal_dslGetLineStats( LineIndex, &pstLineInfo->stLineStats ) )
    {
         CcspTraceError(("%s Failed to get line stats value\n", __FUNCTION__));
         return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslLineSetEnable() */
ANSC_STATUS DmlXdslLineSetEnable( INT LineIndex, BOOL Enable )
{
    hal_param_t req_param;

    //Validate index
    if ( LineIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    memset(&req_param, 0, sizeof(req_param));
    snprintf(req_param.name, sizeof(req_param), XDSL_LINE_ENABLE, LineIndex);
    snprintf(req_param.value, sizeof(req_param.value), "%d", Enable);
    req_param.type = PARAM_BOOLEAN;

    //Set enable/disable
    if ( RETURN_OK != xdsl_hal_dslSetLineEnable( &req_param ) )
    {
        CcspTraceError(("%s Failed to set line index[%d] Flag[%d]\n", __FUNCTION__,LineIndex,Enable));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s - %s:LineIndex:%d Enable:%d\n",__FUNCTION__,XDSL_MARKER_LINE_CFG_CHNG,LineIndex,Enable));

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslLineSetDataGatheringEnable() */
ANSC_STATUS DmlXdslLineSetDataGatheringEnable( INT LineIndex, BOOL Enable )
{
    //TBD

    CcspTraceInfo(("%s - %s:LineIndex:%d Enable:%d\n",__FUNCTION__,XDSL_MARKER_LINE_CFG_CHNG,LineIndex,Enable));

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslStatusStrToEnum() */
static void DmlXdslStatusStrToEnum(char *status, DML_XDSL_IF_STATUS *ifStatus)
{
    if(0 == strcmp( status, "Up" ))
    {
        *ifStatus = XDSL_IF_STATUS_Up;
    }
    else if(0 == strcmp( status, "Down" ))
    {
        *ifStatus = XDSL_IF_STATUS_Down;
    }
    else if(0 == strcmp( status, "Unknown" ))
    {
        *ifStatus = XDSL_IF_STATUS_Unknown;
    }
    else if(0 == strcmp( status, "Dormant" ))
    {
        *ifStatus = XDSL_IF_STATUS_Dormant;
    }
    else if(0 == strcmp( status, "NotPresent" ))
    {
        *ifStatus = XDSL_IF_STATUS_NotPresent;
    }
    else if(0 == strcmp( status, "LowerLayerDown" ))
    {
        *ifStatus = XDSL_IF_STATUS_LowerLayerDown;
    }
    else if(0 == strcmp( status, "Error" ))
    {
        *ifStatus = XDSL_IF_STATUS_Error;
    }
    else
    {
        CcspTraceError(("%s-%d Invalid Status= %s\n",__FUNCTION__,__LINE__,status));
    }
}

/* DmlXdslStatusStrToEnum() */
static void DmlXdslLinkStatusStrToEnum(char *status, DML_XDSL_LINK_STATUS *linkStatus)
{
    if(0 == strcmp( status, "Up" ))
    {
        *linkStatus = XDSL_LINK_STATUS_Up;
    }
    else if(0 == strcmp( status, "Initializing" ))
    {
        *linkStatus = XDSL_LINK_STATUS_Initializing;
    }
    else if(0 == strcmp( status, "EstablishingLink" ))
    {
        *linkStatus = XDSL_LINK_STATUS_EstablishingLink;
    }
    else if(0 == strcmp( status, "NoSignal" ))
    {
        *linkStatus = XDSL_LINK_STATUS_NoSignal;
    }
    else if(0 == strcmp( status, "Disabled" ))
    {
        *linkStatus = XDSL_LINK_STATUS_Disabled;
    }
    else if(0 == strcmp( status, "Error" ))
    {
        *linkStatus = XDSL_LINK_STATUS_Error;
    }
    else
    {
        CcspTraceError(("%s-%d Invalid Link Status= %s\n",__FUNCTION__,__LINE__,status));
    }
}

/* *DmlXdslLineLinkStatusCallback() */
void DmlXdslLineLinkStatusCallback( int line_id, char *status )
{
      XDSLMSGQLineStatusData MSGQLineStatusData = { 0 };

      MSGQLineStatusData.line_id = line_id;
#ifdef _SR300_PRODUCT_REQ_
      DmlXdslStatusStrToEnum(status, &MSGQLineStatusData.LinkStatus);
#else
      DmlXdslLinkStatusStrToEnum(status, &MSGQLineStatusData.LinkStatus);
#endif

 
      //Send message to Queue.
      DmlXdslLineSendLinkStatusToEventQueue( &MSGQLineStatusData );
}

/* *DmlXdslGetParamNames() */
static ANSC_STATUS DmlXdslGetParamNames( char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize )
{
    CCSP_MESSAGE_BUS_INFO  *bus_info         = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterInfoStruct_t  **retInfo;
    char                    *ParamName[ 1 ];
    int                    ret               = 0,
                           nval;

    ret = CcspBaseIf_getParameterNames(
                                    bus_handle,
                                    pComponent,
                                    pBus,
                                    pParamName,
                                    1,
                                    &nval,
                                    &retInfo);

    //Copy the value
    if( CCSP_SUCCESS == ret )
    {
        int iLoopCount;

        *pReturnSize = nval;

        for( iLoopCount = 0; iLoopCount < nval; iLoopCount++ )
        {
           if( NULL != retInfo[iLoopCount]->parameterName )
           {
              snprintf( a2cReturnVal[iLoopCount], strlen( retInfo[iLoopCount]->parameterName ) + 1, "%s", retInfo[iLoopCount]->parameterName );
           }
        }

        if( retInfo )
        {
          free_parameterInfoStruct_t(bus_handle, nval, retInfo);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if( retInfo )
    {
      free_parameterInfoStruct_t(bus_handle, nval, retInfo);
    }

    return ANSC_STATUS_FAILURE;
}

/* *DmlXdslGetParamValues() */
static ANSC_STATUS DmlXdslGetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal )
{
    CCSP_MESSAGE_BUS_INFO  *bus_info         = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   **retVal;
    char                    *ParamName[ 1 ];
    int                    ret               = 0,
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
    if( CCSP_SUCCESS == ret )
    {
        if( NULL != retVal[0]->parameterValue )
        {
            memcpy( pReturnVal, retVal[0]->parameterValue, strlen( retVal[0]->parameterValue ) + 1 );
        }

        if( retVal )
        {
            free_parameterValStruct_t (bus_handle, nval, retVal);
        }

        return ANSC_STATUS_SUCCESS;
    }

    if( retVal )
    {
       free_parameterValStruct_t (bus_handle, nval, retVal);
    }

    return ANSC_STATUS_FAILURE;
}

/* *DmlXdslSetParamValues() */
static ANSC_STATUS DmlXdslSetParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit )
{
    CCSP_MESSAGE_BUS_INFO *bus_info              = (CCSP_MESSAGE_BUS_INFO *)bus_handle;
    parameterValStruct_t   param_val[1]          = { 0 };
    char                  *faultParam            = NULL;
    int                    ret                   = 0;

    //Copy Name
    param_val[0].parameterName  = pParamName;
    //Copy Value
    param_val[0].parameterValue = pParamVal;
    //Copy Type
    param_val[0].type           = type;

    ret = CcspBaseIf_setParameterValues(
                                        bus_handle,
                                        pComponent,
                                        pBus,
                                        0,
                                        0,
                                        param_val,
                                        1,
                                        bCommit,
                                        &faultParam
                                       );

    if( ( ret != CCSP_SUCCESS ) && ( faultParam != NULL ) )
    {
        CcspTraceError(("%s-%d Failed to set %s\n",__FUNCTION__,__LINE__,pParamName));
        bus_info->freefunc( faultParam );
        return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslGetLowerLayersInstanceInOtherAgent() */
static ANSC_STATUS DmlXdslGetLowerLayersInstanceInOtherAgent( XDSL_NOTIFY_ENUM enNotifyAgent, char *pLowerLayers, INT *piInstanceNumber )
{
    //Validate buffer
    if( ( NULL == pLowerLayers ) || ( NULL == piInstanceNumber ) )
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Initialise default value
    *piInstanceNumber   = -1;

    switch( enNotifyAgent )
    {
        case NOTIFY_TO_WAN_AGENT:
        {
            char acTmpReturnValue[ 256 ]    = { 0 };
            INT  iLoopCount,
                 iTotalNoofEntries;

            if ( ANSC_STATUS_FAILURE == DmlXdslGetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, WAN_NOE_PARAM_NAME, acTmpReturnValue ) )
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                return ANSC_STATUS_FAILURE;
            }

            //Total count
            iTotalNoofEntries = atoi( acTmpReturnValue );
            CcspTraceInfo(("%s %d - TotalNoofEntries:%d\n", __FUNCTION__, __LINE__, iTotalNoofEntries));

            if( 0 >= iTotalNoofEntries )
            {
               return ANSC_STATUS_SUCCESS;
            }

            //Traverse from loop
            for ( iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++ )
            {
                char acTmpQueryParam[256] = { 0 };

                //Query
                snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), WAN_BASE_INTERFACE_PARAM_NAME, iLoopCount + 1 );

                memset( acTmpReturnValue, 0, sizeof( acTmpReturnValue ) );
                if ( ANSC_STATUS_FAILURE == DmlXdslGetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, acTmpQueryParam, acTmpReturnValue ) )
                {
                    CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                    continue;
                }

                //Compare name
                if( 0 == strcmp( acTmpReturnValue, pLowerLayers ) )
                {
                    *piInstanceNumber   = iLoopCount + 1;
                    break;
                }
            }
        }
        break; /* * NOTIFY_TO_WAN_AGENT */

        case NOTIFY_TO_PTM_AGENT:
        {
            char acTmpReturnValue[ 256 ]    = { 0 };
            PCONTEXT_LINK_OBJECT pPtmCxtLink = NULL;
            PSINGLE_LINK_ENTRY pSListEntry = NULL;
	    ULONG ulInstanceNumber = 0;
            ULONG  iLoopCount, iTotalNoofEntries;

            //Total count
	    iTotalNoofEntries = PTMLink_GetEntryCount(NULL);

            if( 0 >= iTotalNoofEntries )
            {
               return ANSC_STATUS_FAILURE;
            }
            //Traverse from loop
            for ( iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++ )
            {
                pSListEntry = PTMLink_GetEntry(NULL, iLoopCount, &ulInstanceNumber);
                if (pSListEntry == NULL)
                {
                     CcspTraceError(("%s PTMLink Entry get failed, InstanceNumber=%ld, iLoopCount=%ld \n", __FUNCTION__, ulInstanceNumber, iLoopCount));
                     continue;
                }
                pPtmCxtLink = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
		memset( acTmpReturnValue, 0, sizeof( acTmpReturnValue ) );
		if (PTMLink_GetParamStringValue(pPtmCxtLink, "LowerLayers", acTmpReturnValue, NULL))
                {
                    CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                    continue;
                }

                //Compare lowerlayers
                if( 0 == strcmp( acTmpReturnValue, pLowerLayers ) )
                {
                    *piInstanceNumber   = ulInstanceNumber;
                    break;
                }
            }
        }
        break; /* * NOTIFY_TO_PTM_AGENT */

        case NOTIFY_TO_ATM_AGENT:
        {
            char acTmpReturnValue[ 256 ]    = { 0 };
            PCONTEXT_LINK_OBJECT pAtmCxtLink = NULL;
            PSINGLE_LINK_ENTRY pSListEntry = NULL;
            ULONG ulInstanceNumber = 0;
            ULONG  iLoopCount, iTotalNoofEntries;

            //Total count
	    iTotalNoofEntries = ATMLink_GetEntryCount(NULL);

            if( 0 >= iTotalNoofEntries )
            {
               return ANSC_STATUS_FAILURE;
            }
            //Traverse from loop
            for ( iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++ )
            {
                pSListEntry = ATMLink_GetEntry(NULL, iLoopCount, &ulInstanceNumber);
                if (pSListEntry == NULL)
                {
                     CcspTraceError(("%s PTMLink Entry get failed, InstanceNumber=%ld, iLoopCount=%ld \n", __FUNCTION__, ulInstanceNumber, iLoopCount));
                     continue;
                }
                pAtmCxtLink = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
                memset( acTmpReturnValue, 0, sizeof( acTmpReturnValue ) );
                if (ATMLink_GetParamStringValue(pAtmCxtLink, "LowerLayers", acTmpReturnValue, NULL))
                {
                    CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                    continue;
                }

                //Compare lowerlayers
                if( 0 == strcmp( acTmpReturnValue, pLowerLayers ) )
                {
                    *piInstanceNumber   = ulInstanceNumber;
                    break;
                }
            }
        }
        break; /* * NOTIFY_TO_ATM_AGENT */

        default:
        {
            CcspTraceError(("%s Invalid Case\n", __FUNCTION__));
        }
        break; /* * default */
    }

    return ANSC_STATUS_SUCCESS;
}

static PDML_XDSL_LINE DmlXdlGetLineById(INT LineIndex)
{
    PDATAMODEL_XDSL pDSL = (PDATAMODEL_XDSL)g_pBEManager->hDSL;

    if (LineIndex > pDSL->ulTotalNoofDSLLines)
    {
	return NULL;
    }
    return pDSL->pXDSLLine + (LineIndex - 1);
}

static ANSC_STATUS DmlXdslLineTypeGet( PDML_XDSL_LINE pLine, DML_XDSL_LINE_TYPE *LineType )
{
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;

    pthread_mutex_lock(&pLine->mDataMutex);
    if (strstr(pLine->StandardUsed,"G.992.1") || strstr(pLine->StandardUsed,"T1.413")  ||
	strstr(pLine->StandardUsed,"G.992.2") || strstr(pLine->StandardUsed,"G.992.3") ||
	strstr(pLine->StandardUsed,"G.992.5"))
    {
	*LineType = DML_XDSL_LINE_ADSL;
    }
    else if(strstr(pLine->StandardUsed,"G.993.2"))
    {
	*LineType = DML_XDSL_LINE_VDSL;
    }
    else if(strstr(pLine->StandardUsed,"G.9701"))
    {
	*LineType = DML_XDSL_LINE_GFAST;
    }
    else
    {
	ret = ANSC_STATUS_FAILURE;
    }
    pthread_mutex_unlock(&pLine->mDataMutex);

    return ret;
}

ANSC_STATUS DmlXdslLineTypeGetById( INT LineIndex, DML_XDSL_LINE_TYPE *LineType )
{
    PDML_XDSL_LINE pLine = DmlXdlGetLineById(LineIndex);

    if (pLine == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }

    return DmlXdslLineTypeGet(pLine, LineType);
}

static ANSC_STATUS DmlXdslConfigureXTMLink( PDML_XDSL_LINE pLine, BOOL bEnable)
{
    DML_XDSL_LINE_TYPE LineType;
    ANSC_STATUS ret = ANSC_STATUS_SUCCESS;

    if (DmlXdslLineTypeGet(pLine, &LineType) == ANSC_STATUS_FAILURE)
    {
	return ANSC_STATUS_FAILURE;
    }

    switch (LineType)
    {
    case DML_XDSL_LINE_ADSL:
	ret = DmlConfigureATMLink(pLine, bEnable);
	break;
    case DML_XDSL_LINE_VDSL:
	ret = DmlConfigurePTMLink(pLine, bEnable);
	break;
    default:
	ret = ANSC_STATUS_FAILURE;
	break;
    }

    return ret;
}

/* * DmlXdslCreateXTMLink() */
ANSC_STATUS DmlXdslCreateXTMLink( INT LineIndex )
{
    PDML_XDSL_LINE pLine = DmlXdlGetLineById(LineIndex);

    if (pLine == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }

    if (DmlUpdateXdslStandardUsed(pLine) == ANSC_STATUS_FAILURE)
    {
	return ANSC_STATUS_FAILURE;
    }

    return DmlXdslConfigureXTMLink(pLine, TRUE);
}

static ANSC_STATUS DmlUpdateXdslStandardUsed( PDML_XDSL_LINE pLine )
{
    int rc = ANSC_STATUS_SUCCESS;
    hal_param_t     req_param;

    memset(&req_param, 0, sizeof(req_param));
    if ( RETURN_OK != xdsl_hal_dslGetLineStandardUsed( &req_param, pLine->ulInstanceNumber ) )
    {
        CcspTraceError(("%s Failed to get line StandardUsed value\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    pthread_mutex_lock(&pLine->mDataMutex);
    snprintf(pLine->StandardUsed, XDSL_STANDARD_USED_STR_MAX, "%s", req_param.value);
    pthread_mutex_unlock(&pLine->mDataMutex);
    return rc;
}

ANSC_STATUS DmlConfigurePTMLink( PDML_XDSL_LINE pLine , BOOL bEnable )
{
    PDML_PTM pPtm = NULL;
    char LowerLayers[128] = { 0 };
    ULONG iPTMInstance = -1;
    BOOL isEnabled = FALSE;

    pthread_mutex_lock(&pLine->mDataMutex);
    strncpy(LowerLayers, pLine->LowerLayers, sizeof(LowerLayers) - 1);
    pthread_mutex_unlock(&pLine->mDataMutex);

    //Get Instance for corresponding lower layer
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_PTM_AGENT, LowerLayers, &iPTMInstance );

    if( -1 == iPTMInstance )
    {
        CcspTraceError(("%s Failed to get PTM table Instance\n", __FUNCTION__ ));
        return ANSC_STATUS_FAILURE;
    }
    pPtm = PTMLink_GetEntry(NULL, iPTMInstance - 1, &iPTMInstance);
    if (PTMLink_GetParamBoolValue(pPtm, "Enable", &isEnabled) != TRUE)
    {
	return ANSC_STATUS_FAILURE;
    }

    if (isEnabled == bEnable)
    {
	return ANSC_STATUS_SUCCESS;
    }

    DmlPtmLinkSetEnable(pPtm, bEnable);
    if ((PTMLink_Commit(pPtm)) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s Failed to configure for PTM Link, Instance\n", __FUNCTION__));
	return ANSC_STATUS_FAILURE;
    }
    CcspTraceInfo(("%s %d Successfully configured PTM Link Table Entry Instance=%lu \n", __FUNCTION__,__LINE__, iPTMInstance));

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS DmlConfigureATMLink( PDML_XDSL_LINE pLine , BOOL bEnable )
{
    PDML_ATM pAtm = NULL;
    char LowerLayers[128] = { 0 };
    ULONG iATMInstance = -1;
    BOOL isEnabled = FALSE;


    pthread_mutex_lock(&pLine->mDataMutex);
    strncpy(LowerLayers, pLine->LowerLayers, sizeof(LowerLayers) - 1);
    pthread_mutex_unlock(&pLine->mDataMutex);

    //Get Instance for corresponding lower layer
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_ATM_AGENT, LowerLayers, &iATMInstance );

    if( -1 == iATMInstance )
    {
        CcspTraceError(("%s Failed to get ATM table Instance\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    pAtm = ATMLink_GetEntry(NULL, iATMInstance - 1, &iATMInstance);
    if (ATMLink_GetParamBoolValue(pAtm, "Enable", &isEnabled) != TRUE)
    {
	return ANSC_STATUS_FAILURE;
    }

    if (isEnabled == bEnable)
    {
	return ANSC_STATUS_SUCCESS;
    }

    DmlAtmLinkSetEnable(pAtm, bEnable);
    if ((ATMLink_Commit(pAtm)) != ANSC_STATUS_SUCCESS)
    {
        CcspTraceError(("%s Failed to configure ATM Link, Instance=%lu \n", __FUNCTION__, iATMInstance));
	return ANSC_STATUS_FAILURE;
    }
    CcspTraceInfo(("%s %d Successfully configured ATM Link Table Entry Instance=%lu \n", __FUNCTION__,__LINE__, iATMInstance));

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslDeleteXTMLink() */
ANSC_STATUS DmlXdslDeleteXTMLink( INT LineIndex )
{
    PDML_XDSL_LINE pLine = DmlXdlGetLineById(LineIndex);

    if (pLine == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }


    return DmlXdslConfigureXTMLink(pLine, FALSE);
}

/* * DmlXdslSetLinkStatusForWanManager() */
ANSC_STATUS DmlXdslSetLinkStatusForWanManager( char *BaseInterface, char *LinkStatus )
{
    char                       acSetParamName[256] = {'\0'};
    char                       acSetParamValue[256] = {'\0'};
    INT                        iWANInstance   = -1;

    //Validate buffer
    if( ( NULL == BaseInterface ) || ( NULL == LinkStatus ) )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get Instance for corresponding name
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_WAN_AGENT, BaseInterface, &iWANInstance );

    //Index is not present. so no need to do anything any WAN instance
    if( -1 == iWANInstance )
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    CcspTraceInfo(("%s %d WAN Instance:%d\n",__FUNCTION__, __LINE__,iWANInstance));

    //Set Link Status
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_LINK_STATUS_PARAM_NAME, iWANInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", LinkStatus );
    DmlXdslSetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, LinkStatus, BaseInterface));

    return ANSC_STATUS_SUCCESS;
}

/*DmlXdslChannelInit */
ANSC_STATUS
DmlXdslChannelInit
    (
        PANSC_HANDLE                phContext
    )
{
    PDATAMODEL_XDSL      pMyObject       = (PDATAMODEL_XDSL)phContext;
    PDML_XDSL_CHANNEL    pDSLChannelTmp  = NULL;
    INT                      iTotalChannels  = 0,
                             iLoopCountOuter = 0,
                             iLoopCountInner = 0,
                             iTotalChannelsPerLine = 0;

    //Get total channels for all lines
    for( iLoopCountOuter = 0; iLoopCountOuter < pMyObject->ulTotalNoofDSLLines; iLoopCountOuter++ )
    {
        iTotalChannels += DmlXdslGetTotalNoofChannels( iLoopCountOuter );
    }

    //Allocate total channels for all lines
    pDSLChannelTmp = (PDML_XDSL_CHANNEL) AnscAllocateMemory( sizeof(DML_XDSL_CHANNEL) * iTotalChannels );

    //Return failure if allocation failiure
    if( NULL == pDSLChannelTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    pMyObject->ulTotalNoofDSLChannels = iTotalChannels;

    //Memset all memory
    memset( pDSLChannelTmp, 0, ( sizeof(DML_XDSL_CHANNEL) * iTotalChannels ) );

    //Fill line static information and initialize default values
    for( iLoopCountOuter = 0; iLoopCountOuter < pMyObject->ulTotalNoofDSLLines; iLoopCountOuter++ )
    {
       iTotalChannelsPerLine = DmlXdslGetTotalNoofChannels( iLoopCountOuter );

       for( iLoopCountInner = 0; iLoopCountInner < iTotalChannelsPerLine; iLoopCountInner++ )
       {
           pDSLChannelTmp[iLoopCountInner].Status             = XDSL_IF_STATUS_Down;
           pDSLChannelTmp[iLoopCountInner].ulInstanceNumber   = iLoopCountInner + 1;
           pDSLChannelTmp[iLoopCountInner].LineIndex          = iLoopCountOuter;

           DmlXdslGetChannelStaticInfo( iLoopCountOuter, iLoopCountInner, &pDSLChannelTmp[iLoopCountInner] );
       }
    }

    //Assign the memory address to oringinal structure
    pMyObject->pDSLChannel = pDSLChannelTmp;

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslGetTotalNoofChannels() */
INT DmlXdslGetTotalNoofChannels( INT LineIndex )
{
    //Get total no of channels from HAL
    return ( xdsl_hal_getTotalNoOfChannels( LineIndex ) );
}

/* DmlXdslGetChannelStaticInfo() */
static ANSC_STATUS DmlXdslGetChannelStaticInfo( INT LineIndex, INT ChannelIndex, PDML_XDSL_CHANNEL pstChannelInfo )
{
    //Fill default or static information for the channel index

    //Get Lowerlayers
    snprintf( pstChannelInfo->LowerLayers, sizeof(pstChannelInfo->LowerLayers), "Device.DSL.Line.%d", LineIndex + 1 );

    //Get Name
    snprintf( pstChannelInfo->Name, sizeof(pstChannelInfo->Name), "dsl%d", LineIndex );

    //Get Alias
    snprintf( pstChannelInfo->Alias, sizeof(pstChannelInfo->Alias), "dsl%d", LineIndex );

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslGetChannelCfg() */
ANSC_STATUS DmlXdslGetChannelCfg( INT LineIndex, INT ChannelIndex, PDML_XDSL_CHANNEL pstChannelInfo )
{
    if( NULL == pstChannelInfo )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get channel full information
    if ( RETURN_OK != xdsl_hal_dslGetChannelInfo( LineIndex, ChannelIndex, pstChannelInfo ) )
    {
         CcspTraceError(("%s Failed to get channel info value\n", __FUNCTION__));
         return ANSC_STATUS_FAILURE;
    }

    //Initialize statistics
    memset( &pstChannelInfo->stChannelStats, 0, sizeof(DML_XDSL_CHANNEL_STATS ));

    //Get channel statistics
    if ( RETURN_OK != xdsl_hal_dslGetChannelStats( LineIndex, ChannelIndex, &pstChannelInfo->stChannelStats ) )
    {
         CcspTraceError(("%s Failed to get channel statistics value\n", __FUNCTION__));
         return ANSC_STATUS_FAILURE;
    }
    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslChannelSetEnable() */
ANSC_STATUS DmlXdslChannelSetEnable( INT LineIndex, INT ChannelIndex, BOOL Enable )
{
    //Validate index
    if ( ( ChannelIndex < 0 ) || ( LineIndex < 0 ) )
    {
        CcspTraceError(("%s Invalid index[%d,%d]\n", __FUNCTION__,ChannelIndex,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Set enable/disable
    //TBD

    CcspTraceInfo(("%s - %s:LineIndex:%d ChannelIndex:%d Enable:%d\n",__FUNCTION__,XDSL_MARKER_CHAN_CFG_CHNG,LineIndex,ChannelIndex,Enable));

    return ANSC_STATUS_SUCCESS;
}

/*DmlXdslDiagnosticsInit */
ANSC_STATUS
DmlXdslDiagnosticsInit
    (
        PANSC_HANDLE                phContext
    )
{
    PDATAMODEL_XDSL               pMyObject       = (PDATAMODEL_XDSL)phContext;
    PDML_XDSL_DIAGNOSTICS_FULL    pDSLDiagTmp     = NULL;

    pDSLDiagTmp = (PDML_XDSL_DIAGNOSTICS_FULL) AnscAllocateMemory( sizeof(DML_XDSL_DIAGNOSTICS_FULL) );

    //Return failure if allocation failiure
    if( NULL == pDSLDiagTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    //Memset all memory
    memset( pDSLDiagTmp, 0, sizeof(DML_XDSL_DIAGNOSTICS_FULL) );

    //Initialize
    pDSLDiagTmp->stDiagADSLLineTest.DiagnosticsState  = XDSL_DIAG_STATE_None;
    pDSLDiagTmp->stDiagSELTUER.DiagnosticsState       = XDSL_DIAG_STATE_None;
    pDSLDiagTmp->stDiagSELTQLN.DiagnosticsState       = XDSL_DIAG_STATE_None;
    pDSLDiagTmp->stDiagSELTP.DiagnosticsState         = XDSL_DIAG_STATE_None;

    //Assign the memory address to oringinal structure
    pMyObject->pDSLDiag = pDSLDiagTmp;

    return ANSC_STATUS_SUCCESS;
}


/* DmlXdslReportInit */
ANSC_STATUS
DmlXdslReportInit
(
        PANSC_HANDLE                phContext
)
{
    int retPsmGet                                    = 0;
    ULONG psmValue                                   = 0;
    PDATAMODEL_XDSL               pMyObject          = (PDATAMODEL_XDSL)phContext;
    PDML_X_RDK_REPORT_DSL         pXdslReportTmp     = NULL;
    PDML_X_RDK_REPORT_DSL_DEFAULT pXdslReportDfltTmp = NULL;

    pXdslReportTmp = (PDML_X_RDK_REPORT_DSL) AnscAllocateMemory( sizeof(DML_X_RDK_REPORT_DSL) );
    //Return failure if allocation failiure
    if( NULL == pXdslReportTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    pXdslReportDfltTmp = (PDML_X_RDK_REPORT_DSL_DEFAULT) AnscAllocateMemory (sizeof (DML_X_RDK_REPORT_DSL_DEFAULT));
    //Return failure if allocation failiure
    if( NULL == pXdslReportDfltTmp )
    {
        AnscFreeMemory(pXdslReportTmp);
        return ANSC_STATUS_FAILURE;
    }

    //Memset all memory
    memset( pXdslReportDfltTmp, 0, sizeof(DML_X_RDK_REPORT_DSL_DEFAULT) );
    GetNVRamULONGConfiguration(XdslReportStatusDfltReportingPeriod, &psmValue);
    XdslReportSetDefaultReportingPeriod(psmValue);
    pXdslReportDfltTmp->ReportingPeriod = psmValue;

    //Memset all memory
    memset( pXdslReportTmp, 0, sizeof(DML_X_RDK_REPORT_DSL) );
    GetNVRamULONGConfiguration(XdslReportStatusReportingPeriod, &psmValue);
    XdslReportSetReportingPeriod(psmValue);
    pXdslReportTmp->ReportingPeriod = psmValue;

    GetNVRamULONGConfiguration(XdslReportStatusEnable, &psmValue);
    XdslReportSetStatus(psmValue);
    pXdslReportTmp->Enabled = psmValue;

    //Assign the memory address to oringinal structure
    pXdslReportTmp->pDSLDefaultReport = pXdslReportDfltTmp;
    pMyObject->pDSLReport = pXdslReportTmp;

    return ANSC_STATUS_SUCCESS;
}

/*DmlXdslXRdkNlmInit */
ANSC_STATUS
DmlXdslXRdkNlmInit
    (
        PANSC_HANDLE                phContext
    )
{
    PDATAMODEL_XDSL               pMyObject       = (PDATAMODEL_XDSL)phContext;
    PDML_XDSL_X_RDK_NLNM          pDSLXRdkNlmTmp = NULL;

    pDSLXRdkNlmTmp = (PDML_XDSL_X_RDK_NLNM) AnscAllocateMemory( sizeof(DML_XDSL_X_RDK_NLNM) );

    //Return failure if allocation failiure
    if( NULL == pDSLXRdkNlmTmp )
    {
        return ANSC_STATUS_FAILURE;
    }

    //Memset all memory
    memset( pDSLXRdkNlmTmp, 0, sizeof(DML_XDSL_X_RDK_NLNM) );

    //Assign the memory address to oringinal structure
    pMyObject->pDSLXRdkNlm = pDSLXRdkNlmTmp;

    return ANSC_STATUS_SUCCESS;
}


/* DmlXdslGetXRDKNlm() */
ANSC_STATUS DmlXdslGetXRDKNlm( PDML_XDSL_X_RDK_NLNM  pstXRdkNlm )
{
    if( NULL == pstXRdkNlm )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    if ( ANSC_STATUS_SUCCESS != xdsl_hal_dslGetXRdk_Nlm( pstXRdkNlm ) )
    {
         CcspTraceError(("%s Failed to get value\n", __FUNCTION__));
         return ANSC_STATUS_FAILURE;
    }

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS DmlXdslSetLineLinkStatus( INT LineIndex, INT Status)
{
    PDML_XDSL_LINE pLine = DmlXdlGetLineById(LineIndex);

    if (pLine == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }

    pthread_mutex_lock(&pLine->mDataMutex);
#ifdef _SR300_PRODUCT_REQ_
    pLine->Status = Status;
#else
    pLine->LinkStatus = Status;
#endif
    pthread_mutex_unlock(&pLine->mDataMutex);

    return ANSC_STATUS_SUCCESS;
}

ANSC_STATUS DmlXdslGetLineLinkStatus( INT LineIndex, INT *Status)
{
    PDML_XDSL_LINE pLine = DmlXdlGetLineById(LineIndex);

    if (pLine == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }
    pthread_mutex_lock(&pLine->mDataMutex);
#ifdef _SR300_PRODUCT_REQ_
    *Status = pLine->Status;
#else
    *Status = pLine->LinkStatus;
#endif
    pthread_mutex_unlock(&pLine->mDataMutex);

    return ANSC_STATUS_SUCCESS;
}
