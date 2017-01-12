SOURCES = messages.c collector.cpp debug.cpp nxflowd.cpp

bin_PROGRAMS = nxflowd
nxflowd_SOURCES = $(SOURCES)
nxflowd_LDADD = ../libipfix/libipfix.la @top_srcdir@/src/libnetxms/libnetxms.la @top_srcdir@/src/db/libnxdb/libnxdb.la
nxflowd_CPPFLAGS=-I@top_srcdir@/include -I../libipfix -I@top_srcdir@/server/include

BUILT_SOURCES = messages.h
CLEANFILES = messages.c messages.h static_subagents.cpp

messages.c: messages.h

messages.h: messages.mc
	$(PERL) @top_builddir@/tools/mc.pl messages.mc

EXTRA_DIST = messages.mc messages.h messages.c nxflowd.h resource.h nxflowd.vcproj winsrv.cpp
