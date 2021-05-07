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

    module: xdsl_manager.h

        For CCSP Component:  dsl agent

    ---------------------------------------------------------------

    description:

        This sample implementation file gives the function call prototypes and
        structure definitions used for the RDK-Broadband
        DSL Manager


    ---------------------------------------------------------------

    environment:

        This layer is intended to controls DSL state machine
        through an open API.

    ---------------------------------------------------------------

**********************************************************************/

#ifndef __XDSL_MANAGER_H__
#define __XDSL_MANAGER_H__

/* ---- Include Files ---------------------------------------- */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/time.h>
#include <time.h>

/* ---- Global Constants -------------------------- */

/* ---- Global Types -------------------------- */
typedef  struct
_XDSL_SM_PRIVATE_INFO
{
    char        Name[64];
    char        LowerLayers[128];
}
XDSL_SM_PRIVATE_INFO, *PXDSL_SM_PRIVATE_INFO;

void XdslManager_Start_StateMachine( PXDSL_SM_PRIVATE_INFO pstMPrivateInfo );

#endif /* __XDSL_MANAGER_H__ */
