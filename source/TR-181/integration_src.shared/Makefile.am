##########################################################################
# If not stated otherwise in this file or this component's Licenses.txt
# file the following copyright and licenses apply:
#
# Copyright 2019 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
##########################################################################
AM_CFLAGS =

ACLOCAL_AMFLAGS = -I m4

noinst_LTLIBRARIES = libXdslManagerintegration_src_shared.la

libXdslManagerintegration_src_shared_la_CPPFLAGS = \
    -I$(top_srcdir)/source/TR-181/board_sbapi \
    -I$(top_srcdir)/source/TR-181/middle_layer_src \
    -I$(top_srcdir)/source/TR-181/include \
    -I$(top_srcdir)/source/Custom \
    -I$(top_srcdir)/source/RdkXdslManager

libXdslManagerintegration_src_shared_la_SOURCES = xdsl_apis.c xdsl_manager.c xdsl_hal.c xdsl_report.c xdsl_report_webpa_intf.c xtm_apis.c

libXdslManagerintegration_src_shared_la_LDFLAGS = -lccsp_common -lcm_mgnt -lhal_platform -lsyscfg -lsysevent -ljson_hal_client -ljson-c -lavro -luuid -ltrower-base64 -llibparodus -lutapi -lutctx -lpthread
