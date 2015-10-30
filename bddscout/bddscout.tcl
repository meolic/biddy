#!/bin/sh
# the next line executes the program\
exec wish "$0" "$@"

# MAYBE YOU MUST USE ONE OF THE FOLLOWING LINES
# exec /usr/local/bin/wish "$0" "$@"
# exec /usr/bin/wish8.4-X11 "$0" "$@"
# exec /home/meolic/ActiveTcl/bin/wish "$0" "$@"

# ####################################################################
# OS specific settings
#
# If "Export to PNG" and "Export to PDF" creates image with wrong
# dimensions you have to change DESKTOP_DPI to meet your system
# ####################################################################

set OS $tcl_platform(platform)
set OS1 $tcl_platform(os)

if {($OS == "unix") && !($OS1 == "Darwin")} {

  set DESKTOP_DPI 75.0
  set GHOSTSCRIPT_EXE "/usr/bin/gs"
  set DOT_EXE "/usr/bin/dot"

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
  }

} elseif {($OS == "unix") && ($OS1 == "Darwin")} {

  set DESKTOP_DPI 75.0
  set GHOSTSCRIPT_EXE ""
  set DOT_EXE "/usr/local/bin/dot"

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
  }

} elseif {$OS == "windows"} {

  set DESKTOP_DPI 96.0
  set GHOSTSCRIPT_EXE "C:/Program Files/gs/gs9.16/bin/gswin64c.exe"
  set DOT_EXE "C:/Program Files (x86)/Graphviz2.38/bin/dot.exe"

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE "C:/Program Files (x86)/Graphviz/bin/dot.exe"
  }
  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
  }

} else {

  set DESKTOP_DPI 100.0
  set GHOSTSCRIPT_EXE ""
  set DOT_EXE ""

}

#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 101 $]
#  Date        [$Date: 2015-10-06 15:51:07 +0200 (tor, 06 okt 2015) $]
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

# USAGE:
# ./bddscout.tcl
# ./bddscout.tcl example.bddview
#
# DATA FORMAT:
#
# BDD Scout is builded on bddview thus the native
# data format used in Open/Save is bddview format.
# See bddview.tcl for details.
#
# BDD Scout can import BDD given in prefix form:
# 1. the first word is the name of BDD
# 2. the second word is the name of top variable
# 3. any non-constant variable is followed by two
#    supgraphs in parenthesis, e.g. VAR (...) (...)
# 4. the name of constant variable is "1"
# 5. symbol * is used to denote complement edges.
#
# Example (example.bdd):
# ---------------------------------------------------
# Biddy
# B (* i (d (1) (y (* 1) (1))) (d (y (* 1) (1)) (1)))
#   (  i (d (1) (y (* 1) (1))) (d (y (* 1) (1)) (1)))
# ---------------------------------------------------
#
# BDD Scout can import Boolean functions given in
# prefix form:
# 1. the file is started with the set of variables given
#    in parenthesis - this is optional and is used
#    to give variable ordering
# 2. there can be many Boolean functions in the same file
# 3. one Boolean function can go over many lines, the only
#    requirement is that function name and symbol "=" are
#    given in the same line
# 4. everything is converted to upper case
# 5. supported operators are OR, AND, and EXOR
#
#
# Example (example.bf):
# ---------------------------------------------------
# (B i d y)
#
# s1 = (or B (not y))
# s2 = (or B i d)
# s3 = (or B (not i) (not d))
# s4 = (or (not B) i (not d) y)
# s5 = (or (not B) (not i) d y)
# Biddy = (and s1 s2 s3 s4 s5)
# ---------------------------------------------------
#
# KNOWN BUGS:
#
# - BDD construction does not preserve variable ordering
#
#   therefore, constructed BDD may not be identical
#   to the one being showed, e.g. different node number
#
#   you can redraw BDD (Browse By Name) to see, what is
#   really stored in the memory
#

# Here, version 1.0 should be given due to the problems with
# "package ifneeded" used in pkgIndex.tcl in the packages that use this.
package provide bddscout 1.0

# Use this if you start the program using wish
lappend auto_path .
lappend auto_path /usr/lib/bddscout

# ####################################################################
# Start Bdd View script
# ####################################################################

package require bddview

# ####################################################################
# Load functions written in C
# ####################################################################

puts -nonewline "Importing BDD Scout library ... "
package require bddscout-lib
bddscout_initPkg
puts "OK"

# ####################################################################
# Add new butons to the toolbar
#
# IMAGES FOR TOOLBAR ICONS ARE BASE64 ENCODED GIFS (32x32)
#
# Images are from Crystal Project by Everaldo Coelho
#
# Obtained from:
# http://www.everaldo.com/crystal/
#
# License: LGPL
#
# --------------------------------------------------------------------
#
# ####################################################################

set iconBitmap {
R0lGODlhIAAgAOf/AAACAAMABQADBgIHFwAPAAUNAAoQEgAZAxwOFQ0aABcV
IRUdABIcEw8bIxIcHRIhAxccFBImAhQiHRYZShQpACYgASUiABwpARQwABol
LxklNCgpACEtAB4wASAnLRo0ACIqJiopIiI4ACk1Ai4oPC8tIjskMzkwAzUq
NSY8ACwzICJCAk0uAi4wTC00Oi42MjIyRUQ4AEgvKjk1NEQxOS9JAEw4MTNQ
ADRPA0g1WDpOAD5MADNTADhDTkI8Wz9EPEA/VTtUAE5NAFY8RytHg2A5Pj9U
D0tGOlFSAGU8RzBRaU9HTERcAEpMSklbAE9PN05aAEdbEwlZx1ZOM1FRREhh
D2pJRVpQUE5WUlNTXGxHW1pNcFFnAFlYQFZmAVlVVFhXT2NPaF5VW2lWU25R
Z11tAFZtFF9VfW1VWnNTZWNnMFdib2xqDntTYnZYXl9lW3BbZGhfcHdbZilv
12F2J2hldSxzzmZjimN2LmJmjGZ6I2t3KTJ20ntlbnVrYHF6Hmlte2Fwgn5o
YHNmjVV1j4hja3todoBobDd70Dp7126AMJJkanh9MXVui3huhYlsd5JtZIxx
XzyC3n10eoN4VWN+k5Jva3uJI32HM5NxcnyLLVSIrG2DlJ50cpt5boGPQIt/
gEyP5lCP4IyCiJOAgmeQqZKQRE6W7YuHmJSDl6CAh5CNbV+U4JyFj1+X3GKX
4oeOr1ub5aaIgauId2SY5F6d6KKNimWc4Vig8HGevmib52+b4mif5GSi7Wyi
6GCn92mq7nGn7a+YopKiwW6s93Os626v87yekXmu9XKy93mx8Xez/4Ox8r+i
oXy19H+493q6/4m45bKqw3q9+4K6+om69X7B/7O1sovC4Zq824nA/46/+ojD
/Lu0zITK/5PE/43I/5LH+4fQ/9K5sKTI65DP/5nN/6vL6J/O/aTO9pvT/5je
/73R6rbW8+jJtafe/qPj/6Pn/tzS2Kvm/8fe/qny/7Xw/Mfs/8f1/ML4/rz7
/8P+/PLq9/v4/fn//////yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXMiwocOHCK9JnEixosWLFf3128ixo8ePIPtJ7CfFjsmTKFOm5MMnEaJE
ieaIvNYvEStWrnbddIVzJ09XOV3RAgp0KKtQM/0l0hUrVk+nPHfejAVUl65b
tW7p8rVL1Mx+knTtomX11q5bt3id3bVL7VZevsTqCuar16mkonr52qu319Ze
u+juFewrGLBixYIZC4br66lghiEDYwbscGVgwYphVoaYWTBkxpAh+/UVlzJm
xZQhm+yZGTJlqmHLVubsNG1mxJKall2sGmxo07iBU1bN2TPj1ZIrr0Yt2Vdi
1KhtqzZtWjVu3kZZCVYu+bZt3IL+B9fGbZs2cF+XhQP/fRs4caA6WZoQD3z5
9t++qZtnbx76kdB8E86AA8KDiieGHDPEMvq8U445EKYTTz3qbKJEBgAkxQ06
EKKDDjzDeBKGIe4sQgQ20eSjoj7w5LJGCDLIUAIEX6GTzo3pvEPOLM20Ms4k
GRyBAAAZEFJKJUMUkYkqaPhBhQpfZXNOO+3cg48szYCSRQYD/EAJCQ0YAAAA
kCwCxyOHHEJGFwd8lQ497GTDiQsCCACAAAqMgcYUDIDQAAovLJGGHDPIkcQT
R0TyVSA9BCCBCw00gcUXYmAByhurCHMFGH2YQAANaVABgB+2JFHEVzmcscUg
jQDSijSl3TgSh6x3OAKED7A0wo88MIyiQQt3wABEUkHoQccee1yCCSN/aHKJ
JmZUUQMUURiBxyeY4MGIKWrsoQcXXx2wAg886FADDlAIIcQKO3jhRRVBOMEF
EzescMMNQrABBRQpJAXAAh0UcIAFC1TwwAYjnMACBxhQQEENI3QQwwYdXJDC
BxcU8JVGG3Hcj8cahezPyCR/bPJMGKWsMkUQtezyyzDH3HJAADs=
}

set iconPdf {
R0lGODlhIAAgAOf/AP8AAP8AB/8BFP4ZE/8dJPwpJP4tLf8uNP8xNf04Ov87
PP1EQ9dcXfxVU/xWWf5XVfdaWc9rbP1XW/9YXPpdYf1fXfxfaPdiZ/tlZf5n
Z5uMsvxocqGOr/ttbp6Tsvxwe6OVrsKPj/x2eKKasqaYsqeZs6ObtPx8e6id
sKShpqmesaqfsqais6ekqaqjr/SFiPuDhKylsaqorPmIha2nsv6Gh6mqtK2r
r/6Miq2vrPuMlfmPj7CusvuRkbCyrvOYmfyZlvacnPSgnvqfn/2iovGnrfij
s+6rr/uoq/Surf+qp/mtru23t/mxvfu1s/e1v9nA+Pi8vfu6w93B89rC+tnE
9PbAv/u+v9zD+9rF9t3F8PfBwPPEwfnAzdrJ8uvIzt3I+eLG+OHI8+HI/9/K
+uDM7/XG1vDLy+PK9d3Q8d/O9/fKzeTM9/rLyOPP8uXN+PTOz9/T9OnM/+LR
+uPT7uXR9N7W8PbQ0eHU9uTU8OPY3+TT/N7c4OrR/OTa4ObW8fPV0+jU9+Lb
5+TZ7eTX+eXb4enV+ODe4uLa8+Dc7vXW1efW//PV5+Hf4+bZ++nZ9OLg5PnW
3Ofc7+Xc9uje5OLf8PfZ1+Ph5efb/PjX5Onf5und8evb9+7Z/eXi5ufg7fbZ
6+bj6PPe4evh6Prc2uvf8+3d+ejg+ebi9Oni7ufk6e7e+uvf//zd3Ojl6urh
++3j6e3h9fjg3fXf7+vk8Onm6/Dg/Ozl8ezj/e3n5u/j9+ro7Ojo8+vp7evn
+fjk5u7n8/Dl+e7l//Pj/+zq7u/o9ezp++3r7+/n//Ln+/bp6ezs9+7q/fHq
9u/s8fPo/Pzn6vbo9vDt8vPt7PXp/fDs/vTu7e7v+fbq/u/x7vTt+fLv9PDy
7/Lu//vt7vDx++7z9vPv//Px9fHz8PL08ffw/PXy9/jy8fP18vjx/vb0+Pny
//T38/r18/j1+vb49Pb2//v0//n2+/f59v339vv4/fn7+Pr8+ff9//36//v9
+v78//n///z/+/7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
Bkdo4OABBIkRKFSsYMHChUWKLFaoUIECBcJ/JMbI6bOoEytbwqhRQ4atZctq
KoXhWvERBZaRJTsN28kSGbJqLn1S08XiYwksZN4scoTJ1KthLI18YASzmq9Y
sUoVRXh0DJo5hhydsqUL6gYzZrB1e+nLV4yaU8Kg6WPIkFNdyGZZmJWp27p4
5bpp00bjIwgoVdCoCdTUFjJfoC40kRIPHrx48tatu2EYqRqwmE690qUrGgUd
Uv7Cy1cPXz4eH0dASVqH0KNVuqhVqyZhQaR6+4L7o4fPB1wxaOo4mvTqVDVt
5T4MaGdPn7/r/vYZRwhiCpk5gR7+lYrVbNm1b0sM4MMXfB922NyriFGDB1Mp
Xc20qQOHBIAsfdW5d912B40whRd10PFILL40Y4469DyAQA/2rHePcPAVWMVi
j5ziyzLigDNPK74h4A1x1VlHoEEoiFEHHpOgAsw1EM4DQwf2dFCBPsSx50+G
BpFQRRl/lHKLN9egA446CdyBjzUIAEEPPfbc48+KBaGgRRp/bPLhOOS0s8MC
7bRDjygEzABgPvoAmaUXeCAyyy7bgDPOOQU4AQ4+8+AjigEStHGFEhEAgBAL
ZdBRCSrHcHMOOEFIAE47liBxQga9DRCABAwA4KlBKnD5CS/gZHOONAlYYcUO
MwSxhSz+1uDzCw4ZMPARC3QMgsou02STzQkQtAHHNOCAQ4879LTzDj70hPCp
oQSxkMYmqYBDzC+AJCBKNuKgww446CgpTzt9EgjtQLgKwksuQgACARDcjjOO
OeiQA84779w7j5sExWDHJ4V0cMaYpWazTTbWbHOOOOa0o4479mBJkA2DfAFB
ElwUAEg2xBCTTS4eE3PMNNI4Y406/A4UwyAXQHBGAUx4kosrntTiyc0015JL
L8RIwxlCMiRyxAsQHOHJJXzkogkknlDiiSg33wxLLjJ81EIiglwCySWeNMLH
12B/rQcfhTTCdAsf0ZBKL8fs4ooro3yiySeCzCKIIJ9AMksfKnzDksJHPKjA
guAUuVCR4BJFRBEKK2ikwkeQR/5RQAA7
}

image create photo icon.bitmap.small
icon.bitmap.small put $iconBitmap

image create photo icon.pdf.small
icon.pdf.small put $iconPdf

set ts1 [Separator $toolbar.ts1 -bg $COLORMENU -orient vertical]
pack $ts1 -side left -fill y -padx 6 -anchor w

set tb1 [ButtonBox $toolbar.tb1 -bg $COLORMENU -homogeneous 0 -spacing 2 -padx 1 -pady 1]
$tb1 add -image icon.bitmap.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 -padx 1 -pady 1 \
    -helptext "Export to PNG" -command {bddscout_bitmap $mainwin png png16m}

$tb1 add -image icon.pdf.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 -padx 1 -pady 1 \
    -helptext "Export to PDF" -command {bddscout_bitmap $mainwin pdf pdfwrite}

pack $tb1 -side left -anchor w

# ####################################################################
# Create menus
# ####################################################################

font create MENUFONT -family Helvetica -size 10 -weight normal -slant roman
font create REPORTFONT -family Courier -size 10 -weight normal -slant roman
font create BOLDFONT -family Courier -size 10 -weight bold -slant roman

frame .menuFrame -highlightthickness 0
pack .menuFrame -fill x -before $mainframe

menubutton .menuFrame.file -menu .menuFrame.file.menu -pady 6 -font MENUFONT -text "File"
pack .menuFrame.file -side left

menu .menuFrame.file.menu -font MENUFONT -relief groove -tearoff false

.menuFrame.file.menu add command -command menu_file_open -label "Open ..."
.menuFrame.file.menu add command -command {bddview_saveas $mainwin} -label "Save As ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command menu_file_read_BDD -label "Import BDD ..."
.menuFrame.file.menu add command -command menu_file_read_BF -label "Import Boolean functions ..."
.menuFrame.file.menu add command -command menu_file_read_GraphML -label "Import GraphML ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command menu_file_write_BDD -label "Export BDD ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command {bddscout_bitmap $mainwin png png16m} -label "Export to PNG ..."
.menuFrame.file.menu add command -command {bddscout_bitmap $mainwin pdf pdfwrite} -label "Export to PDF ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command {bddview_print $mainwin} -label "Print to File ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command menu_system_info -label "System Info"
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command {exit} -label "Exit"

menubutton .menuFrame.view -menu .menuFrame.view.menu -pady 6 -font MENUFONT -text "View"
pack .menuFrame.view -side left

menu .menuFrame.view.menu -font MENUFONT -relief groove -tearoff false

.menuFrame.view.menu add command -command menu_view_formulae_byName -label "Browse by name ..."
.menuFrame.view.menu add command -command menu_view_formulae_byNodeNumber -label "Browse by node number ..."
.menuFrame.view.menu add command -command menu_view_formulae_byNodeMaxLevel -label "Browse by graph depth ..."
.menuFrame.view.menu add command -command menu_view_formulae_byNodeAvgLevel -label "Browse by node avg depth ..."

# ####################################################################
# Implement commands assigned to menu buttons
# ####################################################################

proc menu_file_open {  } {
  global mainwin

  set filename [tk_getOpenFile -title "Select bddview file" -parent $mainwin]
  if {[string length $filename] != 0} {
    bddview_draw $filename
    constructBDD

    #remember last path
    cd [file dirname $filename]
  }
}

proc constructBDD { } {
  global BDDNAME
  global BDD
  global BDDNODES

  if {$BDDNODES == ""} return;

  set variables ""
  set graph $BDDNAME
  foreach {num name type} [join $BDDNODES] {
    if {$type == "l"} {
      if {[lindex $BDD(n$num) 0] == "i"} {
        set succ [string range [lindex $BDD(n$num) 1] 1 end]
        lappend graph [list $num $name 1 $succ -1]
      } else {
        set succ [string range $BDD(n$num) 1 end]
        lappend graph [list $num $name 0 $succ -1]
      }
    } elseif {$type == "t"} {
      lappend graph [list $num $name 2 -1 -1]
    } else {
      if {[lsearch -exact $variables $name] == -1} {lappend variables $name}
      if {[lindex $BDD(n$num) 0] == "i"} {
        set succ1 [string range [lindex $BDD(n$num) 1] 1 end]
        set succ2 [string range [lindex $BDD(n$num) 2] 1 end]
        lappend graph [list $num $name 4 $succ1 $succ2]
      } else {
        set succ1 [string range [lindex $BDD(n$num) 0] 1 end]
        set succ2 [string range [lindex $BDD(n$num) 1] 1 end]
        if {$succ2 == ""} {
          lappend graph [list $num $name 4 $succ1 $succ1]
        } else {
          lappend graph [list $num $name 3 $succ1 $succ2]
        }
      }
    }
  }

  bddscout_constructBDD [llength $variables] [join $variables] [expr [llength $graph] -1] [join [join $graph]]
  update_info
}

proc menu_file_read_BDD {  } {
  global mainwin
  global DOT_EXE

  set filename [tk_getOpenFile -title "Import BDD" -parent $mainwin]
  if {[string length $filename] != 0} {

    set bdd [bddscout_readBDD $filename]
    update_info

    set tmpfile "tmp.bddview"
    if {[file executable $DOT_EXE] == 1} {
      set tmpfile [bddscout_writeBDDview $bdd $tmpfile $DOT_EXE]
      bddview_draw $tmpfile
      file delete $tmpfile
    }

    #remember last path
    cd [file dirname $filename]
  }

}

proc menu_file_read_BF {  } {
  global mainwin
  global DOT_EXE

  set filename [tk_getOpenFile -title "Import Boolean functions" -parent $mainwin]
  if {[string length $filename] != 0} {

    set bdd [bddscout_readBF $filename]
    puts BDD
    update_info

    set tmpfile "tmp.bddview"
    if {[file executable $DOT_EXE] == 1} {
      set tmpfile [bddscout_writeBDDview $bdd $tmpfile $DOT_EXE]
      bddview_draw $tmpfile
      file delete $tmpfile
    }

    #remember last path
    cd [file dirname $filename]
  }

}

proc menu_file_read_GraphML {  } {
  global mainwin

  not_implemented_yet "GraphML is not supported, yet"

}

proc menu_file_write_BDD {  } {
  global mainwin
  global BDDNAME

  set filename [tk_getSaveFile -title "Export BDD" -parent $mainwin]
  if {[string length $filename] != 0} {

    bddscout_writeBDD $filename $BDDNAME

    #remember last path
    cd [file dirname $filename]
  }
}

proc menu_view_formulae_byName {  } {
  global DOT_EXE
  global OS

  if {[file executable $DOT_EXE] != 1} {
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
    return
  }

  set fname [browse_formulae_byName]

  if {$fname != ""} {
    set tmpfile "tmp.bddview"
    set tmpfile [bddscout_writeBDDview $fname $tmpfile $DOT_EXE]
    bddview_draw $tmpfile
    file delete $tmpfile
  }
}

proc menu_view_formulae_byNodeNumber {  } {
  global DOT_EXE
  global OS

  if {[file executable $DOT_EXE] != 1} {
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
    return
  }

  set fname [browse_formulae_byNodeNumber]

  if {$fname != ""} {
    set tmpfile "tmp.bddview"
    set tmpfile [bddscout_writeBDDview $fname $tmpfile $DOT_EXE]
    bddview_draw $tmpfile
    file delete $tmpfile
  }
}

proc menu_view_formulae_byNodeMaxLevel {  } {
  global DOT_EXE
  global OS

  if {[file executable $DOT_EXE] != 1} {
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
    return
  }

  set fname [browse_formulae_byNodeMaxLevel]

  if {$fname != ""} {
    set tmpfile "tmp.bddview"
    set tmpfile [bddscout_writeBDDview $fname $tmpfile $DOT_EXE]
    bddview_draw $tmpfile
    file delete $tmpfile
  }
}

proc menu_view_formulae_byNodeAvgLevel {  } {
  global DOT_EXE
  global OS

  if {[file executable $DOT_EXE] != 1} {
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
    return
  }

  set fname [browse_formulae_byNodeAvgLevel]

  if {$fname != ""} {
    set tmpfile "tmp.bddview"
    set tmpfile [bddscout_writeBDDview $fname $tmpfile $DOT_EXE]
    bddview_draw $tmpfile
    file delete $tmpfile
  }
}

proc menu_system_info {  } {
  global bddscoutINFO
  global biddyINFO

  toplevel .dialogInfo
  wm title .dialogInfo "Info"
  wm iconname .dialogInfo "Info"

  frame .dialogInfo.f -relief raised

  label .dialogInfo.f.text1 -relief flat -anchor w -justify left -font BOLDFONT -text "BDD Scout Info" -bg azure3
  label .dialogInfo.f.text2 -relief flat -anchor w -justify left -font MENUFONT -textvariable bddscoutINFO -bg azure1
  label .dialogInfo.f.text3 -relief flat -text "" -bg azure1
  label .dialogInfo.f.text4 -relief flat -anchor w -justify left -font BOLDFONT -text "Biddy Info" -bg azure3
  label .dialogInfo.f.text5 -relief flat -anchor w -justify left -font MENUFONT -textvariable biddyINFO -bg azure1
  label .dialogInfo.f.text6 -relief flat -text "" -bg azure1
  pack .dialogInfo.f.text1 -fill x
  pack .dialogInfo.f.text2 -fill x
  pack .dialogInfo.f.text3 -fill x
  pack .dialogInfo.f.text4 -fill x
  pack .dialogInfo.f.text5 -fill x
  pack .dialogInfo.f.text6 -fill x

  pack .dialogInfo.f

  set x [expr {([winfo screenwidth .] - 32 * [font measure [.dialogInfo.f.text1 cget -font] "0"])}]
  set y 0
  wm geometry .dialogInfo +$x+$y

  frame .dialogInfo.f.buttons -relief raised

  button .dialogInfo.f.buttons.print -borderwidth 2 -command {
    not_implemented_yet "Printing is not implemented, yet"
    focus .dialogInfo
  } -relief raised -text "Print" -width 6
  pack .dialogInfo.f.buttons.print -padx 10 -side left

  button .dialogInfo.f.buttons.update -borderwidth 2 -command {
    update_info
  } -relief raised -text "Update" -width 6
  pack .dialogInfo.f.buttons.update -padx 10 -side left

  button .dialogInfo.f.buttons.ok -borderwidth 2 -command {
    destroy .dialogInfo
  } -relief raised -text "OK" -width 6
  pack .dialogInfo.f.buttons.ok -padx 10 -side left

  pack .dialogInfo.f.buttons

}

# ###############################################################
# ###############################################################
# ###############################################################

proc addTree {mylist t root} {
  set first [lindex $mylist 0]
  if { [llength $first] == 1 } {
    set nameX [string map {"&" ".AND."} [join $first]]
    $t insert end $root $nameX -text [join $first]
    addTree [lrange $mylist 1 end] $t $root
  } elseif { [llength $first] > 1 } {

    set firstname [lindex $first 0]
    while {[llength $firstname] > 1} {
      set firstname [lindex $firstname 0]
    }
    set firstname [join $firstname]

    set lastname [lindex $first end]
    while {[llength $lastname] > 1} {
      set lastname [lindex $lastname end]
    }
    set lastname [join $lastname]

    set nameX [string map {"&" ".AND."} "$firstname.$lastname"]

    $t insert end $root $nameX -text "$firstname ... $lastname" -fill darkblue
    addTree $first $t $nameX
    addTree [lrange $mylist 1 end] $t $root
  }
}

proc createTree {f list} {
  set MAXSIZE 8

  set n [llength $list]
  while {$n >= $MAXSIZE} {
    set g [expr $n / $MAXSIZE]
    for {set i 0} {$i < $g} {incr i} {
      set list [lreplace $list $i [expr $i + $MAXSIZE - 1] [lrange $list $i [expr $i + $MAXSIZE - 1]]]
    }
    set n [llength $list]
  }

  Tree $f.tree

  if {$list != ""} {
    addTree $list $f.tree root
    pack $f.tree -fill both -expand yes
  }

}

# ###############################################################
# ###############################################################
# ###############################################################

proc browse_variables_byName {  } {
  toplevel .dialog
  wm title .dialog "Browse variables by name"
  wm iconname .dialog "Browse variables by name"
  wm geometry .dialog 640x480
  grab set .dialog

  global vname
  set vname ""

  frame .dialog.browseVariablesByName -relief raised
  pack .dialog.browseVariablesByName -expand yes -fill both

  set list [bddscout_listVariablesByName]
  createTree .dialog.browseVariablesByName $list

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  frame .dialog.browseVariablesByName.buttons -relief raised

  button .dialog.browseVariablesByName.buttons.view -borderwidth 2 -command {
    global vname
    set vname [.dialog.browseVariablesByName.tree selection get]
    if {$vname != ""} {
      set vname [join $vname]
      if {[.dialog.browseVariablesByName.tree nodes [join $vname]] == ""} {
        set vname [string map {".AND." "&"} $vname]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6

  pack .dialog.browseVariablesByName.buttons.view -padx 10 -side left

  button .dialog.browseVariablesByName.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseVariablesByName.buttons.cancel -padx 10 -side right

  pack .dialog.browseVariablesByName.buttons

  tkwait window .dialog
  return $vname
}

# ###############################################################
# ###############################################################
# ###############################################################


proc browse_formulae_byName {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by name"
  wm iconname .dialog "Browse formulae by name"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByName -relief raised
  pack .dialog.browseFormulaeByName -expand yes -fill both

  set list [bddscout_listFormulaeByName]
  createTree .dialog.browseFormulaeByName $list

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  frame .dialog.browseFormulaeByName.buttons -relief raised

  button .dialog.browseFormulaeByName.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByName.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByName.tree nodes [join $fname]] == ""} {
        set fname [string map {".AND." "&"} $fname]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6

  pack .dialog.browseFormulaeByName.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByName.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByName.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByName.buttons

  tkwait window .dialog
  return $fname
}

proc browse_formulae_byNodeNumber {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by node number"
  wm iconname .dialog "Browse formulae by node number"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByNodeNumber -relief raised
  pack .dialog.browseFormulaeByNodeNumber -expand yes -fill both

  set list [bddscout_listFormulaeByNodeNumber]
  createTree .dialog.browseFormulaeByNodeNumber $list

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  frame .dialog.browseFormulaeByNodeNumber.buttons -relief raised

  button .dialog.browseFormulaeByNodeNumber.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByNodeNumber.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByNodeNumber.tree nodes $fname] == ""} {
        set fname [string map {".AND." "&"} $fname]
        set fname [string range $fname 0 [expr [string last "(" $fname] -1]]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6
  pack .dialog.browseFormulaeByNodeNumber.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByNodeNumber.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByNodeNumber.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByNodeNumber.buttons

  tkwait window .dialog
  return $fname
}

proc browse_formulae_byNodeMaxLevel {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by graph depth"
  wm iconname .dialog "Browse formulae by graph depth"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByNodeMaxLevel -relief raised
  pack .dialog.browseFormulaeByNodeMaxLevel -expand yes -fill both

  set list [bddscout_listFormulaeByNodeMaxLevel]
  createTree .dialog.browseFormulaeByNodeMaxLevel $list

  frame .dialog.browseFormulaeByNodeMaxLevel.buttons -relief raised

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  button .dialog.browseFormulaeByNodeMaxLevel.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByNodeMaxLevel.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByNodeMaxLevel.tree nodes $fname] == ""} {
        set fname [string map {".AND." "&"} $fname]
        set fname [string range $fname 0 [expr [string last "(" $fname] -1]]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6
  pack .dialog.browseFormulaeByNodeMaxLevel.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByNodeMaxLevel.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByNodeMaxLevel.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByNodeMaxLevel.buttons

  tkwait window .dialog
  return $fname
}

proc browse_formulae_byNodeAvgLevel {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by node avg level"
  wm iconname .dialog "Browse formulae by node avg level"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByNodeAvgLevel -relief raised
  pack .dialog.browseFormulaeByNodeAvgLevel -expand yes -fill both

  set list [bddscout_listFormulaeByNodeAvgLevel]
  createTree .dialog.browseFormulaeByNodeAvgLevel $list

  frame .dialog.browseFormulaeByNodeAvgLevel.buttons -relief raised

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  button .dialog.browseFormulaeByNodeAvgLevel.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByNodeAvgLevel.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByNodeAvgLevel.tree nodes $fname] == ""} {
        set fname [string map {".AND." "&"} $fname]
        set fname [string range $fname 0 [expr [string last "(" $fname] -1]]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6
  pack .dialog.browseFormulaeByNodeAvgLevel.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByNodeAvgLevel.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByNodeAvgLevel.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByNodeAvgLevel.buttons

  tkwait window .dialog
  return $fname
}

proc update_info {  } {
  global bddscoutINFO
  global biddyINFO

  set listName [bddscout_listFormulaeByName]
  set num [llength $listName]

  if {($num == 0) || ($num > 99)} {
    set maxnodes "-"
    set maxdepth "-"
    set avgdepth "-"
  } else {

    set listNumber [bddscout_listFormulaeByNodeNumber]
    set listMax [bddscout_listFormulaeByNodeMaxLevel]
    set listAvg [bddscout_listFormulaeByNodeAvgLevel]

    set f [join [lindex $listNumber end]]
    set maxnodes [string range $f 0 [expr [string last "(" $f] -1]]
    set nodes [findData $maxnodes $listNumber]
    set max [findData $maxnodes $listMax]
    set avg [findData $maxnodes $listAvg]
    set maxnodes "$maxnodes (nodes = $nodes, depth = $max, avg depth = $avg)"

    set f [join [lindex $listMax end]]
    set maxdepth [string range $f 0 [expr [string last "(" $f] -1]]
    set nodes [findData $maxdepth $listNumber]
    set max [findData $maxdepth $listMax]
    set avg [findData $maxdepth $listAvg]
    set maxdepth "$maxdepth (nodes = $nodes, depth = $max, avg depth = $avg)"

    set f [join [lindex $listAvg end]]
    set avgdepth [string range $f 0 [expr [string last "(" $f] -1]]
    set nodes [findData $avgdepth $listNumber]
    set max [findData $avgdepth $listMax]
    set avg [findData $avgdepth $listAvg]
    set avgdepth "$avgdepth (nodes = $nodes, depth = $max, avg depth = $avg)"

  }

  set list [list \
    "\nNumber of functions = $num" \
    "\nMax nodes = $maxnodes" \
    "\nMax depth = $maxdepth" \
    "\nMax avg depth = $avgdepth" \
  ]

  set bddscoutINFO [join $list]

  set list [list \
    "\nBiddy_VariableTableNum = " [biddy_variable_num] \
    "\nBiddy_NodeTableSize = " [biddy_table_size] \
    "\nBiddy_NodeTableMax = " [biddy_table_max] \
    "\nBiddy_NodeTableNum = " [biddy_table_number] \
    "\nBiddy_NodeTableNumF = " [biddy_table_fortified] \
    "\nBiddy_NodeTableFOA = " [biddy_table_foa] \
    "\nBiddy_NodeTableCompare = " [biddy_table_compare] \
    "\nBiddy_NodeTableAdd = " [biddy_table_add] \
    "\nBiddy_NodeTableGenerated = " [biddy_table_generated] \
    "\nBiddy_NodeTableBlockNumber = " [biddy_block_number] \
    "\nBiddy_NodeTableGarbage = " [biddy_garbage_number] \
    "\nBiddy_NodeSwapNumber = " [biddy_swap_number] \
    "\nBiddy_NodeSiftingNumber = " [biddy_sifting_number] \
    "\nBiddy_ListUsed = " [biddy_list_used] \
    "\nBiddy_ListMaxLength = " [biddy_list_max_length] \
    "\nBiddy_ListAvgLength = " [biddy_list_avg_length] \
    "\nBiddy_IteCacheSearch = " [biddy_cache_ITE_search] \
    "\nBiddy_IteCacheFind = " [biddy_cache_ITE_find] \
    "\nBiddy_IteCacheOverwrite = " [biddy_cache_ITE_overwrite] \
  ]

  set biddyINFO [join $list]
}

proc findData { f list} {
  set i [string first "$f\(" $list]
  set j2 [string first " " $list $i]
  if {$j2 == -1} {
    set j2 [expr [string length $list] - 1]
  } else {
    set j2 [expr $j2 - 1]
  }
  set j1 [string last "(" $list $j2]
  return [string range $list [expr $j1 + 1] [expr $j2 - 1]]
}

proc bddscout_bitmap {win type gstype} {
  global OS
  global DESKTOP_DPI
  global GHOSTSCRIPT_EXE
  global ZOOM
  global XMAX
  global YMAX
  global BDDNAME

  set dpi 300

  if {$BDDNAME == ""} return;

  if {[file executable $GHOSTSCRIPT_EXE] != 1} {
    not_implemented_yet "Cannot run Ghostscript ($GHOSTSCRIPT_EXE)"
    return
  }

  puts "Exporting $BDDNAME.$type ..."

  set filename [tk_getSaveFile -title "Export to [string toupper $type]" -initialfile "$BDDNAME.$type" -parent $win]

  if {[string length $filename] != 0} {

    #remember last path
    cd [file dirname $filename]

    set koord [$win bbox all]
    if {$koord == ""} {set koord [list 0 0 0 0]}
    set x1 [lindex $koord 0]
    set y1 [lindex $koord 1]
    set x2 [lindex $koord 2]
    set y2 [lindex $koord 3]

    if {$OS == "unix"} {

      set gs [open "|$GHOSTSCRIPT_EXE -q -sDEVICE=$gstype -g[expr round(($x2-$x1)*($dpi/$DESKTOP_DPI))]x[expr round(($y2-$y1)*($dpi/$DESKTOP_DPI))] -r$dpi -dNOPAUSE -dBATCH -dSAFER -sOutputFile=$filename -" w]

      $win postscript -channel $gs -pageanchor sw -pagex 0 -pagey 0 -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1]

      close $gs

    } elseif {$OS == "windows"} {

      $win postscript -file tmp.ps -pageanchor sw -pagex 0 -pagey 0 -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1]

      exec $GHOSTSCRIPT_EXE -q -sDEVICE=$gstype -g[expr round(($x2-$x1)*($dpi/$DESKTOP_DPI))]x[expr round(($y2-$y1)*($dpi/$DESKTOP_DPI))] -r$dpi -dNOPAUSE -dBATCH -dSAFER -sOutputFile=$filename tmp.ps

      file delete tmp.ps

    }

  }
}

# ####################################################################
# Load extensions
# ####################################################################

puts "Importing extensions ... "

if {[catch {package require bddscoutBRA} errid]} {
  puts "  $errid"
  puts "  Extension bddscoutBRA (Best Reordering Algorithm) ... broken or not available"
} else {
  puts "  Extension bddscoutBRA (Best Reordering Algorithm) ... OK"
}

if {[catch {package require bddscoutIFIP} errid]} {
  puts "  $errid"
  puts "  Extension bddscoutIFIP (IFIP benchmark) ... broken or not available"
} else {
  puts "  Extension bddscoutIFIP (IFIP benchmark) ... OK"
}

if {[catch {package require bddscoutBDDTRACES} errid]} {
  puts "  $errid"
  puts "  Extension bddscoutBDDTRACES (Bdd Traces) ... broken or not available"
} else {
  puts "  Extension bddscoutBDDTRACES (Bdd Traces) ... OK"
}

# ####################################################################
# Final initialization
# ####################################################################

wm title . "BDD Scout"
wm iconname . "bddscout"

update_info
puts "Ready!"

if { $argc != 0 } {
  constructBDD
}
