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

#ifndef _XDSL_REPORT_H
#define _XDSL_REPORT_H

#include <avro.h>

#define XDSL_AVRO_SCHEMA_FILE "/usr/ccsp/harvester/XdslReport.avsc"
#define DEFAULT_REPORTING_INTERVAL 0
#define DEFAULT_OVERRIDE_TTL 0
#define CHK_AVRO_ERR (strlen(avro_strerror()) > 0)
#define WRITER_BUF_SIZE  (1024 * 30) // 30K
#define MAGIC_NUMBER_SIZE 1
#define MAGIC_NUMBER      0x8a
#define SCHEMA_ID_LENGTH  32
#define MAX_PARAMETERNAME_LEN   512
#define RDKB_ETHAGENT_COMPONENT_NAME                  "com.cisco.spvtg.ccsp.ethagent"
#define ETH_WAN_STATUS_PARAM "Device.Ethernet.X_RDKCENTRAL-COM_WAN.Enabled"
#define RDKB_ETHAGENT_DBUS_PATH                       "/com/cisco/spvtg/ccsp/ethagent"
#define CCSP_AGENT_WEBPA_SUBSYSTEM         "eRT."
#define DEVICE_PROPS_FILE  "/etc/device.properties"
#define XDSL_REPORT_NAME "VDSLTelemetryDiagnostics"

typedef struct _XdslReportData
{
        struct timeval timestamp;
        char    XTSUsed[128];
        UINT    DownstreamCurrRate;
        UINT    UpstreamCurrRate;
        UINT    CurrentDayStart;
        int     QuarterHourXTUCCRCErrors;
        int     CurrentDayXTURCRCErrors;
        int     QuarterHourXTURCRCErrors;
        char    StandardUsed[64];
        int  DownstreamAttenuation;
        UINT    DownstreamMaxBitRate;
        int  DownstreamNoiseMargin;
        int  DownstreamPower;
        int     CurrentDayXTUCCRCErrors;
        int     CurrentDayXTUCFECErrors;
        int     CurrentDayXTUCHECErrors;
        int     CurrentDayErroredSecs;
        int     CurrentDayXTURFECErrors;
        int     CurrentDayXTURHECErrors;
        int     CurrentDaySeverelyErroredSecs;
        int  UpstreamAttenuation;
        UINT    UpstreamMaxBitRate;
        int  UpstreamNoiseMargin;
        int  UpstreamPower;
        BOOL    Upstream;
        UINT    TotalStart;
        UINT    QuarterHourStart;
        char    AllowedProfiles[256];
        char    CurrentProfile[64];
        UINT    HLOGGus;

}XdslReportData;


/**
 * @brief API to return size of the buffer contains schema contents
 * @return length of buffer
 */
int XdslReportGetSchemaBufferSize();

/**
 * @brief API to return string contains avro schema contents
 * @return string contains schema contents
 */
char *XdslReportGetSchemaBuffer();

/**
 * @brief API to return size of schemaID
 * @return length of schemaID buffer
 */
int XdslReportGetSchemaIDBufferSize();

/**
 * @brief API to return string contains schema id
 * @return string contains schema id
 */
char *XdslReportGetSchemaIDBuffer();

/**
 * @brief API to check value of Reporting Period.
 * @param value - indicates reporting period
 * @return: TRUE if ReportingPeriod is valid FALSE if ReportingPeriod is not valid
 */
bool XdslReportValidateReportingPeriod(UINT value);

/**
 * @brief API to return the XDSL Report Status
 * @return TRUE if XDSL Reporting is Enabled FALSE if XDSL Reporting is Disabled
 */
BOOL XdslReportGetStatus();

/**
 * @brief API to set the XDSL Reporting Status
 * @param status - Boolean flag indicate to set/reset
 * @return 0 if executed successfully
 */
int XdslReportSetStatus(BOOL status);

/**
 * @brief API to return the XDSL Report Reporting Period
 * @return Return reporting period
 */
ULONG XdslReportGetReportingPeriod();

/**
 * @brief API to set reporting time period
 * @param interval - reporting time peirod value
 * @return 0 if executed successfully
 */
int XdslReportSetReportingPeriod(ULONG interval);

/**
 * @brief Returns MacAdress of Device
 */
char * getDeviceMac();

/**
 * @brief API to get TTL value
 * @return TTL value
 */
ULONG XdslReportGetDefaultOverrideTTL();

/**
 * @brief API to get Default Reporting Period value
 * @return the default Reporting Period
 */
ULONG XdslReportGetDefaultReportingPeriod();
#endif
