#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 159 $]
#  Date        [$Date: 2016-06-03 12:46:41 +0200 (pet, 03 jun 2016) $]
#
#  Copyright   [This file is part of Bdd Scout package.
#               Copyright (C) 2008, 2015 UM-FERI
#               UM-FERI, Smetanova ulica 17, SI-2000 Maribor, Slovenia
#
#               Bdd Scout is free software; you can redistribute it and/or modify
#               it under the terms of the GNU General Public License as
#               published by the Free Software Foundation; either version 2
#               of the License, or (at your option) any later version.
#
#               Bdd Scout is distributed in the hope that it will be useful,
#               but WITHOUT ANY WARRANTY; without even the implied warranty of
#               MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#               GNU General Public License for more details.
#
#               You should have received a copy of the GNU General Public
#               License along with this program; if not, write to the Free
#               Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#               Boston, MA 02110-1301 USA.]

MAINNAME = MAINNAME
EXTNAME1 = EXTNAME1
EXTNAME2 = EXTNAME2
EXTNAME3 = EXTNAME3

OS = $(shell uname | sed "s/_.*//;s/[0-9]//g")
RM = rm -f
CP = cp
MKDIR = mkdir -p
CD = cd

# default folder (used for purge and clean, only)
BINDIR = ./bin

all: default

# used for local testing
local: start $(BINDIR) \
     $(BINDIR)/bddscout.map \
     $(BINDIR)/$(MAINNAME) \
     $(BINDIR)/$(EXTNAME1) \
     $(BINDIR)/$(EXTNAME2) \
     $(BINDIR)/$(EXTNAME3) \
     $(BINDIR)/COPYING \
     $(BINDIR)/bddview.tcl \
     $(BINDIR)/bddscout.tcl \
     $(BINDIR)/bddscoutIFIP.tcl \
     $(BINDIR)/bddscoutBRA.tcl \
     $(BINDIR)/bddscoutBDDTRACES.tcl \
     $(BINDIR)/pkgIndex.tcl \
     $(BINDIR)/pkgExtensionIFIP.tcl \
     $(BINDIR)/pkgExtensionBRA.tcl \
     $(BINDIR)/pkgExtensionBDDTRACES.tcl \
     $(BINDIR)/example.bddview \
     $(BINDIR)/example.bdd \
     $(BINDIR)/example.bf \
     ok

# used for packaging
# produces bddscout.so which is staticaly or dinamicaly linked with Biddy
# (depends on available Biddy package)
package: start $(BINDIR) $(BINDIR)/$(MAINNAME) $(BINDIR)/$(EXTNAME1) $(BINDIR)/$(EXTNAME2) $(BINDIR)/$(EXTNAME3) ok

# -----------------------------------------------------------------------
# help
# -----------------------------------------------------------------------

default:
	@echo "*** DEFAULT BUILDING! ***"
	@echo Please, check README to find other options for building BDD Scout.
	@$(MAKE) --no-print-directory -f Makefile.$(OS)

start:
	@echo "----------------------------------"
	@echo "| Building Bdd Scout package ... |"
	@echo "----------------------------------"
	@echo "CFLAGS="$(CFLAGS)

ok:
	@echo "-------------------------------------------"
	@echo "| Bdd Scout package successfully compiled |"
	@echo "-------------------------------------------"

# -----------------------------------------------------------------------
# create directory, copy files
# -----------------------------------------------------------------------

$(BINDIR):
	$(MKDIR) $(BINDIR)

$(BINDIR)/COPYING: ./COPYING
	$(CP) ./COPYING $(BINDIR)

$(BINDIR)/bddview.tcl: $(BDDVIEWDIR)/bddview.tcl
	$(CP) $(BDDVIEWDIR)/bddview.tcl $(BINDIR)

$(BINDIR)/bddscout.map: ./bddscout.map
	$(CP) ./bddscout.map $(BINDIR)

$(BINDIR)/bddscout.tcl: ./bddscout.tcl
	$(CP) ./bddscout.tcl $(BINDIR)

$(BINDIR)/bddscoutIFIP.tcl: ./bddscoutIFIP.tcl
	$(CP) ./bddscoutIFIP.tcl $(BINDIR)

$(BINDIR)/bddscoutBRA.tcl: ./bddscoutBRA.tcl
	$(CP) ./bddscoutBRA.tcl $(BINDIR)

$(BINDIR)/bddscoutBDDTRACES.tcl: ./bddscoutBDDTRACES.tcl
	$(CP) ./bddscoutBDDTRACES.tcl $(BINDIR)

$(BINDIR)/pkgIndex.tcl: ./pkgIndex.tcl
	$(CP) ./pkgIndex.tcl $(BINDIR)

$(BINDIR)/pkgExtensionIFIP.tcl: ./pkgExtensionIFIP.tcl
	$(CP) ./pkgExtensionIFIP.tcl $(BINDIR)

$(BINDIR)/pkgExtensionBRA.tcl: ./pkgExtensionBRA.tcl
	$(CP) ./pkgExtensionBRA.tcl $(BINDIR)

$(BINDIR)/pkgExtensionBDDTRACES.tcl: ./pkgExtensionBDDTRACES.tcl
	$(CP) ./pkgExtensionBDDTRACES.tcl $(BINDIR)

$(BINDIR)/example.bddview: $(BDDVIEWDIR)/example.bddview
	$(CP) $(BDDVIEWDIR)/example.bddview $(BINDIR)

$(BINDIR)/example.bdd: ./example.bdd
	$(CP) ./example.bdd $(BINDIR)

$(BINDIR)/example.bf: ./example.bf
	$(CP) ./example.bf $(BINDIR)

# -----------------------------------------------------------------------
# make objects
# -----------------------------------------------------------------------

$(BINDIR)/bddscout.o: bddscout.c bddscout.h bddscoutStubInit.c bddscoutStubLib.c
	$(CC) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscout.o -c bddscout.c -D'USECONST=$(USECONST)'
	$(CC) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscoutStubInit.o -c bddscoutStubInit.c
	$(CC) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscoutStubLib.o -c bddscoutStubLib.c

$(BINDIR)/bddscoutIFIP.o: bddscoutIFIP.c bddscout.h
	$(CCEXT) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscoutIFIP.o -c bddscoutIFIP.c -D'USECONST=$(USECONST)'

$(BINDIR)/bddscoutBRA.o: bddscoutBRA.c bddscout.h
	$(CCEXT) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscoutBRA.o -c bddscoutBRA.c -D'USECONST=$(USECONST)'

$(BINDIR)/bddscoutBDDTRACES.o: bddscoutBDDTRACES.c bddscout.h
	$(CCEXT) $(CFLAGS) -DUSE_TCL_STUBS -I$(BIDDYDIR) -o $(BINDIR)/bddscoutBDDTRACES.o -c bddscoutBDDTRACES.c -D'USECONST=$(USECONST)'

# -----------------------------------------------------------------------
# make libraries, use tcl/tk
# -----------------------------------------------------------------------

$(BINDIR)/$(MAINNAME): $(BINDIR)/bddscout.o $(BINDIR)/bddscoutStubInit.o
	$(CD) $(BINDIR); $(LN) $(MAINNAME) $(CFLAGS) bddscout.o bddscoutStubInit.o -L$(BIDDYLIB) -lbiddy $(GMPLIB) $(TCLSTUBLIB)
	$(CD) $(BINDIR); $(AR) $(BDDSCOUTLIB) bddscoutStubLib.o
	$(CC) $(CFLAGS) -o $(BINDIR)/bddscout bddscoutMain.c $(TCLTKLIB)

$(BINDIR)/$(EXTNAME1): $(BINDIR)/bddscoutIFIP.o $(BINDIR)/$(BDDSCOUTLIB)
	$(CD) $(BINDIR); $(LNEXT) $(EXTNAME1) $(CFLAGS) bddscoutIFIP.o $(BDDSCOUTLIB) $(BIDDYLIBEXT) $(TCLSTUBLIB)

$(BINDIR)/$(EXTNAME2): $(BINDIR)/bddscoutBRA.o $(BINDIR)/$(BDDSCOUTLIB)
	$(CD) $(BINDIR); $(LNEXT) $(EXTNAME2) $(CFLAGS) bddscoutBRA.o $(BDDSCOUTLIB) $(BIDDYLIBEXT) $(TCLSTUBLIB)

$(BINDIR)/$(EXTNAME3): $(BINDIR)/bddscoutBDDTRACES.o $(BINDIR)/$(BDDSCOUTLIB)
	$(CD) $(BINDIR); $(LNEXT) $(EXTNAME3) $(CFLAGS) bddscoutBDDTRACES.o $(BDDSCOUTLIB) $(BIDDYLIBEXT) $(TCLSTUBLIB)

# -----------------------------------------------------------------------
# purge = purify source directory, clean = purify bin directory
# -----------------------------------------------------------------------

purge:
	@$(RM) *~
	@$(RM) */*~

clean:
	@$(RM) $(BINDIR)/bddscout.o
	@$(RM) $(BINDIR)/bddscoutIFIP.o
	@$(RM) $(BINDIR)/bddscoutBRA.o
	@$(RM) $(BINDIR)/bddscoutBDDTRACES.o
	@$(RM) $(BINDIR)/bddscoutStubInit.o
	@$(RM) $(BINDIR)/bddscoutStubLib.o
	@$(RM) $(BINDIR)/libbddscoutstub.a
