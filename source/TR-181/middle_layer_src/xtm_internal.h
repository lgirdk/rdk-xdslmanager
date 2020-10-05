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

#ifndef  _XTM_INTERNAL_H
#define  _XTM_INTERNAL_H

#include "cosa_apis.h"
/***********************************
    Actual definition declaration
************************************/
#define  IREP_FOLDER_NAME_PTM                       "PTM"
#define  IREP_FOLDER_NAME_PORTTRIGGER               "PORTTRIGGER"
#define  DML_RR_NAME_XTMNextInsNumber               "NextInstanceNumber"
#define  DML_RR_NAME_PTMAlias                       "Alias"
#define  DML_RR_NAME_PTMbNew                        "bNew"

#define  IREP_FOLDER_NAME_ATM                       "ATM"

#define  DATAMODEL_PTM_CLASS_CONTENT                                                   \
    /* duplication of the base object class content */                                                \
    BASE_CONTENT                                                                       \
    /* start of PTM object class content */                                                        \
    SLIST_HEADER                    PTMPMappingList;                                        \
    SLIST_HEADER                    Q_PtmList;                                        \
    ULONG                           MaxInstanceNumber;                                      \
    ULONG                           ulPtNextInstanceNumber;                                 \
    ULONG                           PreviousVisitTime;                                      \
    UCHAR                           AliasOfPortMapping[64];                                 \
    ANSC_HANDLE                     hIrepFolderPTM;                                         \
    ANSC_HANDLE                     hIrepFolderPTMPt;                                       \

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
    SLIST_HEADER                    ATMPMappingList;                                   \
    SLIST_HEADER                    Q_AtmList;                                         \
    ULONG                           MaxInstanceNumber;                                 \
    ULONG                           ulPtNextInstanceNumber;                            \
    ULONG                           PreviousVisitTime;                                 \
    UCHAR                           AliasOfPortMapping[64];                            \
    ANSC_HANDLE                     hIrepFolderATM;                                    \
    ANSC_HANDLE                     hIrepFolderATMPt;                                  \
    PDML_ATM_DIAG                   pATMDiag;                                          \

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
ANSC_STATUS PtmGen ( ANSC_HANDLE hDml );
ANSC_STATUS PtmGenForTriggerEntry ( ANSC_HANDLE hDml, PDML_PTM pEntry );
ANSC_HANDLE AtmCreate ( VOID );
ANSC_STATUS AtmInitialize ( ANSC_HANDLE hThisObject );
ANSC_STATUS AtmRemove ( ANSC_HANDLE hThisObject );
ANSC_STATUS AtmGen ( ANSC_HANDLE hDml );
ANSC_STATUS AtmGenForTriggerEntry ( ANSC_HANDLE hDml, PDML_ATM pEntry );
#endif
