/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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

#include <mqueue.h>

#include "xdsl_event_queue.h"


#define XDSL_EVENT_QUEUE_NAME  "/DSL_event_queue"
#define MAX_QUEUE_MSG_SIZE         ( 2048 )
#define MAX_QUEUE_LENGTH           ( 100 )
#define EVENT_MSG_MAX_SIZE         ( 1024 )

#define XDSL_LINE_ID "Device.DSL.Line.%d"

#ifdef FEATURE_RDKB_LED_MANAGER
#define SYSEVENT_LED_STATE "led_event"
#define WAN_IP4_UP                     "ipv4_up"
#define WAN_IP6_UP                     "ipv6_up" 
#define DSL_TRAINING_STATE             "rdkb_dsl_training"
#endif

extern int sysevent_fd;

typedef enum
_XDSL_MSGQ_MSG_TYPE
{
    MSG_TYPE_LINE_LINK_STATUS       = 1,
    MSG_TYPE_XTM_LINK_STATUS        = 2,

} XDSL_MSGQ_MSG_TYPE;

typedef struct _XDSLEventQData
{
    XDSL_MSGQ_MSG_TYPE     MsgType;
    char                       Msg[EVENT_MSG_MAX_SIZE]; //Msg structure for the specific event
}DSLEventQData;


#define CHECK(x) \
    do { \
        if (!(x)) { \
            CcspTraceError(("%s:%d: ", __FUNCTION__, __LINE__)); \
            perror(#x); \
            return; \
        } \
    } while (0) \

/* *DmlXdslEventHandlerThread() */
static void *DmlXdslEventHandlerThread( void *arg )
{
  mqd_t     mq;
  struct    mq_attr attr;

  /* initialize the queue attributes */
  attr.mq_flags   = 0;
  attr.mq_maxmsg  = MAX_QUEUE_LENGTH;
  attr.mq_msgsize = sizeof(DSLEventQData);
  attr.mq_curmsgs = 0;

  /* create the message queue */
  mq = mq_open(XDSL_EVENT_QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

  CHECK((mqd_t)-1 != mq);

  do
  {
    DSLEventQData    EventMsg = { 0 };

    /* receive the message */
    CHECK(mq_receive(mq, (char *)&EventMsg, sizeof(DSLEventQData), NULL) >= 0);

    switch (EventMsg.MsgType)
    {
    case MSG_TYPE_LINE_LINK_STATUS:
	{
	    XDSLMSGQLineStatusData *MSGQLineStatusData = (XDSLMSGQLineStatusData *)EventMsg.Msg;

#if defined(FEATURE_RDKB_LED_MANAGER) && !defined(_SR300_PRODUCT_REQ_)
	    if (MSGQLineStatusData->LinkStatus == XDSL_LINK_STATUS_Initializing ||
		MSGQLineStatusData->LinkStatus == XDSL_LINK_STATUS_EstablishingLink)
	    {
		char ledStatus[32] = { 0 };

		sysevent_get(sysevent_fd, sysevent_token, SYSEVENT_LED_STATE, ledStatus, sizeof(ledStatus));
		if(!((strncmp(ledStatus, WANIP4_UP, sizeof(ledStatus)) == 0) || (strncmp(ledStatus, WAN_IP6_UP, sizeof(ledStatus)) == 0)))
		{
		    sysevent_set(sysevent_fd, sysevent_token, SYSEVENT_LED_STATE, DSL_TRAINING_STATE, 0);
		}
	    }
#endif
	    if (MSGQLineStatusData->LinkStatus == LINK_STATUS_UP)
	    {
		DmlXdslCreateXTMLink(MSGQLineStatusData->line_id);
	    }
	    else
	    {
		INT LinkStatus;

		if (DmlXdslGetLineLinkStatus(MSGQLineStatusData->line_id, &LinkStatus) == ANSC_STATUS_FAILURE ||
		    LinkStatus == LINK_STATUS_UP)
		{
                    DmlXdslDeleteXTMLink(MSGQLineStatusData->line_id);
		}
	    }
	    DmlXdslSetLineLinkStatus(MSGQLineStatusData->line_id, MSGQLineStatusData->LinkStatus);
	    break;
	}
    case MSG_TYPE_XTM_LINK_STATUS:
	{
	    XDSLMSGQXtmStatusData *MSGQXtmStatusData = (XDSLMSGQXtmStatusData *)EventMsg.Msg;
	    ULONG ulInstanceNumber;
	    char xtmLowerLayers[256] = { 0 };
	    int line_id = 0;
	    DML_XDSL_LINE_TYPE line_type;

	    if (MSGQXtmStatusData->is_atm == TRUE)
	    {
		PDML_ATM p_Atm = ATMLink_GetEntry(NULL, MSGQXtmStatusData->link_id - 1, &ulInstanceNumber);

		if (p_Atm == NULL)
		{
		    CcspTraceError(("%s Failed to get ATM entry with index %d \n", __FUNCTION__, MSGQXtmStatusData->link_id));
		    break;
		}
		DmlAtmLinkSetStatus(p_Atm, MSGQXtmStatusData->Status);
		if (ATMLink_GetParamStringValue(p_Atm, "LowerLayers", xtmLowerLayers, NULL))
		{
		    CcspTraceError(("%s %d Failed to get ATM LowerLayers value\n", __FUNCTION__, __LINE__));
		    break;
		}
	    }
	    else
	    {
		PDML_PTM p_Ptm = PTMLink_GetEntry(NULL, MSGQXtmStatusData->link_id - 1, &ulInstanceNumber);

		if (p_Ptm == NULL)
		{
		    CcspTraceError(("%s Failed to get PTM entry with index %d \n", __FUNCTION__, MSGQXtmStatusData->link_id));
		    break;
		}
		DmlPtmLinkSetStatus(p_Ptm, MSGQXtmStatusData->Status);
		if (PTMLink_GetParamStringValue(p_Ptm, "LowerLayers", xtmLowerLayers, NULL))
		{
		    CcspTraceError(("%s %d Failed to get PTM LowerLayers value\n", __FUNCTION__, __LINE__));
		    break;
		}
	    }

	    sscanf(xtmLowerLayers, XDSL_LINE_ID, &line_id);
	    if (DmlXdslLineTypeGetById( line_id, &line_type ) == ANSC_STATUS_FAILURE)
	    {
		CcspTraceError(("%s %d Failed to get DSL Line type\n", __FUNCTION__, __LINE__));
		break;
	    }
	    if (MSGQXtmStatusData->is_atm == TRUE && line_type == DML_XDSL_LINE_ADSL ||
		MSGQXtmStatusData->is_atm == FALSE && line_type == DML_XDSL_LINE_VDSL)
	    {
	        DmlXdslSetLinkStatusForWanManager( xtmLowerLayers, MSGQXtmStatusData->Status == Up ? "Up" : "Down" );
	    }
	    break;
	}
    }

  }while(1);

  //exit from thread
  pthread_exit(NULL);

}

/* DmlXdslTriggerEventHandlerThread() */
void DmlXdslTriggerEventHandlerThread( void )
{
    pthread_t EvtThreadId;
    int iErrorCode = 0;

    //DSL event handler thread
    iErrorCode = pthread_create( &EvtThreadId, NULL, &DmlXdslEventHandlerThread, NULL );

    if( 0 != iErrorCode )
    {
        CcspTraceInfo(("%s %d - Failed to start Event Handler Thread EC:%d\n", __FUNCTION__, __LINE__, iErrorCode ));
    }
    else
    {
        CcspTraceInfo(("%s %d - Event Handler Thread Started Successfully\n", __FUNCTION__, __LINE__ ));
    }
}

static ANSC_STATUS SetEventToEventQueue(DSLEventQData *EventMsg, int msg_len)
{
    mqd_t mq;
	
    mq = mq_open(XDSL_EVENT_QUEUE_NAME, O_WRONLY);
    CHECK((mqd_t)-1 != mq);

    CHECK(0 <= mq_send(mq, EventMsg, msg_len, 0));
    CHECK((mqd_t)-1 != mq_close(mq));

    CcspTraceInfo(("%s %d - Successfully sent message to event queue\n", __FUNCTION__, __LINE__));

    return ANSC_STATUS_SUCCESS;
}

/* *DmlXdslLineSendLinkStatusToEventQueue() */
ANSC_STATUS DmlXdslLineSendLinkStatusToEventQueue( XDSLMSGQLineStatusData *MSGQLineStatusData )
{
    DSLEventQData    EventMsg = { 0 };

    //Validate buffer
    if( NULL == MSGQLineStatusData )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__, __LINE__));\
        return ANSC_STATUS_FAILURE;
    }

    EventMsg.MsgType = MSG_TYPE_LINE_LINK_STATUS;
    memcpy(EventMsg.Msg,MSGQLineStatusData,sizeof(XDSLMSGQLineStatusData));

    return SetEventToEventQueue(&EventMsg, sizeof(XDSLMSGQLineStatusData) + sizeof(XDSL_MSGQ_MSG_TYPE));
}

ANSC_STATUS DmlXtmLinkSendStatusToEventQueue( XDSLMSGQXtmStatusData *MSGQXtmStatusData )
{
    DSLEventQData    EventMsg = { 0 };

    //Validate buffer
    if( NULL == MSGQXtmStatusData )
    {
        CcspTraceError(("%s %d Invalid Buffer\n", __FUNCTION__, __LINE__));\
        return ANSC_STATUS_FAILURE;
    }

    EventMsg.MsgType = MSG_TYPE_XTM_LINK_STATUS;
    memcpy(EventMsg.Msg,MSGQXtmStatusData,sizeof(XDSLMSGQXtmStatusData));

    return SetEventToEventQueue(&EventMsg, sizeof(XDSLMSGQXtmStatusData) + sizeof(XDSL_MSGQ_MSG_TYPE));
}
