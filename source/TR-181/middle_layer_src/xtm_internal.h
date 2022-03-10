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

#ifndef  _XTM_INTERNAL_H
#define  _XTM_INTERNAL_H

#include "cosa_apis.h"
/***********************************
    Actual definition declaration
************************************/
#define PSM_ADSL_LINKTYPE           "dmsb.xdslmanager.atm.%d.linktype"    
#define PSM_ADSL_ENCAPSULATION      "dmsb.xdslmanager.atm.%d.encapsulation"
#define PSM_ADSL_AUTOCONFIG         "dmsb.xdslmanager.atm.%d.autoconfig"
#define PSM_ADSL_PVC                "dmsb.xdslmanager.atm.%d.pvc"
#define PSM_ADSL_AAL                "dmsb.xdslmanager.atm.%d.aal"
#define PSM_ADSL_FCSPRESERVED       "dmsb.xdslmanager.atm.%d.fcspreserved"
#define PSM_ADSL_VCSEARCHLIST       "dmsb.xdslmanager.atm.%d.vcsearchlist"
#define PSM_ADSL_QOS_CLASS          "dmsb.xdslmanager.atm.%d.qos.class"
#define PSM_ADSL_QOS_PEAKCELLRATE   "dmsb.xdslmanager.atm.%d.qos.peakcellrate"
#define PSM_ADSL_QOS_MAXBURSTSIZE   "dmsb.xdslmanager.atm.%d.qos.maxburstsize"
#define PSM_ADSL_QOS_CELLRATE       "dmsb.xdslmanager.atm.%d.qos.cellrate"

extern char                g_Subsystem[32];
extern ANSC_HANDLE         bus_handle;

#define _PSM_WRITE_PARAM(_PARAM_NAME) { \
    _ansc_sprintf(param_name, _PARAM_NAME); \
    retPsmSet = PSM_Set_Record_Value2(bus_handle,g_Subsystem, param_name, ccsp_string, param_value); \
    if (retPsmSet != CCSP_SUCCESS) { \
        AnscTraceFlow(("%s Error %d writing %s %s\n", __FUNCTION__, retPsmSet, param_name, param_value));\
    } \
    else \
    { \
        /*AnscTraceFlow(("%s: retPsmSet == CCSP_SUCCESS writing %s = %s \n", __FUNCTION__,param_name,param_value)); */\
    } \
    _ansc_memset(param_name, 0, sizeof(param_name)); \
}

#define  DATAMODEL_PTM_CLASS_CONTENT                                                   \
    /* duplication of the base object class content */                                                \
    BASE_CONTENT                                                                       \
    /* start of PTM object class content */                                                        \
    ULONG                           ulPtmLinkNumberOfEntries;                               \
    PDML_PTM                        PtmLink;                                                \

typedef  struct
_DATAMODEL_PTM
{
    DATAMODEL_PTM_CLASS_CONTENT
}
DATAMODEL_PTM,  *PDATAMODEL_PTM;
#define  DATAMODEL_ATM_CLASS_CONTENT                                                   \
    /* duplication of the base object class content */                                 \
    BASE_CONTENT                                                                       \
    /* start of PTM object class content */                                            \
    PDML_ATM_DIAG                   pATMDiag;                                          \
    ULONG                           ulAtmLinkNumberOfEntries;                          \
    PDML_ATM                        AtmLink;                                           \

typedef  struct
_DATAMODEL_ATM
{
    DATAMODEL_ATM_CLASS_CONTENT
} DATAMODEL_ATM,  *PDATAMODEL_ATM;

/*
*  This struct is for creating entry context link in writable table when call GetEntry()
*/

/**********************************
    Standard function declaration
***********************************/
ANSC_HANDLE PtmCreate ( VOID );
ANSC_STATUS PtmInitialize ( ANSC_HANDLE hThisObject );
ANSC_STATUS PtmRemove ( ANSC_HANDLE hThisObject );
ANSC_HANDLE AtmCreate ( VOID );
ANSC_STATUS AtmInitialize ( ANSC_HANDLE hThisObject );
ANSC_STATUS AtmRemove ( ANSC_HANDLE hThisObject );
#endif
