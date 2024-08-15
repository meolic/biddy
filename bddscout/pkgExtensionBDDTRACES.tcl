#  Authors     [Robert Meolic (robert@meolic.com)]
#  Revision    [$Revision$]
#  Date        [$Date$]
#
#  Copyright   [This file is part of Bdd Scout package.
#               Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia
#               Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia
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

package ifneeded bddscoutBDDTRACES 1.0 [format {

  if {($tcl_platform(platform) == "unix") && !($tcl_platform(os) == "Darwin")} {
    puts "  Loading [file join %s bddscoutBDDTRACES.so]"
    load [file join %s bddscoutBDDTRACES.so] bddscoutBDDTRACES
  }

  if {($tcl_platform(platform) == "unix") && ($tcl_platform(os) == "Darwin")} {
    puts "  Loading [file join %s libbddscoutBDDTRACES.dylib]"
    load [file join %s libbddscoutBDDTRACES.dylib] bddscoutBDDTRACES
  }

  if {$tcl_platform(platform) == "windows"} {
    puts "  Loading [file join %s bddscoutBDDTRACES.dll]"
    load [file join %s bddscoutBDDTRACES.dll] bddscoutBDDTRACES
  }

  source [file join %s bddscoutBDDTRACES.tcl]

} [list $dir] [list $dir] [list $dir] [list $dir] [list $dir] [list $dir] [list $dir]]
