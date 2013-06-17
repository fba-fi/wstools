AM_CFLAGS = -Wall -g3 
INCLUDES=$(all_includes)
AM_LDFLAGS=$(all_libraries)

bin_PROGRAMS = wsclient avgdata
wsclient_SOURCES = wsclient.c network.h network.c common.h proto.h options.c options.h
avgdata_SOURCES = avgdata.c common.h proto.h datautils.c datautils.h options.c options.h
avgdata_LDADD = -lm

