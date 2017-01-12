SOURCES = ipfix.c ipfix_col.c ipfix_print.c ipfix_ssl.c mlog.c mpoll.c libipfix.cpp

lib_LTLIBRARIES = libipfix.la
libipfix_la_SOURCES = $(SOURCES)
libipfix_la_CPPFLAGS= -I@top_srcdir@/include
libipfix_la_LDFLAGS = -version-info $(NETXMS_LIBRARY_VERSION)
libipfix_la_LIBADD = -lssl -lcrypto -lcurl

EXTRA_DIST = gettimeofday.c ipfix.h ipfix_col.h ipfix_def.h ipfix_fields.h ipfix_reverse_fields.h ipfix_ssl.h libipfix.h mlog.h mpoll.h libipfix.vcproj
