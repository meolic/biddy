@echo off

REM  Author: Robert Meolic (robert@meolic.com)
REM  $Revision: 545 $
REM  $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $
REM
REM  This file is part of Bdd Scout package.
REM  Copyright (C) 2008, 2019 UM FERI
REM  UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
REM
REM  Bdd Scout is free software; you can redistribute it and/or modify
REM  it under the terms of the GNU General Public License as
REM  published by the Free Software Foundation; either version 2
REM  of the License, or (at your option) any later version.
REM
REM  Bdd Scout is distributed in the hope that it will be useful,
REM  but WITHOUT ANY WARRANTY; without even the implied warranty of
REM  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
REM  GNU General Public License for more details.
REM
REM  You should have received a copy of the GNU General Public
REM  License along with this program; if not, write to the Free
REM  Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
REM  Boston, MA 02110-1301 USA.]

REM use CFLAGS="-m32" to produce 32-bit libs
REM use CFLAGS="-m64" to produce 64-bit libs
REM leave empty for system default
SET CFLAGS=

SET NAME=bddscout
FOR /F "USEBACKQ" %%T IN (`TYPE VERSION`) DO SET VVERSION=%%T
SET VERSION=%VVERSION:.=-%
SET MYZIP="C:\Program Files\7-Zip\7z.exe" a
SET SFX="C:\Program Files\7-Zip\7zsd_All_x64.sfx"

rm -f %NAME%-bin-%VERSION%-Win.zip
rm -f %NAME%-%VERSION%-Win.zip

rm -fr %NAME%-%VERSION%

echo *** Building Biddy ...
cd ..
make static "BINDIR = ./bddscout/%NAME%-%VERSION%"
make clean "BINDIR = ./bddscout/%NAME%-%VERSION%"
cp biddy.h ./bddscout/%NAME%-%VERSION%
cp CHANGES ./bddscout/%NAME%-%VERSION%/CHANGES_biddy
cp C:/Users/Robert/Documents/mpir-dll-2.7.2/mpir.dll ./bddscout/%NAME%-%VERSION%
echo *** Biddy OK.

echo *** Preparing bddview ...
cd bddview
cp bddview.tcl ../bddscout/%NAME%-%VERSION%
cp CHANGES ../bddscout/%NAME%-%VERSION%/CHANGES_bddview
cp example-robdd.bddview ../bddscout/%NAME%-%VERSION%
cp example-robddce.bddview ../bddscout/%NAME%-%VERSION%
cp example-zbddce.bddview ../bddscout/%NAME%-%VERSION%
cp example-tzbdd.bddview ../bddscout/%NAME%-%VERSION%
echo *** bddview OK.

echo *** Building BDD Scout ...
cd ..\bddscout
cp bddscout.h %NAME%-%VERSION%
cp bddscoutTcl.h %NAME%-%VERSION%
cp bddscout.c %NAME%-%VERSION%
cp bddscoutTcl.c %NAME%-%VERSION%
cp bddscoutMain.c %NAME%-%VERSION%
cp bddscout.tcl %NAME%-%VERSION%
cp bddscoutDecls.h %NAME%-%VERSION%
cp bddscoutStubInit.c %NAME%-%VERSION%
cp bddscoutStubLib.c %NAME%-%VERSION%
cp pkgIndex.tcl %NAME%-%VERSION%
cp bddscoutIFIP.c %NAME%-%VERSION%
cp bddscoutIFIP.tcl %NAME%-%VERSION%
cp pkgExtensionIFIP.tcl %NAME%-%VERSION%
cp bddscoutBRA.c %NAME%-%VERSION%
cp bddscoutBRA.tcl %NAME%-%VERSION%
cp pkgExtensionBRA.tcl %NAME%-%VERSION%
cp bddscoutBDDTRACES.c %NAME%-%VERSION%
cp bddtraces-BIDDY.c %NAME%-%VERSION%
cp bddscoutBDDTRACES.tcl %NAME%-%VERSION%
cp pkgExtensionBDDTRACES.tcl %NAME%-%VERSION%
cp Makefile %NAME%-%VERSION%
cp Makefile.MINGW %NAME%-%VERSION%
cp CHANGES %NAME%-%VERSION%/CHANGES_bddscout
cp example.bf %NAME%-%VERSION%
cp example.bdd %NAME%-%VERSION%
mkdir %NAME%-%VERSION%\create
cp ./create/*.tcl %NAME%-%VERSION%/create
mkdir %NAME%-%VERSION%\scripts
cp ./scripts/*.tcl %NAME%-%VERSION%/scripts

cd %NAME%-%VERSION%
make package -f Makefile.MINGW "CFLAGS = %CFLAGS%" "BIDDYDIR = ." "BIDDYLIB = ." "BIDDYLIBEXT = -L. -lbddscout " "BINDIR = ."
make clean "BINDIR = ."
rm -f biddy.h
rm -f libbiddy.a

REM source code is not included in the package
rm -f bddscout.h
rm -f bddscout.c
rm -f bddscout.map
rm -f bddscoutMain.c
rm -f bddscoutDecls.h
rm -f bddscoutStubInit.c
rm -f bddscoutStubLib.c
rm -f bddscoutIFIP.c
rm -f bddscoutBRA.c
rm -f bddscoutBDDTRACES.c
rm -f bddtraces-BIDDY.c

cd ..
cp COPYING %NAME%-%VERSION%
cp README %NAME%-%VERSION%
cp README.MINGW %NAME%-%VERSION%
echo *** BDD Scout OK.

%MYZIP% %NAME%-%VERSION%-Win.7z %NAME%-%VERSION%\*  -x!%NAME%-%VERSION%\lib*.*
%MYZIP% %NAME%-%VERSION%-Win.7z IFIP\*
%MYZIP% %NAME%-%VERSION%-Win.7z BDDTRACES\*

echo *** Creating %NAME%-%VERSION%-Win.exe ...
echo ;!@Install@!UTF-8! > cfg.cfg
echo Title="BDD Scout" >> cfg.cfg
echo BeginPrompt="BDD Scout\nVersion %VVERSION%\n\nBDD Scout is a demo application demonstrating the capability\nof BDD package Biddy and Tcl/Tk BDD viewer bddview.\n\nThis is free software. See GNU GPL (version 2).\n\nYou need ActiveTcl and graphviz installed on your computer.\nTo start the program click bddscout.exe in target folder.\n\nNOTE: No registry entry will be changed and no file will be\n            created outside the target folder. You can uninstall\n            this software by simply deleting the target folder.\n\nRobert Meolic (robert@meolic.com)" >> cfg.cfg
echo ExtractPathText="Please, enter EMPTY OR NON-EXISTING target folder:" >> cfg.cfg
echo ExtractDialogText="Please, wait..." >> cfg.cfg
echo ExtractTitle="Extracting..." >> cfg.cfg
echo GUIFlags="8+32+64+256+4096" >> cfg.cfg
echo GUIMode="1" >> cfg.cfg
echo InstallPath="%HOMEPATH%\\bddscout" >> cfg.cfg
echo ;!@InstallEnd@! >> cfg.cfg

copy /b /y %SFX% + cfg.cfg + "%NAME%-%VERSION%-Win.7z" "%NAME%-%VERSION%-Win.exe"

rm -f cfg.cfg
rm -fr %NAME%-%VERSION%
rm -f %NAME%-%VERSION%-Win.7z

echo *******************
echo PACKAGES COMPLETED!
echo *******************
