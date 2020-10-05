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

#ifndef  _XDSL_INTERNAL_H
#define  _XDSL_INTERNAL_H

#include "cosa_apis.h"
#include "plugin_main_apis.h"
#include "xdsl_apis.h"

/* Collection */

#define  DATAMODEL_XDSL_CLASS_CONTENT                                                  \
    /* duplication of the base object class content */                                      			\
    BASE_CONTENT                                                                       \
    /* start of DSL object class content */                                                			\    
    UINT             			        ulTotalNoofDSLLines;                                       \
    PDML_XDSL_LINE                  pXDSLLine;                                                   \
    UINT                                ulTotalNoofDSLChannels;                                       \
    PDML_XDSL_CHANNEL               pDSLChannel;                                                   \
    PDML_XDSL_DIAGNOSTICS_FULL      pDSLDiag;                                                   \
    PDML_X_RDK_REPORT_DSL           pDSLReport;                                                   \
    PDML_XDSL_X_RDK_NLNM            pDSLXRdkNlm;

typedef  struct
_DATAMODEL_DSL                                               
{
    DATAMODEL_XDSL_CLASS_CONTENT
}
DATAMODEL_DSL,  *PDATAMODEL_XDSL;

/*
    Standard function declaration 
*/
ANSC_HANDLE
XdslCreate
    (
        VOID
    );

ANSC_STATUS
XdslInitialize
    (
        ANSC_HANDLE                 hThisObject
    );

ANSC_STATUS
XdslRemove
    (
        ANSC_HANDLE                 hThisObject
    );
#endif 
