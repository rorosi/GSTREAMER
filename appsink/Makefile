##################################################################
#  File Name    : Makefile.			                    		 #
##################################################################
CPP	= g++
STRIP   = strip

GSTREMER_PKG	= gstreamer-1.0
GSTREAMER_CFG	= `pkg-config $(GSTREMER_PKG) --cflags`

GSTREMER_APP_PKG	= gstreamer-app-1.0
GSTREAMER_APP_CFG	= `pkg-config $(GSTREMER_APP_PKG) --cflags`

GSTREAMERLIB	= `pkg-config $(GSTREMER_PKG) --libs`
GSTREAMERLIB	+= `pkg-config $(GSTREMER_APP_PKG) --libs`

GSTREAMER_INC_PATH = /usr/include/gstreamer-1.0
# GSTREAMER_INC_PATH += /usr/include/glib-2.0

LIBS += $(GSTREAMERLIB) -lyaml-cpp
LIBDIR = -L$(APP_LIB_PATH)

CFLAGS = -g -Wall -O2
CFLAGS += -I. -I.. -I$(APP_INC_PATH) -I$(GSTREAMER_INC_PATH)
CFLAGS += -D_REENTRANT -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE
CFLAGS += $(GSTREAMER_CFG) $(GSTREAMER_APP_CFG)
CFLAGS += -Wno-unused-function -fno-strict-aliasing -Wno-noexcept-type -Wparentheses


MAIN_EXE = main

OBJS = main.o

.PHONY: clean $(MAIN_EXE) 

all: $(MAIN_EXE)

$(MAIN_EXE) : $(OBJS)
	$(CPP) -o $@ $(OBJS) $(LIBDIR) $(LIBS) 
	$(STRIP) $@	
	
.cpp.o : $(OBJS)
	$(CPP) $(CFLAGS) -c $(LIBS) $<

clean: 
	rm -f *.o $(MAIN_EXE) 


