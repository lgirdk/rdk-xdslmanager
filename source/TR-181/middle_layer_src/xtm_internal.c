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

#include "cosa_apis.h"
#include "xtm_apis.h"
#include "xtm_internal.h"
#include "plugin_main_apis.h"
#include "sys_definitions.h"
#include "xdsl_hal.h"

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        PtmCreate
            (
            );

    description:

        This function constructs xtm object and return handle.

    argument:

    return:     newly created xtm object.

**********************************************************************/

ANSC_HANDLE
PtmCreate
    (
        VOID
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pMyObject    = (PDATAMODEL_PTM)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PDATAMODEL_PTM)AnscAllocateMemory(sizeof(DATAMODEL_PTM));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    //pMyObject->Oid               = DATAMODEL_PTM_OID;
    pMyObject->Create            = PtmCreate;
    pMyObject->Remove            = PtmRemove;
    pMyObject->Initialize        = PtmInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

ANSC_STATUS
PtmLinkInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    PDATAMODEL_PTM pPtm = (PDATAMODEL_PTM)hThisObject;
    INT iLoopCount = 0;

    pPtm->ulPtmLinkNumberOfEntries = DmlXdslGetTotalNoofLines(); 
    pPtm->PtmLink = (PDML_PTM)AnscAllocateMemory(sizeof(DML_PTM) * pPtm->ulPtmLinkNumberOfEntries);

    if (pPtm->PtmLink == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }

    for (iLoopCount = 0; iLoopCount < pPtm->ulPtmLinkNumberOfEntries; iLoopCount++)
    {
        PDML_PTM pPtmLink = pPtm->PtmLink + iLoopCount;
        memset(pPtmLink, 0, sizeof(DML_PTM));

        pPtmLink->InstanceNumber = iLoopCount + 1;
        pPtmLink->Status = Down;
        pPtmLink->Enable = FALSE; 
        xtm_hal_getLinkInfo(pPtmLink->InstanceNumber, pPtmLink);
        snprintf(pPtmLink->LowerLayers, sizeof(pPtmLink->LowerLayers), "Device.DSL.Line.%d", pPtmLink->InstanceNumber);
        snprintf(pPtmLink->Alias, sizeof(pPtmLink->Alias), "dsl%d", iLoopCount);
        snprintf(pPtmLink->Name, sizeof(pPtmLink->Alias), "ptm%d", iLoopCount);
        AnscTraceInfo(("%s-%d: Status=[%d] Enable=[%d] LowerLayers=[%s] Alias=[%s] Name=[%s] \n", __FUNCTION__, __LINE__, pPtmLink->Status, pPtmLink->Enable, pPtmLink->LowerLayers, pPtmLink->Alias, pPtmLink->Name));     
        if(FALSE == pPtmLink->Enable)
        {
            AnscTraceInfo(("%s-%d: PTM ENABLE \n", __FUNCTION__, __LINE__)); 
            DmlSetPtmIfEnable(pPtmLink);
        }
        ptm_hal_subscribeStatusEvent(pPtmLink->InstanceNumber);
    }
    ptm_hal_registerStatusCallback(PtmLinkStatusCallback);

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        PtmInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate xtm object and return handle.

    argument:	ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
PtmInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pMyObject        = (PDATAMODEL_PTM)hThisObject;

    PtmLinkInitialize(pMyObject);

EXIT:

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        PtmRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate xtm object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
PtmRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pMyObject    = (PDATAMODEL_PTM)hThisObject;
    ptm_hal_registerStatusCallback(NULL);
    AnscFreeMemory((ANSC_HANDLE)pMyObject->PtmLink);
    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

ANSC_STATUS
AtmLinkInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    PDATAMODEL_ATM pAtm = (PDATAMODEL_ATM)hThisObject;
    INT iLoopCount = 0;
    CHAR param_name[256] = {0};
    CHAR *param_value = NULL;
    int retPsmGet;

    pAtm->ulAtmLinkNumberOfEntries = DmlXdslGetTotalNoofLines(); 
    pAtm->AtmLink = (PDML_ATM)AnscAllocateMemory(sizeof(DML_ATM) * pAtm->ulAtmLinkNumberOfEntries);

    if (pAtm->AtmLink == NULL)
    {
	return ANSC_STATUS_FAILURE;
    }

    for (iLoopCount = 0; iLoopCount < pAtm->ulAtmLinkNumberOfEntries; iLoopCount++)
    {
	PDML_ATM pAtmLink = pAtm->AtmLink + iLoopCount;

	memset(pAtmLink, 0, sizeof(DML_ATM));
	pAtmLink->InstanceNumber = iLoopCount + 1;
	pAtmLink->Status = Down;
	pAtmLink->Enable = FALSE;
    atm_hal_getLinkInfo(pAtmLink->InstanceNumber, pAtmLink);
	snprintf(pAtmLink->LowerLayers, sizeof(pAtmLink->LowerLayers), "Device.DSL.Line.%d", pAtmLink->InstanceNumber);
	snprintf(pAtmLink->Alias, sizeof(pAtmLink->Alias), "dsl%d", iLoopCount);
	snprintf(pAtmLink->Name, sizeof(pAtmLink->Name), "atm%d", iLoopCount);

    AnscTraceInfo(("%s-%d: Status=[%d] Enable=[%d] LowerLayers=[%s] Alias=[%s] Name=[%s] \n", __FUNCTION__, __LINE__, pAtmLink->Status, pAtmLink->Enable, pAtmLink->LowerLayers, pAtmLink->Alias, pAtmLink->Name));     
	/* Get ADSL Linktype */
	_ansc_sprintf(param_name, PSM_ADSL_LINKTYPE, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    if (strcmp(param_value, "EoA") == 0)
	    {
		pAtmLink->LinkType = EOA;
	    }
	    else if (strcmp(param_value, "IPoA") == 0)
	    {
		pAtmLink->LinkType = IPOA;
	    }
	    else if (strcmp(param_value, "PPPoA") == 0)
	    {
		pAtmLink->LinkType = PPPOA;
	    }
	    else if (strcmp(param_value, "CIP") == 0)
	    {
		pAtmLink->LinkType = CIP;
	    }
	    else if (strcmp(param_value, "Unconfigured") == 0)
	    {
		pAtmLink->LinkType = UNCONFIGURED;
	    }
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL Encapsulation */
	_ansc_sprintf(param_name, PSM_ADSL_ENCAPSULATION, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    if (strcmp(param_value, "LLC") == 0)
	    {
		pAtmLink->Encapsulation = LLC;
	    }
	    else if (strcmp(param_value, "VCMUX") == 0)
	    {
		pAtmLink->Encapsulation = VCMUX;
	    }
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL Autoconfig */
	_ansc_sprintf(param_name, PSM_ADSL_AUTOCONFIG, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    pAtmLink->AutoConfig = atoi(param_value);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL PVC */
	_ansc_sprintf(param_name, PSM_ADSL_PVC, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    strncpy(pAtmLink->DestinationAddress, param_value, sizeof(pAtmLink->DestinationAddress) - 1);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL AAL */
	_ansc_sprintf(param_name, PSM_ADSL_AAL, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    if (strcmp(param_value, "AAL1") == 0)
	    {
		pAtmLink->AAL = AAL1;
	    }
	    else if (strcmp(param_value, "AAL2") == 0)
	    {
		pAtmLink->AAL = AAL2;
	    }
	    else if (strcmp(param_value, "AAL3") == 0)
	    {
		pAtmLink->AAL = AAL3;
	    }
	    else if (strcmp(param_value, "AAL4") == 0)
	    {
		pAtmLink->AAL = AAL4;
	    }
	    else if (strcmp(param_value, "AAL5") == 0)
	    {
		pAtmLink->AAL = AAL5;
	    }
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL FCSPreserved */
	_ansc_sprintf(param_name, PSM_ADSL_FCSPRESERVED, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    pAtmLink->FCSPreserved = atoi(param_value);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL VCSearchList */
	_ansc_sprintf(param_name, PSM_ADSL_VCSEARCHLIST, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    strncpy(pAtmLink->VCSearchList, param_value, sizeof(pAtmLink->VCSearchList) - 1);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL QOS Class */
	_ansc_sprintf(param_name, PSM_ADSL_QOS_CLASS, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    if (strcmp(param_value, "UBR") == 0)
	    {
		pAtmLink->Qos.QoSClass = UBR;
	    }
	    else if (strcmp(param_value, "CBR") == 0)
	    {
		pAtmLink->Qos.QoSClass = CBR;
	    }
	    else if (strcmp(param_value, "GFR") == 0)
	    {
		pAtmLink->Qos.QoSClass = GFR;
	    }
	    else if (strcmp(param_value, "VBR-nrt") == 0)
	    {
		pAtmLink->Qos.QoSClass = VBR_NRT;
	    }
	    else if (strcmp(param_value, "VBR-rt") == 0)
	    {
		pAtmLink->Qos.QoSClass = VBR_RT;
	    }
	    else if (strcmp(param_value, "UBR+") == 0)
	    {
		pAtmLink->Qos.QoSClass = UBR_PLUS;
	    }
	    else if (strcmp(param_value, "ABR") == 0)
	    {
		pAtmLink->Qos.QoSClass = ABR;
	    }
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL QOS Peak cell rate */
	_ansc_sprintf(param_name, PSM_ADSL_QOS_PEAKCELLRATE, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    pAtmLink->Qos.PeakCellRate = atoi(param_value);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL QOS Max. burst rate */
	_ansc_sprintf(param_name, PSM_ADSL_QOS_MAXBURSTSIZE, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    pAtmLink->Qos.MaximumBurstSize = atoi(param_value);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

	/* Get ADSL QOS cell rate */
	_ansc_sprintf(param_name, PSM_ADSL_QOS_CELLRATE, pAtmLink->InstanceNumber);
	retPsmGet = PSM_Get_Record_Value2(bus_handle, g_Subsystem, param_name, NULL, &param_value);
	if (retPsmGet == CCSP_SUCCESS && param_value != NULL)
	{
	    pAtmLink->Qos.SustainableCellRate = atoi(param_value);
	    ((CCSP_MESSAGE_BUS_INFO *)bus_handle)->freefunc(param_value);
	}

    if(FALSE == pAtmLink->Enable)
    {
        AnscTraceInfo(("%s-%d: ATM ENABLE \n", __FUNCTION__, __LINE__)); 
        DmlSetAtmIfEnable(pAtmLink);
    }

	atm_hal_subscribeStatusEvent(pAtmLink->InstanceNumber);
    }

    return ANSC_STATUS_SUCCESS;
}

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        AtmCreate
            (
            );

    description:

        This function constructs xtm object and return handle.

    argument:

    return:     newly created xtm object.

**********************************************************************/

ANSC_HANDLE
AtmCreate
    (
        VOID
    )
{
    ANSC_STATUS                returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pMyObject    = (PDATAMODEL_ATM)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PDATAMODEL_ATM)AnscAllocateMemory(sizeof(DATAMODEL_ATM));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    //pMyObject->Oid               = DATAMODEL_ATM_OID;
    pMyObject->Create            = AtmCreate;
    pMyObject->Remove            = AtmRemove;
    pMyObject->Initialize        = AtmInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        AtmInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate xtm object and return handle.

    argument:	ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
AtmInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM                  pMyObject        = (PDATAMODEL_ATM)hThisObject;

    DmlAtmInit(NULL, (PANSC_HANDLE)pMyObject);
    AtmLinkInitialize((ANSC_HANDLE)pMyObject);
    atm_hal_registerStatusCallback(AtmLinkStatusCallback);

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        PtmRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate xtm object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
AtmRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pMyObject    = (PDATAMODEL_ATM)hThisObject;

    atm_hal_registerStatusCallback(NULL);
    AnscFreeMemory((ANSC_HANDLE)pMyObject->AtmLink);
    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}
