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

    module: xdsl_internal.c

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file implementes back-end apis for the COSA Data Model Library

        *  XdslCreate
        *  XdslInitialize
        *  XdslRemove
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

#include "plugin_main_apis.h"
#include "xdsl_apis.h"
#include "xdsl_dml.h"
#include "xdsl_internal.h"

/**********************************************************************

    caller:     owner of the object

    prototype:

        ANSC_HANDLE
        XdslCreate
            (
            );

    description:

        This function constructs cosa device info object and return handle.

    argument:  

    return:     newly created device info object.

**********************************************************************/

ANSC_HANDLE
XdslCreate
    (
        VOID
    )
{
    ANSC_STATUS           returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XDSL   pMyObject    = (PDATAMODEL_XDSL)NULL;

    /*
     * We create object by first allocating memory for holding the variables and member functions.
     */
    pMyObject = (PDATAMODEL_XDSL)AnscAllocateMemory(sizeof(DATAMODEL_DSL));

    if ( !pMyObject )
    {
        return  (ANSC_HANDLE)NULL;
    }

    AnscZeroMemory(pMyObject, sizeof(DATAMODEL_DSL));

    /*
     * Initialize the common variables and functions for a container object.
     */

    pMyObject->Create            = XdslCreate;
    pMyObject->Remove            = XdslRemove;
    pMyObject->Initialize        = XdslInitialize;

    pMyObject->Initialize   ((ANSC_HANDLE)pMyObject);

    return  (ANSC_HANDLE)pMyObject;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        XdslInitialize
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa device info object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
XdslInitialize
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS           returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XDSL   pMyObject    = (PDATAMODEL_XDSL)hThisObject;

    /* Initiation all functions */

    /* Initialize middle layer for Device.DSL.  */
    DmlXdslInit(NULL, (PANSC_HANDLE)pMyObject);

    return returnStatus;
}

/**********************************************************************

    caller:     self

    prototype:

        ANSC_STATUS
        XdslRemove
            (
                ANSC_HANDLE                 hThisObject
            );

    description:

        This function initiate  cosa device info object and return handle.

    argument:   ANSC_HANDLE                 hThisObject
            This handle is actually the pointer of this object
            itself.

    return:     operation status.

**********************************************************************/

ANSC_STATUS
XdslRemove
    (
        ANSC_HANDLE                 hThisObject
    )
{
    ANSC_STATUS              returnStatus = ANSC_STATUS_SUCCESS;
    PDATAMODEL_XDSL      pMyObject    = (PDATAMODEL_XDSL)hThisObject;
    PDML_XDSL_LINE       pXDSLLineTmp  = pMyObject->pXDSLLine;

    /* Remove necessary resource */

    AnscFreeMemory( pXDSLLineTmp );

    /* Remove self */
    AnscFreeMemory((ANSC_HANDLE)pMyObject);

    return returnStatus;
}



