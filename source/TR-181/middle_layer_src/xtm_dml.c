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

#include "ansc_platform.h"
#include "xtm_dml.h"
#include "xtm_apis.h"
#include "plugin_main_apis.h"
#include "xtm_internal.h"
#include "ccsp_psm_helper.h"

#if     CFG_USE_CCSP_SYSLOG
    #include <ccsp_syslog.h>
#endif

#define PSM_ADSL_LINKTYPE           "dmsb.xdslmanager.atm.linktype"    
#define PSM_ADSL_ENCAPSULATION      "dmsb.xdslmanager.atm.encapsulation"
#define PSM_ADSL_AUTOCONFIG         "dmsb.xdslmanager.atm.autoconfig"
#define PSM_ADSL_PVC                "dmsb.xdslmanager.atm.pvc"
#define PSM_ADSL_AAL                "dmsb.xdslmanager.atm.aal"
#define PSM_ADSL_FCSPRESERVED       "dmsb.xdslmanager.atm.fcspreserved"
#define PSM_ADSL_VCSEARCHLIST       "dmsb.xdslmanager.atm.vcsearchlist"
#define PSM_ADSL_QOS_CLASS          "dmsb.xdslmanager.atm.qos.class"
#define PSM_ADSL_QOS_PEAKCELLRATE   "dmsb.xdslmanager.atm.qos.peakcellrate"
#define PSM_ADSL_QOS_MAXBURSTSIZE   "dmsb.xdslmanager.atm.qos.maxburstsize"
#define PSM_ADSL_QOS_CELLRATE       "dmsb.xdslmanager.atm.qos.cellrate"

extern char                g_Subsystem[32];
extern ANSC_HANDLE         bus_handle;

#define _PSM_WRITE_PARAM(_PARAM_NAME) { \
    _ansc_sprintf(param_name, _PARAM_NAME); \
    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, param_name, ccsp_string, param_value); \
    if (retPsmSet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));\
    } \
    else \
    { \
        /*AnscTraceFlow(("%s: retPsmSet == CCSP_SUCCESS writing %s = %s \n", __FUNCTION__,param_name,param_value)); */\
    } \
    _ansc_memset(param_name, 0, sizeof(param_name)); \
    _ansc_memset(param_value, 0, sizeof(param_value)); \
}


/***********************************************************************
 IMPORTANT NOTE:

 According to TR69 spec:
 On successful receipt of a SetParameterValues RPC, the CPE MUST apply
 the changes to all of the specified Parameters atomically. That is, either
 all of the value changes are applied together, or none of the changes are
 applied at all. In the latter case, the CPE MUST return a fault response
 indicating the reason for the failure to apply the changes.

 The CPE MUST NOT apply any of the specified changes without applying all
 of them.

 In order to set parameter values correctly, the back-end is required to
 hold the updated values until "Validate" and "Commit" are called. Only after
 all the "Validate" passed in different objects, the "Commit" will be called.
 Otherwise, "Rollback" will be called instead.

 The sequence in COSA Data Model will be:

 SetParamBoolValue/SetParamIntValue/SetParamUlongValue/SetParamStringValue
 -- Backup the updated values;

 if( Validate_XXX())
 {
     Commit_XXX();    -- Commit the update all together in the same object
 }
 else
 {
     Rollback_XXX();  -- Remove the update at backup;
 }

***********************************************************************/
/***********************************************************************

 APIs for Object:

    Device.PTM.Link.{i}.

    *  PTMLink_Synchronize
    *  PTMLink_IsUpdated
    *  PTMLink_GetEntryCount
    *  PTMLink_GetEntry
    *  PTMLink_AddEntry
    *  PTMLink_DelEntry
    *  PTMLink_GetParamBoolValue
    *  PTMLink_GetParamUlongValue
    *  PTMLink_GetParamStringValue
    *  PTMLink_Validate
    *  PTMLink_Commit
    *  PTMLink_Rollback

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
PTMLink_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    BOOL    bIsUpdated   = TRUE;

    return bIsUpdated;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
PTMLink_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS            returnStatus   = ANSC_STATUS_SUCCESS;

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/

ULONG PTMLink_GetEntryCount( ANSC_HANDLE hInsContext )
{
    PDATAMODEL_PTM             pPTM         = (PDATAMODEL_PTM)g_pBEManager->hPTM;
    return AnscSListQueryDepth( &pPTM->Q_PtmList );
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        PTMLink_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/

ANSC_HANDLE PTMLink_GetEntry (ANSC_HANDLE  hInsContext, ULONG nIndex, ULONG*  pInsNumber )
{
    PDATAMODEL_PTM             pMyObject         = (PDATAMODEL_PTM)g_pBEManager->hPTM;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PCONTEXT_LINK_OBJECT       pCxtLink          = NULL;

    pSListEntry       = AnscSListGetEntryByIndex(&pMyObject->Q_PtmList, nIndex);

    if ( pSListEntry )
    {
        pCxtLink      = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
        *pInsNumber   = pCxtLink->InstanceNumber;
    }

    return (ANSC_HANDLE)pSListEntry;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        PTMLink_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/

ANSC_HANDLE PTMLink_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber )
{
    ANSC_STATUS                          returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM                  pPTM              = (PDATAMODEL_PTM)g_pBEManager->hPTM;
    PDML_PTM                  p_Ptm      = NULL;
    PCONTEXT_LINK_OBJECT            pPtmCxtLink  = NULL;
    PSINGLE_LINK_ENTRY                   pSListEntry       = NULL;
    BOOL                                      bridgeMode;

    p_Ptm = (PDML_PTM)AnscAllocateMemory(sizeof(DML_PTM));

    if ( !p_Ptm )
    {
        return NULL;
    }

    pPtmCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(CONTEXT_LINK_OBJECT));
    if ( !pPtmCxtLink )
    {
        goto EXIT;
    }

    /* now we have this link content */
    pPtmCxtLink->hContext = (ANSC_HANDLE)p_Ptm;
    pPtmCxtLink->bNew     = TRUE;

    /* Get InstanceNumber and Alias */
    memset( p_Ptm, 0, sizeof( DML_PTM ) );
    PtmGenForTriggerEntry(NULL, p_Ptm);

    pPtmCxtLink->InstanceNumber = p_Ptm->InstanceNumber ;
    *pInsNumber                      = p_Ptm->InstanceNumber ;

    SListPushEntryByInsNum(&pPTM->Q_PtmList, (PCONTEXT_LINK_OBJECT)pPtmCxtLink);
   
    return (ANSC_HANDLE)pPtmCxtLink;

EXIT:
    AnscFreeMemory(p_Ptm);

    return NULL;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/

ULONG PTMLink_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance )
{
    ANSC_STATUS                returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pPTM              = (PDATAMODEL_PTM)g_pBEManager->hPTM;
    PCONTEXT_LINK_OBJECT       pPtmCxtLink       = (PCONTEXT_LINK_OBJECT)hInstance;
    PDML_PTM                   p_Ptm             = (PDML_PTM)pPtmCxtLink->hContext;

    if ( pPtmCxtLink->bNew )
    {
        /* Set bNew to FALSE to indicate this node is not going to save to SysRegistry */
        pPtmCxtLink->bNew = FALSE;
    }
    else
    {
        returnStatus = DmlDelPtm( NULL, p_Ptm );
    }

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        AnscSListPopEntryByLink(&pPTM->Q_PtmList, &pPtmCxtLink->Linkage);

        AnscFreeMemory(pPtmCxtLink->hContext);
        AnscFreeMemory(pPtmCxtLink);
    }

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL PTMLink_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM             p_Ptm  = (PDML_PTM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        BOOLEAN  bEnable = FALSE;

        if ( ANSC_STATUS_SUCCESS == DmlGetPtmIfEnable( &bEnable ) )
        {
            p_Ptm->Enable = bEnable;
            *pBool        = p_Ptm->Enable;
        }

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}



/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_GetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int*                        pInt
            );

    description:

        This function is called to retrieve integer parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int*                        pInt
                The buffer of returned integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL PTMLink_GetParamIntValue ( ANSC_HANDLE  hInsContext, char* ParamName, int* pInt )
{
    PCONTEXT_LINK_OBJECT        pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM              p_Ptm  = (PDML_PTM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL PTMLink_GetParamUlongValue  ( ANSC_HANDLE  hInsContext, char*  ParamName, ULONG* puLong )
{
    PCONTEXT_LINK_OBJECT        pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM           p_Ptm  = (PDML_PTM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */

    if (strcmp(ParamName, "Status") == 0)
    {
        if(ANSC_STATUS_SUCCESS == DmlGetPtmIfStatus(NULL, p_Ptm)) {
            *puLong = p_Ptm->Status;
        }
        return TRUE;
    }
    if (strcmp(ParamName, "LastChange") == 0)
    {
        *puLong = p_Ptm->LastChange;
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/

ULONG PTMLink_GetParamStringValue ( ANSC_HANDLE hInsContext, char*  ParamName, char* pValue, ULONG* pUlSize )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM             p_Ptm      = (PDML_PTM   )pCxtLink->hContext;
    PUCHAR                          pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        if ( AnscSizeOfString(p_Ptm->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, p_Ptm->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(p_Ptm->Alias)+1;
            return 1;
        }
    }
    if (strcmp(ParamName, "Name") == 0)
    {
        AnscCopyString(pValue, p_Ptm->Name);
        return 0;
    }
    if (strcmp(ParamName, "LowerLayers") == 0)
    {
        AnscCopyString(pValue, p_Ptm->LowerLayers);
        return 0;
    }
    if (strcmp(ParamName, "MACAddress") == 0)
    {
        AnscCopyString(pValue, p_Ptm->MACAddress);
        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLinkStats_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL PTMLinkStats_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PCONTEXT_LINK_OBJECT  pCxtLink = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM  p_Ptm = (PDML_PTM   )pCxtLink->hContext;

    //Get PTM statistics
    DmlGetPtmIfStatistics( NULL, p_Ptm );

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "BytesSent") == 0)
    {
        *puLong = p_Ptm->Statistics.BytesSent;
        return TRUE;
    }

    if (strcmp(ParamName, "BytesReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.BytesReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "PacketsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.PacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "PacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.PacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.ErrorsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.ErrorsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.UnicastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.UnicastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.DiscardPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.DiscardPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.MulticastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.MulticastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsSent") == 0)
    {
        *puLong = p_Ptm->Statistics.BroadcastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.BroadcastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnknownProtoPacketsReceived") == 0)
    {
        *puLong = p_Ptm->Statistics.UnknownProtoPacketsReceived;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation.

                ULONG*                      puLength
                The output length of the param name.

    return:     TRUE if there's no validation.

**********************************************************************/

BOOL PTMLink_Validate ( ANSC_HANDLE hInsContext, char*  pReturnParamName,  ULONG* puLength )
{
    return TRUE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/

ULONG PTMLink_Commit ( ANSC_HANDLE  hInsContext )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM          p_Ptm  = (PDML_PTM   )pCxtLink->hContext;
    PDATAMODEL_PTM             pPTM          = (PDATAMODEL_PTM      )g_pBEManager->hPTM;

    if ( pCxtLink->bNew )
    {
        returnStatus = DmlAddPtm(NULL, p_Ptm );

        if ( returnStatus == ANSC_STATUS_SUCCESS)
        {
            pCxtLink->bNew = FALSE;
        }
    }
    else 
    {
        returnStatus = DmlSetPtm( NULL, p_Ptm );
    
        if ( returnStatus != ANSC_STATUS_SUCCESS ) 
        {
            CcspTraceError(("%s %d - Failed to set PTM entry\n",__FUNCTION__,__LINE__));
        }
    }

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        PTMLink_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/

ULONG PTMLink_Rollback ( ANSC_HANDLE hInsContext )
{
    ANSC_STATUS                returnStatus  = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pPTM          = (PDATAMODEL_PTM) g_pBEManager->hPTM;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_PTM                   p_Ptm         = (PDML_PTM   )pCxtLink->hContext;

    if ( p_Ptm->Alias )
        AnscCopyString( p_Ptm->Alias, p_Ptm->Alias );

    if ( !pCxtLink->bNew )
    {
        /* We have nothing to do with this case unless we have one getbyEntry() */
    }
    else
    {
        DML_PTM_INIT(p_Ptm);
        _ansc_sprintf(p_Ptm->Name, "ptm%d", p_Ptm->InstanceNumber);
    }

    return returnStatus;
}
/***********************************************************************

 APIs for Object:

    Device.ATM.Link.{i}.

    *  ATMLink_Synchronize
    *  ATMLink_IsUpdated
    *  ATMLink_GetEntryCount
    *  ATMLink_GetEntry
    *  ATMLink_AddEntry
    *  ATMLink_DelEntry
    *  ATMLink_GetParamBoolValue
    *  ATMLink_GetParamUlongValue
    *  ATMLink_GetParamStringValue
    *  ATMLink_Validate
    *  ATMLink_Commit
    *  ATMLink_Rollback

***********************************************************************/

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ATMLink_IsUpdated
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is checking whether the table is updated or not.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     TRUE or FALSE.

**********************************************************************/
BOOL
ATMLink_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    )
{
    BOOL    bIsUpdated   = TRUE;

    return bIsUpdated;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_Synchronize
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to synchronize the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/
ULONG
ATMLink_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS            returnStatus   = ANSC_STATUS_SUCCESS;

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_GetEntryCount
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to retrieve the count of the table.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The count of the table

**********************************************************************/

ULONG ATMLink_GetEntryCount ( ANSC_HANDLE hInsContext )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM)g_pBEManager->hATM;
    return AnscSListQueryDepth( &pATM->Q_AtmList );
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        ATMLink_GetEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG                       nIndex,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to retrieve the entry specified by the index.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG                       nIndex,
                The index of this entry;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle to identify the entry

**********************************************************************/

ANSC_HANDLE ATMLink_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber )
{
    PDATAMODEL_ATM             pMyObject         = (PDATAMODEL_ATM)g_pBEManager->hATM;
    PSINGLE_LINK_ENTRY         pSListEntry       = NULL;
    PCONTEXT_LINK_OBJECT       pCxtLink          = NULL;

    pSListEntry       = AnscSListGetEntryByIndex(&pMyObject->Q_AtmList, nIndex);

    if ( pSListEntry )
    {
        pCxtLink      = ACCESS_CONTEXT_LINK_OBJECT(pSListEntry);
        *pInsNumber   = pCxtLink->InstanceNumber;
    }

    return (ANSC_HANDLE)pSListEntry;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        ATMLink_AddEntry
            (
                ANSC_HANDLE                 hInsContext,
                ULONG*                      pInsNumber
            );

    description:

        This function is called to add a new entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ULONG*                      pInsNumber
                The output instance number;

    return:     The handle of new added entry.

**********************************************************************/

ANSC_HANDLE ATMLink_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM                  pATM              = (PDATAMODEL_ATM)g_pBEManager->hATM;
    PDML_ATM                        p_Atm             = NULL;
    PCONTEXT_LINK_OBJECT            pAtmCxtLink       = NULL;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    BOOL                            bridgeMode;
    INT                             ret_val           = ANSC_STATUS_SUCCESS;
    INT                             retPsmGet         = CCSP_SUCCESS;
    CHAR                            param_name[256]   = {0};
    CHAR                            *param_value      = NULL;

    p_Atm = (PDML_ATM)AnscAllocateMemory(sizeof(DML_ATM));

    if ( !p_Atm )
    {
        return NULL;
    }

    pAtmCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(CONTEXT_LINK_OBJECT));
    if ( !pAtmCxtLink )
    {
        goto EXIT;
    }

    /* now we have this link content */
    pAtmCxtLink->hContext = (ANSC_HANDLE)p_Atm;
    pAtmCxtLink->bNew     = TRUE;

    /* Get InstanceNumber and Alias */
    memset( p_Atm, 0, sizeof( DML_ATM ) );
    AtmGenForTriggerEntry(NULL, p_Atm);

    pAtmCxtLink->InstanceNumber = p_Atm->InstanceNumber ;
    *pInsNumber                 = p_Atm->InstanceNumber ;

    //Sets default configurations
    p_Atm->Status = Down;
    strncpy(p_Atm->Alias, "dsl0", sizeof(p_Atm->Alias) - 1);

    /* Get ADSL Linktype */
    _ansc_sprintf(param_name, PSM_ADSL_LINKTYPE);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        if (strcmp(param_value, "EoA") == 0)
        {
            p_Atm->LinkType = EOA;
        }
        else if (strcmp(param_value, "IPoA") == 0)
        {
            p_Atm->LinkType = IPOA;
        }
        else if (strcmp(param_value, "PPPoA") == 0)
        {
            p_Atm->LinkType = PPPOA;
        }
        else if (strcmp(param_value, "CIP") == 0)
        {
            p_Atm->LinkType = CIP;
        }
        else if (strcmp(param_value, "Unconfigured") == 0)
        {
            p_Atm->LinkType = UNCONFIGURED;
        }
    }

    /* Get ADSL Encapsulation */
    _ansc_sprintf(param_name, PSM_ADSL_ENCAPSULATION);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        if (strcmp(param_value, "LLC") == 0)
        {
            p_Atm->Encapsulation = LLC;
        }
        else if (strcmp(param_value, "VCMUX") == 0)
        {
            p_Atm->Encapsulation = VCMUX;
        }
    }

    /* Get ADSL Autoconfig */
    memset(param_name, 0, sizeof(param_name));
    _ansc_sprintf(param_name, PSM_ADSL_AUTOCONFIG);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        p_Atm->AutoConfig = atoi(param_value);
    }

    /* Get ADSL PVC */
    _ansc_sprintf(param_name, PSM_ADSL_PVC);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        strncpy(p_Atm->DestinationAddress, param_value, sizeof(p_Atm->DestinationAddress) - 1);
    }

    /* Get ADSL AAL */
    _ansc_sprintf(param_name, PSM_ADSL_AAL);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        if (strcmp(param_value, "AAL1") == 0)
        {
            p_Atm->AAL = AAL1;
        }
        else if (strcmp(param_value, "AAL2") == 0)
        {
            p_Atm->AAL = AAL2;
        }
        else if (strcmp(param_value, "AAL3") == 0)
        {
            p_Atm->AAL = AAL3;
        }
        else if (strcmp(param_value, "AAL4") == 0)
        {
            p_Atm->AAL = AAL4;
        }
        else if (strcmp(param_value, "AAL5") == 0)
        {
            p_Atm->AAL = AAL5;
        }
    }

    /* Get ADSL FCSPreserved */
    _ansc_sprintf(param_name, PSM_ADSL_FCSPRESERVED);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        p_Atm->FCSPreserved = atoi(param_value);
    }

    /* Get ADSL VCSearchList */
    _ansc_sprintf(param_name, PSM_ADSL_VCSEARCHLIST);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        strncpy(p_Atm->VCSearchList, param_value, sizeof(p_Atm->VCSearchList) - 1);
    }

    /* Get ADSL QOS Class */
    _ansc_sprintf(param_name, PSM_ADSL_QOS_CLASS);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        if (strcmp(param_value, "UBR") == 0)
        {
            p_Atm->Qos.QoSClass = UBR;
        }
        else if (strcmp(param_value, "CBR") == 0)
        {
            p_Atm->Qos.QoSClass = CBR;
        }
        else if (strcmp(param_value, "GFR") == 0)
        {
            p_Atm->Qos.QoSClass = GFR;
        }
        else if (strcmp(param_value, "VBR-nrt") == 0)
        {
            p_Atm->Qos.QoSClass = VBR_NRT;
        }
        else if (strcmp(param_value, "VBR-rt") == 0)
        {
            p_Atm->Qos.QoSClass = VBR_RT;
        }
        else if (strcmp(param_value, "UBR+") == 0)
        {
            p_Atm->Qos.QoSClass = UBR_PLUS;
        }
        else if (strcmp(param_value, "ABR") == 0)
        {
            p_Atm->Qos.QoSClass = ABR;
        }
    }

    /* Get ADSL QOS Peak cell rate */
    _ansc_sprintf(param_name, PSM_ADSL_QOS_PEAKCELLRATE);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        p_Atm->Qos.PeakCellRate = atoi(param_value);
    }

    /* Get ADSL QOS Max. burst rate */
    _ansc_sprintf(param_name, PSM_ADSL_QOS_MAXBURSTSIZE);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        p_Atm->Qos.MaximumBurstSize = atoi(param_value);
    }

    /* Get ADSL QOS cell rate */
    _ansc_sprintf(param_name, PSM_ADSL_QOS_CELLRATE);
    retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
    if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
    {
        p_Atm->Qos.SustainableCellRate = atoi(param_value);
    }

    if(param_value != NULL)
    {
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
    }

    SListPushEntryByInsNum(&pATM->Q_AtmList, (PCONTEXT_LINK_OBJECT)pAtmCxtLink);
   
    return (ANSC_HANDLE)pAtmCxtLink;

EXIT:
    AnscFreeMemory(p_Atm);

    return NULL;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_DelEntry
            (
                ANSC_HANDLE                 hInsContext,
                ANSC_HANDLE                 hInstance
            );

    description:

        This function is called to delete an exist entry.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                ANSC_HANDLE                 hInstance
                The exist entry handle;

    return:     The status of the operation.

**********************************************************************/

ULONG ATMLink_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance )
{
    ANSC_STATUS                returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pATM              = (PDATAMODEL_ATM)g_pBEManager->hATM;
    PCONTEXT_LINK_OBJECT       pAtmCxtLink       = (PCONTEXT_LINK_OBJECT)hInstance;
    PDML_ATM                   p_Atm             = (PDML_ATM)pAtmCxtLink->hContext;

    if ( pAtmCxtLink->bNew )
    {
        /* Set bNew to FALSE to indicate this node is not going to save to SysRegistry */
        pAtmCxtLink->bNew = FALSE;
    }
    else
    {
        returnStatus = DmlDelAtm( NULL, p_Atm );
    }

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        AnscSListPopEntryByLink(&pATM->Q_AtmList, &pAtmCxtLink->Linkage);
        AnscFreeMemory(pAtmCxtLink->hContext);
        AnscFreeMemory(pAtmCxtLink);
    }

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ATMLink_GetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL*                       pBool
            );

    description:

        This function is called to retrieve Boolean parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL*                       pBool
                The buffer of returned boolean value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL ATMLink_GetParamBoolValue (  ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool )
{
    PCONTEXT_LINK_OBJECT       pCxtLink   = (PCONTEXT_LINK_OBJECT) hInsContext;
    PDML_ATM                   p_Atm      = (PDML_ATM) pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Enable") == 0)
    {
        BOOLEAN  bEnable = FALSE;

        if ( ANSC_STATUS_SUCCESS == DmlGetAtmIfEnable( &bEnable ) )
        {
            p_Atm->Enable = bEnable;
            *pBool        = p_Atm->Enable;
        }

        return TRUE;
    }
    if (strcmp(ParamName, "AutoConfig") == 0)
    {
        *pBool = p_Atm->AutoConfig;
        return TRUE;
    }
    if (strcmp(ParamName, "FCSPreserved") == 0)
    {
        *pBool = p_Atm->FCSPreserved;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ATMLink_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL ATMLink_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PCONTEXT_LINK_OBJECT        pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM                    p_Atm         = (PDML_ATM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */

    if (strcmp(ParamName, "Status") == 0)
    {
        if(ANSC_STATUS_SUCCESS == DmlGetAtmIfStatus(NULL, p_Atm)) {
            *puLong = p_Atm->Status;
        }
        return TRUE;
    }
    if (strcmp(ParamName, "LastChange") == 0)
    {
        *puLong = p_Atm->LastChange;
        return TRUE;
    }
    if (strcmp(ParamName, "LinkType") == 0)
    {
        *puLong = p_Atm->LinkType;
        return TRUE;
    }
    if (strcmp(ParamName, "Encapsulation") == 0)
    {
        *puLong = p_Atm->Encapsulation;
        return TRUE;
    }
    if (strcmp(ParamName, "AAL") == 0)
    {
        *puLong = p_Atm->AAL;
        return TRUE;
    }
    
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_GetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pValue,
                ULONG*                      pUlSize
            );

    description:

        This function is called to retrieve string parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pValue,
                The string value buffer;

                ULONG*                      pUlSize
                The buffer of length of string value;
                Usually size of 1023 will be used.
                If it's not big enough, put required size here and return 1;

    return:     0 if succeeded;
                1 if short of buffer size; (*pUlSize = required size)
                -1 if not supported.

**********************************************************************/

ULONG ATMLink_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM                   p_Atm         = (PDML_ATM) pCxtLink->hContext;
    PUCHAR                     pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "Alias") == 0)
    {
        if ( AnscSizeOfString(p_Atm->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, p_Atm->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(p_Atm->Alias)+1;
            return 1;
        }
    }
    if (strcmp(ParamName, "Name") == 0)
    {
        AnscCopyString(pValue, p_Atm->Name);
        return 0;
    }
    if (strcmp(ParamName, "LowerLayers") == 0)
    {
        AnscCopyString(pValue, p_Atm->LowerLayers);
        return 0;
    }
    if (strcmp(ParamName, "DestinationAddress") == 0)
    {
        AnscCopyString(pValue, p_Atm->DestinationAddress);
        return 0;
    }
    if (strcmp(ParamName, "VCSearchList") == 0)
    {
        AnscCopyString(pValue, p_Atm->VCSearchList);
        return 0;
    }    

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ATMLinkStats_GetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG*                      puLong
            );

    description:

        This function is called to retrieve ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG*                      puLong
                The buffer of returned ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL ATMLinkStats_GetParamUlongValue  ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PCONTEXT_LINK_OBJECT  pCxtLink = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM  p_Atm                = (PDML_ATM) pCxtLink->hContext;

    //Get ATM statistics
    DmlGetAtmIfStatistics( NULL, p_Atm );

    /* check the parameter name and return the corresponding value */
    if (strcmp(ParamName, "BytesSent") == 0)
    {
        *puLong = p_Atm->Statistics.BytesSent;
        return TRUE;
    }

    if (strcmp(ParamName, "BytesReceived") == 0)
    {
        *puLong = p_Atm->Statistics.BytesReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "PacketsSent") == 0)
    {
        *puLong = p_Atm->Statistics.PacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "PacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.PacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsSent") == 0)
    {
        *puLong = p_Atm->Statistics.ErrorsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "ErrorsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.ErrorsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsSent") == 0)
    {
        *puLong = p_Atm->Statistics.UnicastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "UnicastPacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.UnicastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsSent") == 0)
    {
        *puLong = p_Atm->Statistics.DiscardPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "DiscardPacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.DiscardPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsSent") == 0)
    {
        *puLong = p_Atm->Statistics.MulticastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "MulticastPacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.MulticastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsSent") == 0)
    {
        *puLong = p_Atm->Statistics.BroadcastPacketsSent;
        return TRUE;
    }

    if (strcmp(ParamName, "BroadcastPacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.BroadcastPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "UnknownProtoPacketsReceived") == 0)
    {
        *puLong = p_Atm->Statistics.UnknownProtoPacketsReceived;
        return TRUE;
    }

    if (strcmp(ParamName, "TransmittedBlocks") == 0)
    {
        *puLong = p_Atm->Statistics.TransmittedBlocks;
        return TRUE;
    }

    if (strcmp(ParamName, "ReceivedBlocks") == 0)
    {
        *puLong = p_Atm->Statistics.ReceivedBlocks;
        return TRUE;
    }

    if (strcmp(ParamName, "CRCErrors") == 0)
    {
        *puLong = p_Atm->Statistics.CRCErrors;
        return TRUE;
    }

    if (strcmp(ParamName, "HECErrors") == 0)
    {
        *puLong = p_Atm->Statistics.HECErrors;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ATMLink_Validate
            (
                ANSC_HANDLE                 hInsContext,
                char*                       pReturnParamName,
                ULONG*                      puLength
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       pReturnParamName,
                The buffer (128 bytes) of parameter name if there's a validation.

                ULONG*                      puLength
                The output length of the param name.

    return:     TRUE if there's no validation.

**********************************************************************/

BOOL ATMLink_Validate ( ANSC_HANDLE hInsContext, char* pReturnParamName, ULONG* puLength )
{
    return TRUE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_Commit
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to finally commit all the update.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/

ULONG ATMLink_Commit ( ANSC_HANDLE hInsContext )
{
    ANSC_STATUS                returnStatus  = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM                   p_Atm         = (PDML_ATM) pCxtLink->hContext;
    PDATAMODEL_ATM             pATM          = (PDATAMODEL_ATM) g_pBEManager->hATM;

    if ( pCxtLink->bNew )
    {
        returnStatus = DmlAddAtm(NULL, p_Atm );
        if ( returnStatus == ANSC_STATUS_SUCCESS)
        {
            pCxtLink->bNew = FALSE;
        }
    }
    else 
    {
        returnStatus = DmlSetAtm( NULL, p_Atm );
        if ( returnStatus != ANSC_STATUS_SUCCESS ) 
        {
            CcspTraceError(("%s %d - Failed to set ATM entry\n",__FUNCTION__,__LINE__));
        }
    }

    return returnStatus;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ATMLink_Rollback
            (
                ANSC_HANDLE                 hInsContext
            );

    description:

        This function is called to roll back the update whenever there's a
        validation found.

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

    return:     The status of the operation.

**********************************************************************/

ULONG ATMLink_Rollback ( ANSC_HANDLE hInsContext )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM                  pATM          = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PCONTEXT_LINK_OBJECT            pCxtLink      = (PCONTEXT_LINK_OBJECT) hInsContext;
    PDML_ATM                        p_Atm         = (PDML_ATM) pCxtLink->hContext;

    if ( p_Atm->Alias )
        AnscCopyString( p_Atm->Alias, p_Atm->Alias );

    if ( !pCxtLink->bNew )
    {
        /* We have nothing to do with this case unless we have one getbyEntry() */
    }
    else
    {
        DML_ATM_INIT(p_Atm);
        _ansc_sprintf(p_Atm->Name, "atm%d", p_Atm->InstanceNumber);
    }

    return returnStatus;
}

BOOL ATMLinkQOS_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PCONTEXT_LINK_OBJECT pCxtLink = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM p_Atm = (PDML_ATM) pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "QoSClass") == 0)
    {
        *puLong = p_Atm->Qos.QoSClass;
        return TRUE;
    }
    if (strcmp(ParamName, "PeakCellRate") == 0)
    {
        *puLong = p_Atm->Qos.PeakCellRate;
        return TRUE;
    }
    if (strcmp(ParamName, "MaximumBurstSize") == 0)
    {
        *puLong = p_Atm->Qos.PeakCellRate;
        return TRUE;
    }
    if (strcmp(ParamName, "SustainableCellRate") == 0)
    {
        *puLong = p_Atm->Qos.SustainableCellRate;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL ATMLinkQOS_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PCONTEXT_LINK_OBJECT pCxtLink = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_ATM p_Atm = (PDML_ATM) pCxtLink->hContext;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[256]  = {0};
    char param_value[256] = {0};

    /* check the parameter name and set the corresponding value */
    if (strcmp(ParamName, "QoSClass") == 0)
    {
        p_Atm->Qos.QoSClass = uValue;
        switch (uValue)
        {
            case UBR:
                _ansc_sprintf(param_value, "UBR" );
                break;
            case CBR:
                _ansc_sprintf(param_value, "CBR" );
                break;
            case GFR:
                _ansc_sprintf(param_value, "GFR" );
                break;
            case VBR_NRT:
                _ansc_sprintf(param_value, "VBR-nrt" );
                break;
            case VBR_RT:
                _ansc_sprintf(param_value, "VBR-rt" );
                break;
	        case UBR_PLUS:
                _ansc_sprintf(param_value, "UBR+" );
                break;
            case ABR:
                _ansc_sprintf(param_value, "ABR" );
                break;
        }
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_CLASS);
        return TRUE;
    }
    if (strcmp(ParamName, "PeakCellRate") == 0)
    {
        p_Atm->Qos.PeakCellRate = uValue;
        _ansc_sprintf(param_value, "%d", p_Atm->Qos.PeakCellRate );
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_PEAKCELLRATE);
        return TRUE;
    }
    if (strcmp(ParamName, "MaximumBurstSize") == 0)
    {
        p_Atm->Qos.PeakCellRate = uValue;
        _ansc_sprintf(param_value, "%d", p_Atm->Qos.PeakCellRate);
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_MAXBURSTSIZE);
        return TRUE;
    }
    if (strcmp(ParamName, "SustainableCellRate") == 0)
    {
        p_Atm->Qos.SustainableCellRate = uValue;
        _ansc_sprintf(param_value, "%d", p_Atm->Qos.SustainableCellRate);
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_CELLRATE);
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL ATMLinkDiagnostics_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PDML_ATM_DIAG pAtmDiag = (PDML_ATM_DIAG) pATM->pATMDiag;

    if (strcmp(ParamName, "DiagnosticsState") == 0)
    {
        *puLong = pAtmDiag->DiagnosticsState;
        return TRUE;
    }
    if (strcmp(ParamName, "NumberOfRepetitions") == 0)
    {
        *puLong = pAtmDiag->NumberOfRepetitions;
        return TRUE;
    }
    if (strcmp(ParamName, "Timeout") == 0)
    {
        *puLong = pAtmDiag->Timeout;
        return TRUE;
    }
    if (strcmp(ParamName, "SuccessCount") == 0)
    {
        *puLong = pAtmDiag->SuccessCount;
        return TRUE;
    }
    if (strcmp(ParamName, "FailureCount") == 0)
    {
        *puLong = pAtmDiag->FailureCount;
        return TRUE;
    }
    if (strcmp(ParamName, "AverageResponseTime") == 0)
    {
        *puLong = pAtmDiag->AverageResponseTime;
        return TRUE;
    }
    if (strcmp(ParamName, "MinimumResponseTime") == 0)
    {
        *puLong = pAtmDiag->MinimumResponseTime;
        return TRUE;
    }
    if (strcmp(ParamName, "MaximumResponseTime") == 0)
    {
        *puLong = pAtmDiag->MaximumResponseTime;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL ATMLinkDiagnostics_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PDML_ATM_DIAG pAtmDiag = (PDML_ATM_DIAG) pATM->pATMDiag;

    if (strcmp(ParamName, "DiagnosticsState") == 0)
    {
        if (uValue == DIAG_STATE_REQUESTED)
        {
            pAtmDiag->DiagnosticsState = uValue;
            if ( DmlStartAtmLoopbackDiagnostics(pAtmDiag) == ANSC_STATUS_SUCCESS )
            {
                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
        return TRUE;
    }
    if (strcmp(ParamName, "NumberOfRepetitions") == 0)
    {
        pAtmDiag->NumberOfRepetitions = uValue;
        return TRUE;
    }
    if (strcmp(ParamName, "Timeout") == 0)
    {
        pAtmDiag->Timeout = uValue;
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL ATMLinkDiagnostics_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pString )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PDML_ATM_DIAG pAtmDiag = (PDML_ATM_DIAG) pATM->pATMDiag;

    if (strcmp(ParamName, "Interface") == 0)
    {
        AnscCopyString(pAtmDiag->Interface, pString);
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

ULONG ATMLinkDiagnostics_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize )
{
    PDATAMODEL_ATM pATM = (PDATAMODEL_ATM) g_pBEManager->hATM;
    PDML_ATM_DIAG pAtmDiag = (PDML_ATM_DIAG) pATM->pATMDiag;

    if (strcmp(ParamName, "Interface") == 0)
    {
        if ( AnscSizeOfString(pAtmDiag->Interface) < *pUlSize)
        {
            AnscCopyString(pValue, pAtmDiag->Interface);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(pAtmDiag->Interface) + 1;
            return 1;
        }
        return 0;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}
