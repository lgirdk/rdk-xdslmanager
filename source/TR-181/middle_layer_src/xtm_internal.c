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
        XtmCreate
            (
            );

    description:

        This function constructs xtm object and return handle.

    argument:

    return:     newly created xtm object.

**********************************************************************/

ANSC_HANDLE
XtmCreate
    (
        VOID
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pMyObject    = (PDATAMODEL_XTM)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PDATAMODEL_XTM)AnscAllocateMemory(sizeof(DATAMODEL_XTM));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    /*
     * Initialize the common variables and functions for a container object.
     */
    //pMyObject->Oid               = DATAMODEL_XTM_OID;
    pMyObject->Create            = XtmCreate;
    pMyObject->Remove            = XtmRemove;
    pMyObject->Initialize        = XtmInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        XtmInitialize
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
XtmInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus     = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pMyObject        = (PDATAMODEL_XTM)hThisObject;
    PSLAP_VARIABLE                  pSlapVariable    = (PSLAP_VARIABLE             )NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFo  = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoXTM   = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFoXTMPt = NULL;

    /* Initiation all functions */
    AnscSListInitializeHeader( &pMyObject->XTMPMappingList );
    AnscSListInitializeHeader( &pMyObject->Q_XtmList );
    pMyObject->MaxInstanceNumber        = 0;
    pMyObject->ulPtNextInstanceNumber   = 1;
    pMyObject->PreviousVisitTime        = 0;

    /*Create XTM folder in configuration */
    pPoamIrepFo = (PPOAM_IREP_FOLDER_OBJECT)g_GetRegistryRootFolder(g_pDslhDmlManager);

    if ( !pPoamIrepFo )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }

    pPoamIrepFoXTM =
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFo->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFo,
                IREP_FOLDER_NAME_XTM
            );

    if ( !pPoamIrepFoXTM )
    {
        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE);

        pPoamIrepFoXTM =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFo,
                    IREP_FOLDER_NAME_XTM,
                    0
                );

        pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE);
    }

    if ( !pPoamIrepFoXTM )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderXTM = (ANSC_HANDLE)pPoamIrepFoXTM;
    }

    pPoamIrepFoXTMPt =
        (PPOAM_IREP_FOLDER_OBJECT)pPoamIrepFoXTM->GetFolder
            (
                (ANSC_HANDLE)pPoamIrepFoXTM,
                IREP_FOLDER_NAME_PORTTRIGGER
            );

    if ( !pPoamIrepFoXTMPt )
    {
        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, FALSE); */

        pPoamIrepFoXTMPt =
            pPoamIrepFo->AddFolder
                (
                    (ANSC_HANDLE)pPoamIrepFoXTM,
                    IREP_FOLDER_NAME_PORTTRIGGER,
                    0
                );

        /* pPoamIrepFo->EnableFileSync((ANSC_HANDLE)pPoamIrepFo, TRUE); */
    }

    if ( !pPoamIrepFoXTMPt )
    {
        returnStatus = ANSC_STATUS_FAILURE;

        goto  EXIT;
    }
    else
    {
        pMyObject->hIrepFolderXTMPt = (ANSC_HANDLE)pPoamIrepFoXTMPt;
    }

    /* Retrieve the next instance number for Port Trigger */

    if ( TRUE )
    {
        if ( pPoamIrepFoXTMPt )
        {
            pSlapVariable =
                (PSLAP_VARIABLE)pPoamIrepFoXTMPt->GetRecord
                    (
                        (ANSC_HANDLE)pPoamIrepFoXTMPt,
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
        XtmRemove
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
XtmRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS                     returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pMyObject    = (PDATAMODEL_XTM)hThisObject;
    PSINGLE_LINK_ENTRY              pLink        = NULL;
    PCONTEXT_LINK_OBJECT       pXTM         = NULL;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepFo  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderXTM;
    PPOAM_IREP_FOLDER_OBJECT        pPoamIrepPt  = (PPOAM_IREP_FOLDER_OBJECT)pMyObject->hIrepFolderXTMPt;



    /* Remove resource of writable entry link */
    for( pLink = AnscSListPopEntry(&pMyObject->XTMPMappingList); pLink; )
    {
        pXTM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pXTM->hContext);
        AnscFreeMemory(pXTM);
    }

    for( pLink = AnscSListPopEntry(&pMyObject->Q_XtmList); pLink; )
    {
        pXTM = (PCONTEXT_LINK_OBJECT)ACCESS_CONTEXT_LINK_OBJECT(pLink);
        pLink = AnscSListGetNextEntry(pLink);

        AnscFreeMemory(pXTM->hContext);
        AnscFreeMemory(pXTM);
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
XtmGen
    (
        ANSC_HANDLE                 hDml
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pXTM              = (PDATAMODEL_XTM)g_pBEManager->hXTM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        pXTM->MaxInstanceNumber++;

        if ( pXTM->MaxInstanceNumber <= 0 )
        {
            pXTM->MaxInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pXTM->XTMPMappingList, pXTM->MaxInstanceNumber) )
        {
            break;
        }
    }while(1);

    //pEntry->InstanceNumber            = pXTM->MaxInstanceNumber;

    return returnStatus;
}

ANSC_STATUS
XtmGenForTriggerEntry
    (
        ANSC_HANDLE                 hDml,
        PDML_XTM      pEntry
    )
{
    ANSC_STATUS                     returnStatus      = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XTM             pXTM              = (PDATAMODEL_XTM)g_pBEManager->hXTM;

    /*
            For dynamic and writable table, we don't keep the Maximum InstanceNumber.
            If there is delay_added entry, we just jump that InstanceNumber.
        */
    do
    {
        if ( pXTM->ulPtNextInstanceNumber == 0 )
        {
            pXTM->ulPtNextInstanceNumber   = 1;
        }

        if ( !SListGetEntryByInsNum(&pXTM->Q_XtmList, pXTM->ulPtNextInstanceNumber) )
        {
            break;
        }
        else
        {
            pXTM->ulPtNextInstanceNumber++;
        }
    }while(1);

    pEntry->InstanceNumber            = pXTM->ulPtNextInstanceNumber;

    //Init all memory
    _ansc_sprintf( pEntry->Name, "%s", "ptm0" );
    _ansc_sprintf( pEntry->Path, "%s%d", XTM_LINK_PATH, pEntry->InstanceNumber );
    pEntry->Status = XTM_IF_STATUS_NOT_PRESENT;

    pXTM->ulPtNextInstanceNumber++;

    return returnStatus;
}
