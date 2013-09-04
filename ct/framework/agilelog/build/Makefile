############################################################
#     (c)2011-2012 Broadcom Corporation
#
#  This program is the proprietary software of Broadcom Corporation and/or its licensors,
#  and may only be used, duplicated, modified or distributed pursuant to the terms and
#  conditions of a separate, written license agreement executed between you and Broadcom
#  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
#  no license (express or implied), right to use, or waiver of any kind with respect to the
#  Software, and Broadcom expressly reserves all rights in and to the Software and all
#  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
#  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
#  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
#
#  Except as expressly set forth in the Authorized License,
#
#  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
#  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
#  and to use this information only in connection with your use of Broadcom integrated circuit products.
#
#  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
#  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
#  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
#  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
#  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
#  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
#  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
#  USE OR PERFORMANCE OF THE SOFTWARE.
#
#  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
#  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
#  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
#  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
#  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
#  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
#  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
#  ANY LIMITED REMEDY.
#
# Module Description:
#   Makefile for Jsonrpc.
#
#
############################################################
##include ../../../build/common.inc

SOURCE_DIR  := $(shell cd .. && pwd)
BUILD_DIR   := $(SOURCE_DIR)/build

CXX         := g++
CC          := gcc

MODULE := agilelog

# Only support shared library.
LIB_NAME := lib$(MODULE).so

LIB_INC_PATH := $(SOURCE_DIR)/inc $(SOURCE_DIR)/inc/pub $(SOURCE_DIR)/external/tinyxml2

LIB_SRCS := agilelog.cpp \
agilelog_inf.cpp \
hashTable.cpp \
tinyxml2.cpp


ifndef BINDIR
BINDIR ?= ./bin
$(shell mkdir -p $(BINDIR))
endif

ifndef OBJDIR
OBJDIR ?= ./obj
$(shell mkdir -p $(OBJDIR))
endif

#-DANDROID -DANDROID_NDK
#-Werror
CPPFLAGS += -g -fpermissive
CPPFLAGS += -Wno-unused-parameter -Werror -Wno-missing-field-initializers
CPPFLAGS += $(addprefix -I,$(LIB_INC_PATH))


CFLAGS += -g -fpermissive
CFLAGS += -Wno-unused-parameter -Wno-missing-field-initializers
CFLAGS += $(addprefix -I,$(LIB_INC_PATH))

LIB_OBJS=$(patsubst %.cpp, $(OBJDIR)/%.o, $(LIB_SRCS))

LDFLAGS += -lc -L$(BINDIR) -lagilelog

.PHONY: all clean

all: $(BINDIR)/$(LIB_NAME) $(BINDIR)/agiletest $(BINDIR)/htTest

VPATH = $(SOURCE_DIR)/src $(SOURCE_DIR)/inc $(SOURCE_DIR)/inc/pub $(SOURCE_DIR)/external/tinyxml2 $(SOURCE_DIR)/test/agilelog $(SOURCE_DIR)/test/hashtable

$(OBJDIR)/%.o: %.cpp agilelog_def.h agilelog_priv.h
	@echo "[compiling.. $(notdir $<)]"
	$(CXX) -c -fPIC -o $@ $< $(CPPFLAGS)

$(BINDIR)/$(LIB_NAME): $(LIB_OBJS)
	@echo [creating.. $(notdir $@)]
	$(CXX) -shared $^ -o $@

$(OBJDIR)/agilelog_test.o: agilelog_test.cpp
	@$(CXX) -c $(CPPFLAGS) $< -o $@ 


$(BINDIR)/agiletest: $(OBJDIR)/agilelog_test.o
	@echo "[build agiletest..]"
	$(CXX) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/hashtable_test.o: hashtable_test.cpp
	@$(CXX) -c $(CPPFLAGS) $< -o $@ 


$(BINDIR)/htTest: $(OBJDIR)/hashtable_test.o
	@echo "[build htTest..]"
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	@echo "[clean.. $(MODULE)]"
	@-rm $(BINDIR)/*
	@-rm $(OBJDIR)/*
