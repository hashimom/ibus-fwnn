# vim:set noet ts=4:
#
# ibus-fwnn - The FreeWnn engine for IBus
#
# Copyright (c) 2007-2011 Peng Huang <shawn.p.huang@gmail.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


libexec_PROGRAMS = ibus-engine-fwnn

ibus_engine_fwnn_SOURCES = \
	main.c \
	engine.c \
	engine.h \
	convert.c \
	convert.h \
	fwnnserver.c \
	fwnnserver.h \
	$(NULL)
ibus_engine_fwnn_CFLAGS = \
	@IBUS_CFLAGS@ \
	-DPKGDATADIR=\"$(pkgdatadir)\"
	$(NULL)
ibus_engine_fwnn_LDADD = \
	@IBUS_LIBS@ \
	@WNN_LIBS@ \
	$(NULL)

component_DATA = \
	fwnn.xml \
	$(NULL)
componentdir = @datadir@/ibus/component

EXTRA_DIST = \
	fwnn.xml.in \
	$(NULL)

CLEANFILES = \
	fwnn.xml \
	$(NULL)

fwnn.xml: fwnn.xml.in
	$(AM_V_GEN) \
	( \
		libexecdir=${libexecdir}; \
		pkgdatadir=${pkgdatadir}; \
		s=`cat $<`; \
		eval "echo \"$${s}\""; \
	) > $@

test: ibus-engine-fwnn
	$(builddir)/ibus-engine-fwnn
