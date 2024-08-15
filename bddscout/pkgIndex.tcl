#  Authors     [Robert Meolic (robert@meolic.com)]
#  Revision    [$Revision: 692 $]
#  Date        [$Date: 2024-06-30 18:06:54 +0200 (ned, 30 jun 2024) $]
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

package ifneeded bddview 1.0 [format {
  source [file join %s bddview.tcl]
} [list $dir]]

package ifneeded bddscout 1.0 [format {
  source [file join %s bddscout.tcl]
} [list $dir]]

package ifneeded bddscout-lib 1.0 [format {

  if {($tcl_platform(platform) == "unix") && !($tcl_platform(os) == "Darwin")} {
    load [file join %s bddscout.so] bddscout
  }

  if {($tcl_platform(platform) == "unix") && ($tcl_platform(os) == "Darwin")} {
    load [file join %s libbddscout.dylib] bddscout
  }

  if {$tcl_platform(platform) == "windows"} {
    load [file join %s bddscout.dll] bddscout
  }

} [list $dir] [list $dir] [list $dir]]

foreach extname [glob -nocomplain [file join $dir pkgExtension*.tcl]] {
  source $extname
}
