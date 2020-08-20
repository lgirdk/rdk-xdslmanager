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

#include "ansc_platform.h"
#include "apis_xdslmanagerplugin.h"
#include "ccsp_trace.h"
#include "ccsp_syslog.h"

/*RDKB-7469, CID-33124, defines*/
#define XDSLMANAGER_MAX_MSG_LENGTH    256
#define XDSLMANAGER_MAX_BUF_SIZE      241
#define XDSLMANAGER_MAX_COMMAND_LEN   256
#define XDSLMANAGER_PROC_NAME_LEN     50
#define XDSLMANAGER_MAX_READ_SIZE     120

/* structure defined for object "PluginSampleObj"  */
typedef  struct
_COSA_PLUGIN_SAMPLE_INFO
{
    ULONG                           loglevel; 
    char                            WifiLogMsg[XDSLMANAGER_MAX_MSG_LENGTH];
    char 			    HarvesterLogMsg[XDSLMANAGER_MAX_MSG_LENGTH];   //Added for RDKB-4343
   
}
COSA_PLUGIN_SAMPLE_INFO,  *PCOSA_PLUGIN_SAMPLE_INFO;

COSA_PLUGIN_SAMPLE_INFO  g_BackPluginInfo ;

BOOL
XdslManagerGetParamUlongValue
    (
         ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{

    *puLong=1012019;
    printf("Calling for %s \n",ParamName);
    return TRUE;
}
BOOL
XdslManagerSetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                      uValue
    )
{
    ULONG                           i               = 0;
    printf("Calling for %s \n",ParamName);
    printf("value is %ld \n",uValue);
    return TRUE;
}


BOOL
XdslManagerSetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    printf("Calling for %s \n",ParamName);
    printf("value is %s \n",pString);
	
        return TRUE;
		
}


ULONG
XdslManagerGetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    printf("Calling for %s \n",ParamName);
    AnscCopyString(pValue, "RdkXdslManager");
   
        return TRUE;
}

ULONG
XdslManagerCommit
    (
        ANSC_HANDLE                 hInsContext
    )
{
   
    return 0;
}
BOOL
XdslManagerGetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    printf("Calling for %s \n",ParamName);
    *pBool=TRUE;
    return FALSE;
}
BOOL
XdslManagerSetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    printf("Calling for %s \n",ParamName);
    printf("value is %d \n",bValue);

        return TRUE;
}

