@echo off

REM  Author: Robert Meolic (robert@meolic.com)
REM  $Revision: 692 $
REM  $Date: 2024-06-30 18:06:54 +0200 (ned, 30 jun 2024) $
REM
REM  This file is part of Biddy.
REM  Copyright (C) 2006, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
REM  Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia
REM
REM  Biddy is free software; you can redistribute it and/or modify
REM  it under the terms of the GNU General Public License as
REM  published by the Free Software Foundation; either version 2
REM  of the License, or (at your option) any later version.
REM
REM  Biddy is distributed in the hope that it will be useful,
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

SET NAME=biddy
FOR /F "USEBACKQ" %%T IN (`TYPE VERSION`) DO SET VVERSION=%%T
SET VERSION=%VVERSION:.=-%

REM /c/'Program Files'/7-Zip/7z.exe
SET MYZIP="C:\Program Files\7-Zip\7z.exe"

REM Use this to have an info about the package shown to the user
SET SFX="C:\Program Files\7-Zip\7zsd_All_x64.sfx"

REM Use this to enable plain self-extracting package (no need to download and use additional software)
REM SET SFX="C:\Program Files\7-Zip\7z.sfx"

rm -f %NAME%-bin-%VERSION%-Win.7z
rm -f %NAME%-bin-%VERSION%-Win.exe
rm -f %NAME%-%VERSION%-Win.7z
rm -f %NAME%-%VERSION%-Win.exe

rm -fr .\%NAME%-%VERSION%

make "CFLAGS = %CFLAGS%" "BINDIR = ../biddy/%NAME%-%VERSION%"
make clean "BINDIR = ../biddy/%NAME%-%VERSION%"

cp COPYING ./%NAME%-%VERSION%/
cp CHANGES ./%NAME%-%VERSION%/CHANGES
cp biddy.h ./%NAME%-%VERSION%/

REM mpir.dll IS USED FOR BUILDING WITH Visual Studio
REM /mingw64/bin/libgmp-10.dll from MSYS2 IS USED FOR BUILDING WITH MINGW

%MYZIP% a %NAME%-%VERSION%-Win.7z .\%NAME%-%VERSION%\* -x!.\%NAME%-%VERSION%\*.h -x!.\%NAME%-%VERSION%\*.a -x!.\%NAME%-%VERSION%\*.lib

cp biddy-cudd.h biddy-cudd.c ./%NAME%-%VERSION%/
cp biddy-example-8queens.c ./%NAME%-%VERSION%/
cp biddy-example-independence.c biddy-example-independence-europe.c biddy-example-independence-usa.c biddy-example-independence-test.c ./%NAME%-%VERSION%/
cp biddy-example-hanoi.c ./%NAME%-%VERSION%/
cp biddy-example-dictionary.c biddy-example-dictman.c biddy-example-dict-common.c biddy-example-dict-common.h ./%NAME%-%VERSION%/
cp biddy-example-bra.c ./%NAME%-%VERSION%/
cp biddy-example-pp.c biddy-example-pp-data.c biddy-example-pp-data.csv ./%NAME%-%VERSION%/

%MYZIP% a %NAME%-dev-%VERSION%-Win.7z .\%NAME%-%VERSION%\*

echo "Creating %NAME%-bin-%VERSION%-Win.exe ..."

echo ;!@Install@!UTF-8! > cfg.cfg
echo Title="Biddy x64 edition (binary version)" >> cfg.cfg
echo BeginPrompt="Biddy x64 edition\nVersion %VVERSION%\n\nBiddy is a multi-platform academic Binary Decision Diagrams package.\n\nThis is free software. See GNU GPL (version 2).\n\nNOTE: No registry entry will be changed and no file will be\n            created outside the target folder. You can uninstall\n            this software by simply deleting the target folder.\n\nRobert Meolic (robert@meolic.com)" >> cfg.cfg
echo ExtractPathText="Please, enter EMPTY or NONEXISTENT target folder:" >> cfg.cfg
echo ExtractDialogText="Please, wait..." >> cfg.cfg
echo ExtractTitle="Extracting..." >> cfg.cfg
echo GUIFlags="8+32+64+256+4096" >> cfg.cfg
echo GUIMode="1" >> cfg.cfg
echo InstallPath="%HOMEPATH%\\biddy" >> cfg.cfg
echo ;!@InstallEnd@! >> cfg.cfg

copy /b /y %SFX% + cfg.cfg + "%NAME%-%VERSION%-Win.7z" "%NAME%-%VERSION%-Win.exe"

echo "Creating %NAME%-%VERSION%-Win.exe ..."

echo ;!@Install@!UTF-8! > cfg.cfg
echo Title="Biddy x64 edition (development version)" >> cfg.cfg
echo BeginPrompt="Biddy x64 edition\nVersion %VVERSION%\n\nBiddy is a multi-platform academic Binary Decision Diagrams package.\n\nThis is free software. See GNU GPL (version 2).\n\nNOTE: No registry entry will be changed and no file will be\n            created outside the target folder. You can uninstall\n            this software by simply deleting the target folder.\n\nRobert Meolic (robert@meolic.com)" >> cfg.cfg
echo ExtractPathText="Please, enter EMPTY or NONEXISTENT target folder:" >> cfg.cfg
echo ExtractDialogText="Please, wait..." >> cfg.cfg
echo ExtractTitle="Extracting..." >> cfg.cfg
echo GUIFlags="8+32+64+256+4096" >> cfg.cfg
echo GUIMode="1" >> cfg.cfg
echo InstallPath="%HOMEPATH%\\biddy" >> cfg.cfg
echo ;!@InstallEnd@! >> cfg.cfg

copy /b /y %SFX% + cfg.cfg + "%NAME%-dev-%VERSION%-Win.7z" "%NAME%-dev-%VERSION%-Win.exe"

rm -f cfg.cfg
rm -fr %NAME%-%VERSION%
rm -f %NAME%-%VERSION%-Win.7z
rm -f %NAME%-dev-%VERSION%-Win.7z

echo *******************
echo PACKAGES COMPLETED!
echo *******************
