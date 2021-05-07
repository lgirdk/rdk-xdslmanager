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

#ifndef  _XDSL_APIS_H
#define  _XDSL_APIS_H

#include "cosa_apis.h"
#include "xdsl_manager.h"

/* * Telemetry Markers */
#define XDSL_MARKER_LINE_CFG_CHNG           "RDKB_XDSL_LINE_CFG_CHANGED"
#define XDSL_MARKER_CHAN_CFG_CHNG           "RDKB_XDSL_CHANNEL_CFG_CHANGED"
#define XDSL_MARKER_LINE_LINK_CB            "RDKB_XDSL_LINE_STATUS_CB"
#define XDSL_MARKER_SM_TRANSITION           "RDKB_XDSL_SM_TRANSITION"


#define XDSL_STANDARD_USED_STR_MAX          64



/* Collection */
typedef enum
_DML_XDSL_IF_STATUS
{
    XDSL_IF_STATUS_Up               = 1,
    XDSL_IF_STATUS_Down,
    XDSL_IF_STATUS_Unknown,
    XDSL_IF_STATUS_Dormant,
    XDSL_IF_STATUS_NotPresent,
    XDSL_IF_STATUS_LowerLayerDown,
    XDSL_IF_STATUS_Error
} DML_XDSL_IF_STATUS;

typedef enum
_DML_XDSL_LINK_STATUS
{
    XDSL_LINK_STATUS_Up 		     = 1,
    XDSL_LINK_STATUS_Initializing,
    XDSL_LINK_STATUS_EstablishingLink,
    XDSL_LINK_STATUS_NoSignal,
    XDSL_LINK_STATUS_Disabled,
    XDSL_LINK_STATUS_Error
} DML_XDSL_LINK_STATUS;

/** enum wan status */
typedef enum
_DML_XDSL_LINE_WAN_STATUS
{
    XDSL_LINE_WAN_UP          = 1,
    XDSL_LINE_WAN_DOWN        = 2,
} DML_XDSL_LINE_WAN_STATUS;

/** enum notify */
typedef enum
_XDSL_NOTIFY_ENUM
{
    NOTIFY_TO_WAN_AGENT      = 1,
    NOTIFY_TO_PTM_AGENT      = 2,
    NOTIFY_TO_ATM_AGENT      = 3
} XDSL_NOTIFY_ENUM;

/** enum line encoding */
typedef enum
_XDSL_LINE_ENCODING_ENUM
{
    DML_LINE_ENCODING_DMT = 1,
    DML_LINE_ENCODING_CAP,
    DML_LINE_ENCODING_2B1Q,
    DML_LINE_ENCODING_43BT,
    DML_LINE_ENCODING_PAM,
    DML_LINE_ENCODING_QAM
} XDSL_LINE_ENCODING_ENUM;

/** enum line power management */
typedef enum
_XDSL_LINE_POWER_MGMT_ENUM
{
    PM_STATE_L0 = 1,
    PM_STATE_L1,
    PM_STATE_L2,
    PM_STATE_L3,
    PM_STATE_L4
} XDSL_LINE_POWER_MGMT_ENUM;

typedef  struct
_DML_XDSL_LINE_STATS_TIME
{
    UINT                            ErroredSecs;
    UINT                            SeverelyErroredSecs;
}
DML_XDSL_LINE_STATS_TIME, *PDML_XDSL_LINE_STATS_TIME;

typedef  struct
_DML_XDSL_LINE_STATS_QUARTERHOUR
{
    UINT                            ErroredSecs;
    UINT                            SeverelyErroredSecs;
    UINT                            X_RDK_LinkRetrain;
}
DML_XDSL_LINE_STATS_QUARTERHOUR, *PDML_XDSL_LINE_STATS_QUARTERHOUR;

typedef  struct
_DML_XDSL_LINE_STATS_CURRENTDAY
{
    UINT                            ErroredSecs;
    UINT                            SeverelyErroredSecs;
    UINT                            X_RDK_LinkRetrain;
    UINT                            X_RDK_InitErrors;
    UINT                            X_RDK_InitTimeouts;
    UINT                            X_RDK_SuccessfulRetrains;
}
DML_XDSL_LINE_STATS_CURRENTDAY, *PDML_XDSL_LINE_STATS_CURRENTDAY;

typedef  struct
_DML_XDSL_LINE_STATS
{
    ULONG                           BytesSent;
    ULONG                           BytesReceived;
    ULONG                           PacketsSent;
    ULONG                           PacketsReceived;
    ULONG                           ErrorsSent;
    ULONG                           ErrorsReceived;
    ULONG                           DiscardPacketsSent;
    ULONG                           DiscardPacketsReceived;
    ULONG                           TotalStart;
    ULONG                           ShowtimeStart;
    UINT                            LastShowtimeStart;
    ULONG                           QuarterHourStart;
    ULONG                           CurrentDayStart;
    DML_XDSL_LINE_STATS_TIME         stTotal;
    DML_XDSL_LINE_STATS_TIME         stShowTime;
    DML_XDSL_LINE_STATS_TIME         stLastShowTime;
    DML_XDSL_LINE_STATS_CURRENTDAY   stCurrentDay;
    DML_XDSL_LINE_STATS_QUARTERHOUR  stQuarterHour;
}
DML_XDSL_LINE_STATS, *PDML_XDSL_LINE_STATS;

typedef  struct
_DML_XDSL_LINE
{	
    BOOL                              Enable;
    BOOL                              EnableDataGathering;
    ULONG                             ulInstanceNumber;
    CHAR                              LowerLayers[128];
    CHAR                              Alias[64];
    CHAR                              Name[64];
    UINT                              LastChange;
    CHAR                              StandardsSupported[512];
    CHAR                              XTSE[17];
    CHAR                              AllowedProfiles[256];
    CHAR                              StandardUsed[XDSL_STANDARD_USED_STR_MAX];
    CHAR                              XTSUsed[17];
    CHAR                              FirmwareVersion[64];
    DML_XDSL_IF_STATUS                Status;
    DML_XDSL_LINK_STATUS              LinkStatus;
    BOOL                              Upstream;
    CHAR                              CurrentProfile[64];
    XDSL_LINE_POWER_MGMT_ENUM         PowerManagementState;
    UINT                              UpstreamMaxBitRate;
    UINT                              DownstreamMaxBitRate;
    UINT                              SuccessFailureCause;
    DML_XDSL_LINE_WAN_STATUS          WanStatus;
    UINT                              UPBOKLER;
    CHAR                              UPBOKLEPb[256];
    CHAR                              UPBOKLERPb[256];
    INT                               RXTHRSHds;
    UINT                              ACTRAMODEds;
    UINT                              ACTRAMODEus;
    UINT                              ACTINPROCds;
    UINT                              ACTINPROCus;
    UINT                              SNRMROCds;
    UINT                              SNRMROCus;
    UINT                              LastStateTransmittedDownstream;
    UINT                              LastStateTransmittedUpstream;
    UINT                              UPBOKLE;
    CHAR                              MREFPSDds[145];
    CHAR                              MREFPSDus[145];
    UINT                              LIMITMASK;
    UINT                              US0MASK;
    INT                               UpstreamAttenuation;
    INT                               UpstreamNoiseMargin;
    INT                               UpstreamPower;
    XDSL_LINE_ENCODING_ENUM           LineEncoding;
    INT                               TRELLISds;
    INT                               TRELLISus;
    UINT                              ACTSNRMODEds;
    UINT                              ACTSNRMODEus;
    CHAR                              VirtualNoisePSDds[97];
    CHAR                              VirtualNoisePSDus[49];
    UINT                              ACTUALCE;
    INT                               LineNumber;
    CHAR                              SNRMpbus[24];
    CHAR                              SNRMpbds[24];
    ULONG                             INMIATOds;
    ULONG                             INMIATSds;
    ULONG                             INMCCds;
    INT                               INMINPEQMODEds;
    INT                               DownstreamAttenuation;
    INT                               DownstreamNoiseMargin;
    INT                               DownstreamPower;
    CHAR                              XTURVendor[9];
    CHAR                              XTURCountry[5];
    UINT                              XTURANSIStd;
    UINT                              XTURANSIRev;
    CHAR                              XTUCVendor[9];
    CHAR                              XTUCCountry[5];
    UINT                              XTUCANSIStd;
    UINT                              XTUCANSIRev;
    DML_XDSL_LINE_STATS                stLineStats;
}
DML_XDSL_LINE, *PDML_XDSL_LINE;              

typedef  struct
_DML_XDSL_LINE_GLOBALINFO
{
    BOOL                              Upstream;
    CHAR                              Name[64];
    CHAR                              LowerLayers[128];
    CHAR                              StandardUsed[XDSL_STANDARD_USED_STR_MAX];
    DML_XDSL_LINE_WAN_STATUS           WanStatus;
    DML_XDSL_LINK_STATUS               LinkStatus;
    pthread_t                          iface_thread_id;
}
DML_XDSL_LINE_GLOBALINFO, *PDML_XDSL_LINE_GLOBALINFO;

/* * Channel */

typedef enum
_DML_XDSL_CHANNEL_LINK_ENCAP
{   
    G_992_3_Annex_K_ATM = 1,
    G_992_3_Annex_K_PTM,
    G_993_2_Annex_K_ATM,
    G_993_2_Annex_K_PTM,
    G_994_1_Auto
} DML_XDSL_CHANNEL_LINK_ENCAP;

typedef  struct
_DML_XDSL_CHANNEL_STATS_TIME
{
    UINT                              XTURFECErrors;
    UINT                              XTUCFECErrors;
    UINT                              XTURHECErrors;
    UINT                              XTUCHECErrors;
    UINT                              XTURCRCErrors;
    UINT                              XTUCCRCErrors;
}
DML_XDSL_CHANNEL_STATS_TIME, *PDML_XDSL_CHANNEL_STATS_TIME;

typedef  struct
_DML_XDSL_CHANNEL_STATS_QUARTERHOUR
{
    UINT                              XTURFECErrors;
    UINT                              XTUCFECErrors;
    UINT                              XTURHECErrors;
    UINT                              XTUCHECErrors;
    UINT                              XTURCRCErrors;
    UINT                              XTUCCRCErrors;
    UINT                              X_RDK_LinkRetrain;
}
DML_XDSL_CHANNEL_STATS_QUARTERHOUR, *PDML_XDSL_CHANNEL_STATS_QUARTERHOUR;

typedef  struct
_DML_XDSL_CHANNEL_STATS_CURRENTDAY
{
    UINT                              XTURFECErrors;
    UINT                              XTUCFECErrors;
    UINT                              XTURHECErrors;
    UINT                              XTUCHECErrors;
    UINT                              XTURCRCErrors;
    UINT                              XTUCCRCErrors;
    UINT                              X_RDK_LinkRetrain;
    UINT                              X_RDK_InitErrors;
    UINT                              X_RDK_InitTimeouts;
    UINT                              X_RDK_SeverelyErroredSecs;
    UINT                              X_RDK_ErroredSecs;
}
DML_XDSL_CHANNEL_STATS_CURRENTDAY, *PDML_XDSL_CHANNEL_STATS_CURRENTDAY;

typedef  struct
_DML_XDSL_CHANNEL_STATS
{
    ULONG                                BytesSent;
    ULONG                                BytesReceived;
    ULONG                                PacketsSent;
    ULONG                                PacketsReceived;
    UINT                                 ErrorsSent;
    UINT                                 ErrorsReceived;
    UINT                                 DiscardPacketsSent;
    UINT                                 DiscardPacketsReceived;
    UINT                                 TotalStart;
    UINT                                 ShowtimeStart;
    UINT                                 LastShowtimeStart;
    UINT                                 CurrentDayStart;
    UINT                                 QuarterHourStart;
    DML_XDSL_CHANNEL_STATS_TIME           stTotal;
    DML_XDSL_CHANNEL_STATS_TIME           stShowTime;
    DML_XDSL_CHANNEL_STATS_TIME           stLastShowTime;
    DML_XDSL_CHANNEL_STATS_CURRENTDAY     stCurrentDay;
    DML_XDSL_CHANNEL_STATS_QUARTERHOUR    stQuarterHour;
}
DML_XDSL_CHANNEL_STATS, *PDML_XDSL_CHANNEL_STATS;

typedef  struct
_DML_XDSL_CHANNEL
{
    BOOL                              Enable;
    ULONG                             ulInstanceNumber;
    INT                               LineIndex;
    UINT                              LastChange;
    CHAR                              LowerLayers[1024];
    CHAR                              Alias[64];
    CHAR                              Name[64];
    DML_XDSL_IF_STATUS                 Status;
    CHAR                              LinkEncapsulationSupported[256];
    CHAR                              LinkEncapsulationUsed[32];
    UINT                              LPATH;
    UINT                              INTLVDEPTH;
    INT                               INTLVBLOCK;
    UINT                              ActualInterleavingDelay;
    INT                               ACTINP;
    BOOL                              INPREPORT;
    INT                               NFEC;
    INT                               RFEC;
    INT                               LSYMB;
    UINT                              UpstreamCurrRate;
    UINT                              DownstreamCurrRate;
    UINT                              ACTNDR;
    UINT                              ACTINPREIN;
    DML_XDSL_CHANNEL_STATS             stChannelStats;
}
DML_XDSL_CHANNEL, *PDML_XDSL_CHANNEL;

/* * Diagnostics */

typedef enum
_DML_XDSL_DIAG_STATE
{
    XDSL_DIAG_STATE_None           = 1,
    XDSL_DIAG_STATE_Requested,
    XDSL_DIAG_STATE_Canceled,
    XDSL_DIAG_STATE_Complete,
    XDSL_DIAG_STATE_Error,
    XDSL_DIAG_STATE_Error_Internal,
    XDSL_DIAG_STATE_Error_Other,
} DML_XDSL_DIAG_STATE;

typedef  struct
_DML_XDSL_DIAG_ADSL_LINE_TEST
{   
    DML_XDSL_DIAG_STATE                DiagnosticsState;
    CHAR                              Interface[256];
    INT                               ACTPSDds;
    INT                               ACTPSDus;
    INT                               ACTATPds;
    INT                               ACTATPus;
    INT                               HLINSCds;
    INT                               HLINSCus;
    UINT                              HLINGds;
    UINT                              HLINGus;
    UINT                              HLOGGds;
    UINT                              HLOGGus;
    CHAR                              HLOGpsds[2560];
    CHAR                              HLOGpsus[2560];
    UINT                              HLOGMTds;
    UINT                              HLOGMTus;
    CHAR                              LATNpbds[24];
    CHAR                              LATNpbus[24];
    CHAR                              SATNds[24];
    CHAR                              SATNus[24];
    CHAR                              HLINpsds[61430];
    CHAR                              HLINpsus[61430];
    UINT                              QLNGds;
    UINT                              QLNGus;
    CHAR                              QLNpsds[61430];
    CHAR                              QLNpsus[61430];
    UINT                              QLNMTds;
    UINT                              QLNMTus;
    UINT                              SNRGds;
    UINT                              SNRGus;
    CHAR                              SNRpsds[61430];
    CHAR                              SNRpsus[61430];
    UINT                              SNRMTds;
    UINT                              SNRMTus;
    CHAR                              BITSpsds[61430];
    CHAR                              BITSpsus[61430];
}
DML_XDSL_DIAG_ADSL_LINE_TEST, *PDML_XDSL_DIAG_ADSL_LINE_TEST;

typedef  struct
_DML_XDSL_DIAG_SELTUER
{
    DML_XDSL_DIAG_STATE                DiagnosticsState;
    CHAR                              Interface[256];
    UINT                              UERMaxMeasurementDuration;
    BOOL                              ExtendedBandwidthOperation;
    CHAR                              UER[4096];
    UINT                              UERScaleFactor;
    UINT                              UERGroupSize;
    CHAR                              UERVar[4096];
}
DML_XDSL_DIAG_SELTUER, *PDML_XDSL_DIAG_SELTUER;

typedef  struct
_DML_XDSL_DIAG_SELTQLN
{
    DML_XDSL_DIAG_STATE                DiagnosticsState;
    CHAR                              Interface[256];
    UINT                              QLNMaxMeasurementDuration;
    BOOL                              ExtendedBandwidthOperation;
    CHAR                              QLN[61430];
    UINT                              QLNGroupSize;
}
DML_XDSL_DIAG_SELTQLN, *PDML_XDSL_DIAG_SELTQLN;

typedef  struct
_DML_XDSL_DIAG_SELTP
{
    DML_XDSL_DIAG_STATE                DiagnosticsState;
    CHAR                              Interface[256];
    BOOL                              CapacityEstimateEnabling;
    CHAR                              CapacitySignalPSD[1024];
    CHAR                              CapacityNoisePSD[1024];
    UINT                              CapacityTargetMargin;
    CHAR                              LoopTermination[24];
    UINT                              LoopLength;
    CHAR                              LoopTopology[256];
    CHAR                              AttenuationCharacteristics[61430];
    BOOL                              MissingFilter;
    UINT                              CapacityEstimate;
}
DML_XDSL_DIAG_SELTP, *PDML_XDSL_DIAG_SELTP;

typedef  struct
_DML_XDSL_DIAGNOSTICS_FULL
{
    DML_XDSL_DIAG_ADSL_LINE_TEST  stDiagADSLLineTest;
    DML_XDSL_DIAG_SELTUER         stDiagSELTUER;
    DML_XDSL_DIAG_SELTQLN         stDiagSELTQLN;
    DML_XDSL_DIAG_SELTP           stDiagSELTP;
}
DML_XDSL_DIAGNOSTICS_FULL, *PDML_XDSL_DIAGNOSTICS_FULL;

typedef  struct
_X_RDK_REPORT_DSL_DEFAULT
{
    UINT                        ReportingPeriod;
    UINT                        OverrideTTL;
}
DML_X_RDK_REPORT_DSL_DEFAULT, *PDML_X_RDK_REPORT_DSL_DEFAULT;

typedef  struct
_DML_X_RDK_REPORT_DSL
{
    BOOL                               Enabled;
    UINT                               ReportingPeriod;
    CHAR                               Schema[1024];
    CHAR                               SchemaID[33];
    BOOL                               bEnableChanged;
    BOOL                               bReportingPeriodChanged;
    PDML_X_RDK_REPORT_DSL_DEFAULT      pDSLDefaultReport;  
}
DML_X_RDK_REPORT_DSL, *PDML_X_RDK_REPORT_DSL;

/* X_RDK_NLNM */

typedef  struct
_DML_XDSL_X_RDK_NLNM
{
    INT           echotonoiseratio;
}
DML_XDSL_X_RDK_NLNM, *PDML_XDSL_X_RDK_NLNM;

/*
    Standard function declaration 
*/
ANSC_STATUS
DmlXdslInit
	(
		ANSC_HANDLE 				hDml,
		PANSC_HANDLE				phContext
	);

ANSC_STATUS
DmlXdslLineInit
	(
		PANSC_HANDLE                phContext
	);

ANSC_STATUS
DmlXdslChannelInit
    (
        PANSC_HANDLE                phContext
    );

INT DmlXdslGetTotalNoofLines( VOID );

ANSC_STATUS DmlXdslGetLineCfg( INT LineIndex, PDML_XDSL_LINE pstLineInfo );

ANSC_STATUS DmlXdslLineSetEnable( INT LineIndex, BOOL Enable );

ANSC_STATUS DmlXdslLineSetDataGatheringEnable( INT LineIndex, BOOL Enable );

ANSC_STATUS DmlXdslLineSetUpstream( INT LineIndex, BOOL Upstream );

ANSC_STATUS DmlXdslLineGetWanStatus( INT LineIndex, DML_XDSL_LINE_WAN_STATUS *wan_state );

ANSC_STATUS DmlXdslLineSetWanStatus( INT LineIndex, DML_XDSL_LINE_WAN_STATUS wan_state );

ANSC_STATUS DmlXdslLineGetLinkStatus( INT LineIndex, DML_XDSL_LINK_STATUS *LinkStatus );

ANSC_STATUS DmlXdslLineGetIndexFromIfName( char *ifname, INT *LineIndex );

ANSC_STATUS DmlXdslDeleteXTMLink( char *ifname );

ANSC_STATUS DmlXdslCreateXTMLink( char *ifname );

ANSC_STATUS DmlXdslLine_GetStandardUsedByGivenIfName(char* ifname, char* StandardUsed);

ANSC_STATUS DmlXdslLine_UpdateStandardUsedByGivenIfName(char* ifname, char* StandardUsed);

ANSC_STATUS DmlXdslLine_GetIfaceTidByGivenIfName(char* ifname, pthread_t* thread_id);

ANSC_STATUS DmlXdslLine_UpdateIfaceTidByGivenIfName(char* ifname, pthread_t new_thread_id);

ANSC_STATUS DmlXdslLineGetCopyOfGlobalInfoForGivenIfName( char *ifname, PDML_XDSL_LINE_GLOBALINFO pGlobalInfo );

ANSC_STATUS DmlXdslLineUpdateLinkStatusAndGetGlobalInfoForGivenIfName( char *ifname, DML_XDSL_LINK_STATUS enLinkStatus, PDML_XDSL_LINE_GLOBALINFO pGlobalInfo );

ANSC_STATUS DmlXdslSetWanLinkStatusForWanManager( char *ifname, char *WanStatus );

ANSC_STATUS DmlXdslSetPhyStatusForWanManager( char *ifname, char *PhyStatus );

INT DmlXdslGetTotalNoofChannels( INT LineIndex );

ANSC_STATUS DmlXdslGetChannelCfg( INT LineIndex, INT ChannelIndex, PDML_XDSL_CHANNEL pstChannelInfo );

ANSC_STATUS DmlXdslChannelSetEnable( INT LineIndex, INT ChannelIndex, BOOL Enable );

ANSC_STATUS DmlGetXdslStandardUsed( char *standard_used);

ANSC_STATUS
DmlXdslDiagnosticsInit
    (
        PANSC_HANDLE                phContext
    );

ANSC_STATUS
DmlXdslReportInit
    (
        PANSC_HANDLE                phContext
    );
ANSC_STATUS

DmlXdslXRdkNlmInit
    (
        PANSC_HANDLE                phContext
    );

ANSC_STATUS DmlXdslGetXRDKNlm( PDML_XDSL_X_RDK_NLNM pstXRdkNlm );
#endif /* _XDSL_APIS_H */
