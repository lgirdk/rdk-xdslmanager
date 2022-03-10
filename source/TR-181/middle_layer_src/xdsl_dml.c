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

    module: xdsl_dml.c

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

#include "ansc_platform.h"
#include "plugin_main_apis.h"
#include "xdsl_apis.h"
#include "xdsl_dml.h"
#include "xdsl_internal.h"

char * XdslReportStatusEnable = "eRT.com.cisco.spvtg.ccsp.xdslmanager.Enabled"; 
char * XdslReportStatusDfltReportingPeriod = "eRT.com.cisco.spvtg.ccsp.xdslmanager.Default.ReportingPeriod"; 
char * XdslReportStatusReportingPeriod = "eRT.com.cisco.spvtg.ccsp.xdslmanager.ReportingPeriod"; 

extern ANSC_HANDLE                   bus_handle;
extern char                          g_Subsystem[32];

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}

    *  Line_Synchronize
    *  Line_IsUpdated
    *  Line_GetEntryCount
    *  Line_GetEntry
    *  Line_GetParamBoolValue
    *  Line_GetParamUlongValue
    *  Line_GetParamStringValue
    *  Line_SetParamBoolValue
    *  Line_SetParamStringValue
    *  Line_Validate
    *  Line_Commit
    *  Line_Rollback

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Line_IsUpdated
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
Line_IsUpdated
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
        Line_Synchronize
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
Line_Synchronize
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
        Line_GetEntryCount
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
Line_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDATAMODEL_XDSL   pMyObject   = (PDATAMODEL_XDSL)g_pBEManager->hDSL;

    return pMyObject->ulTotalNoofDSLLines;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ANSC_HANDLE
        Line_GetEntry
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
Line_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{
    PDATAMODEL_XDSL    pMyObject  = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
	
    if ( ( pMyObject->pXDSLLine  ) && ( nIndex < pMyObject->ulTotalNoofDSLLines ) )
    {
    	PDML_XDSL_LINE      pXDSLLine = NULL;
		
        pXDSLLine = &pMyObject->pXDSLLine[nIndex];

        pthread_mutex_lock(&pXDSLLine->mDataMutex);
        *pInsNumber = pXDSLLine->ulInstanceNumber;

        //Sync with current information
#ifdef _SR300_PRODUCT_REQ_
        DmlXdslGetLineCfg( (nIndex +1), pXDSLLine );
#else
        DmlXdslGetLineCfg( nIndex, pXDSLLine );
#endif //_SR300_PRODUCT_REQ_
	pthread_mutex_unlock(&pXDSLLine->mDataMutex);

        return pXDSLLine;
    }
	
    return NULL; /* return the invlalid handle */
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Line_GetParamBoolValue
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
Line_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    BOOL ret = FALSE;
    
    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE) )
    {	
    	*pBool = pXDSLLine->Enable;
		
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "EnableDataGathering", TRUE) )
    {	
    	*pBool = pXDSLLine->EnableDataGathering;
			
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "Upstream", TRUE) )
    {
        *pBool = pXDSLLine->Upstream;

        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        Line_GetParamStringValue
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
Line_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    ULONG ret = -1;
    
    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->Alias ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->Alias );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->Alias );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "Name", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->Name ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->Name );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->Name );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "LowerLayers", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->LowerLayers ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->LowerLayers );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->LowerLayers );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "StandardsSupported", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->StandardsSupported ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->StandardsSupported );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->StandardsSupported );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "StandardUsed", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->StandardUsed ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->StandardUsed );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->StandardUsed );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "AllowedProfiles", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->AllowedProfiles ) - 1 ) < *pUlSize )
       {   
           AnscCopyString( pValue, pXDSLLine->AllowedProfiles );
           ret = 0;
       }
       else
       {   
           *pUlSize = sizeof( pXDSLLine->AllowedProfiles );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "CurrentProfile", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->CurrentProfile ) - 1 ) < *pUlSize )
       {   
           AnscCopyString( pValue, pXDSLLine->CurrentProfile );
           ret = 0;
       }
       else
       {   
           *pUlSize = sizeof( pXDSLLine->CurrentProfile );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "SNRMpbus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->SNRMpbus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->SNRMpbus );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->SNRMpbus );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "SNRMpbds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->SNRMpbds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->SNRMpbds );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->SNRMpbds );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "FirmwareVersion", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->FirmwareVersion ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->FirmwareVersion );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->FirmwareVersion );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTURVendor", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTURVendor ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->XTURVendor );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->XTURVendor );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTUCVendor", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTUCVendor ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->XTUCVendor );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->XTUCVendor );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTURCountry", TRUE) )
    {  
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTURCountry ) - 1 ) < *pUlSize )
       {   
           AnscCopyString( pValue, pXDSLLine->XTURCountry );
           ret = 0;
       }
       else
       {   
           *pUlSize = sizeof( pXDSLLine->XTURCountry );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTUCCountry", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTUCCountry ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->XTUCCountry );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->XTUCCountry );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "UPBOKLEPb", TRUE) )
    {
       /* collect value */
     if ( ( sizeof( pXDSLLine->UPBOKLEPb ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->UPBOKLEPb );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->UPBOKLEPb );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "UPBOKLERPb", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->UPBOKLERPb ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->UPBOKLERPb );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->UPBOKLERPb );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTSE", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTSE ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->XTSE );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->XTSE );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "XTSUsed", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->XTSUsed ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->XTSUsed );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->XTSUsed );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "MREFPSDds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->MREFPSDds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->MREFPSDds );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->MREFPSDds );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "MREFPSDus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->MREFPSDus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->MREFPSDus );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->MREFPSDus );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "VirtualNoisePSDds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->VirtualNoisePSDds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->VirtualNoisePSDds );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->VirtualNoisePSDds );
           ret = 1;
       }
    }
    else if( AnscEqualString(ParamName, "VirtualNoisePSDus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pXDSLLine->VirtualNoisePSDus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pXDSLLine->VirtualNoisePSDus );
           ret = 0;
       }
       else
       {
           *pUlSize = sizeof( pXDSLLine->VirtualNoisePSDus );
           ret = 1;
       }
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Line_SetParamBoolValue
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
Line_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
    	if( bValue != pXDSLLine->Enable )
	{
	    pXDSLLine->Enable  = bValue;
	    //Process DSL enable set
	    DmlXdslLineSetEnable( ( pXDSLLine->ulInstanceNumber - 1 ), pXDSLLine->Enable );
    	}

        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "EnableDataGathering", TRUE))
    {
    	if( bValue == pXDSLLine->EnableDataGathering )
    	{
	    pXDSLLine->EnableDataGathering = bValue;
            //Process Datagather set
	    DmlXdslLineSetDataGatheringEnable( ( pXDSLLine->ulInstanceNumber - 1 ), pXDSLLine->EnableDataGathering );
    	}

        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "Upstream", TRUE))
    {
        if( bValue != pXDSLLine->Upstream )
        {
	    pXDSLLine->Upstream = bValue;
        }

        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Line_SetParamStringValue
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
Line_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pXDSLLine->Alias, pString );
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        Line_GetParamUlongValue
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
Line_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    BOOL ret = FALSE;
    
    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE) )
    {
    	*puLong	= pXDSLLine->Status;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LastChange", TRUE) )
    {
        *puLong = pXDSLLine->LastChange;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LinkStatus", TRUE) )
    {
    	*puLong	= pXDSLLine->LinkStatus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "UpstreamMaxBitRate", TRUE) )
    {
    	*puLong	= pXDSLLine->UpstreamMaxBitRate;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DownstreamMaxBitRate", TRUE) )
    {
    	*puLong	= pXDSLLine->DownstreamMaxBitRate;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LineEncoding", TRUE) )
    {
        *puLong = pXDSLLine->LineEncoding;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "INMIATOds", TRUE) )
    {
        *puLong = pXDSLLine->INMIATOds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "INMIATSds", TRUE) )
    {
        *puLong = pXDSLLine->INMIATSds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "INMCCds", TRUE) )
    {
        *puLong = pXDSLLine->INMCCds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "PowerManagementState", TRUE) )
    {
        *puLong = pXDSLLine->PowerManagementState;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTSNRMODEds", TRUE) )
    {
        *puLong = pXDSLLine->ACTSNRMODEds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTSNRMODEus", TRUE) )
    {
        *puLong = pXDSLLine->ACTSNRMODEus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTUALCE", TRUE) )
    {
        *puLong = pXDSLLine->ACTUALCE;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "XTURANSIStd", TRUE) )
    {
        *puLong = pXDSLLine->XTURANSIStd;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "XTURANSIRev", TRUE) )
    {
        *puLong = pXDSLLine->XTURANSIRev;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "XTUCANSIStd", TRUE) )
    {
        *puLong = pXDSLLine->XTUCANSIStd;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "XTUCANSIRev", TRUE) )
    {
        *puLong = pXDSLLine->XTUCANSIRev;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SuccessFailureCause", TRUE) )
    {
        *puLong = pXDSLLine->SuccessFailureCause;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "UPBOKLE", TRUE) )
    {
        *puLong = pXDSLLine->UPBOKLE;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "UPBOKLER", TRUE) )
    {
        *puLong = pXDSLLine->UPBOKLER;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTRAMODEds", TRUE) )
    {
        *puLong = pXDSLLine->ACTRAMODEds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTRAMODEus", TRUE) )
    {
        *puLong = pXDSLLine->ACTRAMODEus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTINPROCds", TRUE) )
    {
        *puLong = pXDSLLine->ACTINPROCds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTINPROCus", TRUE) )
    {
        *puLong = pXDSLLine->ACTINPROCus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SNRMROCds", TRUE) )
    {
        *puLong = pXDSLLine->SNRMROCds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SNRMROCus", TRUE) )
    {
        *puLong = pXDSLLine->SNRMROCus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LastStateTransmittedDownstream", TRUE) )
    {
        *puLong = pXDSLLine->LastStateTransmittedDownstream;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LastStateTransmittedUpstream", TRUE) )
    {
        *puLong = pXDSLLine->LastStateTransmittedUpstream;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LIMITMASK", TRUE) )
    {
        *puLong = pXDSLLine->LIMITMASK;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "US0MASK", TRUE) )
    {
        *puLong = pXDSLLine->US0MASK;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTSNRMODEds", TRUE) )
    {
        *puLong = pXDSLLine->ACTSNRMODEds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ACTSNRMODEus", TRUE) )
    {
        *puLong = pXDSLLine->ACTSNRMODEus;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Line_GetParamIntValue
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
Line_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PDML_XDSL_LINE      pXDSLLine = (PDML_XDSL_LINE)hInsContext;
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "UpstreamAttenuation", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->UpstreamAttenuation;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "UpstreamNoiseMargin", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->UpstreamNoiseMargin;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "UpstreamPower", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->UpstreamPower;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "TRELLISds", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->TRELLISds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "TRELLISus", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->TRELLISus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LineNumber", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->LineNumber;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "INMINPEQMODEds", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->INMINPEQMODEds;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DownstreamAttenuation", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->TRELLISus;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DownstreamNoiseMargin", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->DownstreamNoiseMargin;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DownstreamPower", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->DownstreamPower;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "RXTHRSHds", TRUE))
    {
        /* collect value */
        *pInt = pXDSLLine->RXTHRSHds;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Line_Validate
    	(
        	ANSC_HANDLE                 hInsContext,
        	char*                       pReturnParamName,
        	ULONG*                      puLength
    	)
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
Line_Validate
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
        Line_Commit
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
Line_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        Line_Rollback
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
Line_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStats_GetParamUlongValue
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
LineStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE          pXDSLLine       = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS    pXDSLLineStats  = &(pXDSLLine->stLineStats);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "BytesSent", TRUE) )
    {
        *puLong = pXDSLLineStats->BytesSent;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "BytesReceived", TRUE) )
    {
        *puLong = pXDSLLineStats->BytesSent;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "PacketsSent", TRUE) )
    {
        *puLong = pXDSLLineStats->PacketsSent;
        ret = TRUE;
    }    
    else if( AnscEqualString(ParamName, "PacketsReceived", TRUE) )
    {
        *puLong = pXDSLLineStats->PacketsReceived;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ErrorsSent", TRUE) )
    {
        *puLong = pXDSLLineStats->ErrorsSent;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ErrorsReceived", TRUE) )
    {
        *puLong = pXDSLLineStats->ErrorsReceived;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DiscardPacketsSent", TRUE) )
    {
        *puLong = pXDSLLineStats->DiscardPacketsSent;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "DiscardPacketsReceived", TRUE) )
    {
        *puLong = pXDSLLineStats->DiscardPacketsReceived;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "TotalStart", TRUE) )
    {
        *puLong = pXDSLLineStats->TotalStart;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "ShowtimeStart", TRUE) )
    {
        *puLong = pXDSLLineStats->ShowtimeStart;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "LastShowtimeStart", TRUE) )
    {
        *puLong = pXDSLLineStats->LastShowtimeStart;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "QuarterHourStart", TRUE) )
    {
        *puLong = pXDSLLineStats->QuarterHourStart;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "CurrentDayStart", TRUE) )
    {
        *puLong = pXDSLLineStats->CurrentDayStart;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}.Stats.Total.

    *  LineStatsTotal_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStatsTotal_GetParamUlongValue
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
LineStatsTotal_GetParamUlongValue
    (   
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE              pXDSLLine            = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS        pXDSLLineStats       = &(pXDSLLine->stLineStats);
    PDML_XDSL_LINE_STATS_TIME   pXDSLLineStatsTotal  = &(pXDSLLineStats->stTotal);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ErroredSecs", TRUE) )
    {   
        *puLong = pXDSLLineStatsTotal->ErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SeverelyErroredSecs", TRUE) )
    {   
        *puLong = pXDSLLineStatsTotal->SeverelyErroredSecs;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}.Stats.Showtime.

    *  LineStatsShowtime_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStatsShowtime_GetParamUlongValue
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
LineStatsShowtime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE              pXDSLLine               = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS        pXDSLLineStats          = &(pXDSLLine->stLineStats);
    PDML_XDSL_LINE_STATS_TIME   pXDSLLineStatsShowTime  = &(pXDSLLineStats->stShowTime);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsShowTime->ErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SeverelyErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsShowTime->SeverelyErroredSecs;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}.Stats.LastShowtime.

    *  LineStatsLastShowtime_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStatsLastShowtime_GetParamUlongValue
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
LineStatsLastShowtime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE              pXDSLLine                   = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS        pXDSLLineStats              = &(pXDSLLine->stLineStats);
    PDML_XDSL_LINE_STATS_TIME   pXDSLLineStatsLastShowTime  = &(pXDSLLineStats->stLastShowTime);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsLastShowTime->ErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SeverelyErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsLastShowTime->SeverelyErroredSecs;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}.Stats.CurrentDay.

    *  LineStatsCurrentDay_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStatsCurrentDay_GetParamUlongValue
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
LineStatsCurrentDay_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE                    pXDSLLine                 = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS              pXDSLLineStats            = &(pXDSLLine->stLineStats);
    PDML_XDSL_LINE_STATS_CURRENTDAY   pXDSLLineStatsCurrentDay  = &(pXDSLLineStats->stCurrentDay);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->ErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SeverelyErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->SeverelyErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "X_RDK_LinkRetrain", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->X_RDK_LinkRetrain;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "X_RDK_InitErrors", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->X_RDK_InitErrors;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "X_RDK_InitTimeouts", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->X_RDK_InitTimeouts;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "X_RDK_SuccessfulRetrains", TRUE) )
    {
        *puLong = pXDSLLineStatsCurrentDay->X_RDK_SuccessfulRetrains;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}.Stats.QuarterHour.

    *  LineStatsQuarterHour_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        LineStatsQuarterHour_GetParamUlongValue
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
LineStatsQuarterHour_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_LINE                     pXDSLLine                  = (PDML_XDSL_LINE)hInsContext;
    PDML_XDSL_LINE_STATS               pXDSLLineStats             = &(pXDSLLine->stLineStats);
    PDML_XDSL_LINE_STATS_QUARTERHOUR   pXDSLLineStatsQuarterHour  = &(pXDSLLineStats->stQuarterHour);
    BOOL ret = FALSE;

    pthread_mutex_lock(&pXDSLLine->mDataMutex);
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "ErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsQuarterHour->ErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "SeverelyErroredSecs", TRUE) )
    {
        *puLong = pXDSLLineStatsQuarterHour->SeverelyErroredSecs;
        ret = TRUE;
    }
    else if( AnscEqualString(ParamName, "X_RDK_LinkRetrain", TRUE) )
    {
        *puLong = pXDSLLineStatsQuarterHour->X_RDK_LinkRetrain;
        ret = TRUE;
    }
    pthread_mutex_unlock(&pXDSLLine->mDataMutex);

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return ret;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}

    *  Channel_Synchronize
    *  Channel_IsUpdated
    *  Channel_GetEntryCount
    *  Channel_GetEntry
    *  Channel_GetParamBoolValue
    *  Channel_GetParamUlongValue
    *  Channel_GetParamStringValue
    *  Channel_SetParamBoolValue
    *  Channel_SetParamUlongValue
    *  Channel_SetParamStringValue
    *  Channel_Validate
    *  Channel_Commit
    *  Channel_Rollback

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_IsUpdated
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
Channel_IsUpdated
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
        Channel_Synchronize
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
Channel_Synchronize
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
        Channel_GetEntryCount
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
Channel_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDATAMODEL_XDSL   pMyObject   = (PDATAMODEL_XDSL)g_pBEManager->hDSL;

    return pMyObject->ulTotalNoofDSLChannels;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ANSC_HANDLE
        Channel_GetEntry
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
Channel_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    )
{   
    PDATAMODEL_XDSL    pMyObject  = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    
    if ( ( pMyObject->pDSLChannel  ) && ( nIndex < pMyObject->ulTotalNoofDSLChannels ) )
    {   
        PDML_XDSL_CHANNEL      pDSLChannel = NULL;
        
        pDSLChannel = pMyObject->pDSLChannel + nIndex;
        
        pDSLChannel->ulInstanceNumber = nIndex + 1;
        
        *pInsNumber = pDSLChannel->ulInstanceNumber;
        
        //Sync with current information
#ifdef _SR300_PRODUCT_REQ_
        DmlXdslGetChannelCfg( pDSLChannel->LineIndex, (nIndex + 1), pDSLChannel );
#else
        DmlXdslGetChannelCfg( pDSLChannel->LineIndex, nIndex, pDSLChannel );
#endif //_SR300_PRODUCT_REQ_


        return pDSLChannel;
    }

    return NULL; /* return the invlalid handle */
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_GetParamBoolValue
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
Channel_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE) )
    {
        *pBool = pDSLChannel->Enable;

        return TRUE;
    }

    if( AnscEqualString(ParamName, "INPREPORT", TRUE) )
    {
        *pBool = pDSLChannel->INPREPORT;

        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_SetParamBoolValue
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
Channel_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "Enable", TRUE))
    {
        if( bValue == pDSLChannel->Enable )
        {
            return TRUE;    //No need to proceed when same value comes
        }

        pDSLChannel->Enable = bValue;

        //Process DSL channel enable set
        DmlXdslChannelSetEnable( pDSLChannel->LineIndex, ( pDSLChannel->ulInstanceNumber - 1 ), pDSLChannel->Enable );

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_GetParamIntValue
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
Channel_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "INTLVBLOCK", TRUE))
    {
        /* collect value */
        *pInt = pDSLChannel->INTLVBLOCK;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTINP", TRUE))
    {
        /* collect value */
        *pInt = pDSLChannel->ACTINP;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "NFEC", TRUE))
    {
        /* collect value */
        *pInt = pDSLChannel->NFEC;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "RFEC", TRUE))
    {
        /* collect value */
        *pInt = pDSLChannel->RFEC;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LSYMB", TRUE))
    {
        /* collect value */
        *pInt = pDSLChannel->LSYMB;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_GetParamUlongValue
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
Channel_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Status", TRUE) )
    {
        *puLong = pDSLChannel->Status;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LastChange", TRUE) )
    {
        *puLong = pDSLChannel->LastChange;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LPATH", TRUE) )
    {
        *puLong = pDSLChannel->LPATH;
        return TRUE;
    }     

    if( AnscEqualString(ParamName, "INTLVDEPTH", TRUE) )
    {
        *puLong = pDSLChannel->INTLVDEPTH;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ActualInterleavingDelay", TRUE) )
    {
        *puLong = pDSLChannel->ActualInterleavingDelay;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UpstreamCurrRate", TRUE) )
    {
        *puLong = pDSLChannel->UpstreamCurrRate;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DownstreamCurrRate", TRUE) )
    {
        *puLong = pDSLChannel->DownstreamCurrRate;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTNDR", TRUE) )
    {
        *puLong = pDSLChannel->ACTNDR;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTINPREIN", TRUE) )
    {
        *puLong = pDSLChannel->ACTINPREIN;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        Channel_GetParamStringValue
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
Channel_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pDSLChannel->Alias ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pDSLChannel->Alias );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pDSLChannel->Alias );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LowerLayers", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pDSLChannel->LowerLayers ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pDSLChannel->LowerLayers );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pDSLChannel->LowerLayers );
           return 1;
       }
    }
     
    if( AnscEqualString(ParamName, "Name", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pDSLChannel->Name ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pDSLChannel->Name );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pDSLChannel->Name );
           return 1;
       }
    }
 
    if( AnscEqualString(ParamName, "LinkEncapsulationSupported", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pDSLChannel->LinkEncapsulationSupported ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pDSLChannel->LinkEncapsulationSupported );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pDSLChannel->LinkEncapsulationSupported );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LinkEncapsulationUsed", TRUE) )
    {  
       /* collect value */
       if ( ( sizeof( pDSLChannel->LinkEncapsulationUsed ) - 1 ) < *pUlSize )
       {   
           AnscCopyString( pValue, pDSLChannel->LinkEncapsulationUsed );
           return 0;
       }
       else
       {   
           *pUlSize = sizeof( pDSLChannel->LinkEncapsulationUsed );
           return 1;
       }
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_SetParamStringValue
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
Channel_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDML_XDSL_CHANNEL      pDSLChannel = (PDML_XDSL_CHANNEL)hInsContext;

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Alias", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pDSLChannel->Alias, pString );
        return TRUE;
    }
    
    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        Channel_Validate
        (
            ANSC_HANDLE                 hInsContext,
            char*                       pReturnParamName,
            ULONG*                      puLength
        )
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
Channel_Validate
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
        Channel_Commit
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
Channel_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        Channel_Rollback
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
Channel_Rollback
    (
        ANSC_HANDLE                 hInsContext
    )
{
    return 0;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.

    *  ChannelStats_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStats_GetParamUlongValue
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
ChannelStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL          pDSLChannel       = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS    pDSLChannelStats  = &(pDSLChannel->stChannelStats);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "BytesSent", TRUE) )
    {
        *puLong = pDSLChannelStats->BytesSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "BytesReceived", TRUE) )
    {
        *puLong = pDSLChannelStats->BytesSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsSent", TRUE) )
    {
        *puLong = pDSLChannelStats->PacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "PacketsReceived", TRUE) )
    {
        *puLong = pDSLChannelStats->PacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsSent", TRUE) )
    {
        *puLong = pDSLChannelStats->ErrorsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ErrorsReceived", TRUE) )
    {
        *puLong = pDSLChannelStats->ErrorsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsSent", TRUE) )
    {
        *puLong = pDSLChannelStats->DiscardPacketsSent;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "DiscardPacketsReceived", TRUE) )
    {
        *puLong = pDSLChannelStats->DiscardPacketsReceived;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "TotalStart", TRUE) )
    {
        *puLong = pDSLChannelStats->TotalStart;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ShowtimeStart", TRUE) )
    {
        *puLong = pDSLChannelStats->ShowtimeStart;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LastShowtimeStart", TRUE) )
    {
        *puLong = pDSLChannelStats->LastShowtimeStart;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QuarterHourStart", TRUE) )
    {
        *puLong = pDSLChannelStats->QuarterHourStart;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CurrentDayStart", TRUE) )
    {
        *puLong = pDSLChannelStats->CurrentDayStart;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.Total.

    *  ChannelStatsTotal_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStatsTotal_GetParamUlongValue
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
ChannelStatsTotal_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL               pDSLChannel            = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS         pDSLChannelStats       = &(pDSLChannel->stChannelStats);
    PDML_XDSL_CHANNEL_STATS_TIME    pDSLChannelStatsTotal  = &(pDSLChannelStats->stTotal);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "XTURFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTURFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTUCFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTURHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTUCHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTURCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsTotal->XTUCCRCErrors;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.ShowTime.

    *  ChannelStatsShowTime_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStatsShowTime_GetParamUlongValue
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
ChannelStatsShowTime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL              pDSLChannel               = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS        pDSLChannelStats          = &(pDSLChannel->stChannelStats);
    PDML_XDSL_CHANNEL_STATS_TIME   pDSLChannelStatsShowTime  = &(pDSLChannelStats->stShowTime);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "XTURFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTURFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTUCFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTURHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTUCHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTURCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsShowTime->XTUCCRCErrors;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.LastShowTime.

    *  ChannelStatsLastShowTime_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStatsLastShowTime_GetParamUlongValue
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
ChannelStatsLastShowTime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL              pDSLChannel                   = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS        pDSLChannelStats              = &(pDSLChannel->stChannelStats);
    PDML_XDSL_CHANNEL_STATS_TIME   pDSLChannelStatsLastShowTime  = &(pDSLChannelStats->stLastShowTime);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "XTURFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTURFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTUCFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTURHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTUCHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTURCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsLastShowTime->XTUCCRCErrors;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.CurrentDay.

    *  ChannelStatsCurrentDay_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStatsCurrentDay_GetParamUlongValue
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
ChannelStatsCurrentDay_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL                    pDSLChannel                   = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS              pDSLChannelStats              = &(pDSLChannel->stChannelStats);
    PDML_XDSL_CHANNEL_STATS_CURRENTDAY   pDSLChannelStatsCurrentDay    = &(pDSLChannelStats->stCurrentDay);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "XTURFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTURFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCFECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTUCFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTURHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTUCHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTURCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->XTUCCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_LinkRetrain", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->X_RDK_LinkRetrain;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_InitErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->X_RDK_InitErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_InitTimeouts", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->X_RDK_InitTimeouts;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_SeverelyErroredSecs", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->X_RDK_SeverelyErroredSecs;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_ErroredSecs", TRUE) )
    {
        *puLong = pDSLChannelStatsCurrentDay->X_RDK_ErroredSecs;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}.Stats.QuarterHour.

    *  ChannelStatsQuarterHour_GetParamUlongValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ChannelStatsQuarterHour_GetParamUlongValue
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
ChannelStatsQuarterHour_GetParamUlongValue
    (   
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDML_XDSL_CHANNEL                      pDSLChannel                    = (PDML_XDSL_CHANNEL)hInsContext;
    PDML_XDSL_CHANNEL_STATS                pDSLChannelStats               = &(pDSLChannel->stChannelStats);
    PDML_XDSL_CHANNEL_STATS_QUARTERHOUR    pDSLChannelStatsQuarterHour    = &(pDSLChannelStats->stQuarterHour);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "XTURFECErrors", TRUE) )
    {   
        *puLong = pDSLChannelStatsQuarterHour->XTURFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCFECErrors", TRUE) )
    {   
        *puLong = pDSLChannelStatsQuarterHour->XTUCFECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURHECErrors", TRUE) )
    {   
        *puLong = pDSLChannelStatsQuarterHour->XTURHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCHECErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsQuarterHour->XTUCHECErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTURCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsQuarterHour->XTURCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "XTUCCRCErrors", TRUE) )
    {
        *puLong = pDSLChannelStatsQuarterHour->XTUCCRCErrors;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "X_RDK_LinkRetrain", TRUE) )
    {
        *puLong = pDSLChannelStatsQuarterHour->X_RDK_LinkRetrain;
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.ADSLLineTest.

    *  ADSLLineTest_GetParamUlongValue
    *  ADSLLineTest_GetParamStringValue
    *  ADSLLineTest_SetParamStringValue
    *  ADSLLineTest_GetParamIntValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ADSLLineTest_GetParamUlongValue
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
ADSLLineTest_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_ADSL_LINE_TEST    pstADSLLineTest  = (PDML_XDSL_DIAG_ADSL_LINE_TEST)&(pDSLDiag->stDiagADSLLineTest);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE) )
    {
        *puLong = pstADSLLineTest->DiagnosticsState;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLINGds", TRUE) )
    {
        *puLong = pstADSLLineTest->HLINGds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLINGus", TRUE) )
    {
        *puLong = pstADSLLineTest->HLINGus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLOGGds", TRUE) )
    {
        *puLong = pstADSLLineTest->HLOGGds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLOGGus", TRUE) )
    {
        *puLong = pstADSLLineTest->HLOGGus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLOGMTds", TRUE) )
    {
        *puLong = pstADSLLineTest->HLOGMTds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLOGMTus", TRUE) )
    {
        *puLong = pstADSLLineTest->HLOGMTus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QLNGds", TRUE) )
    {
        *puLong = pstADSLLineTest->QLNGds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QLNGus", TRUE) )
    {
        *puLong = pstADSLLineTest->QLNGus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QLNMTds", TRUE) )
    {
        *puLong = pstADSLLineTest->QLNMTds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QLNMTus", TRUE) )
    {
        *puLong = pstADSLLineTest->QLNMTus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SNRGds", TRUE) )
    {
        *puLong = pstADSLLineTest->SNRGds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SNRGus", TRUE) )
    {
        *puLong = pstADSLLineTest->SNRGus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SNRMTds", TRUE) )
    {
        *puLong = pstADSLLineTest->SNRMTds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "SNRMTus", TRUE) )
    {
        *puLong = pstADSLLineTest->SNRMTus;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ADSLLineTest_SetParamUlongValue
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
ADSLLineTest_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_ADSL_LINE_TEST    pstADSLLineTest  = (PDML_XDSL_DIAG_ADSL_LINE_TEST)&(pDSLDiag->stDiagADSLLineTest);

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "DiagnosticsState", TRUE))
    {
        pstADSLLineTest->DiagnosticsState = uValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ADSLLineTest_SetParamStringValue
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
ADSLLineTest_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_ADSL_LINE_TEST    pstADSLLineTest  = (PDML_XDSL_DIAG_ADSL_LINE_TEST)&(pDSLDiag->stDiagADSLLineTest);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Interface", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstADSLLineTest->Interface, pString );
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        ADSLLineTest_GetParamStringValue
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
ADSLLineTest_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_ADSL_LINE_TEST    pstADSLLineTest  = (PDML_XDSL_DIAG_ADSL_LINE_TEST)&(pDSLDiag->stDiagADSLLineTest);

    if( AnscEqualString(ParamName, "Interface", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->Interface ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->Interface );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->Interface );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "HLOGpsds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->HLOGpsds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->HLOGpsds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->HLOGpsds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "HLOGpsus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->HLOGpsus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->HLOGpsus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->HLOGpsus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LATNpbds", TRUE) )
    {
       /* collect value */ 
       if ( ( sizeof( pstADSLLineTest->LATNpbds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->LATNpbds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->LATNpbds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LATNpbus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->LATNpbus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->LATNpbus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->LATNpbus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "SATNds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->SATNds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->SATNds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->SATNds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "SATNus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->SATNus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->SATNus );
           return 0;     
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->SATNus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "HLINpsds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->HLINpsds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->HLINpsds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->HLINpsds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "HLINpsus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->HLINpsus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->HLINpsus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->HLINpsus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "QLNpsds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->QLNpsds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->QLNpsds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->QLNpsds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "QLNpsus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->QLNpsus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->QLNpsus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->QLNpsus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "SNRpsds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->SNRpsds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->SNRpsds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->SNRpsds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "SNRpsus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->SNRpsus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->SNRpsus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->SNRpsus );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "BITSpsds", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->BITSpsds ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->BITSpsds );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->BITSpsds );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "BITSpsus", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstADSLLineTest->BITSpsus ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstADSLLineTest->BITSpsus );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstADSLLineTest->BITSpsus );
           return 1;
       }
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        ADSLLineTest_GetParamIntValue
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
ADSLLineTest_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_ADSL_LINE_TEST    pstADSLLineTest  = (PDML_XDSL_DIAG_ADSL_LINE_TEST)&(pDSLDiag->stDiagADSLLineTest);

    if( AnscEqualString(ParamName, "ACTPSDds", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->ACTPSDds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTPSDus", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->ACTPSDus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTATPds", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->ACTATPds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "ACTATPus", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->ACTATPus;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLINSCds", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->HLINSCds;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "HLINSCus", TRUE))
    {
        /* collect value */
        *pInt = pstADSLLineTest->HLINSCus;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTUER.

    *  SELTUER_GetParamUlongValue
    *  SELTUER_GetParamStringValue
    *  SELTUER_SetParamStringValue
    *  SELTUER_GetParamBoolValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTUER_GetParamUlongValue
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
SELTUER_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTUER           pstSELTUERTest   = (PDML_XDSL_DIAG_SELTUER)&(pDSLDiag->stDiagSELTUER);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE) )
    {
        *puLong = pstSELTUERTest->DiagnosticsState;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UERMaxMeasurementDuration", TRUE) )
    {
        *puLong = pstSELTUERTest->UERMaxMeasurementDuration;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UERScaleFactor", TRUE) )
    {
        *puLong = pstSELTUERTest->UERScaleFactor;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "UERGroupSize", TRUE) )
    {
        *puLong = pstSELTUERTest->UERGroupSize;
        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTUER_SetParamUlongValue
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
SELTUER_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTUER           pstSELTUERTest   = (PDML_XDSL_DIAG_SELTUER)&(pDSLDiag->stDiagSELTUER);

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "DiagnosticsState", TRUE))
    {
        pstSELTUERTest->DiagnosticsState = uValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SELTUER_GetParamStringValue
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
SELTUER_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTUER           pstSELTUERTest   = (PDML_XDSL_DIAG_SELTUER)&(pDSLDiag->stDiagSELTUER);

    if( AnscEqualString(ParamName, "Interface", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTUERTest->Interface ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTUERTest->Interface );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTUERTest->Interface );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "UER", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTUERTest->UER ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTUERTest->UER );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTUERTest->UER );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "UERVar", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTUERTest->UERVar ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTUERTest->UERVar );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTUERTest->UERVar );
           return 1;
       }
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTUER_GetParamBoolValue
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
SELTUER_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTUER           pstSELTUERTest   = (PDML_XDSL_DIAG_SELTUER)&(pDSLDiag->stDiagSELTUER);

    if( AnscEqualString(ParamName, "ExtendedBandwidthOperation", TRUE) )
    {
        *pBool = pstSELTUERTest->ExtendedBandwidthOperation;

        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTUER_SetParamStringValue
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
SELTUER_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTUER           pstSELTUERTest   = (PDML_XDSL_DIAG_SELTUER)&(pDSLDiag->stDiagSELTUER);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Interface", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstSELTUERTest->Interface, pString );
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTQLN.

    *  SELTQLN_GetParamUlongValue
    *  SELTQLN_SetParamUlongValue
    *  SELTQLN_GetParamStringValue
    *  SELTQLN_SetParamStringValue
    *  SELTQLN_GetParamIntValue
    *  SELTQLN_GetParamBoolValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTQLN_GetParamUlongValue
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
SELTQLN_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{   
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTQLN           pstSELTQLNTest   = (PDML_XDSL_DIAG_SELTQLN)&(pDSLDiag->stDiagSELTQLN);
    
    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE) )
    {
        *puLong = pstSELTQLNTest->DiagnosticsState;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "QLNMaxMeasurementDuration", TRUE) )
    {   
        *puLong = pstSELTQLNTest->QLNMaxMeasurementDuration;
        return TRUE;
    }
    
    if( AnscEqualString(ParamName, "QLNGroupSize", TRUE) )
    {   
        *puLong = pstSELTQLNTest->QLNGroupSize;
        return TRUE;
    }
    
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTQLN_SetParamUlongValue
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
SELTQLN_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTQLN           pstSELTQLNTest   = (PDML_XDSL_DIAG_SELTQLN)&(pDSLDiag->stDiagSELTQLN);

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "DiagnosticsState", TRUE))
    {
        pstSELTQLNTest->DiagnosticsState = uValue;

        return TRUE;
    }

    if(AnscEqualString(ParamName, "QLNMaxMeasurementDuration", TRUE))
    {
        pstSELTQLNTest->QLNMaxMeasurementDuration = uValue;

        return TRUE;
    }
   
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTQLN_GetParamBoolValue
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
SELTQLN_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTQLN           pstSELTQLNTest   = (PDML_XDSL_DIAG_SELTQLN)&(pDSLDiag->stDiagSELTQLN);

    if( AnscEqualString(ParamName, "ExtendedBandwidthOperation", TRUE) )
    {
        *pBool = pstSELTQLNTest->ExtendedBandwidthOperation;

        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SELTQLN_GetParamStringValue
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
SELTQLN_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTQLN           pstSELTQLNTest   = (PDML_XDSL_DIAG_SELTQLN)&(pDSLDiag->stDiagSELTQLN);

    if( AnscEqualString(ParamName, "Interface", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTQLNTest->Interface ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTQLNTest->Interface );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTQLNTest->Interface );
           return 1;
       }
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTQLN_SetParamStringValue
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
SELTQLN_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTQLN           pstSELTQLNTest   = (PDML_XDSL_DIAG_SELTQLN)&(pDSLDiag->stDiagSELTQLN);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Interface", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstSELTQLNTest->Interface, pString );
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTP.

    *  SELTP_GetParamUlongValue
    *  SELTP_SetParamUlongValue
    *  SELTP_GetParamStringValue
    *  SELTP_SetParamStringValue
    *  SELTP_GetParamBoolValue
    *  SELTP_SetParamBoolValue

***********************************************************************/
/**********************************************************************

    caller:     owner of this object

    prototype:

        ULONG
        SELTP_GetParamStringValue
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
SELTP_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{   
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);

    if( AnscEqualString(ParamName, "Interface", TRUE) )
    {  
       /* collect value */
       if ( ( sizeof( pstSELTPTest->Interface ) - 1 ) < *pUlSize )
       {   
           AnscCopyString( pValue, pstSELTPTest->Interface );
           return 0;
       }
       else
       {   
           *pUlSize = sizeof( pstSELTPTest->Interface );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "CapacitySignalPSD", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTPTest->CapacitySignalPSD ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTPTest->CapacitySignalPSD );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTPTest->CapacitySignalPSD );
           return 1;
       }
    }
   
    if( AnscEqualString(ParamName, "CapacityNoisePSD", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTPTest->CapacityNoisePSD ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTPTest->CapacityNoisePSD );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTPTest->CapacityNoisePSD );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LoopTermination", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTPTest->LoopTermination ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTPTest->LoopTermination );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTPTest->LoopTermination );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "LoopTopology", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTPTest->LoopTopology ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTPTest->LoopTopology );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTPTest->LoopTopology );
           return 1;
       }
    }

    if( AnscEqualString(ParamName, "AttenuationCharacteristics", TRUE) )
    {
       /* collect value */
       if ( ( sizeof( pstSELTPTest->AttenuationCharacteristics ) - 1 ) < *pUlSize )
       {
           AnscCopyString( pValue, pstSELTPTest->AttenuationCharacteristics );
           return 0;
       }
       else
       {
           *pUlSize = sizeof( pstSELTPTest->AttenuationCharacteristics );
           return 1;
       }
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return -1;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTP_SetParamStringValue
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
SELTP_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);

    /* check the parameter name and return the corresponding value */
    if( AnscEqualString(ParamName, "Interface", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstSELTPTest->Interface, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CapacitySignalPSD", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstSELTPTest->CapacitySignalPSD, pString );
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CapacityNoisePSD", TRUE))
    {
        /* save update to backup */
        AnscCopyString( pstSELTPTest->CapacityNoisePSD, pString );
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTP_GetParamBoolValue
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
SELTP_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{   
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);

    if( AnscEqualString(ParamName, "CapacityEstimateEnabling", TRUE) )
    {
        *pBool = pstSELTPTest->CapacityEstimateEnabling;
    
        return TRUE;
    }
   
    if( AnscEqualString(ParamName, "MissingFilter", TRUE) )
    {
        *pBool = pstSELTPTest->MissingFilter;
    
        return TRUE;
    }

    /* AnscTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTP_SetParamBoolValue
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
SELTP_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "CapacityEstimateEnabling", TRUE))
    {
        pstSELTPTest->CapacityEstimateEnabling  = bValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTP_GetParamUlongValue
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
SELTP_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);

    /* check the parameter name and set the corresponding value */
    if( AnscEqualString(ParamName, "DiagnosticsState", TRUE) )
    {
        *puLong = pstSELTPTest->DiagnosticsState;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "CapacityTargetMargin", TRUE) )
    {
        *puLong = pstSELTPTest->CapacityTargetMargin;
        return TRUE;
    }

    if( AnscEqualString(ParamName, "LoopLength", TRUE) )
    {
        *puLong = pstSELTPTest->LoopLength;
        return TRUE;
    }
    
    if( AnscEqualString(ParamName, "CapacityEstimate", TRUE) )
    {   
        *puLong = pstSELTPTest->CapacityEstimate;
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        SELTP_SetParamUlongValue
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
SELTP_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_DIAGNOSTICS_FULL       pDSLDiag         = (PDML_XDSL_DIAGNOSTICS_FULL)pMyObject->pDSLDiag;
    PDML_XDSL_DIAG_SELTP             pstSELTPTest     = (PDML_XDSL_DIAG_SELTP)&(pDSLDiag->stDiagSELTP);
  
    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "DiagnosticsState", TRUE))
    {
        pstSELTPTest->DiagnosticsState = uValue;

        return TRUE;
    }

    if(AnscEqualString(ParamName, "CapacityTargetMargin", TRUE))
    {
        pstSELTPTest->CapacityTargetMargin = uValue;

        return TRUE;
    }

    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}


/***********************************************************************

 APIs for Object:

    Device.DSL.X_RDK_Report.DSL

    *  X_RDK_Report_DSL_GetParamUlongValue
    *  X_RDK_Report_DSL_SetParamUlongValue
    *  X_RDK_Report_DSL_GetParamStringValue
    *  X_RDK_Report_DSL_GetParamBoolValue
    *  X_RDK_Report_DSL_SetParamBoolValue

***********************************************************************/


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        X_RDK_Report_DSL_GetParamUlongValue
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
X_RDK_Report_DSL_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "ReportingPeriod", TRUE) && (puLong != NULL))
    {
        *puLong = pXdslReport->ReportingPeriod;
        return TRUE;
    }

    return FALSE;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        X_RDK_Report_DSL_SetParamUlongValue
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
X_RDK_Report_DSL_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{

    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "ReportingPeriod", TRUE))
    {
        pXdslReport->bReportingPeriodChanged = TRUE;
        pXdslReport->ReportingPeriod = uValue;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        ULONG
        X_RDK_Report_DSL_GetParamStringValue
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
X_RDK_Report_DSL_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL 			 pXdslReport 	  = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "Schema", TRUE))
    {
        /* collect value */
        int bufsize = XdslReportGetSchemaBufferSize();
        if(!bufsize)
        {
            char result[1024] = "Schema Buffer is empty";
            AnscCopyString(pValue, (char*)&result);
            return FALSE;
        }
        else
        {
            if (bufsize < *pUlSize)
            {
                AnscCopyString(pValue, XdslReportGetSchemaBuffer());
                return FALSE;
            }
            else
            {
                *pUlSize = bufsize + 1;
                return TRUE;
            }
        }
    }

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "SchemaID", TRUE))
    {
        /* collect value */
        int bufsize = XdslReportGetSchemaIDBufferSize();
        if(!bufsize)
        {
            char result[1024] = "Schema Buffer is empty";
            AnscCopyString(pValue, (char*)&result);
            return FALSE;
        }
        else
        {
            if (bufsize < *pUlSize)
            {
                AnscCopyString(pValue, XdslReportGetSchemaIDBuffer());
                return FALSE;
            }
            else
            {
                *pUlSize = bufsize + 1;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        X_RDK_Report_DSL_GetParamBoolValue
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
X_RDK_Report_DSL_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    )
{

    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    if(AnscEqualString(ParamName, "Enabled", TRUE) && (pBool != NULL))
    {
        *pBool = pXdslReport->Enabled;
        return TRUE;
    }

    return FALSE;
}

/**********************************************************************  

    caller:     owner of this object 

    prototype: 

        BOOL
        X_RDK_Report_DSL_SetParamBoolValue
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
X_RDK_Report_DSL_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    )
{

    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL 			 pXdslReport 	  = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    if(AnscEqualString(ParamName, "Enabled", TRUE))
    {
        pXdslReport->Enabled = bValue;
        pXdslReport->bEnableChanged = TRUE;
        return TRUE;
    }

    return FALSE;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        X_RDK_Report_DSL_Validate
        (
            ANSC_HANDLE                 hInsContext,
            char*                       pReturnParamName,
            ULONG*                      puLength
        )
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
X_RDK_Report_DSL_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    if (pXdslReport->bReportingPeriodChanged)
    {
            BOOL validated = XdslReportValidateReportingPeriod(pXdslReport->ReportingPeriod);    
            if(!validated)
            {
                AnscCopyString(pReturnParamName, "ReportingPeriod");
                *puLength = AnscSizeOfString("ReportingPeriod");
                return FALSE;
            }
    }
    return TRUE;
}


/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        X_RDK_Report_DSL_Commit
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
X_RDK_Report_DSL_Commit
    (
        ANSC_HANDLE                 hInsContext
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 
    ULONG psmValue = 0;
    
    if (pXdslReport->bEnableChanged)
    {
        XdslReportSetStatus(pXdslReport->Enabled);
        pXdslReport->bEnableChanged = false;
        psmValue = pXdslReport->Enabled;
        SetNVRamULONGConfiguration (XdslReportStatusEnable, psmValue); 
    }
    
    if (pXdslReport->bReportingPeriodChanged)
    {
        XdslReportSetReportingPeriod(pXdslReport->ReportingPeriod);
        pXdslReport->bReportingPeriodChanged = false;
        psmValue = pXdslReport->ReportingPeriod;
        SetNVRamULONGConfiguration (XdslReportStatusReportingPeriod, psmValue); 
    }
    return 0;
}


/**********************************************************************
    caller:     owner of this object

    prototype:
        ULONG
        X_RDK_Report_DSL_Rollback
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
X_RDK_Report_DSL_Rollback
(
    ANSC_HANDLE                 hInsContext
)
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 

    if (pXdslReport->bEnableChanged)
    {
        pXdslReport->Enabled = XdslReportGetStatus();
        pXdslReport->bEnableChanged = false;
    }
    
    if (pXdslReport->bReportingPeriodChanged)
    {
        pXdslReport->ReportingPeriod = XdslReportGetReportingPeriod();
        pXdslReport->bReportingPeriodChanged = false;
    }

    return 0;
}

/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
        X_RDK_Report_DSL_Default_GetParamUlongValue
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
X_RDK_Report_DSL_Default_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    )
{

    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 
    PDML_X_RDK_REPORT_DSL_DEFAULT    pXdslReportDflt  = (PDML_X_RDK_REPORT_DSL_DEFAULT)pXdslReport->pDSLDefaultReport;

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "OverrideTTL", TRUE) && (puLong != NULL))
    {
        pXdslReportDflt->OverrideTTL = XdslReportGetDefaultOverrideTTL();
        *puLong = pXdslReportDflt->OverrideTTL;
        return TRUE;
    }

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "ReportingPeriod", TRUE) && (puLong != NULL))
    {
        *puLong = pXdslReportDflt->ReportingPeriod;
        return TRUE;
    }

    return FALSE;
}


/**********************************************************************

    caller:     owner of this object

    prototype:

        BOOL
    X_RDK_Report_DSL_Default_SetParamUlongValue
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
X_RDK_Report_DSL_Default_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    )
{

    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_X_RDK_REPORT_DSL            pXdslReport      = (PDML_X_RDK_REPORT_DSL)pMyObject->pDSLReport; 
    PDML_X_RDK_REPORT_DSL_DEFAULT    pXdslReportDflt  = (PDML_X_RDK_REPORT_DSL_DEFAULT)pXdslReport->pDSLDefaultReport;

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "OverrideTTL", TRUE))
    {
        pXdslReportDflt->OverrideTTL = uValue;
        XdslReportSetDefaultOverrideTTL (uValue);
        return TRUE;
    }

    /* check the parameter name and set the corresponding value */
    if(AnscEqualString(ParamName, "ReportingPeriod", TRUE)) 
    {
        if (!XdslReportValidateReportingPeriod(uValue))
        {
            return FALSE;
        }
        ULONG psmValue = 0;
        pXdslReportDflt->ReportingPeriod = uValue;
        psmValue = pXdslReportDflt->ReportingPeriod;
        XdslReportSetDefaultReportingPeriod (uValue);
        SetNVRamULONGConfiguration (XdslReportStatusDfltReportingPeriod, psmValue); 
        return TRUE;
    }

    return FALSE;
}

ANSC_STATUS GetNVRamULONGConfiguration(char* setting, ULONG* value)
{
    char *strValue = NULL;
    int retPsmGet = 0;
    
    retPsmGet = PSM_Get_Record_Value2(bus_handle,g_Subsystem, setting, NULL, &strValue);
    if (retPsmGet == CCSP_SUCCESS) 
    {
        *value = _ansc_atoi(strValue);
        ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(strValue);
    }
    return retPsmGet;
}

ANSC_STATUS SetNVRamULONGConfiguration (char * setting, ULONG value)
{
    int retPsmSet = 0;
    char psmValue[32] = {};
    ULONG psm_value = 0;

    retPsmSet = GetNVRamULONGConfiguration(setting,&psm_value);
    if ((retPsmSet == CCSP_SUCCESS) && (psm_value == value))
    {
        return retPsmSet;
    }

    sprintf(psmValue,"%d",value);
    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, setting, ccsp_string, psmValue);
    return retPsmSet;
}

/***********************************************************************

    Device.DSL.X_RDK_NLNM.

    *  X_RDK_NLNM_GetParamIntValue

***********************************************************************/
/**********************************************************************
    caller:     owner of this object

    prototype:

        BOOL
        X_RDK_NLNM_GetParamIntValue 
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
X_RDK_NLNM_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    )
{
    PDATAMODEL_XDSL                  pMyObject        = (PDATAMODEL_XDSL)g_pBEManager->hDSL;
    PDML_XDSL_X_RDK_NLNM             pXdslXRdkNlm     = (PDML_XDSL_X_RDK_NLNM)pMyObject->pDSLXRdkNlm;

    if( ANSC_STATUS_FAILURE == DmlXdslGetXRDKNlm( pXdslXRdkNlm ))
    {
        return FALSE;
    }

    if( AnscEqualString(ParamName, "echotonoiseratio", TRUE))
    {
        /* collect value */
        *pInt = pXdslXRdkNlm->echotonoiseratio;
        return TRUE;
    }
    /* CcspTraceWarning(("Unsupported parameter '%s'\n", ParamName)); */
    return FALSE;
}
