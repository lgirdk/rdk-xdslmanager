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

        13/08/2020    initial revision.

**************************************************************************/

//!!!  This code assumes that all data structures are the SAME in middle-layer APIs and HAL layer APIs
//!!!  So it uses casting from one to the other

//Standard library
#include <stdlib.h>
#include <errno.h>
#include <mqueue.h>
#include <sysevent/sysevent.h>

//Specific includes
#include "xdsl_apis.h"
#include "xdsl_hal.h"
#include "xdsl_internal.h"
#include "xdsl_report.h"


#define XDSL_EVENT_QUEUE_NAME  "/DSL_event_queue"
#define MAX_QUEUE_MSG_SIZE         ( 2048 )
#define MAX_QUEUE_LENGTH           ( 100 )
#define EVENT_MSG_MAX_SIZE         ( 1024 )

#define CHECK(x) \
    do { \
        if (!(x)) { \
            CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__)); \
            perror(#x); \
            return; \
        } \
    } while (0) \

#define DATAMODEL_PARAM_LENGTH 256

//XTM Manager
#define XTM_DBUS_PATH                     "/com/cisco/spvtg/ccsp/xdslmanager"
#define XTM_COMPONENT_NAME                "eRT.com.cisco.spvtg.ccsp.xdslmanager"
#define XTM_LINK_TABLE_NAME               "Device.PTM.Link."
#define XTM_NOE_PARAM_NAME                "Device.PTM.LinkNumberOfEntries"
#define XTM_LINK_LOWERLAYER_PARAM_NAME    "Device.PTM.Link.%d.LowerLayers"
#define XTM_LINK_ENABLE_PARAM_NAME        "Device.PTM.Link.%d.Enable"
#define XTM_LINK_ALIAS_PARAM_NAME         "Device.PTM.Link.%d.Alias"

//WAN Agent
#define WAN_DBUS_PATH                     "/com/cisco/spvtg/ccsp/wanmanager"
#define WAN_COMPONENT_NAME                "eRT.com.cisco.spvtg.ccsp.wanmanager"
#define WAN_NOE_PARAM_NAME                "Device.X_RDK_WanManager.CPEInterfaceNumberOfEntries"
#define WAN_PHY_STATUS_PARAM_NAME         "Device.X_RDK_WanManager.CPEInterface.%d.Phy.Status"
#define WAN_PHY_PATH_PARAM_NAME           "Device.X_RDK_WanManager.CPEInterface.%d.Phy.Path"
#define WAN_LINK_STATUS_PARAM_NAME        "Device.X_RDK_WanManager.CPEInterface.%d.Wan.LinkStatus"
#define WAN_IF_NAME_PARAM_NAME            "Device.X_RDK_WanManager.CPEInterface.%d.Name"

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

typedef enum
_XDSL_MSGQ_MSG_TYPE
{
    MSG_TYPE_WAN       = 1,

} XDSL_MSGQ_MSG_TYPE;

typedef struct _XDSLEventQData
{
    char                       Msg[EVENT_MSG_MAX_SIZE]; //Msg structure for the specific event
    XDSL_MSGQ_MSG_TYPE     MsgType; // WAN = 1
}DSLEventQData;

typedef struct _XDSLMSGQWanData
{
    CHAR                                 Name[64];
    CHAR                                 LowerLayers[128];
    DML_XDSL_LINK_STATUS             LinkStatus;
}XDSLMSGQWanData;

PDML_XDSL_LINE_GLOBALINFO     gpstLineGInfo     = NULL;
static pthread_mutex_t        gmXdslGInfo_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t        mUpdationMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t         cond = PTHREAD_COND_INITIALIZER;

static ANSC_STATUS DmlXdslGetLineStaticInfo( INT LineIndex, PDML_XDSL_LINE pstLineInfo );
static ANSC_STATUS DmlXdslLinePrepareGlobalInfo( ANSC_HANDLE   hContext );
static ANSC_STATUS DmlXdslGetParamValues( char *pComponent, char *pBus, char *pParamName, char *pReturnVal );
static ANSC_STATUS DmlXdslSetParamValues( char *pComponent, char *pBus, char *pParamName, char *pParamVal, enum dataType_e type, BOOLEAN bCommit );
static ANSC_STATUS DmlXdslGetParamNames( char *pComponent, char *pBus, char *pParamName, char a2cReturnVal[][256], int *pReturnSize );
static ANSC_STATUS DmlXdslLineSendLinkStatusToEventQueue( XDSLMSGQWanData *MSGQWanData );
static void DmlXdslTriggerEventHandlerThread( void );
static void *DmlXdslEventHandlerThread( void *arg );
static ANSC_STATUS DmlXdslGetChannelStaticInfo( INT LineIndex, INT ChannelIndex, PDML_XDSL_CHANNEL pstChannelInfo );
static void DmlXdslStatusStrToEnum(char *status, DML_XDSL_IF_STATUS *ifStatus);
void DmlXdslLineLinkStatusCallback( char *ifname, DslLinkStatus_t dsl_link_state );
static ANSC_STATUS DmlCreateXTMLink( char *ifname );
static ANSC_STATUS DmlDeleteXTMLink( char *ifname );
static void *XTM_DMLUpdationHandlerThread(void *arg);
static void *XTM_DMLDeletionHandlerThread(void *arg);

int sysevent_fd = -1;
token_t sysevent_token;

#ifdef _HUB4_PRODUCT_REQ_
#define SYSEVENT_WAN_LED_STATE "wan_led_state"
#define FLASHING_AMBER "Flashing Amber"
#define SOLID_GREEN "Solid Green"
#endif

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

    // Parodus init 
    initparodusTask();

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
        pXDSLLineTmp[iLoopCount].WanStatus          = XDSL_LINE_WAN_DOWN;
        pXDSLLineTmp[iLoopCount].LinkStatus         = XDSL_LINK_STATUS_Disabled;
        pXDSLLineTmp[iLoopCount].Status             = XDSL_IF_STATUS_Down;
        pXDSLLineTmp[iLoopCount].ulInstanceNumber   = iLoopCount + 1;

        DmlXdslGetLineStaticInfo( iLoopCount, &pXDSLLineTmp[iLoopCount] );
    }

    //Assign the memory address to oringinal structure
    pMyObject->pXDSLLine	= pXDSLLineTmp;

    //Prepare Global Information
    DmlXdslLinePrepareGlobalInfo( pMyObject );

    return ANSC_STATUS_SUCCESS;
}

/* *DmlXdslLinePrepareGlobalInfo() */
static ANSC_STATUS DmlXdslLinePrepareGlobalInfo( ANSC_HANDLE   hContext )
{
    PDATAMODEL_XDSL      pMyObject    = (PDATAMODEL_XDSL)hContext;
    INT                      iLoopCount   = 0,
                             TotalLines   = 0;

    TotalLines = pMyObject->ulTotalNoofDSLLines;

    //Allocate memory for DSL Global Status Information
    gpstLineGInfo = (PDML_XDSL_LINE_GLOBALINFO) AnscAllocateMemory( sizeof(DML_XDSL_LINE_GLOBALINFO) * TotalLines );

    //Return failure if allocation failiure
    if( NULL == gpstLineGInfo )
    {
        return ANSC_STATUS_FAILURE;
    }

    //Memset all memory
    memset( gpstLineGInfo, 0, ( sizeof(DML_XDSL_LINE_GLOBALINFO) * TotalLines ) );

    //Assign default value
    for( iLoopCount = 0; iLoopCount < TotalLines; ++iLoopCount )
    {
        PDML_XDSL_LINE  pXDSLLineTmp  = NULL;

        pXDSLLineTmp = pMyObject->pXDSLLine + iLoopCount;
    
        gpstLineGInfo[iLoopCount].Upstream       = FALSE; 
        gpstLineGInfo[iLoopCount].WanStatus      = XDSL_LINE_WAN_DOWN;
        gpstLineGInfo[iLoopCount].LinkStatus     = XDSL_LINK_STATUS_Disabled;
        snprintf( gpstLineGInfo[iLoopCount].LowerLayers, sizeof(gpstLineGInfo[iLoopCount].LowerLayers), "%s", pXDSLLineTmp->LowerLayers );
    }

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
    snprintf( pstLineInfo->StandardsSupported, sizeof(pstLineInfo->StandardsSupported), "%s", "G.992.1_Annex_A, G.992.1_Annex_B, G.992.1_Annex_C, T1.413, G.992, G.992.3_Annex_A, G.992.3_Annex_B, G.992.3_Annex_C, G.993.1, G.993.1_Annex_A, G.993.2_Annex_B, G.993_Annex_C" );

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

/* DmlXdslLineSetUpstream() */
ANSC_STATUS DmlXdslLineSetUpstream( INT LineIndex, BOOL Upstream )
{
    //Validate index 
    if ( LineIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Set Upstream flag
    pthread_mutex_lock(&gmXdslGInfo_mutex);
    gpstLineGInfo[LineIndex].Upstream = Upstream;
    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    //Needs to start thread based on instance - TBD. As of now I started
    if( TRUE == Upstream )
    {
        XDSL_SM_PRIVATE_INFO stSMPrivateInfo = { 0 };

        /* Create and Start DSL state machine */
        pthread_mutex_lock(&gmXdslGInfo_mutex);
        snprintf( stSMPrivateInfo.Name, sizeof( stSMPrivateInfo.Name ), "%s", gpstLineGInfo[LineIndex].Name );
        snprintf( stSMPrivateInfo.LowerLayers, sizeof( stSMPrivateInfo.LowerLayers ),"%s", gpstLineGInfo[LineIndex].LowerLayers );
        pthread_mutex_unlock(&gmXdslGInfo_mutex);

        XdslManager_Start_StateMachine( &stSMPrivateInfo );
    }

    CcspTraceInfo(("%s - %s:LineIndex:%d Upstream:%d\n",__FUNCTION__,XDSL_MARKER_LINE_CFG_CHNG,LineIndex,Upstream));

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslLineGetWanStatus() */
ANSC_STATUS DmlXdslLineGetWanStatus( INT LineIndex, DML_XDSL_LINE_WAN_STATUS *wan_state )
{
    //Validate index
    if ( LineIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Get Wan Status
    pthread_mutex_lock(&gmXdslGInfo_mutex);
    *wan_state = gpstLineGInfo[LineIndex].WanStatus;
    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    return ( ANSC_STATUS_SUCCESS );
}

/* DmlXdslLineSetWanStatus() */
ANSC_STATUS DmlXdslLineSetWanStatus( INT LineIndex, DML_XDSL_LINE_WAN_STATUS wan_state )
{
    //Validate index
    if ( LineIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Set Wan Status
    pthread_mutex_lock(&gmXdslGInfo_mutex);
    gpstLineGInfo[LineIndex].WanStatus = wan_state;
    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    CcspTraceInfo(("%s - %s:WAN Status Changed Index:%d [%d,%s]\n",__FUNCTION__,XDSL_MARKER_LINE_CFG_CHNG,LineIndex,wan_state,( wan_state == XDSL_LINE_WAN_UP ) ?  "Up" : "Down" ));

    return ANSC_STATUS_SUCCESS;
}

/* DmlXdslLineGetLinkStatus() */
ANSC_STATUS DmlXdslLineGetLinkStatus( INT LineIndex, DML_XDSL_LINK_STATUS *LinkStatus )
{
    //Validate index
    if ( LineIndex < 0 )
    {
        CcspTraceError(("%s Invalid index[%d]\n", __FUNCTION__,LineIndex));
        return ANSC_STATUS_FAILURE;
    }

    //Get the training status
    pthread_mutex_lock(&gmXdslGInfo_mutex);
    *LinkStatus = gpstLineGInfo[LineIndex].LinkStatus;
    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    return ( ANSC_STATUS_SUCCESS );
}

/* *DmlXdslLineGetIndexFromIfName() */
ANSC_STATUS DmlXdslLineGetIndexFromIfName( char *ifname, INT *LineIndex )
{
    INT iTotalLines = DmlXdslGetTotalNoofLines();
    INT iLoopCount;

    //Check buffer
    if( ( NULL == gpstLineGInfo ) || ( NULL == LineIndex ) || ( NULL == ifname ) )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    *LineIndex = -1;

    pthread_mutex_lock(&gmXdslGInfo_mutex);

    //Parse and return DSL index
    for( iLoopCount = 0 ; iLoopCount < iTotalLines; iLoopCount++ )
    {
        if( ( NULL != &gpstLineGInfo[iLoopCount] ) && ( 0 == strcmp( gpstLineGInfo[iLoopCount].Name, ifname ) ) )
        {
            *LineIndex = iLoopCount;
            pthread_mutex_unlock(&gmXdslGInfo_mutex);
            return ANSC_STATUS_SUCCESS; 
        }
    }

    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    return ANSC_STATUS_FAILURE;
}

/* DmlXdslLineGetCopyOfGlobalInfoForGivenIfName() */
ANSC_STATUS DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( char *ifname, PDML_XDSL_LINE_GLOBALINFO pGlobalInfo )
{
    ANSC_STATUS   retStatus;
    INT           LineIndex = -1;

    //Validate index
    if ( ( NULL == pGlobalInfo ) || ( NULL == ifname ) )
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    retStatus = DmlXdslLineGetIndexFromIfName( ifname, &LineIndex );

    if( ( ANSC_STATUS_FAILURE == retStatus ) || ( -1 == LineIndex ) )
    {
        CcspTraceError(("%s Failed to get index for %s\n", __FUNCTION__,ifname));
        return ANSC_STATUS_FAILURE;
    }
    
    //Copy of the data
    pthread_mutex_lock(&gmXdslGInfo_mutex);
    memcpy( pGlobalInfo, &gpstLineGInfo[LineIndex], sizeof(DML_XDSL_LINE_GLOBALINFO));
    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    return ( ANSC_STATUS_SUCCESS );
}

/* * DmlXdslLineUpdateLinkStatusAndGetGlobalInfoForGivenIfName() */
ANSC_STATUS DmlXdslLineUpdateLinkStatusAndGetGlobalInfoForGivenIfName( char *ifname, DML_XDSL_LINK_STATUS enLinkStatus, PDML_XDSL_LINE_GLOBALINFO pGlobalInfo )
{
    ANSC_STATUS    retStatus   = ANSC_STATUS_FAILURE;
    INT            iTotalLines = DmlXdslGetTotalNoofLines();
    INT            iLoopCount;

    //Check buffer
    if( ( NULL == gpstLineGInfo ) || ( NULL == pGlobalInfo ) || ( NULL == ifname ) )
    {
        CcspTraceError(("%s Invalid Buffer\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    pthread_mutex_lock(&gmXdslGInfo_mutex);

    /* 
     * Verify whether Name is NULL or Empty.
     * If yes then needs to store passed ifname into that particular entry and copy the global information
     * If no then needs to parse and match till total entries and copy particular global information
     */
    for( iLoopCount = 0 ; iLoopCount < iTotalLines; iLoopCount++ )
    {
        //Match the ifname and copy the corresponding global data
        if( ( NULL != &gpstLineGInfo[iLoopCount] ) && \
            ( '\0' != gpstLineGInfo[iLoopCount].Name[0] ) && \
            ( 0 == strcmp( gpstLineGInfo[iLoopCount].Name, ifname ) )
          )
        {   
            //Update link status
            gpstLineGInfo[iLoopCount].LinkStatus = enLinkStatus; 
            memcpy( pGlobalInfo, &gpstLineGInfo[iLoopCount], sizeof(DML_XDSL_LINE_GLOBALINFO));
            retStatus = ANSC_STATUS_SUCCESS;
            break;
        }
        else
        {
            if( ( NULL != &gpstLineGInfo[iLoopCount] ) && ( ( '\0' == gpstLineGInfo[iLoopCount].Name[0] ) || ( 0 == strlen( gpstLineGInfo[iLoopCount].Name ) ) ) )
            {
                //Update interface name and copy the global information
                snprintf( gpstLineGInfo[iLoopCount].Name, sizeof( gpstLineGInfo[iLoopCount].Name ), "%s", ifname );
                
                //Update link status
                gpstLineGInfo[iLoopCount].LinkStatus = enLinkStatus;

                memcpy( pGlobalInfo, &gpstLineGInfo[iLoopCount], sizeof(DML_XDSL_LINE_GLOBALINFO));
                retStatus = ANSC_STATUS_SUCCESS;
                CcspTraceInfo(("%s %d Updated received ifname %s into index %d\n", __FUNCTION__, __LINE__, ifname, iLoopCount));
                break;
            }
        }
    }

    pthread_mutex_unlock(&gmXdslGInfo_mutex);

    return retStatus;
}

/* *DmlXdslLineSendLinkStatusToEventQueue() */
static ANSC_STATUS DmlXdslLineSendLinkStatusToEventQueue( XDSLMSGQWanData *MSGQWanData )
{
    DSLEventQData    EventMsg = { 0 };
    mqd_t                mq;
    char                 buffer[ MAX_QUEUE_MSG_SIZE ];

    //Validate buffer
    if( NULL == MSGQWanData )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__, __LINE__));\
        return ANSC_STATUS_FAILURE;
    }

    //message queue send
    mq = mq_open(XDSL_EVENT_QUEUE_NAME, O_WRONLY);
    CHECK((mqd_t)-1 != mq);
    memset(buffer, 0, MAX_QUEUE_MSG_SIZE);
    EventMsg.MsgType = MSG_TYPE_WAN;

    memcpy(EventMsg.Msg,MSGQWanData,sizeof(XDSLMSGQWanData));
    memcpy(buffer,&EventMsg,sizeof(EventMsg));
    CHECK(0 <= mq_send(mq, buffer, MAX_QUEUE_MSG_SIZE, 0));
    CHECK((mqd_t)-1 != mq_close(mq));

    CcspTraceInfo(("%s %d - Successfully sent WAN message to event queue\n", __FUNCTION__, __LINE__));

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

/* *DmlXdslLineLinkStatusCallback() */
void DmlXdslLineLinkStatusCallback( char *ifname, DslLinkStatus_t dsl_link_state )
{
    DML_XDSL_LINK_STATUS tmpLinkStatus;
    INT                      LineIndex     = -1;
    BOOL                     IsValidStatus = TRUE;

    CcspTraceInfo(("%s - %s:IfName:%s Linkstate:%d\n",__FUNCTION__, XDSL_MARKER_LINE_LINK_CB, ifname, dsl_link_state ));

    switch( dsl_link_state )
    {
        case LINK_UP:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_Up;
        }
        break; /* * LINK_UP */

        case LINK_INITIALIZING:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_Initializing;
        }
        break; /* * LINK_INITIALIZING */

        case LINK_ESTABLISHING:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_EstablishingLink;
        }
        break; /* * LINK_ESTABLISHING */

        case LINK_NOSIGNAL:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_NoSignal;
        }
        break; /* * LINK_NOSIGNAL */

        case LINK_DISABLED:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_Disabled;
        }
        break; /* * LINK_DISABLED */

        case LINK_ERROR:
        {
            tmpLinkStatus = XDSL_LINK_STATUS_Error;
        }
        break; /* * LINK_ERROR */

        default:
        {
            IsValidStatus = FALSE;
        }
        break; /* * default */

    }

    //Update link status in global info if valid status and ignore if invalid
    if( TRUE == IsValidStatus )
    {
       ANSC_STATUS    retStatus;
       DML_XDSL_LINE_GLOBALINFO  stGlobalInfo = { 0 };

       //Check and get copy of the data
       retStatus = DmlXdslLineUpdateLinkStatusAndGetGlobalInfoForGivenIfName( ifname, tmpLinkStatus, &stGlobalInfo );

       //Validate return status
       if ( ANSC_STATUS_SUCCESS == retStatus )
       {
           XDSLMSGQWanData MSGQWanData = { 0 };

           //Prepare MSGQData buffer
           snprintf( MSGQWanData.Name, sizeof( MSGQWanData.Name ), "%s", stGlobalInfo.Name );
           snprintf( MSGQWanData.LowerLayers, sizeof( MSGQWanData.LowerLayers ), "%s", stGlobalInfo.LowerLayers );
           MSGQWanData.LinkStatus = stGlobalInfo.LinkStatus;

           //Send message to Queue.
           DmlXdslLineSendLinkStatusToEventQueue( &MSGQWanData );
       }  
    }
}

/* DmlXdslTriggerEventHandlerThread() */
void DmlXdslTriggerEventHandlerThread( void )
{
    pthread_t EvtThreadId;
    int iErrorCode = 0;

    //DSL event handler thread
    iErrorCode = pthread_create( &EvtThreadId, NULL, &DmlXdslEventHandlerThread, NULL );

    if( 0 != iErrorCode )
    {
        CcspTraceInfo(("%s %d - Failed to start Event Handler Thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode ));
    }
    else
    {
        CcspTraceInfo(("%s %d - Event Handler Thread Started Successfully\n", __FUNCTION__, __LINE__ ));
    }
}

/* *DmlXdslEventHandlerThread() */
static void *DmlXdslEventHandlerThread( void *arg )
{
  mqd_t     mq;
  struct    mq_attr attr;
  char      buffer[MAX_QUEUE_MSG_SIZE + 1];

  /* initialize the queue attributes */
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = MAX_QUEUE_LENGTH;
  attr.mq_msgsize = MAX_QUEUE_MSG_SIZE;
  attr.mq_curmsgs = 0;

  /* create the message queue */
  mq = mq_open(XDSL_EVENT_QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

  CHECK((mqd_t)-1 != mq);

  do
  {
    ssize_t              bytes_read;
    DSLEventQData    EventMsg = { 0 };

    /* receive the message */
    bytes_read = mq_receive(mq, buffer, MAX_QUEUE_MSG_SIZE, NULL);

    CHECK(bytes_read >= 0);

    buffer[bytes_read] = '\0';

    memcpy(&EventMsg, buffer, sizeof(EventMsg));

    //WAN Event
    if( MSG_TYPE_WAN == EventMsg.MsgType )
    {
       XDSLMSGQWanData MSGQWanData        = { 0 };
       char               acTmpPhyStatus[32] = { 0 };
       BOOL               IsValidStatus      = TRUE;
#ifdef _HUB4_PRODUCT_REQ_
       char               ledStatus[32]      = { 0 };
#endif
  
       memcpy(&MSGQWanData, EventMsg.Msg, sizeof(XDSLMSGQWanData));

       CcspTraceInfo(("%s - MSGQ Name:%s LowerLayers:%s LinkStatus:%d\n", __FUNCTION__, MSGQWanData.Name, MSGQWanData.LowerLayers, MSGQWanData.LinkStatus));

       switch( MSGQWanData.LinkStatus )
       {   
           case XDSL_LINK_STATUS_Up:
           {
               snprintf( acTmpPhyStatus, sizeof( acTmpPhyStatus ), "%s", "Up" );
           }
           break; /* * XDSL_LINK_STATUS_Up */
        
           case XDSL_LINK_STATUS_Initializing:
           case XDSL_LINK_STATUS_EstablishingLink:
           {
#ifdef _HUB4_PRODUCT_REQ_
               sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_WAN_LED_STATE, ledStatus, sizeof(ledStatus));
               if(strncmp(ledStatus, SOLID_GREEN, sizeof(ledStatus)) != 0)
               {
                   sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_WAN_LED_STATE, FLASHING_AMBER, 0);
                   snprintf( acTmpPhyStatus, sizeof( acTmpPhyStatus ), "%s", "Initializing" );
               }
#endif
           }
           break; /* * XDSL_LINK_STATUS_Initializing XDSL_LINK_STATUS_EstablishingLink */
        
           case XDSL_LINK_STATUS_NoSignal:
           case XDSL_LINK_STATUS_Disabled:
           case XDSL_LINK_STATUS_Error: 
           {
               snprintf( acTmpPhyStatus, sizeof( acTmpPhyStatus ), "%s", "Down" );
           }
           break; /* * XDSL_LINK_STATUS_NoSignal XDSL_LINK_STATUS_Disabled XDSL_LINK_STATUS_Error */
        
           default:
           {
               IsValidStatus = FALSE;
           }
           break; /* * default */
       }

       //Check whether valid status or not
       if( TRUE == IsValidStatus )
       {
          DmlXdslSetPhyStatusForWanManager( MSGQWanData.Name, acTmpPhyStatus );
       }
    }

  }while(1);

  //exit from thread
  pthread_exit(NULL);

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
                snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), WAN_IF_NAME_PARAM_NAME, iLoopCount + 1 );

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

        case NOTIFY_TO_XTM_AGENT:
        {
            char acTmpReturnValue[ 256 ]    = { 0 },
                 a2cTmpTableParams[16][256] = { 0 };
            INT  iLoopCount,
                 iTotalNoofEntries;

            if ( ANSC_STATUS_FAILURE == DmlXdslGetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, XTM_NOE_PARAM_NAME, acTmpReturnValue ) )
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

            //Get table names
            iTotalNoofEntries = 0;
            if ( ANSC_STATUS_FAILURE == DmlXdslGetParamNames( XTM_COMPONENT_NAME, XTM_DBUS_PATH, XTM_LINK_TABLE_NAME, a2cTmpTableParams , &iTotalNoofEntries ))
            {
                CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                return ANSC_STATUS_FAILURE;
            }

            //Traverse from loop
            for ( iLoopCount = 0; iLoopCount < iTotalNoofEntries; iLoopCount++ )
            {
                char acTmpQueryParam[256] = { 0 };

                //Query 
                snprintf( acTmpQueryParam, sizeof(acTmpQueryParam ), "%sLowerLayers", a2cTmpTableParams[ iLoopCount ] );

                memset( acTmpReturnValue, 0, sizeof( acTmpReturnValue ) );
                if ( ANSC_STATUS_FAILURE == DmlXdslGetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, acTmpQueryParam, acTmpReturnValue ) )
                {
                    CcspTraceError(("%s %d Failed to get param value\n", __FUNCTION__, __LINE__));
                    continue;
                }

                //Compare lowerlayers
                if( 0 == strcmp( acTmpReturnValue, pLowerLayers ) )
                {
                    char  tmpTableParam[ 256 ] = { 0 };
                    const char *last_two;

                    //Copy table param
                    snprintf( tmpTableParam, sizeof(tmpTableParam), "%s", a2cTmpTableParams[ iLoopCount ] );

                    //Get last two chareters from return value and cut the instance
                    last_two = &tmpTableParam[strlen(tmpTableParam) - 2];
                    
                    *piInstanceNumber   = atoi(last_two);
                    break;
                }
            }
        }
        break; /* * NOTIFY_TO_XTM_AGENT */

        default:
        {
            CcspTraceError(("%s Invalid Case\n", __FUNCTION__));
        }
        break; /* * default */
    }

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslCreateXTMLink() */
ANSC_STATUS DmlXdslCreateXTMLink( char *ifname )
{
    pthread_t  XtmObjCreationThread;
    int        iErrorCode   = 0;
    struct timespec _ts;
    struct timespec _now;
    int n;

    memset(&_ts, 0, sizeof(struct timespec));
    memset(&_now, 0, sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, &_now);
    _ts.tv_sec = _now.tv_sec + 15;

    iErrorCode = pthread_create(&XtmObjCreationThread, NULL, XTM_DMLUpdationHandlerThread, (void *)ifname);
    if (iErrorCode != 0)
    {
       CcspTraceError(("%s %d - Failed to start the handler thread\n",__FUNCTION__, __LINE__));
       return ANSC_STATUS_FAILURE;
    }
    n = pthread_cond_timedwait(&cond, &mUpdationMutex, &_ts);
    if(n == ETIMEDOUT)
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait TIMED OUT!!!\n",__FUNCTION__));
    }
    else if (n == 0)
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait SIGNALLED OK!!!\n",__FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait ERROR!!!\n",__FUNCTION__));
    }

    CcspTraceInfo(("%s %d - Successfully started XTM creation thread\n",__FUNCTION__, __LINE__));
    return ANSC_STATUS_SUCCESS;
}

/* * XTM_DMLUpdationHandlerThread() */
static void *XTM_DMLUpdationHandlerThread( void *arg )
{
    char* ifname = (char*)arg;

    if ( NULL == ifname)
    {
        CcspTraceError(("%s Invalid Argument\n",__FUNCTION__));
        pthread_exit(NULL);
    }

    //detach thread from caller stack
    pthread_detach(pthread_self());

    pthread_mutex_lock(&mUpdationMutex);

    if (ANSC_STATUS_SUCCESS == DmlCreateXTMLink(ifname))
    {
        CcspTraceInfo(("%s Successfully created XTMLink\n",__FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("%s - Failed to create XTMLink\n",__FUNCTION__));
    }

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mUpdationMutex);

    //Exit thread.
    pthread_exit(NULL);
}

/* * DmlCreateXTMLink() */
ANSC_STATUS DmlCreateXTMLink( char *ifname )
{
    DML_XDSL_LINE_GLOBALINFO   stGlobalInfo   = { 0 };
    char                       *acSetParamName = NULL;
    char                       *acSetParamValue = NULL;
    INT                        iXTMInstance   = -1;

    //Validate buffer
    if( NULL == ifname )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( ifname, &stGlobalInfo );

    //Get Instance for corresponding lower layer
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_XTM_AGENT, stGlobalInfo.LowerLayers, &iXTMInstance );

    //Create XTM Link
    //Index is not present. so needs to create a PTM instance
    if( -1 == iXTMInstance )
    {
        char  acTableName[ 128 ] = { 0 }; 
        INT   iNewTableInstance  = -1;

        sprintf( acTableName, "%s", XTM_LINK_TABLE_NAME );
        if ( CCSP_SUCCESS != CcspBaseIf_AddTblRow (
                                                    bus_handle,
                                                    XTM_COMPONENT_NAME,
                                                    XTM_DBUS_PATH,
                                                    0,          /* session id */
                                                    acTableName,
                                                    &iNewTableInstance
                                                  ) )
       {  
            CcspTraceError(("%s Failed to add table %s\n", __FUNCTION__,acTableName));
            return ANSC_STATUS_FAILURE;
       }

       //Assign new instance
       iXTMInstance = iNewTableInstance;
    }

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        CcspTraceError(("%s Memory allocation failed \n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    memset( acSetParamName, 0, DATAMODEL_PARAM_LENGTH );
    memset( acSetParamValue, 0, DATAMODEL_PARAM_LENGTH );

    CcspTraceInfo(("%s %d XTM Instance:%d\n",__FUNCTION__, __LINE__,iXTMInstance));

    //Set Lower Layer
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, XTM_LINK_LOWERLAYER_PARAM_NAME, iXTMInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", stGlobalInfo.LowerLayers );
    DmlXdslSetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, FALSE );

    //Set Alias
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, XTM_LINK_ALIAS_PARAM_NAME, iXTMInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", stGlobalInfo.Name );
    DmlXdslSetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, FALSE );

    //Set XTM Enable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, XTM_LINK_ENABLE_PARAM_NAME, iXTMInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "true" );
    DmlXdslSetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    if(acSetParamName != NULL)
    {
        free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
        free(acSetParamValue);
    }

    CcspTraceInfo(("%s %d Successfully notified Up event to XTM Agent for %s interface\n", __FUNCTION__,__LINE__,ifname));

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslDeleteXTMLink() */
ANSC_STATUS DmlXdslDeleteXTMLink( char *ifname )
{
    pthread_t                 XtmObjDeletionThread;
    int                       iErrorCode   = 0;
    struct timespec _ts;
    struct timespec _now;
    int n;

    memset(&_ts, 0, sizeof(struct timespec));
    memset(&_now, 0, sizeof(struct timespec));
    clock_gettime(CLOCK_REALTIME, &_now);
    _ts.tv_sec = _now.tv_sec + 15;

    iErrorCode = pthread_create( &XtmObjDeletionThread, NULL, XTM_DMLDeletionHandlerThread, (void *)ifname );
    if ( iErrorCode != 0 )
    {
       CcspTraceError(("%s %d - Failed to start deletion handler thread\n",__FUNCTION__, __LINE__));
       return ANSC_STATUS_FAILURE;
    }

    n = pthread_cond_timedwait(&cond, &mUpdationMutex, &_ts);
    if(n == ETIMEDOUT)
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait TIMED OUT!!!\n",__FUNCTION__));
    }
    else if (n == 0)
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait SIGNALLED OK!!!\n",__FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("%s : pthread_cond_timedwait ERROR!!!\n",__FUNCTION__));
    }

    return ANSC_STATUS_SUCCESS;
}

static void *XTM_DMLDeletionHandlerThread( void *arg )
{
    char* ifname = (char*)arg;

    if ( NULL == ifname )
    {
        CcspTraceError(("%s Invalid Argument\n",__FUNCTION__));
        pthread_exit(NULL);
    }

    //detach thread from caller stack
    pthread_detach(pthread_self());

    pthread_mutex_lock(&mUpdationMutex);

    if (ANSC_STATUS_SUCCESS != DmlDeleteXTMLink(ifname))
    {
        CcspTraceError(("%s:Failed to delete XTM table\n ",__FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("%s:Successfully deleted XTM table\n",__FUNCTION__));
    }

    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mUpdationMutex);

    //Exit thread.
    pthread_exit(NULL);
}

/* * DmlDeleteXTMLink() */
ANSC_STATUS DmlDeleteXTMLink( char *ifname )
{
    DML_XDSL_LINE_GLOBALINFO   stGlobalInfo   = { 0 };
    char                       acTableName[ 128 ] = { 0 };
    char                       *acSetParamName = NULL;
    char                       *acSetParamValue = NULL;
    INT                        iXTMInstance   = -1;
    
    //Validate buffer
    if( NULL == ifname )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( ifname, &stGlobalInfo );

    //Get Instance for corresponding lower layer
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_XTM_AGENT, stGlobalInfo.LowerLayers, &iXTMInstance );

    //Index is not present. so no need to do anything any PTM instance
    if( -1 == iXTMInstance )
    {
        CcspTraceError(("%s %d PTM instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    memset(acSetParamName, 0, DATAMODEL_PARAM_LENGTH);
    memset(acSetParamValue, 0, DATAMODEL_PARAM_LENGTH);

    CcspTraceInfo(("%s %d XTM Instance:%d\n",__FUNCTION__, __LINE__,iXTMInstance));

    //Set XTM Enable
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, XTM_LINK_ENABLE_PARAM_NAME, iXTMInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", "false" );
    DmlXdslSetParamValues( XTM_COMPONENT_NAME, XTM_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_boolean, TRUE );

    //Delay - to set param.
    sleep(2);

    //Delete XTM instance
    sprintf( acTableName, "%s%d.", XTM_LINK_TABLE_NAME, iXTMInstance );
    if ( CCSP_SUCCESS != CcspBaseIf_DeleteTblRow (
                                                   bus_handle,
                                                   XTM_COMPONENT_NAME,
                                                   XTM_DBUS_PATH,
                                                   0,          /* session id */
                                                   acTableName
                                                 ) )
    {
         CcspTraceError(("%s Failed to delete table %s\n", __FUNCTION__,acTableName));
         return ANSC_STATUS_FAILURE;
    }

    if(acSetParamName != NULL)
    {
        free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
        free(acSetParamValue);
    }

    CcspTraceInfo(("%s %d Successfully notified Down event to XTM Agent for %s interface\n", __FUNCTION__,__LINE__,ifname));

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslSetWanLinkStatusForWanManager() */
ANSC_STATUS DmlXdslSetWanLinkStatusForWanManager( char *ifname, char *WanStatus )
{
    DML_XDSL_LINE_GLOBALINFO   stGlobalInfo   = { 0 };
    char                       *acSetParamName = NULL;
    char                       *acSetParamValue = NULL;

    INT                            iWANInstance   = -1;

    //Validate buffer
    if( ( NULL == ifname ) || ( NULL == WanStatus ) )
    {   
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( ifname, &stGlobalInfo );

    //Get Instance for corresponding name
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance );

    //Index is not present. so no need to do anything any WAN instance
    if( -1 == iWANInstance )
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if (acSetParamName == NULL || acSetParamValue == NULL)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    memset( acSetParamName, 0, DATAMODEL_PARAM_LENGTH );
    memset( acSetParamValue, 0, DATAMODEL_PARAM_LENGTH );

    CcspTraceInfo(("%s %d WAN Instance:%d\n",__FUNCTION__, __LINE__,iWANInstance));

    //Set WAN Link Status
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_LINK_STATUS_PARAM_NAME, iWANInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", WanStatus );
    DmlXdslSetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    if(acSetParamName != NULL)
    {
        free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
        free(acSetParamValue);
    }

    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, WanStatus, ifname));

    return ANSC_STATUS_SUCCESS;
}

/* * DmlXdslSetPhyStatusForWanManager() */
ANSC_STATUS DmlXdslSetPhyStatusForWanManager( char *ifname, char *PhyStatus )
{
    DML_XDSL_LINE_GLOBALINFO   stGlobalInfo   = { 0 };
    char                       *acSetParamName = NULL;
    char                       *acSetParamValue = NULL;
    INT                        iWANInstance   = -1;

    //Validate buffer
    if( ( NULL == ifname ) || ( NULL == PhyStatus ) )
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    //Get global copy of the data from interface name
    DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( ifname, &stGlobalInfo );

    //Get Instance for corresponding name
    DmlXdslGetLowerLayersInstanceInOtherAgent( NOTIFY_TO_WAN_AGENT, stGlobalInfo.Name, &iWANInstance );

    //Index is not present. so no need to do anything any WAN instance
    if( -1 == iWANInstance )
    {
        CcspTraceError(("%s %d WAN instance not present\n", __FUNCTION__, __LINE__));
        return ANSC_STATUS_FAILURE;
    }

    acSetParamName = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);
    acSetParamValue = (char *) malloc(sizeof(char) * DATAMODEL_PARAM_LENGTH);

    if(acSetParamName == NULL || acSetParamValue == NULL)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }

    memset( acSetParamName, 0, DATAMODEL_PARAM_LENGTH );
    memset( acSetParamValue, 0, DATAMODEL_PARAM_LENGTH );

    CcspTraceInfo(("%s %d WAN Instance:%d\n",__FUNCTION__, __LINE__,iWANInstance));

    //Set PHY path
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_PHY_PATH_PARAM_NAME, iWANInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", stGlobalInfo.LowerLayers );
    DmlXdslSetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, FALSE );

    //Set PHY Status
    snprintf( acSetParamName, DATAMODEL_PARAM_LENGTH, WAN_PHY_STATUS_PARAM_NAME, iWANInstance );
    snprintf( acSetParamValue, DATAMODEL_PARAM_LENGTH, "%s", PhyStatus );
    DmlXdslSetParamValues( WAN_COMPONENT_NAME, WAN_DBUS_PATH, acSetParamName, acSetParamValue, ccsp_string, TRUE );

    if(acSetParamName != NULL)
    {
        free(acSetParamName);
    }
    if(acSetParamValue != NULL)
    {
        free(acSetParamValue);
    }

    CcspTraceInfo(("%s %d Successfully notified %s event to WAN Agent for %s interface\n", __FUNCTION__, __LINE__, PhyStatus, ifname));

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
    memset( pXdslReportTmp, 0, sizeof(DML_X_RDK_REPORT_DSL) );
    GetNVRamULONGConfiguration(XdslReportStatusEnable, &psmValue);
    pXdslReportTmp->Enabled = psmValue;
    GetNVRamULONGConfiguration(XdslReportStatusReportingPeriod, &psmValue);
    pXdslReportTmp->ReportingPeriod = psmValue;

    //Memset all memory
    memset( pXdslReportDfltTmp, 0, sizeof(DML_X_RDK_REPORT_DSL_DEFAULT) );
    GetNVRamULONGConfiguration(XdslReportStatusDfltReportingPeriod, &psmValue);
    pXdslReportDfltTmp->ReportingPeriod = psmValue;

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
