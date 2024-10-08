#  Authors     [Robert Meolic (robert@meolic.com)]
#  Revision    [$Revision: 692 $]
#  Date        [$Date: 2024-06-30 18:06:54 +0200 (ned, 30 jun 2024) $]
#
#  Copyright   [
#    This file is part of Biddy.
#    Copyright (C) 2006, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
#    Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia
#
#    Biddy is free software; you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as
#    published by the Free Software Foundation; either version 2
#    of the License, or (at your option) any later version.
#
#    Biddy is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public
#    License along with this program; if not, write to the Free
#    Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301 USA.
#  ]

# About shared libraries on Linux:
# http://www.debian.org/doc/debian-policy/ch-sharedlibs.html
# http://www.netfort.gr.jp/~dancer/column/libpkg-guide/libpkg-guide.html

LIBNAME = LIBNAME
OS = $(shell uname | sed "s/_.*//;s/[0-9]//g")
RM = rm -f
RMFR = rm -fr
CP = cp
MKDIR = mkdir -p
CD = cd
BINDIR = "./bin"

all: start dynamic clean ok
	$(CP) ./biddy.h $(BINDIR)

dynamic: start_dynamic make_dynamic

static: start_static make_static

debug: start_static make_debug

profile: start_static make_profile

lib: $(BINDIR) $(BINDIR)/$(LIBNAME)

# -----------------------------------------------------------------------
# initialization

make_dynamic:
	@$(MAKE) "BINDIR = $(BINDIR)" --no-print-directory dynamic -f Makefile.$(OS)

make_static:
	@$(MAKE) "BINDIR = $(BINDIR)" --no-print-directory static -f Makefile.$(OS)

make_debug:
	@$(MAKE) "BINDIR = $(BINDIR)" --no-print-directory debug -f Makefile.$(OS)

make_profile:
	@$(MAKE) "BINDIR = $(BINDIR)" --no-print-directory profile -f Makefile.$(OS)

# -----------------------------------------------------------------------
# help

start:
	@echo "------------------------------"
	@echo "| Building Biddy package ... |"
	@echo "------------------------------"

start_dynamic:
	@echo "*** Building dynamic library ..."

start_static:
	@echo "*** Building static library ..."

ok:
	@echo "---------------------------------------"
	@echo "| Biddy package successfully compiled |"
	@echo "---------------------------------------"

# -----------------------------------------------------------------------
# create a directory for binary files

$(BINDIR):
	$(MKDIR) $(BINDIR)

# -----------------------------------------------------------------------
# make library

$(BINDIR)/biddyMain.o: biddyMain.c biddy.h biddyInt.h
	$(CC) -o $(BINDIR)/biddyMain.o -c biddyMain.c -D'BIDDYVERSION="$(BIDDYVERSION)"'

$(BINDIR)/biddyOp.o: biddyOp.c biddy.h biddyInt.h
	$(CC) -o $(BINDIR)/biddyOp.o -c biddyOp.c

$(BINDIR)/biddyStat.o: biddyStat.c biddy.h biddyInt.h
	$(CC) -o $(BINDIR)/biddyStat.o -c biddyStat.c

$(BINDIR)/biddyInOut.o: biddyInOut.c biddy.h biddyInt.h
	$(CC) -o $(BINDIR)/biddyInOut.o -c biddyInOut.c

#$(BINDIR)/biddy4cudd.o: biddy4cudd.c biddy.h biddyInt.h biddy4cudd.h
#	$(CC) -o $(BINDIR)/biddy4cudd.o -c biddy4cudd.c

$(BINDIR)/$(LIBNAME): $(BINDIR)/biddyMain.o $(BINDIR)/biddyOp.o $(BINDIR)/biddyStat.o $(BINDIR)/biddyInOut.o
	$(CD) $(BINDIR); $(LN) $(LIBNAME) biddyMain.o biddyOp.o biddyStat.o biddyInOut.o $(LIBGMP)

# -----------------------------------------------------------------------
# purge = purify source directory, clean = purify bin directory

purge:
	@$(RM) *~
	@$(RM) */*~

purgevs:
	@$(RMFR) VS/BDDScoutInstaller-cache
	@$(RMFR) VS/BDDScoutInstaller-SetupFiles
	@$(RMFR) VS/BiddyDevInstaller-cache
	@$(RMFR) VS/BiddyDevInstaller-SetupFiles
	@$(RMFR) VS/BiddyInstaller-cache
	@$(RMFR) VS/BiddyInstaller-SetupFiles
	@$(RMFR) VS/x64
	@$(RM) VS/BDDScout.vcxproj.user
	@$(RM) VS/BDDScoutBDDTRACES.vcxproj.user
	@$(RM) VS/BDDScoutBRA.vcxproj.user
	@$(RM) VS/BDDScoutIFIP.vcxproj.user
	@$(RM) VS/BiddyExample8Queens.vcxproj.user
	@$(RM) VS/BiddyExampleIndependence.vcxproj.user

clean:
	@$(RM) $(BINDIR)/biddyMain.o
	@$(RM) $(BINDIR)/biddyOp.o
	@$(RM) $(BINDIR)/biddyStat.o
	@$(RM) $(BINDIR)/biddyInOut.o
#	@$(RM) $(BINDIR)/biddy4cudd.o

cleanall: clean
	@$(RM) $(BINDIR)/libbiddy.a
	@$(RM) $(BINDIR)/biddy.lib
	@$(RM) $(BINDIR)/biddy.dll
