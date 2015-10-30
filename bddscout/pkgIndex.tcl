#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 78 $]
#  Date        [$Date: 2015-04-20 23:07:29 +0200 (pon, 20 apr 2015) $]
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
