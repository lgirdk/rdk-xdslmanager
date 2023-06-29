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
    return pPTM->ulPtmLinkNumberOfEntries;
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

    if ( pMyObject && pInsNumber && pMyObject->PtmLink && nIndex < pMyObject->ulPtmLinkNumberOfEntries )
    {
        *pInsNumber = pMyObject->PtmLink[nIndex].InstanceNumber;
         return pMyObject->PtmLink + nIndex;
    }

    return NULL;
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
    PDML_PTM             p_Ptm  = (PDML_PTM   )hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        *pBool        = p_Ptm->Enable;
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
    PDML_PTM              p_Ptm  = (PDML_PTM   )hInsContext;

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
    PDML_PTM           p_Ptm  = (PDML_PTM   )hInsContext;

    /* check the parameter name and return the corresponding value */

    if( AnscEqualString(ParamName, "Status", TRUE))
    {
        if(ANSC_STATUS_SUCCESS == DmlGetPtmIfStatus(NULL, p_Ptm)) {
            *puLong = p_Ptm->Status;
        }
        return TRUE;
    }
    if( AnscEqualString(ParamName, "LastChange", TRUE))
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
    PDML_PTM             p_Ptm      = (PDML_PTM   )hInsContext;
    PUCHAR                          pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
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
    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        AnscCopyString(pValue, p_Ptm->Name);
        return 0;
    }
    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        AnscCopyString(pValue, p_Ptm->LowerLayers);
        return 0;
    }
    if( AnscEqualString(ParamName, "MACAddress", TRUE))
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
    PDML_PTM  p_Ptm = (PDML_PTM   )hInsContext;

    //Get PTM statistics
    DmlGetPtmIfStatistics( NULL, p_Ptm );

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "BytesSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.BytesSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BytesReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.BytesReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.PacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.PacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.ErrorsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.ErrorsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.UnicastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.UnicastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.DiscardPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.DiscardPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.MulticastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.MulticastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsSent", TRUE) )
    {
        *puLong = p_Ptm->Statistics.BroadcastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsReceived", TRUE) )
    {
        *puLong = p_Ptm->Statistics.BroadcastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnknownProtoPacketsReceived", TRUE) )
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
    PDML_PTM          p_Ptm  = (PDML_PTM)hInsContext;

    return DmlSetPtm( NULL, p_Ptm );

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

    return returnStatus;
}
/***********************************************************************

 APIs for Object:

    Device.ATM.Link.{i}.

    *  ATMLink_Synchronize
    *  ATMLink_IsUpdated
    *  ATMLink_GetEntryCount
    *  ATMLink_GetEntry
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
    return pATM->ulAtmLinkNumberOfEntries;
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

    if ( pMyObject && pInsNumber && pMyObject->AtmLink && nIndex < pMyObject->ulAtmLinkNumberOfEntries )
    {
        *pInsNumber = pMyObject->AtmLink[nIndex].InstanceNumber;
        return pMyObject->AtmLink + nIndex;
    }

    return NULL;
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
    PDML_ATM                   p_Atm      = (PDML_ATM) hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        *pBool        = p_Atm->Enable;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "AutoConfig", TRUE))
    {
        *pBool = p_Atm->AutoConfig;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "FCSPreserved", TRUE))
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
    PDML_ATM                    p_Atm         = (PDML_ATM   )hInsContext;

    /* check the parameter name and return the corresponding value */

    if( AnscEqualString(ParamName, "Status", TRUE))
    {
	*puLong = p_Atm->Status;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "LastChange", TRUE))
    {
        *puLong = p_Atm->LastChange;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "LinkType", TRUE))
    {
        *puLong = p_Atm->LinkType;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "Encapsulation", TRUE))
    {
        *puLong = p_Atm->Encapsulation;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "AAL", TRUE))
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
    PDML_ATM                   p_Atm         = (PDML_ATM) hInsContext;
    PUCHAR                     pString       = NULL;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
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
    if( AnscEqualString(ParamName, "Name", TRUE))
    {
        AnscCopyString(pValue, p_Atm->Name);
        return 0;
    }
    if( AnscEqualString(ParamName, "LowerLayers", TRUE))
    {
        AnscCopyString(pValue, p_Atm->LowerLayers);
        return 0;
    }
    if( AnscEqualString(ParamName, "DestinationAddress", TRUE))
    {
        AnscCopyString(pValue, p_Atm->DestinationAddress);
        return 0;
    }
    if( AnscEqualString(ParamName, "VCSearchList", TRUE))
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
    PDML_ATM  p_Atm                = (PDML_ATM) hInsContext;

    //Get ATM statistics
    DmlGetAtmIfStatistics( NULL, p_Atm );

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "BytesSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.BytesSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BytesReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.BytesReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.PacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.PacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.ErrorsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.ErrorsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.UnicastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnicastPacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.UnicastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.DiscardPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.DiscardPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.MulticastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "MulticastPacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.MulticastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsSent", TRUE) )
    {
        *puLong = p_Atm->Statistics.BroadcastPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BroadcastPacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.BroadcastPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UnknownProtoPacketsReceived", TRUE) )
    {
        *puLong = p_Atm->Statistics.UnknownProtoPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TransmittedBlocks", TRUE) )
    {
        *puLong = p_Atm->Statistics.TransmittedBlocks;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ReceivedBlocks", TRUE) )
    {
        *puLong = p_Atm->Statistics.ReceivedBlocks;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CRCErrors", TRUE) )
    {
        *puLong = p_Atm->Statistics.CRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HECErrors", TRUE) )
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
    PDML_ATM                   p_Atm         = (PDML_ATM)hInsContext;

    return DmlSetAtm( NULL, p_Atm );
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

    return returnStatus;
}

BOOL ATMLinkQOS_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong )
{
    PDML_ATM p_Atm = (PDML_ATM) hInsContext;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "QoSClass", TRUE))
    {
        *puLong = p_Atm->Qos.QoSClass;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "PeakCellRate", TRUE))
    {
        *puLong = p_Atm->Qos.PeakCellRate;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "MaximumBurstSize", TRUE))
    {
        *puLong = p_Atm->Qos.PeakCellRate;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "SustainableCellRate", TRUE))
    {
        *puLong = p_Atm->Qos.SustainableCellRate;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

BOOL ATMLinkQOS_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue )
{
    PDML_ATM p_Atm = (PDML_ATM) hInsContext;
    int retPsmSet = CCSP_SUCCESS;
    char param_name[256]  = {0};
    char param_value[256] = {0};

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "QoSClass", TRUE))
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
    if( AnscEqualString(ParamName, "PeakCellRate", TRUE))
    {
        p_Atm->Qos.PeakCellRate = uValue;
        _ansc_sprintf(param_value, "%d", p_Atm->Qos.PeakCellRate );
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_PEAKCELLRATE);
        return TRUE;
    }
    if( AnscEqualString(ParamName, "MaximumBurstSize", TRUE))
    {
        p_Atm->Qos.PeakCellRate = uValue;
        _ansc_sprintf(param_value, "%d", p_Atm->Qos.PeakCellRate);
        _PSM_WRITE_PARAM(PSM_ADSL_QOS_MAXBURSTSIZE);
        return TRUE;
    }
    if( AnscEqualString(ParamName, "SustainableCellRate", TRUE))
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

    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE))
    {
        *puLong = pAtmDiag->DiagnosticsState;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "NumberOfRepetitions", TRUE))
    {
        *puLong = pAtmDiag->NumberOfRepetitions;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "Timeout", TRUE))
    {
        *puLong = pAtmDiag->Timeout;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "SuccessCount", TRUE))
    {
        *puLong = pAtmDiag->SuccessCount;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "FailureCount", TRUE))
    {
        *puLong = pAtmDiag->FailureCount;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "AverageResponseTime", TRUE))
    {
        *puLong = pAtmDiag->AverageResponseTime;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "MinimumResponseTime", TRUE))
    {
        *puLong = pAtmDiag->MinimumResponseTime;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "MaximumResponseTime", TRUE))
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

    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE))
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
    if( AnscEqualString(ParamName, "NumberOfRepetitions", TRUE))
    {
        pAtmDiag->NumberOfRepetitions = uValue;
        return TRUE;
    }
    if( AnscEqualString(ParamName, "Timeout", TRUE))
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

    if( AnscEqualString(ParamName, "Interface", TRUE))
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

    if( AnscEqualString(ParamName, "Interface", TRUE))
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
