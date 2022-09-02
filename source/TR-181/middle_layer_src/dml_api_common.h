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

/**********************************************************************

    module:	dml_api_common.h

        For Data Model Library Implementation (DML),
        BroadWay Service Delivery System

    ---------------------------------------------------------------

        This wrapper file defines the common data structure and
        constants for DML API.

    ---------------------------------------------------------------

    environment:

        platform independent

    ---------------------------------------------------------------

    revision:

        23/10/2019    initial revision.

**********************************************************************/


#ifndef  _DML_API_COMMON_
#define  _DML_API_COMMON_

#include "ansc_platform.h"

/*
 *  Define custom trace module ID
 */
#ifdef   ANSC_TRACE_MODULE_ID
    #undef  ANSC_TRACE_MODULE_ID
#endif

#define  ANSC_TRACE_MODULE_ID                       ANSC_TRACE_ID_DM_API


/**********************************************************************
                STRUCTURE AND CONSTANT DEFINITIONS
**********************************************************************/

typedef  enum
_DML_IF_STATUS
{
    DML_IF_STATUS_Up               = 1,
    DML_IF_STATUS_Down,
    DML_IF_STATUS_Unknown,
    DML_IF_STATUS_Dormant,
    DML_IF_STATUS_NotPresent,
    DML_IF_STATUS_LowerLayerDown,
    DML_IF_STATUS_Error
}
COSA_DML_IF_STATUS, *PCOSA_DML_IF_STATUS;


typedef  struct
_COSA_DML_IF_STATS
{
    ULONG                           BytesSent;
    ULONG                           BytesReceived;
    ULONG                           PacketsSent;
    ULONG                           PacketsReceived;
    ULONG                           ErrorsSent;
    ULONG                           ErrorsReceived;
    ULONG                           UnicastPacketsSent;
    ULONG                           UnicastPacketsReceived;
    ULONG                           DiscardPacketsSent;
    ULONG                           DiscardPacketsReceived;
    ULONG                           MulticastPacketsSent;
    ULONG                           MulticastPacketsReceived;
    ULONG                           BroadcastPacketsSent;
    ULONG                           BroadcastPacketsReceived;
    ULONG                           UnknownProtoPacketsReceived;
}
COSA_DML_IF_STATS, *PCOSA_DML_IF_STATS;


typedef  enum
_COSA_DML_STATUS
{
    COSA_DML_STATUS_Disabled    = 1,
    COSA_DML_STATUS_Enabled,
    COSA_DML_STATUS_Error_Misconfigured,
    COSA_DML_STATUS_Error
}
COSA_DML_STATUS, *PCOSA_DML_STATUS;


#endif

