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

#ifndef  _XTM_APIS_H
#define  _XTM_APIS_H

#include "cosa_apis.h"
#include "ssp_global.h"

#define XTM_IF_STATUS_ERROR            7
#define XTM_IF_STATUS_NOT_PRESENT      5
#define XTM_LINK_PATH                  "Device.PTM.Link."

/* Telemetry Markers  */
#define XTM_MARKER_VLAN_CREATE         "RDKB_XTM_VLAN_CREATE"
#define XTM_MARKER_VLAN_DELETE         "RDKB_XTM_VLAN_DELETE" 
#define XTM_MARKER_VLAN_CFG_CHNG       "RDKB_XTM_CFG_CHANGED" 

/***********************************
    Actual definition declaration
************************************/
typedef enum
_XTM_NOTIFY_ENUM
{
    NOTIFY_TO_VLAN_AGENT      = 1,
} XTM_NOTIFY_ENUM;

typedef  struct
_DML_PTM_STATS
{
    ULONG    BytesSent;
    ULONG    BytesReceived;
    ULONG    PacketsSent;
    ULONG    PacketsReceived;
    UINT     ErrorsSent;
    UINT     ErrorsReceived;
    ULONG    UnicastPacketsSent;
    ULONG    UnicastPacketsReceived;
    UINT     DiscardPacketsSent;
    UINT     DiscardPacketsReceived;
    ULONG    MulticastPacketsSent;
    ULONG    MulticastPacketsReceived;
    ULONG    BroadcastPacketsSent;
    ULONG    BroadcastPacketsReceived;
    UINT     UnknownProtoPacketsReceived;
}
DML_PTM_STATS , *PDML_PTM_STATS;
/*
    XTM Part
*/
typedef enum { Up = 1, 
               Down, 
               Unknown, 
               Dormant, 
               NotPresent, 
               LowerLayerDown, 
               Error 
}xtm_link_status_e;

typedef  struct
_DML_XTM
{
    ULONG                InstanceNumber;
    BOOLEAN              Enable;
    xtm_link_status_e    Status;
    CHAR                 Alias[64];
    CHAR                 Name[64];
    CHAR                 Path[128];
    UINT                 LastChange;
    CHAR                 LowerLayers[1024];
    CHAR                 MACAddress[64];
    DML_PTM_STATS   Statistics;
}
DML_XTM,  *PDML_XTM;

#define DML_XTM_INIT(pXtm)                                            \
{                                                                                  \
    (pXtm)->Enable            = FALSE;                                      \
}                                                                                  \

/*
Description:
    This callback routine is provided for backend to call middle layer
Arguments:
    hDml          Opaque handle passed in from DmlXtmInit.
    pEntry        Pointer to XTM xtm mapping to receive the generated values.
Return:
    Status of operation


*/
typedef ANSC_STATUS
(*PFN_DML_XTM_GEN)
    (
        ANSC_HANDLE                 hDml
);


/*************************************
    The actual function declaration
**************************************/

ANSC_STATUS
DmlAddXtm
    (
        ANSC_HANDLE                 hContext,
        PDML_XTM              pEntry
    );


ANSC_STATUS
DmlGetXtmIfEnable
    (
        BOOLEAN     *pbEnable
    );

ANSC_STATUS
DmlSetXtmIfEnable
    (
        BOOLEAN     bEnable
    );

ANSC_STATUS
DmlGetXtmIfStatus
    (
        ANSC_HANDLE                 hContext,
        PDML_XTM               p_Xtm
    );

ANSC_STATUS
DmlGetXtmIfStatistics
    (
        ANSC_HANDLE                 hContext,
        PDML_XTM               p_Xtm
    );

ANSC_STATUS
DmlSetXtm
    (
        ANSC_HANDLE                 hContext,
        PDML_XTM              p_Xtm
    );

ANSC_STATUS
DmlDelXtm
    (
        ANSC_HANDLE                 hContext,
        PDML_XTM         pEntry
    );

ANSC_STATUS DmlXtmCreateEthLink( PDML_XTM   pEntry );

ANSC_STATUS DmlXtmDeleteEthLink( char *l3ifName );

#endif
