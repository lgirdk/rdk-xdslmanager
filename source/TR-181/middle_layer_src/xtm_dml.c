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

#include "ansc_platform.h"
#include "xtm_dml.h"
#include "xtm_apis.h"
#include "plugin_main_apis.h"
#include "xtm_internal.h"

#if     CFG_USE_CCSP_SYSLOG
    #include <ccsp_syslog.h>
#endif
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

    Ethernet.XTMTermination.{i}.

    *  PTMLink_GetEntryCount
    *  PTMLink_GetEntry
    *  PTMLink_AddEntry
    *  PTMLink_DelEntry
    *  PTMLink_GetParamBoolValue
    *  PTMLink_GetParamUlongValue
    *  PTMLink_GetParamStringValue
    *  PTMLink_SetParamBoolValue
    *  PTMLink_SetParamUlongValue
    *  PTMLink_SetParamStringValue
    *  PTMLink_Validate
    *  PTMLink_Commit
    *  PTMLink_Rollback

***********************************************************************/

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

ULONG
PTMLink_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDATAMODEL_XTM             pXTM         = (PDATAMODEL_XTM)g_pBEManager->hXTM;

    return AnscSListQueryDepth( &pXTM->Q_XtmList );

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

ANSC_HANDLE
PTMLink_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PDATAMODEL_XTM             pMyObject         = (PDATAMODEL_XTM)g_pBEManager->hXTM;
    PSINGLE_LINK_ENTRY              pSListEntry       = NULL;
    PCONTEXT_LINK_OBJECT       pCxtLink          = NULL;

    pSListEntry       = AnscSListGetEntryByIndex(&pMyObject->Q_XtmList, nIndex);

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

ANSC_HANDLE
PTMLink_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    )
{
    ANSC_STATUS                          returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM                  pXTM              = (PDATAMODEL_XTM)g_pBEManager->hXTM;
    PDML_XTM                  p_Xtm      = NULL;
    PCONTEXT_LINK_OBJECT            pXtmCxtLink  = NULL;
    PSINGLE_LINK_ENTRY                   pSListEntry       = NULL;
    BOOL                                      bridgeMode;

    p_Xtm = (PDML_XTM)AnscAllocateMemory(sizeof(DML_XTM));

    if ( !p_Xtm )
    {
        return NULL;
    }

    pXtmCxtLink = (PCONTEXT_LINK_OBJECT)AnscAllocateMemory(sizeof(CONTEXT_LINK_OBJECT));
    if ( !pXtmCxtLink )
    {
        goto EXIT;
    }

    /* now we have this link content */
    pXtmCxtLink->hContext = (ANSC_HANDLE)p_Xtm;
    pXtmCxtLink->bNew     = TRUE;

    /* Get InstanceNumber and Alias */
    memset( p_Xtm, 0, sizeof( DML_XTM ) );
    XtmGenForTriggerEntry(NULL, p_Xtm);

    pXtmCxtLink->InstanceNumber = p_Xtm->InstanceNumber ;
    *pInsNumber                      = p_Xtm->InstanceNumber ;

    SListPushEntryByInsNum(&pXTM->Q_XtmList, (PCONTEXT_LINK_OBJECT)pXtmCxtLink);
   
    return (ANSC_HANDLE)pXtmCxtLink;

EXIT:
    AnscFreeMemory(p_Xtm);

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

ULONG
PTMLink_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pXTM              = (PDATAMODEL_XTM)g_pBEManager->hXTM;
    PCONTEXT_LINK_OBJECT       pXtmCxtLink  = (PCONTEXT_LINK_OBJECT)hInstance;
    PDML_XTM             p_Xtm      = (PDML_XTM)pXtmCxtLink->hContext;


    if ( pXtmCxtLink->bNew )
    {
        /* Set bNew to FALSE to indicate this node is not going to save to SysRegistry */
        pXtmCxtLink->bNew = FALSE;
    }
    else
    {
        returnStatus = DmlDelXtm( NULL, p_Xtm );
    }

    if ( returnStatus == ANSC_STATUS_SUCCESS )
    {
        AnscSListPopEntryByLink(&pXTM->Q_XtmList, &pXtmCxtLink->Linkage);

        AnscFreeMemory(pXtmCxtLink->hContext);
        AnscFreeMemory(pXtmCxtLink);
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

BOOL
PTMLink_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm  = (PDML_XTM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        BOOLEAN  bEnable = FALSE;

        if ( ANSC_STATUS_SUCCESS == DmlGetXtmIfEnable( &bEnable ) )
        {
            p_Xtm->Enable = bEnable;
            *pBool        = p_Xtm->Enable;
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
BOOL
PTMLink_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PCONTEXT_LINK_OBJECT        pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM              p_Xtm  = (PDML_XTM   )pCxtLink->hContext;

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

BOOL
PTMLink_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCONTEXT_LINK_OBJECT        pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM           p_Xtm  = (PDML_XTM   )pCxtLink->hContext;

    /* check the parameter name and return the corresponding value */

    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        if(ANSC_STATUS_SUCCESS == DmlGetXtmIfStatus(NULL, p_Xtm)) {
            *puLong = p_Xtm->Status;
        }
        return TRUE;
    }
    if( AnscEqualString(ParamName, "LastChange", TRUE))
    {
        *puLong = p_Xtm->LastChange;
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

ULONG
PTMLink_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm      = (PDML_XTM   )pCxtLink->hContext;
    PUCHAR                          pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        if ( AnscSizeOfString(p_Xtm->Alias) < *pUlSize)
        {
            AnscCopyString(pValue, p_Xtm->Alias);
            return 0;
        }
        else
        {
            *pUlSize = AnscSizeOfString(p_Xtm->Alias)+1;
            return 1;
        }
    }
    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        AnscCopyString(pValue, p_Xtm->Name);
        return 0;
    }
    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        AnscCopyString(pValue, p_Xtm->LowerLayers);
        return 0;
    }
    if( AnscEqualString(ParamName, "MACAddress", TRUE))
    {
        AnscCopyString(pValue, p_Xtm->MACAddress);
        return 0;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_SetParamBoolValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                BOOL                        bValue
            );

    description:

        This function is called to set BOOL parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                BOOL                        bValue
                The updated BOOL value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL
PTMLink_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink  = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm  = (PDML_XTM) pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        /* save update to backup */
        p_Xtm->Enable  = bValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_SetParamIntValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                int                         iValue
            );

    description:

        This function is called to set integer parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                int                         iValue
                The updated integer value;

    return:     TRUE if succeeded.

**********************************************************************/
BOOL
PTMLink_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    )
{
    PCONTEXT_LINK_OBJECT pCosaContext = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM p_Xtm = (PDML_XTM) pCosaContext->hContext;

    /* check the parameter name and set the corresponding value */

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_SetParamUlongValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                ULONG                       uValue
            );

    description:

        This function is called to set ULONG parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                ULONG                       uValue
                The updated ULONG value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL
PTMLink_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm  = (PDML_XTM   )pCxtLink->hContext;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "LastChange", TRUE))
    {
        p_Xtm->LastChange = uValue;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        PTMLink_SetParamStringValue
            (
                ANSC_HANDLE                 hInsContext,
                char*                       ParamName,
                char*                       pString
            );

    description:

        This function is called to set string parameter value;

    argument:   ANSC_HANDLE                 hInsContext,
                The instance handle;

                char*                       ParamName,
                The parameter name;

                char*                       pString
                The updated string value;

    return:     TRUE if succeeded.

**********************************************************************/

BOOL
PTMLink_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDATAMODEL_XTM             pXTM          = (PDATAMODEL_XTM      )g_pBEManager->hXTM;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm      = (PDML_XTM   )pCxtLink->hContext;


    /* check the parameter name and set the corresponding value */
   
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        AnscCopyString(p_Xtm->Alias, pString);
        return TRUE;
    }
    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        AnscCopyString(p_Xtm->Name, pString);
        return TRUE;
    }
    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        AnscCopyString(p_Xtm->LowerLayers, pString);
        return TRUE;
    }
    if( AnscEqualString(ParamName, "MACAddress", TRUE))
    {
        AnscCopyString(p_Xtm->MACAddress, pString);
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
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
BOOL
PTMLinkStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PCONTEXT_LINK_OBJECT  pCxtLink = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM  p_Xtm = (PDML_XTM   )pCxtLink->hContext;

    //Get XTM statistics
    DmlGetXtmIfStatistics( NULL, p_Xtm );

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "BytesSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.BytesSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BytesReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.BytesReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.PacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.PacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.ErrorsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.ErrorsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.UnicastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.UnicastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.DiscardPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.DiscardPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.MulticastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.MulticastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsSent", TRUE) )
    {
        *puLong = p_Xtm->Statistics.BroadcastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.BroadcastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnknownProtoPacketsReceived", TRUE) )
    {
        *puLong = p_Xtm->Statistics.UnknownProtoPacketsReceived;
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

BOOL
PTMLink_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
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

ULONG
PTMLink_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM          p_Xtm  = (PDML_XTM   )pCxtLink->hContext;
    PDATAMODEL_XTM             pXTM          = (PDATAMODEL_XTM      )g_pBEManager->hXTM;

    if ( pCxtLink->bNew )
    {
        returnStatus = DmlAddXtm(NULL, p_Xtm );

        if ( returnStatus == ANSC_STATUS_SUCCESS)
        {
            pCxtLink->bNew = FALSE;
        }
    }
    else 
    {
        returnStatus = DmlSetXtm( NULL, p_Xtm );
    
        if ( returnStatus != ANSC_STATUS_SUCCESS ) 
        {
            CcspTraceError(("%s %d - Failed to set XTM entry\n",__FUNCTION__,__LINE__));
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

ULONG
PTMLink_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    ANSC_STATUS                     returnStatus  = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pXTM          = (PDATAMODEL_XTM      )g_pBEManager->hXTM;
    PCONTEXT_LINK_OBJECT       pCxtLink      = (PCONTEXT_LINK_OBJECT)hInsContext;
    PDML_XTM             p_Xtm  = (PDML_XTM   )pCxtLink->hContext;

    if ( p_Xtm->Alias )
        AnscCopyString( p_Xtm->Alias, p_Xtm->Alias );

    if ( !pCxtLink->bNew )
    {
        /* We have nothing to do with this case unless we have one getbyEntry() */
    }
    else
    {
        DML_XTM_INIT(p_Xtm);
        _ansc_sprintf(p_Xtm->Name, "xtm%d", p_Xtm->InstanceNumber);
    }

    return returnStatus;
}
