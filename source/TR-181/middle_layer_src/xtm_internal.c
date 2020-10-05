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

#include "cosa_apis.h"
#include "xtm_apis.h"
#include "xtm_internal.h"
#include "plugin_main_apis.h"
#include "poam_irepfo_interface.h"
#include "sys_definitions.h"

extern void * g_pDslhDmlManager;

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
    PSLAP_VARIABLE                  pSlapVariable    = (PSLAP_VARIABLE             )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFo  = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoPTM   = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoPTMPt = NULL;

    /* Initiation all functions */
    AnscSListInitializeHeader( &pMyObject->PTMPMappingList );
    AnscSListInitializeHeader( &pMyObject->Q_PtmList );
    pMyObject->MaxInstanceNumber        = 0;
    pMyObject->ulPtNextInstanceNumber   = 1;
    pMyObject->PreviousVisitTime        = 0;

    /*Create PTM folder in configuration */
    pPoamIrepFo = (PPOAM_IREP_FOLDER_OBJECT)g_GetRegistryRootFolder(g_pDslhDmlManager);

    if ( !pPoamIrepFo )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }

    pPoamIrepFoPTM = (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFo->GetFolder((ANSC_HANDLE)pPoamIrepFo, IREP_FOLDER_NAME_PTM );

    if ( !pPoamIrepFoPTM )
    {
        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE);

        pPoamIrepFoPTM =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFo,
                    IREP_FOLDER_NAME_PTM,
                    0
                );

        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE);
    }

    if ( !pPoamIrepFoPTM )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderPTM = (ANSC_HANDLE)pPoamIrepFoPTM;
    }

    pPoamIrepFoPTMPt =
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoPTM->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFoPTM,
                IREP_FOLDER_NAME_PORTTRIGGER
            );

    if ( !pPoamIrepFoPTMPt )
    {
        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE); */

        pPoamIrepFoPTMPt =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoPTM,
                    IREP_FOLDER_NAME_PORTTRIGGER,
                    0
                );

        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE); */
    }

    if ( !pPoamIrepFoPTMPt )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderPTMPt = (ANSC_HANDLE)pPoamIrepFoPTMPt;
    }

    /* Retrieve the next instance number for Port Trigger */

    if ( TRUE )
    {
        if ( pPoamIrepFoPTMPt )
        {
            pSlapVariable =
                (PSLAP_VARIABLE)pPoamIrepFoPTMPt->GetRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoPTMPt,
                        DML_RR_NAME_XTMNextInsNumber,
                        NULL
                    );

            if ( pSlapVariable )
            {
                pMyObject->ulPtNextInstanceNumber = pSlapVariable->Variant.varUint32;

                SlapFreeVariable(pSlapVariable);
            }
        }
    }

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
    PSINGLE_LINK_ENTRY              pLink        = NULL;
    PCONTEXT_LINK_OBJECT       pPTM         = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFo  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderPTM;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepPt  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderPTMPt;



    /* Remove resource of writable entry link */
    for( pLink = AnscSListPopEntry(&pMyObject->PTMPMappingList); pLink; )
    {
        pPTM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pPTM->hContext);
        AnscFreeMemory(pPTM);
    }

    for( pLink = AnscSListPopEntry(&pMyObject->Q_PtmList); pLink; )
    {
        pPTM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pPTM->hContext);
        AnscFreeMemory(pPTM);
    }

    if ( pPoamIrepPt )
    {
        pPoamIrepPt->Remove( (ANSC_HANDLE)pPoamIrepPt);
    }

    if ( pPoamIrepFo )
    {
        pPoamIrepFo->Remove( (ANSC_HANDLE)pPoamIrepFo);
    }

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

ANSC_STATUS
PtmGen
    (
        ANSC_HANDLE                 hDml
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pPTM              = (PDATAMODEL_PTM)g_pBEManager->hPTM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        pPTM->MaxInstanceNumber++;

        if ( pPTM->MaxInstanceNumber <= 0 )
        {
            pPTM->MaxInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pPTM->PTMPMappingList, pPTM->MaxInstanceNumber) )
        {
            break;
        }
    }while(1);

    //pEntry->InstanceNumber            = pPTM->MaxInstanceNumber;

    return returnStatus;
}

ANSC_STATUS
PtmGenForTriggerEntry
    (
        ANSC_HANDLE                 hDml,
        PDML_PTM      pEntry
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_PTM             pPTM              = (PDATAMODEL_PTM)g_pBEManager->hPTM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        if ( pPTM->ulPtNextInstanceNumber == 0 )
        {
            pPTM->ulPtNextInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pPTM->Q_PtmList, pPTM->ulPtNextInstanceNumber) )
        {
            break;
        }
        else
        {
            pPTM->ulPtNextInstanceNumber++;
        }
    }while(1);

    pEntry->InstanceNumber            = pPTM->ulPtNextInstanceNumber;

    //Init all memory
    _ansc_sprintf( pEntry->Name, "%s", "ptm0" );
    _ansc_sprintf( pEntry->Path, "%s%d", PTM_LINK_PATH, pEntry->InstanceNumber );
    pEntry->Status = XTM_IF_STATUS_NOT_PRESENT;

    pPTM->ulPtNextInstanceNumber++;

    return returnStatus;
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
    PSLAP_VARIABLE                  pSlapVariable    = (PSLAP_VARIABLE) NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFo  = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoATM   = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoATMPt = NULL;

    /* Initiation all functions */
    AnscSListInitializeHeader( &pMyObject->ATMPMappingList );
    AnscSListInitializeHeader( &pMyObject->Q_AtmList );
    pMyObject->MaxInstanceNumber        = 0;
    pMyObject->ulPtNextInstanceNumber   = 1;
    pMyObject->PreviousVisitTime        = 0;

    /*Create ATM folder in configuration */
    pPoamIrepFo = (PPOAM_IREP_FOLDER_OBJECT)g_GetRegistryRootFolder(g_pDslhDmlManager);

    if ( !pPoamIrepFo )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }

    pPoamIrepFoATM =
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFo->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFo,
                IREP_FOLDER_NAME_ATM
            );

    if ( !pPoamIrepFoATM )
    {
        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE);

        pPoamIrepFoATM =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFo,
                    IREP_FOLDER_NAME_ATM,
                    0
                );

        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE);
    }

    if ( !pPoamIrepFoATM )
    {
        returnStatus = ANSC_STATUS_FAILURE;
        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderATM = (ANSC_HANDLE)pPoamIrepFoATM;
    }

    pPoamIrepFoATMPt =
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoATM->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFoATM,
                IREP_FOLDER_NAME_PORTTRIGGER
            );

    if ( !pPoamIrepFoATMPt )
    {
        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE); */

        pPoamIrepFoATMPt =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoATM,
                    IREP_FOLDER_NAME_PORTTRIGGER,
                    0
                );

        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE); */
    }

    if ( !pPoamIrepFoATMPt )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderATMPt = (ANSC_HANDLE)pPoamIrepFoATMPt;
    }

    /* Retrieve the next instance number for Port Trigger */

    if ( pPoamIrepFoATMPt )
    {
        pSlapVariable =
        (PSLAP_VARIABLE)pPoamIrepFoATMPt->GetRecord
             (
                 (ANSC_HANDLE)pPoamIrepFoATMPt,
                  DML_RR_NAME_XTMNextInsNumber,
                  NULL
                 );

        if ( pSlapVariable )
        {
            pMyObject->ulPtNextInstanceNumber = pSlapVariable->Variant.varUint32;
            SlapFreeVariable(pSlapVariable);
        }
    }

    DmlAtmInit(NULL, (PANSC_HANDLE)pMyObject);

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
AtmRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pMyObject    = (PDATAMODEL_ATM)hThisObject;
    PSINGLE_LINK_ENTRY         pLink        = NULL;
    PCONTEXT_LINK_OBJECT       pATM         = NULL;
    PPOAM_IREP_FOLDER_OBJECT   pPoamIrepFo  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderATM;
    PPOAM_IREP_FOLDER_OBJECT   pPoamIrepPt  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderATMPt;

    /* Remove resource of writable entry link */
    for( pLink = AnscSListPopEntry(&pMyObject->ATMPMappingList); pLink; )
    {
        pATM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pATM->hContext);
        AnscFreeMemory(pATM);
    }

    for( pLink = AnscSListPopEntry(&pMyObject->Q_AtmList); pLink; )
    {
        pATM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pATM->hContext);
        AnscFreeMemory(pATM);
    }

    if ( pPoamIrepPt )
    {
        pPoamIrepPt->Remove( (ANSC_HANDLE)pPoamIrepPt);
    }

    if ( pPoamIrepFo )
    {
        pPoamIrepFo->Remove( (ANSC_HANDLE)pPoamIrepFo);
    }

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}

ANSC_STATUS
AtmGen
    (
        ANSC_HANDLE                 hDml
    )
{
    ANSC_STATUS                returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pATM              = (PDATAMODEL_ATM)g_pBEManager->hATM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        pATM->MaxInstanceNumber++;

        if ( pATM->MaxInstanceNumber <= 0 )
        {
            pATM->MaxInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pATM->ATMPMappingList, pATM->MaxInstanceNumber) )
        {
            break;
        }
    }while(1);

    //pEntry->InstanceNumber            = pXTM->MaxInstanceNumber;

    return returnStatus;
}

ANSC_STATUS
AtmGenForTriggerEntry
    (
        ANSC_HANDLE   hDml,
        PDML_ATM      pEntry
    )
{
    ANSC_STATUS                returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_ATM             pATM              = (PDATAMODEL_ATM)g_pBEManager->hATM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        if ( pATM->ulPtNextInstanceNumber == 0 )
        {
            pATM->ulPtNextInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pATM->Q_AtmList, pATM->ulPtNextInstanceNumber) )
        {
            break;
        }
        else
        {
            pATM->ulPtNextInstanceNumber++;
        }
    }while(1);

    pEntry->InstanceNumber            = pATM->ulPtNextInstanceNumber;

    //Init all memory
    _ansc_sprintf( pEntry->Name, "%s", "atm0" );
    //_ansc_sprintf( pEntry->Path, "%s%d", ATM_LINK_PATH, pEntry->InstanceNumber );
    pEntry->Status = XTM_IF_STATUS_NOT_PRESENT;

    pATM->ulPtNextInstanceNumber++;

    return returnStatus;
}
