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

#ifndef  _XTM_APIS_H
#define  _XTM_APIS_H

#include "cosa_apis.h"
#include "ssp_global.h"
#include "xdsl_apis.h"

#define XTM_IF_STATUS_ERROR            7
#define XTM_IF_STATUS_NOT_PRESENT      5
#define PTM_LINK_PATH                  "Device.PTM.Link."
#define ATM_LINK_PATH                  "Device.ATM.Link."

/* Telemetry Markers  */
#define PTM_MARKER_VLAN_CREATE         "RDKB_PTM_VLAN_CREATE"
#define PTM_MARKER_VLAN_DELETE         "RDKB_PTM_VLAN_DELETE" 
#define PTM_MARKER_VLAN_CFG_CHNG       "RDKB_PTM_CFG_CHANGED"

/***********************************
    Actual definition declaration
************************************/
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
    PTM Part
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
_DML_PTM
{
    ULONG                InstanceNumber;
    BOOLEAN              Enable;
    xtm_link_status_e    Status;
    CHAR                 Alias[64];
    CHAR                 Name[64];
    UINT                 LastChange;
    CHAR                 LowerLayers[1024];
    CHAR                 MACAddress[64];
    DML_PTM_STATS   Statistics;
}
DML_PTM,  *PDML_PTM;

/* ATM Link types */
typedef enum
{
    EOA            = 1,
    IPOA,
    PPPOA,
    CIP,
    UNCONFIGURED,
} linktype_e;

/* ATM Encapsulation*/
typedef enum
{
    LLC            = 1,
    VCMUX
} encapsulation_e;

/* ATM AAL types */
typedef enum
{
    AAL1            = 1,
    AAL2,
    AAL3,
    AAL4,
    AAL5
} aal_e;

/* ATM QOS Class */
typedef enum
{
    UBR            = 1,
    CBR,
    GFR,
    VBR_NRT,
    VBR_RT,
    UBR_PLUS,
    ABR
} QOS_CLASS_TYPE;

/* ATM Diagnostics states */
typedef enum
{
    DIAG_STATE_NONE = 1,
    DIAG_STATE_REQUESTED,
    DIAG_STATE_CANCELED,
    DIAG_STATE_COMPLETE,
    DIAG_STATE_ERROR,
    DIAG_STATE_ERROR_INTERNAL,
    DIAG_STATE_ERROR_OTHER
} ATM_DIAG_STATES;

typedef  struct
_DML_ATM_STATS
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
    UINT     TransmittedBlocks;
    UINT     ReceivedBlocks;
    UINT     CRCErrors;
    UINT     HECErrors;
}
DML_ATM_STATS , *PDML_ATM_STATS;

typedef  struct
_DML_ATM_QOS
{
    QOS_CLASS_TYPE    QoSClass;
    UINT              PeakCellRate;
    UINT              MaximumBurstSize;
    UINT              SustainableCellRate;
}DML_ATM_QOS , *PDML_ATM_QOS;

typedef  struct
_DML_ATM_DIAG
{
    ATM_DIAG_STATES   DiagnosticsState;
    CHAR              Interface[256];
    UINT              NumberOfRepetitions;
    UINT              Timeout;
    UINT              SuccessCount;
    UINT              FailureCount;
    UINT              AverageResponseTime;
    UINT              MinimumResponseTime;
    UINT              MaximumResponseTime;
}DML_ATM_DIAG, *PDML_ATM_DIAG;

typedef  struct
_DML_ATM
{
    ULONG                InstanceNumber;
    BOOLEAN              Enable;
    xtm_link_status_e    Status;
    CHAR                 Alias[64];
    CHAR                 Name[64];
    linktype_e           LinkType;
    encapsulation_e      Encapsulation;
    aal_e                AAL;
    BOOLEAN              AutoConfig;
    BOOLEAN              FCSPreserved;
    CHAR                 DestinationAddress[256];
    UINT                 LastChange;
    CHAR                 LowerLayers[1024];
    CHAR                 VCSearchList[256];
    DML_ATM_QOS          Qos;
    DML_ATM_STATS        Statistics;
}
DML_ATM,  *PDML_ATM;
#define DML_PTM_INIT(pPtm)                     \
{                                              \
    (pPtm)->Enable            = FALSE;         \
}                                              \

#define DML_ATM_INIT(pAtm)                     \
{                                              \
    (pAtm)->Enable            = FALSE;         \
}                                              \

/*************************************
    The actual function declaration
**************************************/

ANSC_STATUS DmlAddPtm(ANSC_HANDLE hContext, PDML_PTM pEntry);
ANSC_STATUS DmlSetPtmIfEnable(PDML_PTM p_Ptm);
ANSC_STATUS DmlGetPtmIfStatus( ANSC_HANDLE hContext, PDML_PTM p_Ptm);
ANSC_STATUS DmlGetPtmIfStatistics (ANSC_HANDLE hContext, PDML_PTM p_Ptm);
ANSC_STATUS DmlSetPtm (ANSC_HANDLE hContext, PDML_PTM p_Ptm);
ANSC_STATUS DmlDelPtm (ANSC_HANDLE hContext, PDML_PTM pEntry);
ANSC_STATUS DmlAddInit (ANSC_HANDLE hContext, PDML_ATM pEntry);
ANSC_STATUS DmlAtmDiagnosticsInit (PANSC_HANDLE phContext);
ANSC_STATUS DmlAddAtm (ANSC_HANDLE hContext, PDML_ATM pEntry);
ANSC_STATUS DmlSetAtmIfEnable (PDML_ATM p_Atm);
ANSC_STATUS DmlGetAtmIfStatus (ANSC_HANDLE hContext, PDML_ATM p_Atm);
ANSC_STATUS DmlGetAtmIfStatistics (ANSC_HANDLE hContext, PDML_ATM  p_Atm);
ANSC_STATUS DmlSetAtm (ANSC_HANDLE hContext, PDML_ATM  p_Atm);
ANSC_STATUS DmlDelAtm (ANSC_HANDLE hContext, PDML_ATM p_Atm);
ANSC_STATUS DmlStartAtmLoopbackDiagnostics (PDML_ATM_DIAG pDiag);
int PtmLinkStatusCallback(int atm_id, char *status);
int AtmLinkStatusCallback(int atm_id, char *status);
void DmlPtmLinkSetEnable(PDML_PTM p_Ptm, BOOL bValue);
void DmlPtmLinkSetStatus(PDML_PTM p_Ptm, xtm_link_status_e status);
void DmlAtmLinkSetEnable(PDML_ATM p_Atm, BOOL bValue);
void DmlAtmLinkSetStatus(PDML_ATM p_Atm, xtm_link_status_e status);
#endif
