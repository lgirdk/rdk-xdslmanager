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

#ifndef  _XTM_DML_H
#define  _XTM_DML_H

/***********************************************************************

 APIs for Object:

    Device.PTM.Link.{i}.

    *  PTMLink_GetEntryCount
    *  PTMLink_GetEntry
    *  PTMLink_AddEntry
    *  PTMLink_DelEntry
    *  PTMLink_GetParamBoolValue
    *  PTMLink_GetParamUlongValue
    *  PTMLink_GetParamStringValue
    *  PTMLink_SetParamBoolValue
    *  PTMLink_SetParamUlongValue
    *  PTMLink_SetParamStringValue
    *  PTMLink_Validate
    *  PTMLink_Commit
    *  PTMLink_Rollback

***********************************************************************/

ULONG
PTMLink_GetEntryCount
    (
        ANSC_HANDLE                 hInsContext
    );

ANSC_HANDLE
PTMLink_GetEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG                       nIndex,
        ULONG*                      pInsNumber
    );

ANSC_HANDLE
PTMLink_AddEntry
    (
        ANSC_HANDLE                 hInsContext,
        ULONG*                      pInsNumber
    );

ULONG
PTMLink_DelEntry
    (
        ANSC_HANDLE                 hInsContext,
        ANSC_HANDLE                 hInstance
    );

BOOL
PTMLink_GetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL*                       pBool
    );

BOOL
PTMLink_GetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int*                        pInt
    );

BOOL
PTMLink_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

ULONG
PTMLink_GetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pValue,
        ULONG*                      pUlSize
    );

BOOL
PTMLink_SetParamBoolValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        BOOL                        bValue
    );

BOOL
PTMLink_SetParamIntValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        int                         iValue
    );

BOOL
PTMLink_SetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG                       uValue
    );

BOOL
PTMLink_SetParamStringValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        char*                       pString
    );

BOOL
PTMLink_Validate
    (
        ANSC_HANDLE                 hInsContext,
        char*                       pReturnParamName,
        ULONG*                      puLength
    );

ULONG
PTMLink_Commit
    (
        ANSC_HANDLE                 hInsContext
    );

ULONG
PTMLink_Rollback
    (
        ANSC_HANDLE                 hInsContext
    );

BOOL
PTMLinkStats_GetParamUlongValue
    (
        ANSC_HANDLE                 hInsContext,
        char*                       ParamName,
        ULONG*                      puLong
    );

/***********************************************************************

 APIs for Object:

    Device.ATM.{i}.Link.

    *  ATMLink_GetEntryCount
    *  ATMLink_GetEntry
    *  ATMLink_AddEntry
    *  ATMLink_DelEntry
    *  ATMLink_GetParamBoolValue
    *  ATMLink_GetParamUlongValue
    *  ATMLink_GetParamStringValue
    *  ATMLink_SetParamBoolValue
    *  ATMLink_SetParamUlongValue
    *  ATMLink_SetParamStringValue
    *  ATMLink_Validate
    *  ATMLink_Commit
    *  ATMLink_Rollback

***********************************************************************/

ULONG ATMLink_GetEntryCount ( ANSC_HANDLE hInsContext );
ANSC_HANDLE ATMLink_GetEntry ( ANSC_HANDLE hInsContext, ULONG nIndex, ULONG* pInsNumber );
ANSC_HANDLE ATMLink_AddEntry ( ANSC_HANDLE hInsContext, ULONG* pInsNumber );
ULONG ATMLink_DelEntry ( ANSC_HANDLE hInsContext, ANSC_HANDLE hInstance );
BOOL ATMLink_GetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL* pBool );
BOOL ATMLink_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong );
ULONG ATMLink_GetParamStringValue  ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );
BOOL ATMLink_SetParamBoolValue ( ANSC_HANDLE hInsContext, char* ParamName, BOOL bValue );
BOOL ATMLink_SetParamIntValue( ANSC_HANDLE hInsContext, char* ParamName, int iValue );
BOOL ATMLink_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue );
BOOL ATMLink_SetParamStringValue ( ANSC_HANDLE hInsContext, char*ParamName, char*pString );
BOOL ATMLink_Validate ( ANSC_HANDLE hInsContext, char*pReturnParamName, ULONG *puLength );
ULONG ATMLink_Commit ( ANSC_HANDLE  hInsContext );
ULONG ATMLink_Rollback ( ANSC_HANDLE hInsContext );
BOOL ATMLinkQOS_GetParamUlongValue ( ANSC_HANDLE hInsContext, char *ParamName, ULONG *puLong );
BOOL ATMLinkQOS_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue  );
BOOL ATMLinkStats_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong );
BOOL ATMLinkDiagnostics_GetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG* puLong );
BOOL ATMLinkDiagnostics_SetParamUlongValue ( ANSC_HANDLE hInsContext, char* ParamName, ULONG uValue );
BOOL ATMLinkDiagnostics_SetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char *pString );
ULONG ATMLinkDiagnostics_GetParamStringValue ( ANSC_HANDLE hInsContext, char* ParamName, char* pValue, ULONG* pUlSize );

#endif
