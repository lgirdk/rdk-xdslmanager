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
#define  IREP_FOLDER_NAME_XTM                       "XTM"
#define  IREP_FOLDER_NAME_PORTTRIGGER               "PORTTRIGGER"
#define  DML_RR_NAME_XTMNextInsNumber               "NextInstanceNumber"
#define  DML_RR_NAME_XTMAlias                       "Alias"
#define  DML_RR_NAME_XTMbNew                        "bNew"

#define  DATAMODEL_XTM_CLASS_CONTENT                                                   \
    /* duplication of the base object class content */                                                \
    BASE_CONTENT                                                                       \
    /* start of XTM object class content */                                                        \
    SLIST_HEADER                    XTMPMappingList;                                        \
    SLIST_HEADER                    Q_XtmList;                                        \
    ULONG                           MaxInstanceNumber;                                      \
    ULONG                           ulPtNextInstanceNumber;                                 \
    ULONG                           PreviousVisitTime;                                      \
    UCHAR                           AliasOfPortMapping[64];                                 \
    ANSC_HANDLE                     hIrepFolderXTM;                                         \
    ANSC_HANDLE                     hIrepFolderXTMPt;                                       \

typedef  struct
_DATAMODEL_XTM
{
    DATAMODEL_XTM_CLASS_CONTENT
}
DATAMODEL_XTM,  *PDATAMODEL_XTM;

/*
*  This struct is for creating entry context link in writable table when call GetEntry()
*/

/**********************************
    Standard function declaration
***********************************/
ANSC_HANDLE
XtmCreate
    (
        VOID
    );

ANSC_STATUS
XtmInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
XtmRemove
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
XtmGen
    (
        ANSC_HANDLE                 hDml
    );

ANSC_STATUS
XtmGenForTriggerEntry
    (
        ANSC_HANDLE                 hDml,
        PDML_XTM      pEntry
    );

#endif
