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

/*********************************************************************** 
  
    module: plugin_main.c

        Implement COSA Data Model Library Init and Unload apis.
 
    ---------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    ---------------------------------------------------------------

    revision:

        23/10/2019    initial revision.

**********************************************************************/

#include "ansc_platform.h"
#include "ansc_load_library.h"
#include "cosa_plugin_api.h"
#include "plugin_main.h"
#include "plugin_main_apis.h"
#include "apis_xdslmanagerplugin.h"
#include "xdsl_dml.h"
#include "xtm_dml.h"

PBACKEND_MANAGER_OBJECT g_pBEManager;
void *                       g_pDslhDmlManager;
extern ANSC_HANDLE     g_MessageBusHandle_Irep;
extern char            g_SubSysPrefix_Irep[32];
extern COSARepopulateTableProc      g_COSARepopulateTable;

#define THIS_PLUGIN_VERSION                         1

int ANSC_EXPORT_API
COSA_Init
    (
        ULONG                       uMaxVersionSupported, 
        void*                       hCosaPlugInfo         /* PCOSA_PLUGIN_INFO passed in by the caller */
    )
{
    PCOSA_PLUGIN_INFO               pPlugInfo  = (PCOSA_PLUGIN_INFO)hCosaPlugInfo;
    COSAGetParamValueByPathNameProc pGetParamValueByPathNameProc = (COSAGetParamValueByPathNameProc)NULL;
    COSASetParamValueByPathNameProc pSetParamValueByPathNameProc = (COSASetParamValueByPathNameProc)NULL;
    COSAGetParamValueStringProc     pGetStringProc              = (COSAGetParamValueStringProc       )NULL;
    COSAGetParamValueUlongProc      pGetParamValueUlongProc     = (COSAGetParamValueUlongProc        )NULL;
    COSAGetParamValueIntProc        pGetParamValueIntProc       = (COSAGetParamValueIntProc          )NULL;
    COSAGetParamValueBoolProc       pGetParamValueBoolProc      = (COSAGetParamValueBoolProc         )NULL;
    COSASetParamValueStringProc     pSetStringProc              = (COSASetParamValueStringProc       )NULL;
    COSASetParamValueUlongProc      pSetParamValueUlongProc     = (COSASetParamValueUlongProc        )NULL;
    COSASetParamValueIntProc        pSetParamValueIntProc       = (COSASetParamValueIntProc          )NULL;
    COSASetParamValueBoolProc       pSetParamValueBoolProc      = (COSASetParamValueBoolProc         )NULL;
    COSAGetInstanceNumbersProc      pGetInstanceNumbersProc     = (COSAGetInstanceNumbersProc        )NULL;

    COSAGetCommonHandleProc         pGetCHProc                  = (COSAGetCommonHandleProc           )NULL;
    COSAValidateHierarchyInterfaceProc
                                    pValInterfaceProc           = (COSAValidateHierarchyInterfaceProc)NULL;
    COSAGetHandleProc               pGetRegistryRootFolder      = (COSAGetHandleProc                 )NULL;
    COSAGetInstanceNumberByIndexProc
                                    pGetInsNumberByIndexProc    = (COSAGetInstanceNumberByIndexProc  )NULL;
    COSAGetHandleProc               pGetMessageBusHandleProc    = (COSAGetHandleProc                 )NULL;
    COSAGetInterfaceByNameProc      pGetInterfaceByNameProc     = (COSAGetInterfaceByNameProc        )NULL;
    ULONG                           ret                         = 0;

    if ( uMaxVersionSupported < THIS_PLUGIN_VERSION )
    {
      /* this version is not supported */
        return -1;
    }   
    
    pPlugInfo->uPluginVersion       = THIS_PLUGIN_VERSION;
    g_pDslhDmlManager                 = pPlugInfo->hDmlAgent;

/*
    pGetCHProc = (COSAGetCommonHandleProc)pPlugInfo->AcquireFunction("COSAGetDiagPluginInfo");

    if( pGetCHProc != NULL)
    {
        g_pCosaDiagPluginInfo = pGetCHProc(NULL);
    }
    else
    {
        goto EXIT;
    }
*/

    pGetParamValueByPathNameProc = (COSAGetParamValueByPathNameProc)pPlugInfo->AcquireFunction("COSAGetParamValueByPathName");

    if( pGetParamValueByPathNameProc != NULL)
    {
        g_GetParamValueByPathNameProc = pGetParamValueByPathNameProc;
    }
    else
    {
        goto EXIT;
    }

    pSetParamValueByPathNameProc = (COSASetParamValueByPathNameProc)pPlugInfo->AcquireFunction("COSASetParamValueByPathName");

    if( pSetParamValueByPathNameProc != NULL)
    {
        g_SetParamValueByPathNameProc = pSetParamValueByPathNameProc;
    }
    else
    {
        goto EXIT;
    }

    pGetStringProc = (COSAGetParamValueStringProc)pPlugInfo->AcquireFunction("COSAGetParamValueString");

    if( pGetStringProc != NULL)
    {
        g_GetParamValueString = pGetStringProc;
    }
    else
    {
        goto EXIT;
    }

    pGetParamValueUlongProc = (COSAGetParamValueUlongProc)pPlugInfo->AcquireFunction("COSAGetParamValueUlong");

    if( pGetParamValueUlongProc != NULL)
    {
        g_GetParamValueUlong = pGetParamValueUlongProc;
    }
    else
    {
        goto EXIT;
    }
    pGetParamValueIntProc = (COSAGetParamValueUlongProc)pPlugInfo->AcquireFunction("COSAGetParamValueInt");

    if( pGetParamValueIntProc != NULL)
    {
        g_GetParamValueInt = pGetParamValueIntProc;
    }
    else
    {
        goto EXIT;
    }

    pGetParamValueBoolProc = (COSAGetParamValueBoolProc)pPlugInfo->AcquireFunction("COSAGetParamValueBool");

    if( pGetParamValueBoolProc != NULL)
    {
        g_GetParamValueBool = pGetParamValueBoolProc;
    }
    else
    {
        goto EXIT;
    }

    pSetStringProc = (COSASetParamValueStringProc)pPlugInfo->AcquireFunction("COSASetParamValueString");

    if( pSetStringProc != NULL)
    {
        g_SetParamValueString = pSetStringProc;
    }
    else
    {
        goto EXIT;
    }

    pSetParamValueUlongProc = (COSASetParamValueUlongProc)pPlugInfo->AcquireFunction("COSASetParamValueUlong");

    if( pSetParamValueUlongProc != NULL)
    {
        g_SetParamValueUlong = pSetParamValueUlongProc;
    }
    else
    {
        goto EXIT;
    }

    pSetParamValueIntProc = (COSASetParamValueIntProc)pPlugInfo->AcquireFunction("COSASetParamValueInt");

    if( pSetParamValueIntProc != NULL)
    {
        g_SetParamValueInt = pSetParamValueIntProc;
    }
    else
    {
        goto EXIT;
    }

    pSetParamValueBoolProc = (COSASetParamValueBoolProc)pPlugInfo->AcquireFunction("COSASetParamValueBool");

    if( pSetParamValueBoolProc != NULL)
    {
        g_SetParamValueBool = pSetParamValueBoolProc;
    }
    else
    {
        goto EXIT;
    }

    pGetInstanceNumbersProc = (COSAGetInstanceNumbersProc)pPlugInfo->AcquireFunction("COSAGetInstanceNumbers");

    if( pGetInstanceNumbersProc != NULL)
    {
        g_GetInstanceNumbers = pGetInstanceNumbersProc;
    }
    else
    {
        goto EXIT;
    }

    pValInterfaceProc = (COSAValidateHierarchyInterfaceProc)pPlugInfo->AcquireFunction("COSAValidateHierarchyInterface");

    if ( pValInterfaceProc )
    {
        g_ValidateInterface = pValInterfaceProc;
    }
    else
    {
        goto EXIT;
    }
/*
#ifndef _ANSC_WINDOWSNT
#ifdef _SOFTWAREMODULES_SUPPORT_NAF
    CosaSoftwareModulesInit(hPlugInfo);
#endif
#endif
*/
    pGetRegistryRootFolder = (COSAGetHandleProc)pPlugInfo->AcquireFunction("COSAGetRegistryRootFolder");

    if ( pGetRegistryRootFolder != NULL )
    {
        g_GetRegistryRootFolder = pGetRegistryRootFolder;
    }
    else
    {
        printf("!!! haha, catcha !!!\n");
        goto EXIT;
    }

    pGetInsNumberByIndexProc = (COSAGetInstanceNumberByIndexProc)pPlugInfo->AcquireFunction("COSAGetInstanceNumberByIndex");

    if ( pGetInsNumberByIndexProc != NULL )
    {
        g_GetInstanceNumberByIndex = pGetInsNumberByIndexProc;
    }
    else
    {
        goto EXIT;
    }

    pGetInterfaceByNameProc = (COSAGetInterfaceByNameProc)pPlugInfo->AcquireFunction("COSAGetInterfaceByName");

    if ( pGetInterfaceByNameProc != NULL )
    {
        g_GetInterfaceByName = pGetInterfaceByNameProc;
    }
    else
    {
        goto EXIT;
    }

    g_pPnmCcdIf = g_GetInterfaceByName(g_pDslhDmlManager, CCSP_CCD_INTERFACE_NAME);

    if ( !g_pPnmCcdIf )
    {
        CcspTraceError(("g_pPnmCcdIf is NULL !\n"));

        goto EXIT;
    }

    g_RegisterCallBackAfterInitDml = (COSARegisterCallBackAfterInitDmlProc)pPlugInfo->AcquireFunction("COSARegisterCallBackAfterInitDml");

    if ( !g_RegisterCallBackAfterInitDml )
    {
        goto EXIT;
    }

    g_COSARepopulateTable = (COSARepopulateTableProc)pPlugInfo->AcquireFunction("COSARepopulateTable");

    if ( !g_COSARepopulateTable )
    {
        goto EXIT;
    }

    /* Get Message Bus Handle */
    g_GetMessageBusHandle = (PFN_CCSPCCDM_APPLY_CHANGES)pPlugInfo->AcquireFunction("COSAGetMessageBusHandle");
    if ( g_GetMessageBusHandle == NULL )
    {
        goto EXIT;
    }

    g_MessageBusHandle = (ANSC_HANDLE)g_GetMessageBusHandle(g_pDslhDmlManager);
    if ( g_MessageBusHandle == NULL )
    {
        goto EXIT;
    }
    g_MessageBusHandle_Irep = g_MessageBusHandle;

    /* Get Subsystem prefix */
    g_GetSubsystemPrefix = (COSAGetSubsystemPrefixProc)pPlugInfo->AcquireFunction("COSAGetSubsystemPrefix");
    if ( g_GetSubsystemPrefix != NULL )
    {
        char*   tmpSubsystemPrefix;

        if ( tmpSubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlManager) )
        {
            AnscCopyString(g_SubSysPrefix_Irep, tmpSubsystemPrefix);
        }

        /* retrieve the subsystem prefix */
        g_SubsystemPrefix = g_GetSubsystemPrefix(g_pDslhDmlManager);
    }

    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_Synchronize",  Line_Synchronize);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_IsUpdated",  Line_IsUpdated);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetEntryCount",  Line_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetEntry",  Line_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetParamBoolValue",  Line_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetParamUlongValue",  Line_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetParamStringValue",  Line_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_SetParamBoolValue",  Line_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_SetParamStringValue",  Line_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_GetParamIntValue",  Line_GetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_Commit",  Line_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_Validate",  Line_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Line_Rollback",  Line_Rollback);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStats_GetParamUlongValue",  LineStats_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStatsTotal_GetParamUlongValue",  LineStatsTotal_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStatsShowtime_GetParamUlongValue",  LineStatsShowtime_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStatsLastShowtime_GetParamUlongValue",  LineStatsLastShowtime_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStatsQuarterHour_GetParamUlongValue",  LineStatsQuarterHour_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineStatsCurrentDay_GetParamUlongValue",  LineStatsCurrentDay_GetParamUlongValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineTestParams_GetParamUlongValue", LineTestParams_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "LineTestParams_GetParamStringValue", LineTestParams_GetParamStringValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_Synchronize",  Channel_Synchronize);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_IsUpdated",  Channel_IsUpdated);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetEntryCount",  Channel_GetEntryCount);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetEntry",  Channel_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetParamBoolValue",  Channel_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetParamUlongValue",  Channel_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetParamStringValue",  Channel_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_SetParamBoolValue",  Channel_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_SetParamStringValue",  Channel_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_GetParamIntValue",  Channel_GetParamIntValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_Commit",  Channel_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_Validate",  Channel_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "Channel_Rollback",  Channel_Rollback);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStats_GetParamUlongValue",  ChannelStats_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStatsTotal_GetParamUlongValue",  ChannelStatsTotal_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStatsShowTime_GetParamUlongValue",  ChannelStatsShowTime_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStatsLastShowTime_GetParamUlongValue",  ChannelStatsLastShowTime_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStatsQuarterHour_GetParamUlongValue",  ChannelStatsQuarterHour_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ChannelStatsCurrentDay_GetParamUlongValue",  ChannelStatsCurrentDay_GetParamUlongValue);
    
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ADSLLineTest_GetParamUlongValue",  ADSLLineTest_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ADSLLineTest_SetParamUlongValue",  ADSLLineTest_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ADSLLineTest_GetParamStringValue", ADSLLineTest_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ADSLLineTest_SetParamStringValue",  ADSLLineTest_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ADSLLineTest_GetParamIntValue",  ADSLLineTest_GetParamIntValue);
   
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTUER_GetParamUlongValue",  SELTUER_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTUER_SetParamUlongValue",  SELTUER_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTUER_GetParamStringValue", SELTUER_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTUER_SetParamStringValue",  SELTUER_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTUER_GetParamBoolValue",  SELTUER_GetParamBoolValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTQLN_GetParamUlongValue",  SELTQLN_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTQLN_SetParamUlongValue",  SELTQLN_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTQLN_GetParamStringValue", SELTQLN_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTQLN_SetParamStringValue",  SELTQLN_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTQLN_GetParamBoolValue",  SELTQLN_GetParamBoolValue);

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_GetParamUlongValue",  SELTP_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_SetParamUlongValue",  SELTP_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_GetParamStringValue", SELTP_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_SetParamStringValue",  SELTP_SetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_GetParamBoolValue",  SELTP_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "SELTP_SetParamBoolValue",  SELTP_SetParamBoolValue);
    
    /* DSL Diagnostic Collection */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_GetParamUlongValue",  X_RDK_Report_DSL_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_SetParamUlongValue",  X_RDK_Report_DSL_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_GetParamStringValue", X_RDK_Report_DSL_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_GetParamBoolValue",  X_RDK_Report_DSL_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_SetParamBoolValue",  X_RDK_Report_DSL_SetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_Default_GetParamUlongValue",  X_RDK_Report_DSL_Default_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_Default_SetParamUlongValue",  X_RDK_Report_DSL_Default_SetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_Commit",  X_RDK_Report_DSL_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_Validate",  X_RDK_Report_DSL_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_Report_DSL_Rollback", X_RDK_Report_DSL_Rollback);

    /* DSL X_RDK_NLNM*/
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "X_RDK_NLNM_GetParamIntValue",  X_RDK_NLNM_GetParamIntValue);

    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_Synchronize",  PTMLink_Synchronize);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_IsUpdated",  PTMLink_IsUpdated);    
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_GetEntryCount", PTMLink_GetEntryCount );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_GetEntry",  PTMLink_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_GetParamBoolValue",  PTMLink_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_GetParamUlongValue",  PTMLink_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_GetParamStringValue",  PTMLink_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_Validate",  PTMLink_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_Commit",  PTMLink_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLink_Rollback", PTMLink_Rollback );

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "PTMLinkStats_GetParamUlongValue", PTMLinkStats_GetParamUlongValue );

    /* register the back-end apis for the data model */
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_Synchronize",  ATMLink_Synchronize);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_IsUpdated",  ATMLink_IsUpdated);    
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_GetEntryCount", ATMLink_GetEntryCount );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_GetEntry",  ATMLink_GetEntry);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_GetParamBoolValue",  ATMLink_GetParamBoolValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_GetParamUlongValue",  ATMLink_GetParamUlongValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_GetParamStringValue",  ATMLink_GetParamStringValue);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_Validate",  ATMLink_Validate);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_Commit",  ATMLink_Commit);
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLink_Rollback", ATMLink_Rollback );
    
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkStats_GetParamUlongValue", ATMLinkStats_GetParamUlongValue );

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkQOS_GetParamUlongValue", ATMLinkQOS_GetParamUlongValue );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkQOS_SetParamUlongValue", ATMLinkQOS_SetParamUlongValue );

    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkDiagnostics_GetParamUlongValue", ATMLinkDiagnostics_GetParamUlongValue );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkDiagnostics_SetParamUlongValue", ATMLinkDiagnostics_SetParamUlongValue );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkDiagnostics_GetParamStringValue", ATMLinkDiagnostics_GetParamStringValue );
    pPlugInfo->RegisterFunction(pPlugInfo->hContext, "ATMLinkDiagnostics_SetParamStringValue", ATMLinkDiagnostics_SetParamStringValue );

    /* Create backend framework */
    g_pBEManager = (PBACKEND_MANAGER_OBJECT)BackEndManagerCreate();

    if ( g_pBEManager && g_pBEManager->Initialize )
    {
        g_pBEManager->hCosaPluginInfo = pPlugInfo;

        g_pBEManager->Initialize   ((ANSC_HANDLE)g_pBEManager);
    }
    
    return  0;
EXIT:

    return -1;
}

BOOL ANSC_EXPORT_API
COSA_IsObjectSupported
    (
        char*                        pObjName
    )
{
    
    return TRUE;
}

void ANSC_EXPORT_API
COSA_Unload
    (
        void
    )
{
    ANSC_STATUS                     returnStatus            = ANSC_STATUS_SUCCESS;

    /* unload the memory here */

    returnStatus  =  BackEndManagerRemove(g_pBEManager);
        
    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        g_pBEManager = NULL;
    }
    else
    {
        /* print error trace*/
        g_pBEManager = NULL;
    }
}

void ANSC_EXPORT_API
XtmManager_MemoryCheck
    (
        void
    )
{
    ANSC_STATUS                     returnStatus            = ANSC_STATUS_SUCCESS;
    PCOSA_PLUGIN_INFO               pPlugInfo               = (PCOSA_PLUGIN_INFO)g_pBEManager->hCosaPluginInfo;

    /* unload the memory here */

    returnStatus  =  BackEndManagerRemove(g_pBEManager);

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        g_pBEManager = NULL;
    }
    else
    {
        g_pBEManager = NULL;
    }


    g_pBEManager = (PBACKEND_MANAGER_OBJECT)BackEndManagerCreate();

    if ( g_pBEManager && g_pBEManager->Initialize )
    {
        g_pBEManager->hCosaPluginInfo = pPlugInfo;

        g_pBEManager->Initialize   ((ANSC_HANDLE)g_pBEManager);
    }
}

