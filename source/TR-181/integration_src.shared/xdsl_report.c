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

#include "ssp_global.h"
#include "stdlib.h"
#include "ccsp_dm_api.h"
#include <uuid/uuid.h>
#include "xdsl_apis.h"
#include "xdsl_report.h"
#include "xdsl_hal.h"

char *rt_schema_buffer = NULL;

static BOOL XdslReportStatus = FALSE;
static ULONG XdslReportReportingPeriod = DEFAULT_REPORTING_INTERVAL;
static ULONG XdslReportDfltReportingPeriod = DEFAULT_REPORTING_INTERVAL;
static ULONG XdslReportOverrideTTL = DEFAULT_OVERRIDE_TTL;
static ULONG CurrentOverrideReportingPeriod = 0;
extern ANSC_HANDLE bus_handle;
char deviceMAC[32] = {'\0'};

static pthread_mutex_t XdslReportMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t XdslReportCond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t device_mac_mutex = PTHREAD_MUTEX_INITIALIZER;
static BOOL rt_schema_file_parsed = FALSE;
static avro_value_iface_t *iface = NULL;
static char ReportSource[] = "XDSL";
static char CPE_TYPE_STRING[] = "Gateway";
static char *macStr = NULL;
static char CpemacStr[32];
static int consoleDebugEnable = 0;

static size_t AvroRTSerializedSize;
static size_t OneAvroRTSerializedSize;
static char AvroRTSerializedBuf[WRITER_BUF_SIZE];

// MD5SUM XdslReport.avsc = 61bd110e2971f6a61f5c2c6ff41b18b1 > version_2

static uint8_t RT_HASH[16] = {0x61,0xbd,0x11,0x0e,0x29,0x71,0xf6,0xa6,
                              0x1f,0x5c,0x2c,0x6f,0xf4,0x1b,0x18,0xb1};

// TODO: Need to check and include correct UUID below
static uint8_t RT_UUID[16] = {0xc3,0x93,0x4a,0xec,0x72,0x3e,0x4c,0x98,
                              0x88,0xb5,0xaa,0x02,0x93,0x1d,0x5d,0xe5};

char * XdslReportSchemaID = "c3934aec-723e-4c98-88b5-aa02931d5de5/61bd110e2971f6a61f5c2c6ff41b18b1";

#define DEFAULT_WAIT_TIME_1_SEC 1

/*
 * Returns the size of Schema contents
 */
int XdslReportGetSchemaBufferSize()
{
    if (rt_schema_buffer)
    {
        return strlen(rt_schema_buffer);
    }
    return 0;
}

/*
 * Returns the schema contents
 */
char *XdslReportGetSchemaBuffer()
{
    CcspTraceInfo(("FUNC %s LINE %d\n", __FUNCTION__, __LINE__));
    if (rt_schema_buffer)
        return rt_schema_buffer;

    CcspTraceInfo(("FUNC %s LINE %d\n", __FUNCTION__, __LINE__));
    return NULL;
}

/*
 * Returns the schemaID size
 */
int XdslReportGetSchemaIDBufferSize()
{
    if (XdslReportGetSchemaIDBuffer())
        return strlen(XdslReportSchemaID);
    return 0;
}

/*
 * Returns the schemaID
 */
char *XdslReportGetSchemaIDBuffer()
{
    return XdslReportSchemaID;
}

/*
 * Checks value of Reporting Period.
 * Returns:     TRUE if ReportingPeriod is valid
 *              FALSE if ReportingPeriod is not valid
 */
bool XdslReportValidateReportingPeriod(UINT value)
{
    int arr[] = {0, 1, 5, 15, 30, 60, 300, 900, 1800, 3600, 10800, 21600, 43200, 86400};
    int i = 0;
    for (i = 0; i < (sizeof(arr) / sizeof(int)); i++)
    {
        if (arr[i] == value)
        {
            return true;
        }
    }
    return false;
}

/*
 * Retuns the XDSL Report Status
 * Returns:     TRUE if XDSL Reporting is Enabled
 *              FALSE if XDSL Reporting is Disabled
 */
BOOL XdslReportGetStatus()
{
    return XdslReportStatus;
}

/**
 * @brief Retrieve xDSL data and pack the report structure.
 * This API communicates with HAL layer to retrieve different statistics information
 * and filled with incoming structure object.
 */
static ANSC_STATUS XdslPrepareReportData(int line_id, int TotalChannels, XdslReportData *stReportData)
{
    int rc = ANSC_STATUS_SUCCESS;

    if (NULL == stReportData)
    {
        CcspTraceError(("%s Invalid Memory\n", __FUNCTION__));
        return ANSC_STATUS_FAILURE;
    }
    /**
     * Get xDSL Line Information
     */
    DML_XDSL_LINE stLineInfo;
    memset(&stLineInfo, 0, sizeof(stLineInfo));
    rc = xdsl_hal_dslGetLineInfo(line_id, &stLineInfo);
    if (rc == ANSC_STATUS_SUCCESS)
    {

        strncpy(stReportData->StandardUsed, stLineInfo.StandardUsed, sizeof(stReportData->StandardUsed) - 1);
        strncpy(stReportData->XTSUsed, stLineInfo.XTSUsed, sizeof(stReportData->XTSUsed) - 1);
        stReportData->DownstreamAttenuation = stLineInfo.DownstreamAttenuation;
        stReportData->DownstreamMaxBitRate = stLineInfo.DownstreamMaxBitRate;
        stReportData->DownstreamNoiseMargin = stLineInfo.DownstreamNoiseMargin;
        stReportData->DownstreamPower = stLineInfo.DownstreamPower;
        stReportData->UpstreamAttenuation = stLineInfo.UpstreamAttenuation;
        stReportData->UpstreamMaxBitRate = stLineInfo.UpstreamMaxBitRate;
        stReportData->UpstreamNoiseMargin = stLineInfo.UpstreamNoiseMargin;
        stReportData->UpstreamPower = stLineInfo.UpstreamPower;
        strncpy(stReportData->AllowedProfiles, stLineInfo.AllowedProfiles, sizeof(stReportData->AllowedProfiles) - 1);
        strncpy(stReportData->CurrentProfile, stLineInfo.CurrentProfile, sizeof(stReportData->CurrentProfile) - 1);

        stReportData->Upstream = DmlXdslLineGetUpstream(line_id);
    }
    else
    {
        CcspTraceError(("%s Failed to get xDSL Line information \n", __FUNCTION__));
    }

    /**
     * Get DSL line statistics information.
     */
    DML_XDSL_LINE_STATS stLineStats;
    memset(&stLineStats, 0, sizeof(stLineStats));
    rc = xdsl_hal_dslGetLineStats(line_id, &stLineStats);
    if (rc == ANSC_STATUS_SUCCESS)
    {
        stReportData->TotalStart = stLineStats.TotalStart;
        stReportData->QuarterHourStart = stLineStats.QuarterHourStart;
        stReportData->CurrentDayErroredSecs = stLineStats.stCurrentDay.ErroredSecs;
        stReportData->CurrentDaySeverelyErroredSecs = stLineStats.stCurrentDay.SeverelyErroredSecs;
    }
    else
    {
        CcspTraceError(("%s Failed to get xDSL line statistics information \n", __FUNCTION__));
    }

    /**
     * Get DSL line test params information.
     */
    DML_XDSL_LINE_TESTPARAMS stLineTestParams;
    memset(&stLineTestParams, 0, sizeof(stLineTestParams));
    rc = xdsl_hal_dslGetLineTestParams(line_id, &stLineTestParams);
    if (rc == ANSC_STATUS_SUCCESS)
    {
        stReportData->HLOGGus = stLineTestParams.HLOGGus;
    }
    else
    {
        CcspTraceError(("%s Failed to get xDSL line test params information \n", __FUNCTION__));
    }

    /**
     * Get xDSL channel information.
     */
    DML_XDSL_CHANNEL stChannelInfo;
    memset(&stChannelInfo, 0, sizeof(stChannelInfo));
    
    for (INT ChannelIdx = 0; ChannelIdx < TotalChannels; ChannelIdx++ )
    {
        rc = xdsl_hal_dslGetChannelInfo((ChannelIdx + 1), &stChannelInfo);
        if ( (rc == ANSC_STATUS_SUCCESS) && (stChannelInfo.Status == XDSL_IF_STATUS_Up))
        {
            stReportData->DownstreamCurrRate = stChannelInfo.DownstreamCurrRate;
            stReportData->UpstreamCurrRate = stChannelInfo.UpstreamCurrRate;
        }
        else
        {
            if (rc != ANSC_STATUS_SUCCESS)
                CcspTraceError(("%s Failed to get xDSL channel information \n", __FUNCTION__));
             continue;
        }

        /**
         * Get DSL Channel Statistics information.
         */
        DML_XDSL_CHANNEL_STATS stChannelStats;
        memset(&stChannelStats, 0, sizeof(stChannelStats));
        rc = xdsl_hal_dslGetChannelStats((ChannelIdx + 1), &stChannelStats);
        if (rc == ANSC_STATUS_SUCCESS)
        {
            stReportData->CurrentDayStart = stChannelStats.CurrentDayStart;
            stReportData->QuarterHourXTUCCRCErrors = stChannelStats.stQuarterHour.XTUCCRCErrors;
            stReportData->QuarterHourXTURCRCErrors = stChannelStats.stQuarterHour.XTURCRCErrors;
            stReportData->CurrentDayXTUCCRCErrors = stChannelStats.stCurrentDay.XTUCCRCErrors;
            stReportData->CurrentDayXTUCFECErrors = stChannelStats.stCurrentDay.XTUCFECErrors;
            stReportData->CurrentDayXTUCHECErrors = stChannelStats.stCurrentDay.XTUCHECErrors;
            stReportData->CurrentDayXTURFECErrors = stChannelStats.stCurrentDay.XTURFECErrors;
            stReportData->CurrentDayXTURHECErrors = stChannelStats.stCurrentDay.XTURHECErrors;
            stReportData->CurrentDayXTURCRCErrors = stChannelStats.stCurrentDay.XTURCRCErrors;
            stReportData->CurrentDayLinkRetrain = stChannelStats.stCurrentDay.X_RDK_LinkRetrain;
            stReportData->QuarterHourLinkRetrain = stChannelStats.stQuarterHour.X_RDK_LinkRetrain;
        }
        else
        {
            CcspTraceError(("%s Failed to get xDSL statistics information \n", __FUNCTION__));
        }
    }
    /**
     * Get DSL nlm information.
     */
    DML_XDSL_X_RDK_NLNM   stXRdkNlm;
    memset(&stXRdkNlm, 0, sizeof(stXRdkNlm));
    rc = xdsl_hal_dslGetXRdk_Nlm( &stXRdkNlm );
    if (rc == ANSC_STATUS_SUCCESS)
    {
        stReportData->EchotoNoiseRatio = stXRdkNlm.echotonoiseratio;
    }
    else
    {
        CcspTraceError(("%s Failed to get DSL Nlm information \n", __FUNCTION__));
    }

    return ANSC_STATUS_SUCCESS;
}

/*
 * Function to prepare Avro Writer
 */
static avro_writer_t prepare_rt_writer()
{
    avro_writer_t writer = NULL;
    long lsSize = 0;

    CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));

    CcspTraceInfo(("Avro prepares to serialize data\n"));

    if (rt_schema_file_parsed == FALSE)
    {
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
        FILE *fp;

        /* open schema file */
        fp = fopen(XDSL_AVRO_SCHEMA_FILE, "rb");
        if (!fp)
        {
            CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
            perror(XDSL_AVRO_SCHEMA_FILE " doesn't exist.");
            CcspTraceInfo(("%s file does'nt exist\n", XDSL_AVRO_SCHEMA_FILE));
        }
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));

        /* seek through file and get file size*/
        fseek(fp, 0L, SEEK_END);
        lsSize = ftell(fp);
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));

        /*back to the start of the file*/
        rewind(fp);

        /* allocate memory for entire content */
        rt_schema_buffer = calloc(1, lsSize + 1);
        memset(rt_schema_buffer, 0, lsSize + 1);
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));

        if (!rt_schema_buffer)
        {
            fclose(fp);
            fputs("memory alloc fails", stderr);
            CcspTraceInfo(("Unable to allocate memory\n"));
            return NULL;
        }
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));

        /* copy the file into the RT_schema_buffer */
        if (1 != fread(rt_schema_buffer, lsSize, 1, fp))
        {
            fclose(fp);
            CcspTraceInfo(("XDSL REPORT %s : LINE %d rt_schema_buffer = %p\n", __FUNCTION__, __LINE__, rt_schema_buffer));
            free(rt_schema_buffer);
            fputs("entire read fails", stderr);
            CcspTraceInfo(("fread() failed\n"));
            return NULL;
        }

        CcspTraceInfo(("XDSL REPORT %s : LINE %d rt_schema_buffer = %p\n", __FUNCTION__, __LINE__, rt_schema_buffer));
        fclose(fp);

        //schemas
        avro_schema_error_t error = NULL;

        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
        //Master report/datum
        avro_schema_t xdsl_report_schema = NULL;
        CcspTraceInfo(("XDSL REPORT %s : LINE %d rt_schema_buffer - %s\n", __FUNCTION__, __LINE__, rt_schema_buffer));
        avro_schema_from_json(rt_schema_buffer, strlen(rt_schema_buffer),
                              &xdsl_report_schema, &error);

        if (CHK_AVRO_ERR)
        {
            CcspTraceInfo(("%s %s : %d avro_schema_from_json fail:\n", avro_strerror(), __func__, __LINE__));
            return NULL;
        }
        CcspTraceInfo(("XDSL REPORT %s : LINE %d xdsl_report_schema = %p\n", __FUNCTION__, __LINE__, xdsl_report_schema));

        //generate an avro class from our schema and get a pointer to the value interface
        iface = avro_generic_class_from_schema(xdsl_report_schema);
        if (CHK_AVRO_ERR)
        {
            CcspTraceInfo(("%s iface = %p\n", avro_strerror(), iface));
            return NULL;
        }
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
        avro_schema_decref(xdsl_report_schema);
        CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
        rt_schema_file_parsed = TRUE; // parse schema file once only
        CcspTraceInfo(("Read Avro schema file ONCE, lsSize = %ld, RT_schema_buffer = 0x%lx.\n", lsSize + 1, (ulong)rt_schema_buffer));
    }
    else
        CcspTraceInfo(("Stored lsSize = %ld, pRT_schema_buffer = 0x%lx.\n", lsSize + 1, (ulong)rt_schema_buffer));

    CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
    memset(&AvroRTSerializedBuf[0], 0, sizeof(AvroRTSerializedBuf));

    AvroRTSerializedBuf[0] = MAGIC_NUMBER; /* fill MAGIC number */
    memcpy(&AvroRTSerializedBuf[MAGIC_NUMBER_SIZE], RT_UUID, sizeof(RT_UUID));
    memcpy(&AvroRTSerializedBuf[MAGIC_NUMBER_SIZE + sizeof(RT_UUID)], RT_HASH, sizeof(RT_HASH));
    CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
    writer = avro_writer_memory((char *)&AvroRTSerializedBuf[MAGIC_NUMBER_SIZE + SCHEMA_ID_LENGTH],
                                sizeof(AvroRTSerializedBuf) - MAGIC_NUMBER_SIZE - SCHEMA_ID_LENGTH);

    CcspTraceInfo(("XDSL REPORT %s : LINE %d \n", __FUNCTION__, __LINE__));
    CcspTraceInfo(("XDSL REPORT %s : EXIT \n", __FUNCTION__));

    return writer;
}

/*
 * XDSL Data Packing Function
 */
static int harvester_report_Xdsl(XdslReportData *head)
{
    int k = 0;
    avro_writer_t writer;
    uint8_t *b64buffer = NULL;
    char *serviceName = "XDSL";
    char *dest = "event:raw.kestrel.reports.XdslReport";
    char *contentType = "avro/binary"; // contentType "application/json", "avro/binary"
    size_t decodesize = 0;
    char trans_id[37];
    uuid_t transaction_id;
    XdslReportData *ptr = head;

    CcspTraceInfo(("XDSL REPORT %s : ENTER \n", __FUNCTION__));

    // goes thru total number of elements in link list
    writer = prepare_rt_writer();

    if (writer == NULL)
        return 1;

    //Reset out writer
    avro_writer_reset(writer);

    avro_value_t adr = {0};
    avro_value_t adrField = {0};
    avro_value_t optional = {0};

    avro_generic_value_new(iface, &adr);
    if (CHK_AVRO_ERR)
    {
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
        return 1;
    }
    CcspTraceInfo(("XDSL Report\tType: %d\n", avro_value_get_type(&adr)));

    // header block
    // timestamp
    avro_value_get_by_name(&adr, "header", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));
    avro_value_get_by_name(&adrField, "timestamp", &adrField, NULL);
    avro_value_set_branch(&adrField, 1, &optional);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d \n", avro_strerror(), __LINE__));

    struct timeval ts;
    gettimeofday(&ts, NULL);

    uint64_t tstamp_av_main = ((uint64_t)(ts.tv_sec) * 1000000) + (uint64_t)ts.tv_usec;
    tstamp_av_main = tstamp_av_main / 1000; //timestamp in ms

    avro_value_set_long(&optional, tstamp_av_main);
    CcspTraceInfo(("timestamp = %llu\n", tstamp_av_main));
    CcspTraceInfo(("timestamp\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));

    // uuid
    uuid_generate_random(transaction_id);
    uuid_unparse(transaction_id, trans_id);

    avro_value_get_by_name(&adr, "header", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s __LINE__\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "uuid", &adrField, NULL);
    avro_value_set_branch(&adrField, 1, &optional);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_fixed(&optional, transaction_id, 16);
    unsigned char *ptxn = (unsigned char *)transaction_id;
    CcspTraceInfo(("uuid = 0x%02X, 0x%02X ... 0x%02X, 0x%02X\n", ptxn[0], ptxn[1], ptxn[14], ptxn[15]));
    CcspTraceInfo(("uuid\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d \n", avro_strerror(), __LINE__));

    //source
    avro_value_get_by_name(&adr, "header", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d \n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "source", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 0, &optional);
    avro_value_set_string(&optional, ReportSource);
    CcspTraceInfo(("source = \"%s\"\n", ReportSource));
    CcspTraceInfo(("source\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));

    //cpe_id block
    // MacAddress
    if (macStr == NULL)
    {
        macStr = getDeviceMac();

        strncpy(CpemacStr, macStr, sizeof(CpemacStr));
        CcspTraceInfo(("Received DeviceMac from Atom side: %s\n", macStr));
    }

    char CpeMacHoldingBuf[20] = {0};
    unsigned char CpeMacid[7] = {0};
    for (k = 0; k < 6; k++)
    {
        /* copy 2 bytes */
        CpeMacHoldingBuf[k * 2] = CpemacStr[k * 2];
        CpeMacHoldingBuf[k * 2 + 1] = CpemacStr[k * 2 + 1];
        CpeMacid[k] = (unsigned char)strtol(&CpeMacHoldingBuf[k * 2], NULL, 16);
    }
    avro_value_get_by_name(&adr, "cpe_id", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "mac_address", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_fixed(&optional, CpeMacid, 6);
    unsigned char *pMac = (unsigned char *)CpeMacid;
    CcspTraceInfo(("mac_address = 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X\n", pMac[0], pMac[1], pMac[2], pMac[3], pMac[4], pMac[5]));
    CcspTraceInfo(("mac_address\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // cpe_type
    avro_value_get_by_name(&adr, "cpe_id", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "cpe_type", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d \n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 0, &optional);
    avro_value_set_string(&optional, CPE_TYPE_STRING);
    CcspTraceInfo(("cpe_type = \"%s\"\n", CPE_TYPE_STRING));
    CcspTraceInfo(("cpe_type\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));

    //Data Field block
    // XTSUsed
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "XTSUsed", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_string(&optional, ptr->XTSUsed);
    CcspTraceInfo(("XTSUsed\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // DownstreamCurrRate
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "DownstreamCurrRate", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->DownstreamCurrRate);
    CcspTraceInfo(("DownstreamCurrRate\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // UpstreamCurrRate
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "UpstreamCurrRate", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->UpstreamCurrRate);
    CcspTraceInfo(("UpstreamCurrRate\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayStart
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayStart", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->CurrentDayStart);
    CcspTraceInfo(("CurentDayStart\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // QuarterHourXTUCCRCErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "QuarterHourXTUCCRCErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->QuarterHourXTUCCRCErrors);
    CcspTraceInfo(("QuarterHourXTUCCRCErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTURCRCErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTURCRCErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTURCRCErrors);
    CcspTraceInfo(("CurrentDayXTURCRCErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // QuarterHourXTURCRCErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "QuarterHourXTURCRCErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->QuarterHourXTURCRCErrors);
    CcspTraceInfo(("QuarterHourXTURCRCErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // StandardUsed - string
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "StandardUsed", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_string(&optional, ptr->StandardUsed);
    CcspTraceInfo(("StandardUsed\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // DownstreamAttenuation
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "DownstreamAttenuation", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->DownstreamAttenuation);
    CcspTraceInfo(("DownstreamAttenuation\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // DownstreamMaxBitRate
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "DownstreamMaxBitRate", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->DownstreamMaxBitRate);
    CcspTraceInfo(("DownstreamMaxBitRate\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // DownstreamNoiseMargin
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "DownstreamNoiseMargin", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->DownstreamNoiseMargin);
    CcspTraceInfo(("DownstreamNoiseMargin\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // DownstreamPower
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "DownstreamPower", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->DownstreamPower);
    CcspTraceInfo(("DownstreamPower\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTUCCRCErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTUCCRCErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTUCCRCErrors);
    CcspTraceInfo(("CurrentDayXTUCCRCErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTUCFECErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTUCFECErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTUCFECErrors);
    CcspTraceInfo(("CurrentDayXTUCFECErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTUCHECErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTUCHECErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTUCHECErrors);
    CcspTraceInfo(("CurrentDayXTUCHECErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayErroredSecs
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayErroredSecs", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayErroredSecs);
    CcspTraceInfo(("CurrentDayErroredSecs\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTURFECErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTURFECErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTURFECErrors);
    CcspTraceInfo(("CurrentDayXTURFECErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayXTURHECErrors
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayXTURHECErrors", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDayXTURHECErrors);
    CcspTraceInfo(("CurrentDayXTURHECErrors\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDaySeverelyErroredSecs
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDaySeverelyErroredSecs", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_int(&optional, ptr->CurrentDaySeverelyErroredSecs);
    CcspTraceInfo(("CurrentDaySeverelyErroredSecs\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // UpstreamAttenuation
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "UpstreamAttenuation", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->UpstreamAttenuation);
    CcspTraceInfo(("UpstreamAttenuation\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // UpstreamMaxBitRate
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "UpstreamMaxBitRate", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->UpstreamMaxBitRate);
    CcspTraceInfo(("UpstreamMaxBitRate\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // UpstreamNoiseMargin
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "UpstreamNoiseMargin", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->UpstreamNoiseMargin);
    CcspTraceInfo(("UpstreamNoiseMargin\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // UpstreamPower
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "UpstreamPower", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_double(&optional, (double)ptr->UpstreamPower);
    CcspTraceInfo(("UpstreamPower\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // Upstream
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
    if ( CHK_AVRO_ERR ) CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "Upstream", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_boolean(&optional, ptr->Upstream);
    CcspTraceInfo(("Upstream\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // TotalStart
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "TotalStart", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->TotalStart);
    CcspTraceInfo(("TotalStart\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // QuarterHourStart
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "QuarterHourStart", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->QuarterHourStart);
    CcspTraceInfo(("QuarterHourStart\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // AllowedProfiles
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "AllowedProfiles", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_string(&optional, ptr->AllowedProfiles);
    CcspTraceInfo(("AllowedProfiles\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentProfile - string
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentProfile", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_string(&optional, ptr->CurrentProfile);
    CcspTraceInfo(("CurrentProfile\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // CurrentDayLinkRetrain
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "CurrentDayLinkRetrain", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->CurrentDayLinkRetrain);
    CcspTraceInfo(("CurrentDayLinkRetrain\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // QuarterHourLinkRetrain
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "QuarterHourLinkRetrain", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->QuarterHourLinkRetrain);
    CcspTraceInfo(("QuarterHourLinkRetrain\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));

    // EchotoNoiseRatio
    avro_value_get_by_name(&adr, "data", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_get_by_name(&adrField, "EchotoNoiseRatio", &adrField, NULL);
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s LINE %d\n", avro_strerror(), __LINE__));
    avro_value_set_branch(&adrField, 1, &optional);
    avro_value_set_long(&optional, (long)ptr->EchotoNoiseRatio);
    CcspTraceInfo(("EchotoNoiseRatio\tType: %d\n", avro_value_get_type(&optional)));
    if (CHK_AVRO_ERR)
        CcspTraceInfo(("%s\n", avro_strerror()));


    /* check for writer size, if buffer is almost full, skip trailing linklist */
    avro_value_sizeof(&adr, &AvroRTSerializedSize);
    OneAvroRTSerializedSize = (OneAvroRTSerializedSize == 0) ? AvroRTSerializedSize : OneAvroRTSerializedSize;

    if ((WRITER_BUF_SIZE - AvroRTSerializedSize) < OneAvroRTSerializedSize)
    {
        CcspTraceWarning(("AVRO write buffer is almost full, size = %d func %s, exit!\n", (int)AvroRTSerializedSize, __FUNCTION__));
    }

    //Thats the end of that
    avro_value_write(writer, &adr);

    avro_value_sizeof(&adr, &AvroRTSerializedSize);
    AvroRTSerializedSize += MAGIC_NUMBER_SIZE + SCHEMA_ID_LENGTH;
    CcspTraceInfo(("Serialized writer size %d\n", (int)AvroRTSerializedSize));

    //Free up memory
    avro_value_decref(&adr);

    CcspTraceInfo(("XDSL REPORT %s : LINE %d free()\n", __FUNCTION__, __LINE__));
    avro_writer_free(writer);

    // b64 encoding
    decodesize = b64_get_encoded_buffer_size(AvroRTSerializedSize);
    b64buffer = malloc(decodesize * sizeof(uint8_t));
    if (b64buffer != NULL)
    {
        b64_encode((uint8_t *)AvroRTSerializedBuf, AvroRTSerializedSize, b64buffer);

        if (consoleDebugEnable)
        {
            fprintf(stderr, "\nAVro serialized data\n");
            for (k = 0; k < (int)AvroRTSerializedSize; k++)
            {
                char buf[30];
                if ((k % 32) == 0)
                    fprintf(stderr, "\n");
                sprintf(buf, "%02X", (unsigned char)AvroRTSerializedBuf[k]);
                fprintf(stderr, "%c%c", buf[0], buf[1]);
            }

            fprintf(stderr, "\n\nB64 data\n");
            for (k = 0; k < (int)decodesize; k++)
            {
                if ((k % 32) == 0)
                    fprintf(stderr, "\n");
                fprintf(stderr, "%c", b64buffer[k]);
            }
            fprintf(stderr, "\n\n");
        }
        free(b64buffer);
    }

    CcspTraceInfo(("Before RT WebPA SEND message call\n"));
    CcspTraceInfo(("serviceName: %s\n", serviceName));
    CcspTraceInfo(("dest: %s\n", dest));
    CcspTraceInfo(("trans_id: %s\n", trans_id));
    CcspTraceInfo(("contentType: %s\n", contentType));
    CcspTraceInfo(("AvroRTSerializedBuf: %s\n", AvroRTSerializedBuf));
    CcspTraceInfo(("AvroRTSerializedSize: %d\n", (int)AvroRTSerializedSize));
    // Send data from XDSL REPORT to webpa using CCSP bus interface
    sendWebpaMsg(serviceName, dest, trans_id, contentType, AvroRTSerializedBuf, AvroRTSerializedSize);
    CcspTraceInfo(("%s report sent to Webpa, Destination=%s, Transaction-Id=%s  \n", XDSL_REPORT_NAME, dest, trans_id));

    CcspTraceInfo(("After RT WebPA SEND message call\n"));

    CcspTraceInfo(("XDSL REPORT %s : LINE %d free()\n", __FUNCTION__, __LINE__));

    CcspTraceInfo(("XDSL REPORT %s : EXIT \n", __FUNCTION__));
    return 0;
}

/*
 * Avroe cleanup function
 */
static void rt_avro_cleanup()
{
    if (rt_schema_buffer != NULL)
    {
        CcspTraceInfo(("XDSL REPORT %s : LINE %d free()\n", __FUNCTION__, __LINE__));
        free(rt_schema_buffer);
        rt_schema_buffer = NULL;
    }
    if (iface != NULL)
    {
        avro_value_iface_decref(iface);
        iface = NULL;
    }
    rt_schema_file_parsed = FALSE;
}

/*
 * Function to wait for ReportingPeriod
 */
static void WaitForPthreadConditionTimeout(ULONG waitingPeriod)
{
    struct timespec _ts;
    struct timespec _now;
    int n;

    memset(&_ts, 0, sizeof(struct timespec));
    memset(&_now, 0, sizeof(struct timespec));

    pthread_mutex_lock(&XdslReportMutex);

    clock_gettime(CLOCK_REALTIME, &_now);
    _ts.tv_sec = _now.tv_sec + waitingPeriod;
    CcspTraceInfo(("XDSL REPORT %s : Waiting for %d sec\n", __FUNCTION__, waitingPeriod));

    n = pthread_cond_timedwait(&XdslReportCond, &XdslReportMutex, &_ts);
    if (n == 0)
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_timedwait SIGNALLED OK!!!\n", __FUNCTION__));
    }
    else if (n != ETIMEDOUT)
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_timedwait ERROR(%d)!!!. %s \n", __FUNCTION__, n, strerror(n)));
    }

    pthread_mutex_unlock(&XdslReportMutex);
}

static int PrepareAndSendXdslReport()
{
    int ret = 0;
    int line_id = 0;
    XdslReportData ptr;
    DML_XDSL_LINE_GLOBALINFO   stGlobalInfo   = { 0 };
    INT iTotalLines = 0;
    INT iTotalChannels = 0;

    iTotalLines = DmlXdslGetTotalNoofLines( );
    if (iTotalLines > 1)
    {
        AnscTraceError(("%s-%d: Error, Report support only one Line Info, but TotalLines=%d \n", __FUNCTION__, __LINE__, iTotalLines));
        return ANSC_STATUS_FAILURE;
    }

    iTotalChannels = DmlXdslGetTotalNoofChannels( iTotalLines );
    if (iTotalChannels > 2)
    {
        AnscTraceError(("%s-%d: Error, Report support only Two Channel per Line Info, but TotalChannel=%d \n", __FUNCTION__, __LINE__, iTotalChannels));
        return ANSC_STATUS_FAILURE;
    }

    AnscTraceInfo(("%s-%d: TotalLines=%d, TotalChannels=%d \n", __FUNCTION__, __LINE__, iTotalLines, iTotalChannels));
    memset(&stGlobalInfo, 0, sizeof(stGlobalInfo));
    DmlXdslLineGetCopyOfGlobalInfoForGivenIndex( (iTotalLines - 1), &stGlobalInfo );

    line_id = iTotalLines;
    if( stGlobalInfo.LinkStatus == XDSL_LINK_STATUS_Up ){
        memset(&ptr, 0, sizeof(XdslReportData));

        ret = XdslPrepareReportData(line_id, iTotalChannels, &ptr);
        if (ret)
        {
            CcspTraceWarning(("XdslReportGetData returned error [%d] \n", ret));
        }

        ret = harvester_report_Xdsl(&ptr);
        if (ret)
        {
            CcspTraceWarning(("harvester_report_Xdsl returned error [%d] \n", ret));
        }
        return ret;
    }
    else{
        CcspTraceInfo(("Dsl Link is down, not sending xdsl report \n"));
        return ret;
    }
}

/*
 *  XDSL Report Thread Function
 */
static void *StartXdslReporting()
{
    CcspTraceInfo(("XDSL REPORT %s ENTER \n", __FUNCTION__));
    CcspTraceWarning(("XDSL REPORT %s Starting Thread to start XDSL Data Harvesting  \n", __FUNCTION__));
    int ret = 0;
    ULONG uDftOverrideTTL = 0;
    ULONG uDftReportingPeriod = 0;
    ULONG uOverrideReportingPeriod = 0;
    ULONG waitingTimePeriod = DEFAULT_WAIT_TIME_1_SEC;

    while (!ret && XdslReportGetStatus())
    {
        uDftOverrideTTL = XdslReportGetDefaultOverrideTTL();
        uDftReportingPeriod = XdslReportGetDefaultReportingPeriod();
        uOverrideReportingPeriod = XdslReportGetReportingPeriod();
        if (uDftOverrideTTL != 0)
        {
            if (uOverrideReportingPeriod == 0)
            {
                waitingTimePeriod = uDftOverrideTTL;
                XdslReportSetDefaultOverrideTTL(0);
            }
            else
            {
                if (CurrentOverrideReportingPeriod >= uDftOverrideTTL)
                {
                    CurrentOverrideReportingPeriod = 0;
                    XdslReportSetDefaultOverrideTTL(0);
                    // Do not wait here to send the report in Default ReportingPeriod.
                    waitingTimePeriod = 0; 
                }
                else
                {
                    CurrentOverrideReportingPeriod = CurrentOverrideReportingPeriod + uOverrideReportingPeriod;
                    CcspTraceInfo(("CurrentOverrideReportingPeriod[%ld]\n", CurrentOverrideReportingPeriod));
                    ret = PrepareAndSendXdslReport();
		    
		    /*
		     * Needs to get current reporting period. Because in between if there any changes happen then
                     * it would have not updated after report 
		     */
		    uOverrideReportingPeriod = XdslReportGetReportingPeriod();
                    waitingTimePeriod = uOverrideReportingPeriod;
                }
            }
        }
        else
        {
            if (uDftReportingPeriod != 0)
            {
                ret = PrepareAndSendXdslReport();
                waitingTimePeriod = uDftReportingPeriod;

		//We need to overwrite Waitingtime when overrideTTL value set case.
		//Otherwise it will take previous default reporting time instead of overritten period
		uDftOverrideTTL = XdslReportGetDefaultOverrideTTL();
		if(uDftOverrideTTL != 0)
		{
		   waitingTimePeriod = DEFAULT_WAIT_TIME_1_SEC;
		}
            }
            else
            {
                waitingTimePeriod = DEFAULT_WAIT_TIME_1_SEC; 
            }
        }
        WaitForPthreadConditionTimeout(waitingTimePeriod);
    }

    XdslReportSetStatus(FALSE);
    rt_avro_cleanup();
    CcspTraceInfo(("XDSL REPORT %s EXIT \n", __FUNCTION__));
    CcspTraceInfo(("XDSL REPORT %s Stopped Thread for XDSL Data Harvesting  \n", __FUNCTION__));

    return NULL; // shouldn't return;
}

/*
 *  XDSL Report Thread Function
 */
void * XdslReportingThread(void *args)
{
    int ret = 0;
    ret = ParodusClientInit();
    if (ret == ANSC_STATUS_SUCCESS) 
    {
        StartXdslReporting();
    }
    return NULL;
}

/*
 * Sets the XDSL Reporting Status
 */
int XdslReportSetStatus(BOOL status)
{
    CcspTraceInfo(("XDSL REPORT %s ENTER\n", __FUNCTION__));
    CcspTraceInfo(("XDSL REPORT %s  Old[%d] New[%d] \n", __FUNCTION__, XdslReportGetStatus, status));
    if (XdslReportStatus != status)
        XdslReportStatus = status;
    else
        return 0;


    if (XdslReportStatus)
    {
        pthread_t tid;
        CcspTraceInfo(("XDSL REPORT %s Starting Thread to start XDSL Data Harvesting  \n", __FUNCTION__));

        if (pthread_create(&tid, NULL, XdslReportingThread, NULL))
        {
            CcspTraceWarning(("XDSL REPORT %s : Failed to Start Thread to start XDSL Harvesting  \n", __FUNCTION__));
            return ANSC_STATUS_FAILURE;
        }
    }
    else
    {
        int ret;
        pthread_mutex_lock(&XdslReportMutex);
        // Send signal to unblock thread that uses conditional variable XdslReportCond
        ret = pthread_cond_signal(&XdslReportCond);
        pthread_mutex_unlock(&XdslReportMutex);
        if (ret == 0)
        {
            CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal success\n", __FUNCTION__));
        }
        else
        {
            CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal fail\n", __FUNCTION__));
        }
    }
    CcspTraceInfo(("XDSL REPORT %s : EXIT \n", __FUNCTION__));

    return 0;
}

/*
 * Returns the XDSL Temporary Report Reporting Period - used till TTL is expired
 */
ULONG XdslReportGetReportingPeriod()
{
    return XdslReportReportingPeriod;
}

/*
 * Sets the XDSL Temporary Report Reporting Period - used till TTL is expired
 */
int XdslReportSetReportingPeriod(ULONG interval)
{
    int ret;
    XdslReportReportingPeriod = interval;
    //Neeeds to reset this counter value when we configure new value to avoid TTL 0 after lower value than previous value
    CurrentOverrideReportingPeriod = 0;
    pthread_mutex_lock(&XdslReportMutex);
    // Send signal to unblock thread that uses conditional variable XdslReportCond
    ret = pthread_cond_signal(&XdslReportCond);
    pthread_mutex_unlock(&XdslReportMutex);
    if (ret == 0)
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal success\n", __FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal fail\n", __FUNCTION__));
    }
    return 0;
}

/*
 * Returns the XDSL Default Report Reporting Period - Used after TTL is expired
 */
ULONG XdslReportGetDefaultReportingPeriod()
{
    return XdslReportDfltReportingPeriod;
}

/*
 * Sets the XDSL Default Report Reporting Period - Used after TTL is expired
 */
int XdslReportSetDefaultReportingPeriod(ULONG interval)
{
    int ret;
    XdslReportDfltReportingPeriod = interval;
    pthread_mutex_lock(&XdslReportMutex);
    // Send signal to unblock thread that uses conditional variable XdslReportCond
    ret = pthread_cond_signal(&XdslReportCond);
    pthread_mutex_unlock(&XdslReportMutex);
    if (ret == 0)
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal success\n", __FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal fail\n", __FUNCTION__));
    }
    return 0;
}

/*
 * Returns the XDSL Report Overrride TTL
 */
ULONG XdslReportGetDefaultOverrideTTL()
{
    return XdslReportOverrideTTL;
}

/*
 * Sets the XDSL Report Overrride TTL
 */
int XdslReportSetDefaultOverrideTTL(ULONG interval)
{
    int ret;
    XdslReportOverrideTTL = interval;
    //Neeeds to reset this counter value when we configure new value to avoid TTL 0 after lower value than previous value
    CurrentOverrideReportingPeriod = 0;
    pthread_mutex_lock(&XdslReportMutex);
    // Send signal to unblock thread that uses conditional variable XdslReportCond
    ret = pthread_cond_signal(&XdslReportCond);
    pthread_mutex_unlock(&XdslReportMutex);
    if (ret == 0)
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal success\n", __FUNCTION__));
    }
    else
    {
        CcspTraceInfo(("XDSL REPORT %s : pthread_cond_signal fail\n", __FUNCTION__));
    }
    return 0;
}

int _syscmd(char *cmd, char *retBuf, int retBufSize)
{
    CcspTraceInfo((" XDSL REPORT %s ENTER\n", __FUNCTION__));

    FILE *f;
    char *ptr = retBuf;
    int bufSize = retBufSize, bufbytes = 0, readbytes = 0;

    if ((f = popen(cmd, "r")) == NULL)
    {
        CcspTraceInfo((" XDSL REPORT %s : popen %s error\n", __FUNCTION__, cmd));
        return -1;
    }

    while (!feof(f))
    {
        *ptr = 0;
        if (bufSize >= 128)
        {
            bufbytes = 128;
        }
        else
        {
            bufbytes = bufSize - 1;
        }

        fgets(ptr, bufbytes, f);
        readbytes = strlen(ptr);
        if (readbytes == 0)
            break;
        bufSize -= readbytes;
        ptr += readbytes;
    }
    pclose(f);
    retBuf[retBufSize - 1] = 0;

    CcspTraceInfo((" XDSL REPORT %s EXIT\n", __FUNCTION__));

    return 0;
}

void macToLower(char macValue[])
{
    CcspTraceInfo((" XDSL REPORT %s ENTER\n", __FUNCTION__));

    int i = 0;
    int j;
    char *token[32];
    char tmp[32];
    strncpy(tmp, macValue, sizeof(tmp) - 1);
    token[i] = strtok(tmp, ":");
    if (token[i] != NULL)
    {
        strncpy(deviceMAC, token[i], sizeof(deviceMAC) - 1);
        deviceMAC[31] = '\0';
        i++;
    }
    while ((token[i] = strtok(NULL, ":")) != NULL)
    {
        strncat(deviceMAC, token[i], sizeof(deviceMAC) - 1);
        deviceMAC[31] = '\0';
        i++;
    }
    deviceMAC[31] = '\0';
    for (j = 0; deviceMAC[j]; j++)
    {
        deviceMAC[j] = tolower(deviceMAC[j]);
    }

    CcspTraceInfo((" Inside macToLower:: Device MAC: %s check\n", deviceMAC));
    CcspTraceInfo((" XDSL REPORT %s EXIT\n", __FUNCTION__));
}

char *getDeviceMac()
{
    CcspTraceInfo((" XDSL REPORT %s ENTER\n", __FUNCTION__));

    while (!strlen(deviceMAC))
    {
        pthread_mutex_lock(&device_mac_mutex);
        int ret = -1, val_size = 0, fd = 0;
        parameterValStruct_t **parameterval = NULL;

        unsigned int token;
        char deviceMACValue[32] = {'\0'};

        if (strlen(deviceMAC))
        {
            pthread_mutex_unlock(&device_mac_mutex);
            break;
        }

        fd = s_sysevent_connect(&token);
        if (CCSP_SUCCESS == sysevent_get(fd, token, "eth_wan_mac", deviceMACValue, sizeof(deviceMACValue)) == 0 && deviceMACValue[0] != '\0')
        {
            macToLower(deviceMACValue);
            CcspTraceInfo(("deviceMAC is %s\n", deviceMAC));
        }
        pthread_mutex_unlock(&device_mac_mutex);
    }

    CcspTraceInfo((" XDSL REPORT %s EXIT\n", __FUNCTION__));

    return deviceMAC;
}

