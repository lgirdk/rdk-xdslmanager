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

    module: plugin_main_apis.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------

    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        23/10/2019    initial revision.

**************************************************************************/


#ifndef  _PLUGIN_MAIN_APIS_H
#define  _PLUGIN_MAIN_APIS_H

#include "ansc_platform.h"
#include "cosa_apis.h"
#include "dslh_cpeco_interface.h"

// include files needed by diagnostic
/*
#include "dslh_definitions_diagnostics.h"
#include "bbhm_diag_lib.h"
*/
#include "dslh_dmagnt_interface.h"
#include "ccsp_ifo_ccd.h"

/*
#include "bbhm_diageo_interface.h"
#include "bbhm_diagip_interface.h"
#include "bbhm_diagit_interface.h"
#include "bbhm_diagns_interface.h"
#include "bbhm_download_interface.h"
#include "bbhm_upload_interface.h"
#include "bbhm_udpecho_interface.h"
*/

/*extern PCOSA_DIAG_PLUGIN_INFO             g_pCosaDiagPluginInfo;*/
extern COSAGetParamValueByPathNameProc    g_GetParamValueByPathNameProc;
extern COSASetParamValueByPathNameProc    g_SetParamValueByPathNameProc;
extern COSAGetParamValueStringProc        g_GetParamValueString;
extern COSAGetParamValueUlongProc         g_GetParamValueUlong;
extern COSAGetParamValueIntProc           g_GetParamValueInt;
extern COSAGetParamValueBoolProc          g_GetParamValueBool;
extern COSASetParamValueStringProc        g_SetParamValueString;
extern COSASetParamValueUlongProc         g_SetParamValueUlong;
extern COSASetParamValueIntProc           g_SetParamValueInt;
extern COSASetParamValueBoolProc          g_SetParamValueBool;
extern COSAGetInstanceNumbersProc         g_GetInstanceNumbers;

extern COSAValidateHierarchyInterfaceProc g_ValidateInterface;
extern COSAGetHandleProc                  g_GetRegistryRootFolder;
extern COSAGetInstanceNumberByIndexProc   g_GetInstanceNumberByIndex;
extern COSAGetHandleProc                  g_GetMessageBusHandle;
extern COSAGetSubsystemPrefixProc         g_GetSubsystemPrefix;
extern COSAGetInterfaceByNameProc         g_GetInterfaceByName;
extern PCCSP_CCD_INTERFACE                g_pPnmCcdIf;
extern ANSC_HANDLE                        g_MessageBusHandle;
extern char*                              g_SubsystemPrefix;
extern COSARegisterCallBackAfterInitDmlProc  g_RegisterCallBackAfterInitDml;

/* The OID for all objects s*/
#define DATAMODEL_BASE_OID                                 0

/*
 * This is the cosa datamodel backend manager which is used to manager all backend object
 */
#define  BACKEND_MANAGER_CLASS_CONTENT \
    /* duplication of the base object class content */ \
    BASE_CONTENT \
    ANSC_HANDLE                  hDSL; \
    ANSC_HANDLE hPTM; \
    ANSC_HANDLE hATM; \
    PCOSA_PLUGIN_INFO hCosaPluginInfo; \

typedef  struct
_BACKEND_MANAGER_OBJECT
{
    BACKEND_MANAGER_CLASS_CONTENT
#ifdef _COSA_SIM_                                                                           
            ULONG                        has_wifi_slap;                                             
            ULONG                        has_moca_slap;                                             
#endif                                                                                      
}
BACKEND_MANAGER_OBJECT,  *PBACKEND_MANAGER_OBJECT;

extern PBACKEND_MANAGER_OBJECT g_pBEManager;

ANSC_HANDLE
BackEndManagerCreate
    (
        VOID
    );

ANSC_STATUS
BackEndManagerInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
BackEndManagerRemove
    (
        ANSC_HANDLE                 hThisObject
    );

#endif
