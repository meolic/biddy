#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 368 $]
#  Date        [$Date: 2018-01-28 23:23:28 +0100 (ned, 28 jan 2018) $]
#
#  Copyright   [This file is part of Bdd Scout package.
#               Copyright (C) 2008, 2018 UM FERI
#               UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
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

package ifneeded bddscoutBRA 1.0 [format {

  if {($tcl_platform(platform) == "unix") && !($tcl_platform(os) == "Darwin")} {
    puts "  Loading [file join %s bddscoutBRA.so]"
    load [file join %s bddscoutBRA.so] bddscoutBRA
  }

  if {($tcl_platform(platform) == "unix") && ($tcl_platform(os) == "Darwin")} {
    puts "  Loading [file join %s libbddscoutBRA.dylib]"
    load [file join %s libbddscoutBRA.dylib] bddscoutBRA
  }

  if {$tcl_platform(platform) == "windows"} {
    puts "  Loading [file join %s bddscoutBRA.dll]"
    load [file join %s bddscoutBRA.dll] bddscoutBRA
  }

  source [file join %s bddscoutBRA.tcl]

} [list $dir] [list $dir] [list $dir] [list $dir] [list $dir] [list $dir] [list $dir]]
