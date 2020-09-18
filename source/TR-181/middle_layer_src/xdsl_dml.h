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


/**************************************************************************

    module: xdsl_dml.h

        For COSA Data Model Library Development

    -------------------------------------------------------------------

    description:

        This file defines the apis for objects to support Data Model Library.

    -------------------------------------------------------------------


    author:

        COSA XML TOOL CODE GENERATOR 1.0

    -------------------------------------------------------------------

    revision:

        13/08/2020    initial revision.

**************************************************************************/


#ifndef  _COSA_XDSL_DML_H
#define  _COSA_XDSL_DML_H

/***********************************************************************

 APIs for Object:

    DSL.Line.{i}

    *  Line_IsUpdated
    *  Line_Synchronize
    *  Line_GetEntryCount
    *  Line_GetEntry
    *  Line_GetParamBoolValue
    *  Line_GetParamUlongValue
    *  Line_GetParamStringValue
    *  Line_SetParamBoolValue
    *  Line_GetParamIntValue
    *  Line_SetParamUlongValue
    *  Line_SetParamStringValue
    *  Line_Validate
    *  Line_Commit
    *  Line_Rollback

***********************************************************************/
BOOL
Line_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Line_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Line_GetEntryCount
	(
		ANSC_HANDLE
	);

ANSC_HANDLE
Line_GetEntry
	(
		ANSC_HANDLE 				hInsContext,
		ULONG						nIndex,
		ULONG*						pInsNumber
	);

BOOL
Line_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );
	
ULONG
Line_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
Line_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
Line_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );	

BOOL
Line_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      pUlong
    );

BOOL
Line_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValuepUlong
    );

BOOL
Line_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    );

BOOL
Line_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
Line_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Line_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
LineStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
LineStatsTotal_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
LineStatsShowtime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
LineStatsLastShowtime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
LineStatsCurrentDay_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
LineStatsQuarterHour_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );
/***********************************************************************

 APIs for Object:

    DSL.Channel.{i}

    *  Channel_IsUpdated
    *  Channel_Synchronize
    *  Channel_GetEntryCount
    *  Channel_GetEntry
    *  Channel_GetParamBoolValue
    *  Channel_GetParamUlongValue
    *  Channel_GetParamStringValue
    *  Channel_GetParamIntValue
    *  Channel_SetParamBoolValue
    *  Channel_SetParamStringValue
    *  Channel_Validate
    *  Channel_Commit
    *  Channel_Rollback

***********************************************************************/
BOOL
Channel_IsUpdated
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Channel_Synchronize
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Channel_GetEntryCount
    (
        ANSC_HANDLE
    );

ANSC_HANDLE
Channel_GetEntry
    (   
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

BOOL
Channel_GetParamBoolValue
    (   
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );
   
ULONG
Channel_GetParamStringValue
    (   
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
Channel_SetParamBoolValue
    (   
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
Channel_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
Channel_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      pUlong
    );

BOOL
Channel_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    );

BOOL
Channel_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
Channel_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
Channel_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
ChannelStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
ChannelStatsTotal_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
ChannelStatsShowTime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
ChannelStatsLastShowTime_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
ChannelStatsCurrentDay_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );
    
BOOL
ChannelStatsQuarterHour_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.ADSLLineTest.

    *  ADSLLineTest_GetParamUlongValue
    *  ADSLLineTest_GetParamStringValue
    *  ADSLLineTest_SetParamStringValue
    *  ADSLLineTest_GetParamIntValue

***********************************************************************/
BOOL
ADSLLineTest_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
ADSLLineTest_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

BOOL
ADSLLineTest_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

ULONG
ADSLLineTest_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
ADSLLineTest_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    );

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTUER.

    *  SELTUER_GetParamUlongValue
    *  SELTUER_GetParamStringValue
    *  SELTUER_SetParamStringValue
    *  SELTUER_GetParamIntValue
    *  SELTUER_GetParamBoolValue

***********************************************************************/
BOOL
SELTUER_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
SELTUER_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

ULONG
SELTUER_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
SELTUER_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
SELTUER_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTQLN.

    *  SELTQLN_GetParamUlongValue
    *  SELTQLN_SetParamUlongValue
    *  SELTQLN_GetParamStringValue
    *  SELTQLN_SetParamStringValue
    *  SELTQLN_GetParamIntValue
    *  SELTQLN_GetParamBoolValue

***********************************************************************/
BOOL
SELTQLN_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
SELTQLN_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

BOOL
SELTQLN_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

ULONG
SELTQLN_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
SELTQLN_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

/***********************************************************************

 APIs for Object:

    Device.DSL.Diagnostics.SELTP.

    *  SELTP_GetParamUlongValue
    *  SELTP_SetParamUlongValue
    *  SELTP_GetParamStringValue
    *  SELTP_SetParamStringValue
    *  SELTP_GetParamBoolValue
    *  SELTP_SetParamBoolValue

***********************************************************************/
ULONG
SELTP_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
SELTP_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
SELTP_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
SELTP_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
SELTP_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
SELTP_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );


/***********************************************************************

 APIs for Object:

    Device.DSL.X_RDK_Report.DSL

    *  X_RDK_Report_DSL_GetParamUlongValue
    *  X_RDK_Report_DSL_SetParamUlongValue
    *  X_RDK_Report_DSL_GetParamStringValue
    *  X_RDK_Report_DSL_GetParamBoolValue
    *  X_RDK_Report_DSL_SetParamBoolValue
    *  X_RDK_Report_DSL_Commit
    *  X_RDK_Report_DSL_Validate

***********************************************************************/

BOOL
X_RDK_Report_DSL_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
X_RDK_Report_DSL_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

ULONG
X_RDK_Report_DSL_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
X_RDK_Report_DSL_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
X_RDK_Report_DSL_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
X_RDK_Report_DSL_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
X_RDK_Report_DSL_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
X_RDK_Report_DSL_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );



/***********************************************************************

 APIs for Object:

    Device.DSL.X_RDK_Report.DSL

    *  X_RDK_Report_DSL_Default_GetParamUlongValue
    *  X_RDK_Report_DSL_Default_SetParamUlongValue

***********************************************************************/

BOOL
X_RDK_Report_DSL_Default_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

BOOL
X_RDK_Report_DSL_Default_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );


ANSC_STATUS GetNVRamULONGConfiguration(char* setting, ULONG* value);
ANSC_STATUS SetNVRamULONGConfiguration (char * setting, ULONG value);

/***********************************************************************
 APIs for Object:
    Device.DSL.X_RDK_NLNM.

    *  X_RDK_NLNM_GetParamIntValue

***********************************************************************/

BOOL
X_RDK_NLNM_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    );
#endif /* _COSA_XDSL_DML_H */

