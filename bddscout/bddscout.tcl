#!/bin/sh
# the next line executes the program\
exec wish "$0" "$@"

# MAYBE YOU MUST USE ONE OF THE FOLLOWING LINES
# exec /usr/local/bin/wish "$0" "$@"
# exec /usr/bin/wish8.4-X11 "$0" "$@"
# exec /home/meolic/ActiveTcl/bin/wish "$0" "$@"

# HELP ON Tcl/Tk + BWidget
# http://docs.activestate.com/activetcl/8.6/full_toc.html

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
  set GHOSTSCRIPT_EXE "C:/Program Files/gs/gs9.18/bin/gswin64c.exe"
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
#  Revision    [$Revision: 320 $]
#  Date        [$Date: 2017-10-01 12:02:23 +0200 (ned, 01 okt 2017) $]
#
#  Copyright   [This file is part of Bdd Scout package.
#               Copyright (C) 2008, 2017 UM FERI
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
#   therefore, constructed BDD may not be identical
#   to the one being showed, e.g. different node number
#   you can redraw BDD (e.g. Browse By Name) to see, what is
#   really stored in the memory
#

# Here, version 1.0 should be given due to the problems with
# "package ifneeded" used in pkgIndex.tcl in the packages that use this.
package provide bddscout 1.0

# Use this if you start the program using wish
lappend auto_path .
lappend auto_path /usr/lib/bddscout

# ####################################################################
# Show splash screen
# ####################################################################

wm withdraw .
toplevel .splash
frame .splash.f -width 600 -height 400 -bg WHITE
pack propagate .splash.f 0
wm geometry .splash +[expr {([winfo screenwidth .]-600)/2}]+[expr {([winfo screenheight .]-400)/2}]
wm attributes .splash -topmost yes
wm overrideredirect .splash 1
update idletasks

set SPLASHTEXT ""
label .splash.f.l -text "BDD Scout v1.7.3" -font [list TkHeadingFont 36] -fg WHITE -bg BLACK
pack .splash.f.l -expand 1
label .splash.f.m1 -text "Copyright (C) 2008, 2017 UM FERI" -font [list TkFixedFont 12] -fg BLACK -bg WHITE
pack .splash.f.m1 -fill x -expand 0
label .splash.f.m2 -text "Robert Meolic (robert.meolic@um.si)" -font [list TkFixedFont 12] -fg BLACK -bg WHITE
pack .splash.f.m2 -fill x -expand 0
label .splash.f.m3 -text "This is free software!" -font [list TkFixedFont 12 bold] -fg BLACK -bg WHITE
pack .splash.f.m3 -fill x -expand 0
label .splash.f.t -textvariable SPLASHTEXT -font [list TkCaptionFont 12] -fg BLACK -bg WHITE
pack .splash.f.t -fill x -expand 1
pack .splash.f -expand 1
update idletasks

# ####################################################################
# Start Bdd View script
# ####################################################################

package require bddview
wm iconify .
wm title . "BDD Scout v1.7.3"
wm iconname . "bddscout"
update idletasks

# ####################################################################
# Create and add new window (Input Boolean function)
# ####################################################################

set INPUT ""
set INPUTTYPE 0
set inputwin [frame $verticalwindow.inputline -relief flat -highlightthickness 1 -highlightcolor gray80 -bg $COLORBG]
$verticalwindow add $inputwin -after $horizontalwindow -height 32 -stretch never
entry $inputwin.entry -font [list $FONTLABEL 10] -relief solid -bd 1 -bg $COLORBG -exportselection yes -textvariable INPUT

#label $inputwin.label -text "Boolean expression (infix +*~^>< format):" -font [list $FONTINFO 10] -relief flat -bg $COLORBG
#pack $inputwin.label -side left -fill y -expand no -padx 2 
#pack $inputwin.entry -side right -fill x -expand yes -padx 2
#bind $inputwin.entry <Return> {parseinput}
#update idletasks

if {($OS == "unix")} {
  frame $inputwin.label -width 300 -height 32 -bg $COLORBG
}
if {($OS == "windows")} {
  frame $inputwin.label -width 300 -height 32 -bg $COLORBG
}
if {($OS == "Darwin")} {
  frame $inputwin.label -width 300 -height 32 -bg $COLORBG
}

pack propagate $inputwin.label 0
set inputtype [tk_optionMenu $inputwin.label.menu INPUTTYPETEXT \
    "Boolean expression (infix +*~^>< format):" \
    "Tcl command (e.g. biddy_printf_sop F):" \
]
$inputtype entryconfigure 0 -command {set INPUTTYPE 0}
$inputtype entryconfigure 1 -command {set INPUTTYPE 1}

if {($OS == "unix")} {
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -indicatoron 0 -anchor e -font [list $FONTINFO 10] -activebackground $COLORBG
  $inputtype configure -relief flat -bd 0 -font [list $FONTINFO 12] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
}
if {($OS == "windows")} {
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -indicatoron 0 -anchor e -font [list $FONTINFO 8] -activebackground $COLORBG 
  $inputtype configure -relief flat -bd 0 -font [list $FONTINFO 10] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
}
if {($OS == "Darwin")} {
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -indicatoron 0 -anchor e -font [list $FONTINFO 10] -activebackground $COLORBG
  $inputtype configure -relief flat -bd 0 -font [list $FONTINFO 12] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
}

pack $inputwin.label.menu -fill both -expand yes
pack $inputwin.label -side left -fill y -expand no
pack $inputwin.entry -side right -fill x -expand yes -padx 2
bind $inputwin.entry <Return> {parseinput}
update idletasks

# ####################################################################
# Create and add new window (Select Boolean function)
# ####################################################################

set selectwin [frame $mainframe.verticalwindow.horizontalwindow.form -relief flat -highlightthickness 1 -highlightcolor gray80 -bg $COLORBG]
$mainframe.verticalwindow.horizontalwindow add $selectwin -before $mainwin -width 300 -stretch never
Tree $selectwin.browser -relief flat -highlightthickness 0 -bg $COLORBG -deltay 24 -selectcommand changeform
pack $selectwin.browser -fill both -expand yes -padx 0 -pady 0
update idletasks

# ####################################################################
# Create and add new window (Variables)
# ####################################################################

set varwin [frame $mainframe.verticalwindow.horizontalwindow.var -relief flat -highlightthickness 1 -highlightcolor gray80 -bg $COLORBG]
$mainframe.verticalwindow.horizontalwindow add $varwin -after $mainwin -width 120 -stretch never
ListBox $varwin.browser -multicolumn no -selectmode none -relief flat -highlightthickness 0 -fg $COLORFG -bg $COLORBG -deltay 24
pack $varwin.browser -fill both -expand yes -padx 0 -pady 0
update idletasks

$varwin.browser bindImage <ButtonPress-1> swapwithhigher
$varwin.browser bindText <Double-Button-1> swapwithhigher
$varwin.browser bindText <Double-Button-3> swapwithlower

# ####################################################################
# Load functions written in C
# ####################################################################

after 800
set SPLASHTEXT "Importing BDD Scout library... "
update idletasks
after 400
.splash.f configure -bg $COLORBG
.splash.f.m1 configure -fg BLACK -bg $COLORBG
.splash.f.m2 configure -fg BLACK -bg $COLORBG
.splash.f.m3 configure -fg BLACK -bg $COLORBG
.splash.f.t configure -fg BLACK -bg $COLORBG
update idletasks

puts -nonewline "Importing BDD Scout library... "
package require bddscout-lib
bddscout_initPkg
puts "OK"

# ####################################################################
# Add new butons to the toolbar
#
# IMAGES FOR TOOLBAR ICONS ARE BASE64 ENCODED GIFS
#
# Some images are from Crystal Project by Everaldo Coelho
#
# Obtained from:
# http://www.iconarchive.com/show/crystal-clear-icons-by-everaldo.html
#
# License: LGPL
#
# --------------------------------------------------------------------
#
# Some images are from 
#
# Obtained from: iOS 7 Icons Project
# http://www.iconarchive.com/show/ios7-icons-by-icons8.html
#
# License: Linkware (Backlink to http://icons8.com required)
#
# Details:
#
# The icons are free for personal use and also free for commercial use,
# but we require linking to our web site.
# We distribute them under the license called
# Creative Commons Attribution-NoDerivs 3.0 Unported.
#
# --------------------------------------------------------------------
#
# Some images are from Boomy toolbar icon set by Milosz Wlazlo
#
# Obtained from:
# http://www.iconarchive.com/category/application/boomy-icons-by-milosz-wlazlo.html
#
# License: Free for non-commercial use
#
# Details:
# All icons in the Boomy toolbar package are for personal use only.
# You may also use the icons in open source and freeware projects as
# long as you give credits to the author of the icons and link to his
# website (miloszwl.deviantart.com).
#
# The image data may be changed by You in a limited way, only by
# combining the included icons and adjusting hue, saturation, brightness.
#
# --------------------------------------------------------------------
#
# ####################################################################

set iconNew {
R0lGODlhIAAgAPYAAH1VAIBXAIVbAIpeAI1jAZFmAJVqAZtuAJRsCZ1wAZ51
DKJ0AKh3AKd4AKx8AKR6C7B/AKF6Ea+BAq6FDrODAbmHAL2KAKyEE7SLErqP
ErWPG76WFraRHL6XHL6YH76YIMKOAMWTAsmVAMeaCsOdHMyjFtWsHMKeJMei
JMmkJM6pI8OjLcqmKM2rLNGsJdGtK9eyJti2K9y4LcSlMta1Mt28NN2+Ot/B
PeXBMeHCPdK2QNq9Qt/DS93EUeLFQuTJR+jLROXLS+jOS+vST+XNUuTNWufR
VenSVOfTWenVW+vZX+rWY+zbZO3dafDfcO/hbfHgZfDibe/icfHkc/PodfHl
evTrfPfxfvHkgfbugfXqj/fxg/nzhfr1ivz7jfTrkvfxpPv2o/78oPr1q///
rfz6tv/+uv37wv/+zf/+0///3P//4v//9f///wAAAAAAAAAAAAAAAAAAAAAA
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAACH5BAEAAG4A
LAAAAAAgACAAAAf+gG6Cg4SFhoeIiRUUEA4NCwsJiZNuIBYWixQUEhIOCweg
lIUiICAjKjY9Ras9PDYkCQcGooIiIjBIWmNnaWlqv2hmVRgGxaIhIT9gamxs
a2dgWlVMS0tOHMUFBZQgMWVtbWlhWU5QSkVEPz4/F8UGBASJFiBLbG1mXVdX
U1FMSkfphjxwB0/RCDBt0Hjxko9fEyVIhhBBEgHegAEEBCCqUMIMmzBcumy5
QqUfxCNHlNCYseIEhowbaaRZs6VmFir8mCRBgiSJEn9HgJC4eKgChB1pzkyx
YoWfSSRGeP5ccsTHhQEbKfRQAyZKlClSnlDjeQTJEiZokeRQoPEQoyL+ar4w
gQIFLUSJQ46gbUKVBoK2hig4KLIGi08lJ4kQCRIkZZMmTIawKID1UCciapoM
4TlEYpAfjIn8hDzkxADAhRrxSJMENOPPPmL7CJKEycMhHACgJuSAAQ80RnL8
UBf7xg0bs5HY/sdBwO5BnnacIWK8+PHjyW0zOfJBAABEDBa8IBOkxo0cN4rL
Fv2k/ZEWpxFBSiGmiPl1oF2LZiIFrJIbCHx3CCQbdLFEDfopxtNZUlSRRRVN
BHEBAAIW8gkFYCEYxERSndXEUg9KgUQKFB4SywJHVFFDaAChZBYTTyxVhYgy
BIhIAgmwcEUNwjE2nBBEbPYTXVTVoECJhshpQgEUP8hQgwwuoNCBBhp4gEIL
PgCZDg4TIFmIAbKQcEMJGTxwAAIFwKMmAhFwkIILJhwZQACHuBOJNgTgSQBG
3gEQAAAEBDgnnYZko82haarpnHN/UujnnIgguk0ig1Y6SZ60ZKpppoEAADs=
}

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

set iconUp {
R0lGODlhEAAQAIABAAAAAP///yH5BAEKAAEALAAAAAAQABAAAAIajI+py+0P
GwCRBisRVtt0z3SfdoyJGaWqWgAAOw==
}

set iconDown {
R0lGODlhEAAQAIABAAAAAP///yH5BAEKAAEALAAAAAAQABAAAAIZjI+py+0O
XoiGMlvvwlkfrlDgNkrmiaZOAQA7
}

image create photo icon.new.small
icon.new.small put $iconNew

image create photo icon.bitmap.small
icon.bitmap.small put $iconBitmap

image create photo icon.pdf.small
icon.pdf.small put $iconPdf

image create photo icon.up
icon.up put $iconUp

image create photo icon.down
icon.down put $iconDown

$bb0 insert 0 -image icon.new.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -helptext "Clear all" -command {clear}

set ts1 [Separator $toolbar.ts1 -bg $COLORMENU -orient vertical]
pack $ts1 -side left -fill y -padx 6 -anchor w

set tb1 [ButtonBox $toolbar.tb1 -bg $COLORMENU -homogeneous 0 -spacing 2 -padx 1 -pady 1]

$tb1 add -image icon.bitmap.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -helptext "Export to PNG" -command {bddscout_bitmap $mainwin png png16m}

$tb1 add -image icon.pdf.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -helptext "Export to PDF" -command {bddscout_bitmap $mainwin pdf pdfwrite}

pack $tb1 -side left -anchor w

# ####################################################################
# Change and convert BDD type
# ####################################################################

set ACTIVEBDDTYPE ""

if {($OS == "unix")} {
  frame $toolbar.type -width 180 -height 32 -bg $COLORMENU
}
if {($OS == "windows")} {
  frame $toolbar.type -width 188 -height 32 -bg $COLORMENU
}
if {($OS == "Darwin")} {
  frame $toolbar.type -width 180 -height 32 -bg $COLORMENU
}

pack propagate $toolbar.type 0
set bddtype [tk_optionMenu $toolbar.type.menu BDDTYPE \
    "ROBDD" \
    "ROBDD with CE" \
    "ZBDD with CE" \
    "TZBDD" \
]
$bddtype entryconfigure 0 -command {changetype "BIDDYTYPEOBDD"}
$bddtype entryconfigure 1 -command {changetype "BIDDYTYPEOBDDC"}
$bddtype entryconfigure 2 -command {changetype "BIDDYTYPEZBDDC"}
$bddtype entryconfigure 3 -command {changetype "BIDDYTYPETZBDD"}

if {($OS == "unix")} {
  $toolbar.type.menu configure -relief flat -bd 0 -fg black -bg $COLORMENU \
      -highlightbackground black -highlightthickness 1\
      -activebackground $COLORMENU -font [list TkHeadingFont 12]
  $bddtype configure -relief flat -bd 0 -font [list TkHeadingFont 10] \
      -fg black -bg $COLORMENU -activeforeground black -activebackground $COLORGRID
}
if {($OS == "windows")} {
  $toolbar.type.menu configure -relief flat -bd 0 -fg black -bg $COLORMENU \
      -highlightbackground black -highlightthickness 1\
      -activebackground $COLORMENU -font [list TkHeadingFont 9]
  $bddtype configure -relief flat -bd 0 -font [list TkHeadingFont 9] \
      -fg black -bg $COLORMENU -activeforeground black -activebackground $COLORGRID
}
if {($OS == "Darwin")} {
  $toolbar.type.menu configure -relief flat -bd 0 -fg black -bg $COLORMENU \
      -highlightbackground black -highlightthickness 1\
      -activebackground $COLORMENU -font [list TkHeadingFont 12]
  $bddtype configure -relief flat -bd 0 -font [list TkHeadingFont 10] \
      -fg black -bg $COLORMENU -activeforeground black -activebackground $COLORGRID
}

pack $toolbar.type.menu -fill both -expand yes -padx 4
pack $toolbar.type -side left -anchor w -before $bb0

proc bddtypeUp {} {
  global BDDTYPE
  global BDDNAME
  global bddtype

  set type -1
  if {$BDDTYPE == "ROBDD"} {
    set type 0
  } elseif {$BDDTYPE == "ROBDD with CE"} {
    set type 1
  } elseif {$BDDTYPE == "ZBDD with CE"} {
    set type 2
  } elseif {$BDDTYPE == "TZBDD"} {
    set type 3
  }
  while { $type != -1 } {
    set type [expr $type - 1]
    if { $type == 0 } {
      if { [bddscout_check_formula "BIDDYTYPEOBDD" $BDDNAME] == 1 } {
        $bddtype invoke 0
        set type -1
      }
    } elseif { $type == 1 } {
      if { [bddscout_check_formula "BIDDYTYPEOBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 1
        set type -1
      }
    } elseif { $type == 2 } {
      if { [bddscout_check_formula "BIDDYTYPEZBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 2
        set type -1
      }
    } elseif { $type == 3 } {
      # this is not possible
    }
  }
}

proc bddtypeDown {} {
  global BDDTYPE
  global BDDNAME
  global bddtype

  set type 4
  if {$BDDTYPE == "ROBDD"} {
    set type 0
  } elseif {$BDDTYPE == "ROBDD with CE"} {
    set type 1
  } elseif {$BDDTYPE == "ZBDD with CE"} {
    set type 2
  } elseif {$BDDTYPE == "TZBDD"} {
    set type 3
  }
  while { $type != 4 } {
    set type [expr $type + 1]
    if { $type == 0 } {
      # this is not possible
    } elseif { $type == 1 } {
      if { [bddscout_check_formula "BIDDYTYPEOBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 1
        set type 4
      }
    } elseif { $type == 2 } {
      if { [bddscout_check_formula "BIDDYTYPEZBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 2
        set type 4
      }
    } elseif { $type == 3 } {
      if { [bddscout_check_formula "BIDDYTYPETZBDD" $BDDNAME] == 1 } {
        $bddtype invoke 3
        set type 4
      }
    }
  }
}

# necessary for GNU/Linux, not needed on MS Windows
bind $toolbar.type.menu <4> {bddtypeUp}
bind $toolbar.type.menu <5> {bddtypeDown}

# necessary for MS Windows, not needed on GNU/Linux
bind $toolbar.type.menu <MouseWheel> {
  if {%D > 0} {bddtypeUp}
  if {%D < 0} {bddtypeDown}
}

proc bddconvertUp {} {
  global BDDTYPE
  global bddtype

  if {$BDDTYPE == "ROBDD"} {
  } elseif {$BDDTYPE == "ROBDD with CE"} {
    converttype "BIDDYTYPEOBDD" false
    $bddtype invoke 0
  } elseif {$BDDTYPE == "ZBDD with CE"} {
    converttype "BIDDYTYPEOBDDC" false
    $bddtype invoke 1
  } elseif {$BDDTYPE == "TZBDD"} {
    converttype "BIDDYTYPEZBDDC" false
    $bddtype invoke 2
  }
}

proc bddconvertDown {} {
  global BDDTYPE
  global bddtype

  if {$BDDTYPE == "ROBDD"} {
    converttype "BIDDYTYPEOBDDC" false
    $bddtype invoke 1
  } elseif {$BDDTYPE == "ROBDD with CE"} {
    converttype "BIDDYTYPEZBDDC" false
    $bddtype invoke 2
  } elseif {$BDDTYPE == "ZBDD with CE"} {
    converttype "BIDDYTYPETZBDD" false
    $bddtype invoke 3
  } elseif {$BDDTYPE == "TZBDD"} {
  }
}

# necessary for GNU/Linux, not needed on MS Windows
bind $toolbar.type.menu <Control-4> {bddconvertUp}
bind $toolbar.type.menu <Control-5> {bddconvertDown}

# necessary for MS Windows, not needed on GNU/Linux
bind $toolbar.type.menu <Control-MouseWheel> {
  if {%D > 0} {bddconvertUp}
  if {%D < 0} {bddconvertDown}
}

# you should call "add command" as many times as you have entries
set formulamenu [menu .formulamenu -tearoff false -relief solid -bd 1 -bg $COLORBG]
$formulamenu add command
$formulamenu add command
$formulamenu add command
$formulamenu add command

$selectwin.browser bindText <ButtonPress-3> {showformulamenu %X %Y}

proc showformulamenu { x y fname } {
  global formulamenu
  global selectwin
  global BDDNAME
  if {$fname == $BDDNAME} {
    $formulamenu entryconfigure 0 -command {converttype "BIDDYTYPEOBDD" false}
    $formulamenu entryconfigure 1 -command {converttype "BIDDYTYPEOBDDC" false}
    $formulamenu entryconfigure 2 -command {converttype "BIDDYTYPEZBDDC" false}
    $formulamenu entryconfigure 3 -command {converttype "BIDDYTYPETZBDD" false}
  } else {
    $selectwin.browser selection set $fname
    $formulamenu entryconfigure 0 -command {converttype "BIDDYTYPEOBDD" true}
    $formulamenu entryconfigure 1 -command {converttype "BIDDYTYPEOBDDC" true}
    $formulamenu entryconfigure 2 -command {converttype "BIDDYTYPEZBDDC" true}
    $formulamenu entryconfigure 3 -command {converttype "BIDDYTYPETZBDD" true}
  }
  $formulamenu entryconfigure 0 -label "Draw ROBDD for $fname" -font [list TkHeadingFont 10]
  $formulamenu entryconfigure 1 -label "Draw ROBDD with CE for $fname" -font [list TkHeadingFont 10]
  $formulamenu entryconfigure 2 -label "Draw ZBDD with CE for $fname" -font [list TkHeadingFont 10]
  $formulamenu entryconfigure 3 -label "Draw TZBDD for $fname" -font [list TkHeadingFont 10]
  tk_popup $formulamenu [expr $x+16] [expr $y+8]
}

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
.menuFrame.file.menu add command -command menu_file_run_tclscript -label "Run tcl script ..."
.menuFrame.file.menu add separator
.menuFrame.file.menu add command -command menu_file_read_BDD -label "Import BDD ..."
.menuFrame.file.menu add command -command menu_file_read_BF -label "Import Boolean functions ..."
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
.menuFrame.view.menu add command -command menu_view_formulae_byPathNumber -label "Browse by path number ..."
.menuFrame.view.menu add command -command menu_view_formulae_byMintermNumber -label "Browse by minterm number ..."

# ####################################################################
# Implement commands assigned to menu buttons
# ####################################################################

proc menu_file_open {  } {
  global mainwin

  set filename [tk_getOpenFile -title "Select bddview file" -parent $mainwin]
  if {[string length $filename] != 0} {
    bddview_draw $filename
    constructBDD
    update_info

    #remember last path
    cd [file dirname $filename]
  }
}

proc menu_file_run_tclscript {  } {
  global mainwin

  set filename [tk_getOpenFile -title "Select tcl script" -parent $mainwin]
  if {[string length $filename] != 0} {
    source $filename

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

  bddscout_construct [llength $variables] [join $variables] [expr [llength $graph] -1] [join [join $graph]]
}

proc menu_file_read_BDD {  } {
  global mainwin
  global DOT_EXE

  set filename [tk_getOpenFile -title "Import BDD" -parent $mainwin]
  if {[string length $filename] != 0} {

    set bdd [bddscout_read_bdd $filename]
    drawbdd $bdd
    update_info

    #remember last path
    cd [file dirname $filename]
  }

}

proc menu_file_read_BF {  } {
  global mainwin
  global DOT_EXE

  set filename [tk_getOpenFile -title "Import Boolean functions" -parent $mainwin]
  if {[string length $filename] != 0} {

    set bdd [bddscout_read_bf $filename]
    drawbdd $bdd
    update_info

    #remember last path
    cd [file dirname $filename]
  }

}

proc menu_file_write_BDD {  } {
  global mainwin
  global BDDNAME

  set filename [tk_getSaveFile -title "Export BDD" -parent $mainwin]
  if {[string length $filename] != 0} {

    biddy_write_bdd $filename $BDDNAME

    #remember last path
    cd [file dirname $filename]
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
# THIS CODE IS FROM Donal Fellows.
# Unfortunatelly, not working for me.
# https://stackoverflow.com/questions/14530354/stdout-redirection
# ###############################################################

# Use a class to simplify the capture code
oo::class create CapturingTransform {
    variable var
    constructor {varName} {
        # Make an alias from the instance variable to the global variable
        my eval [list upvar \#0 $varName var]
    }
    method initialize {handle mode} {
        if {$mode ne "write"} {error "can't handle reading"}
        return {finalize initialize write}
    }
    method finalize {handle} {
        # Do nothing, but mandatory that it exists
    }

    method write {handle bytes} {
        append var $bytes
        # Return the empty string, as we are swallowing the bytes
        return ""
    }
}

# ###############################################################
# ###############################################################
# ###############################################################

proc clear { } {
  global BDDNAME
  global ACTIVEBDDTYPE
  global selectwin

  bddview_clear
  set t $ACTIVEBDDTYPE
  set BDDNAME ""
  update_info

  bddscout_exitPkg
  bddscout_initPkg
  # this should be compatible with MNGACTIVE in bddscout.c */
  set ACTIVEBDDTYPE "BIDDYTYPEOBDD"
  update_info

  changetype $t
  $selectwin.browser selection set "0"
}

proc changetype { t } {
  global BDDNAME
  global ACTIVEBDDTYPE
  global selectwin

  #puts "DEBUG changetype IN: t = <$t>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"

  if {$t != $ACTIVEBDDTYPE} {

    bddscout_change_type $t
    set ACTIVEBDDTYPE $t

    if { ($BDDNAME == "") || ([bddscout_check_formula $ACTIVEBDDTYPE $BDDNAME] == 0) } {
     set BDDNAME "0"
    }
    set name $BDDNAME
    set BDDNAME ""
    update_info

    $selectwin.browser selection set $name

  }

  #puts "DEBUG changetype OUT: t = <$t>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"
}

proc changeform { t fname } {
  global BDDNAME

  #puts "DEBUG changeform: fname = <$fname>, BDDNAME = <$BDDNAME>"

  if {($fname != "") && ($fname != $BDDNAME)} {
    set BDDNAME $fname
    drawbdd $fname
  }
}

proc converttype { t fchange} {
  global BDDNAME
  global ACTIVEBDDTYPE
  global bddtype
  global selectwin

  #puts "DEBUG converttype IN: t = <$t>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"

  if {$t == $ACTIVEBDDTYPE} {
    if {$fchange == true} {
      if { [bddscout_check_formula $ACTIVEBDDTYPE $BDDNAME] == false } {
        puts "Formula $BDDNAME does not exist!"
      } else {
        set name $BDDNAME
        set BDDNAME ""
        update_info
        $selectwin.browser selection set $name
      }
    }
  } else {
    if { [bddscout_check_formula $t $BDDNAME] == 0 } {
      #puts "DEBUG converttype: COPY $BDDNAME FROM $ACTIVEBDDTYPE TO $t"
      bddscout_copy_formula $BDDNAME $ACTIVEBDDTYPE $t
    }
    if {$t == "BIDDYTYPEOBDD"} {$bddtype invoke 0}
    if {$t == "BIDDYTYPEOBDDC"} {$bddtype invoke 1}
    if {$t == "BIDDYTYPEZBDDC"} {$bddtype invoke 2}
    if {$t == "BIDDYTYPETZBDD"} {$bddtype invoke 3}
  }

  #puts "DEBUG converttype OUT: t = <$t>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"
}

proc parseinput { } {
  global INPUT
  global INPUTTYPE
  global BDDNAME
  global selectwin

  if {$INPUT != ""} {

    set TRYCMD 0
    if {($INPUT == "biddy_node_number") ||
        ($INPUT == "biddy_node_max_level") ||
        ($INPUT == "biddy_node_avg_level") ||
        ($INPUT == "biddy_node_number_plain") ||
        ($INPUT == "biddy_dependent_variable_number") ||
        ($INPUT == "biddy_count_complemented") ||
        ($INPUT == "biddy_count_paths") ||
        ($INPUT == "biddy_count_minterm") ||
        ($INPUT == "biddy_density_function") ||
        ($INPUT == "biddy_density_bdd") ||
        ($INPUT == "biddy_printf_bdd") ||
        ($INPUT == "biddy_printf_table") ||
        ($INPUT == "biddy_printf_sop")
    } then {
      set INPUT "$INPUT $BDDNAME"
      set TRYCMD 1
    }

    if {($INPUTTYPE == 0) && ($TRYCMD == 0)} {

      set name [bddscout_parse_input_infix $INPUT]

      #puts "DEBUG parseinput: <$name>"

      if {$name != ""} {
        set BDDNAME ""
        update_info
        $selectwin.browser selection set $name
      } else {
        set TRYCMD 2
      }
      
    }

    if {($INPUTTYPE == 1) || ($TRYCMD == 1)} {

      #set mystdout ""
      #chan push stdout [CapturingTransform new mystdout]
      set result [eval $INPUT]
      #chan pop stdout

      if {$result != ""} {

        toplevel .result
        wm title .result "RESULT"
        wm iconname .result "RESULT"
        grab set .result

        message .result.m -width 800 -text "$INPUT\n$result\n"
        pack .result.m -fill both -expand yes

#        set x [expr {([winfo screenwidth .]-[.result.m cget -width])/2}]
#        set y [expr {([winfo screenheight .])/2}]
#        wm geometry .result +$x+$y

        frame .result.buttons -relief raised

        button .result.buttons.ok -borderwidth 2 -command {
          destroy .result
        } -relief raised -text "OK" -width 6
        pack .result.buttons.ok -padx 10 -side left

        pack .result.buttons

        tkwait window .result
      }

    }

  }
  set INPUT ""
}

proc drawbdd {fname} {
  global DOT_EXE
  global OS
  global ACTIVEBDDTYPE

  if {[file executable $DOT_EXE] != 1} {
    not_implemented_yet "Cannot run dot from Graphviz ($DOT_EXE)"
    return
  }

  set fname [lindex $fname 0]

  #puts "DEBUG drawbdd: <$fname>"

  if {$fname != ""} {
    set tmpfile "tmp.bddview"
    set tmpfile [bddscout_write_bddview $fname $tmpfile $DOT_EXE]
    bddview_draw $tmpfile
    file delete $tmpfile
  }
}

proc swapwithlower {varname} {
  global BDDNAME

  #puts "DEBUG swapwithlower: <$BDDNAME>"

  if {$varname == ".c."} {set varname "c"}
  set varname [string map {".AND." "&"} $varname]
  biddy_swap_with_lower $varname
  drawbdd $BDDNAME
  update_info
}

proc swapwithhigher {varname} {
  global BDDNAME
  global ACTIVEBDDTYPE

  #puts "DEBUG swapwithhigher: <$BDDNAME>"

  if {$varname == ".c."} {set varname "c"}
  set varname [string map {".AND." "&"} $varname]
  biddy_swap_with_higher $varname
  drawbdd $BDDNAME
  update_info
}

proc menu_view_formulae_byName {  } {
  global selectwin

  set fname [browse_formulae_byName]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

proc menu_view_formulae_byNodeNumber {  } {
  global selectwin

  set fname [browse_formulae_byNodeNumber]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

proc menu_view_formulae_byNodeMaxLevel {  } {
  global selectwin

  set fname [browse_formulae_byNodeMaxLevel]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

proc menu_view_formulae_byNodeAvgLevel {  } {
  global selectwin

  set fname [browse_formulae_byNodeAvgLevel]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

proc menu_view_formulae_byPathNumber {  } {
  global selectwin

  set fname [browse_formulae_byPathNumber]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

proc menu_view_formulae_byMintermNumber {  } {
  global selectwin

  set fname [browse_formulae_byMintermNumber]
  if {$fname != ""} {
    $selectwin.browser selection clear
    drawbdd $fname
  }
}

# ###############################################################
# ###############################################################
# ###############################################################

proc addList {mylist t root} {
  global icon.edit.small
  global FONTLABEL

  set first [lindex $mylist 0]
  while { [llength $first] == 1 } {
    set nameX [join $first]
    if {$nameX == "c"} {set nameX ".c."}
    set nameX [string map {"&" ".AND."} $nameX]
    $t insert end $nameX -text [join $first] -font [list $FONTLABEL 10] -image icon.down
    set mylist [lrange $mylist 1 end]
    set first [lindex $mylist 0]
  }
}

proc addTree {mylist t root} {
  global FONTLABEL
  global formulamenu
  
  set first [lindex $mylist 0]
  while { [llength $first] == 1 } {
    set nameX [string map {"&" ".AND."} [join $first]]
    $t insert end $root $nameX -text [join $first] -font [list $FONTLABEL 10]
    set mylist [lrange $mylist 1 end]
    set first [lindex $mylist 0]
  }

  if { [llength $first] > 0 } {

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

#   $t insert end $root $nameX -text "$firstname ... $lastname" -fill darkblue -font [list $FONTLABEL 10]
    $t insert end $root $nameX -text "$firstname..." -fill darkblue -font [list $FONTLABEL 10]
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

  Tree $f.tree -deltay 24

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

  set list [bddscout_list_variables_by_name]
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

  set list [bddscout_list_formulae_by_name]
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

  set list [bddscout_list_formulae_by_node_number]
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

  set list [bddscout_list_formulae_by_node_max_level]
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

  set list [bddscout_list_formulae_by_node_avg_level]
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

proc browse_formulae_byPathNumber {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by path number"
  wm iconname .dialog "Browse formulae by path number"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByPathNumber -relief raised
  pack .dialog.browseFormulaeByPathNumber -expand yes -fill both

  set list [bddscout_list_formulae_by_path_number]
  createTree .dialog.browseFormulaeByPathNumber $list

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  frame .dialog.browseFormulaeByPathNumber.buttons -relief raised

  button .dialog.browseFormulaeByPathNumber.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByPathNumber.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByPathNumber.tree nodes $fname] == ""} {
        set fname [string map {".AND." "&"} $fname]
        set fname [string range $fname 0 [expr [string last "(" $fname] -1]]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6
  pack .dialog.browseFormulaeByPathNumber.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByPathNumber.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByPathNumber.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByPathNumber.buttons

  tkwait window .dialog
  return $fname
}

proc browse_formulae_byMintermNumber {  } {
  toplevel .dialog
  wm title .dialog "Browse formulae by minterm number"
  wm iconname .dialog "Browse formulae by minterm number"
  wm geometry .dialog 640x480
  grab set .dialog

  global fname
  set fname ""

  frame .dialog.browseFormulaeByMintermNumber -relief raised
  pack .dialog.browseFormulaeByMintermNumber -expand yes -fill both

  set list [bddscout_list_formulae_by_minterm_number]
  createTree .dialog.browseFormulaeByMintermNumber $list

  set x [expr {([winfo screenwidth .]-800)/2}]
  set y 60
  wm geometry .dialog +$x+$y

  frame .dialog.browseFormulaeByMintermNumber.buttons -relief raised

  button .dialog.browseFormulaeByMintermNumber.buttons.view -borderwidth 2 -command {
    global fname
    set fname [.dialog.browseFormulaeByMintermNumber.tree selection get]
    if {$fname != ""} {
      set fname [join $fname]
      if {[.dialog.browseFormulaeByMintermNumber.tree nodes $fname] == ""} {
        set fname [string map {".AND." "&"} $fname]
        set fname [string range $fname 0 [expr [string last "(" $fname] -1]]
        destroy .dialog
      }
    }
  } -relief raised -text "Select" -width 6
  pack .dialog.browseFormulaeByMintermNumber.buttons.view -padx 10 -side left

  button .dialog.browseFormulaeByMintermNumber.buttons.cancel -borderwidth 2 -command {
    destroy .dialog
  } -relief raised -text "Cancel" -width 6
  pack .dialog.browseFormulaeByMintermNumber.buttons.cancel -padx 10 -side right

  pack .dialog.browseFormulaeByMintermNumber.buttons

  tkwait window .dialog
  return $fname
}

proc update_info {  } {
  global BDDNAME
  global varwin
  global selectwin
  global bddscoutINFO
  global biddyINFO

  #puts "DEBUG update_info START"

  set listName [bddscout_list_variables_by_order]
  set num [llength $listName]

  $varwin.browser delete [$varwin.browser items]
  if {$num > 0} {
    addList $listName $varwin.browser root
  }

  set listName [bddscout_list_formulae_by_name]
  set num [llength $listName]

  $selectwin.browser delete [$selectwin.browser nodes root]
  if {$num > 0} {
    addTree $listName $selectwin.browser root
  }

  if { $BDDNAME != "" } {
    $selectwin.browser selection set $BDDNAME
    $selectwin.browser see $BDDNAME
  }
 
  if {($num == 0) || ($num > 99)} {
    set maxnodes "-"
    set maxdepth "-"
    set avgdepth "-"
  } else {

    set listNumber [bddscout_list_formulae_by_node_number]
    set listMax [bddscout_list_formulae_by_node_max_level]
    set listAvg [bddscout_list_formulae_by_node_avg_level]

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
    "\nbiddy_variabletable_num = " [biddy_variabletable_num] \
    "\biddy_nodetable_size = " [biddy_nodetable_size] \
    "\nbiddy_nodetable_block_number = " [biddy_nodetable_block_number] \
    "\nbiddy_nodetable_generated = " [biddy_nodetable_generated] \
    "\nbiddy_nodetable_max = " [biddy_nodetable_max] \
    "\nbiddy_nodetable_num = " [biddy_nodetable_num] \
    "\nbiddy_nodetable_gc_number = " [biddy_nodetable_gc_number] \
    "\nbiddy_nodetable_swap_number = " [biddy_nodetable_swap_number] \
    "\nbiddy_nodetable_sifting_number = " [biddy_nodetable_sifting_number] \
    "\nbiddy_list_used = " [biddy_list_used] \
    "\nbiddy_list_max_length = " [biddy_list_max_length] \
    "\nbiddy_list_avg_length = " [biddy_list_avg_length] \
    "\nbiddy_opcache_search = " [biddy_opcache_search] \
    "\nbiddy_opcache_find = " [biddy_opcache_find] \
    "\nbiddy_opcache_overwrite = " [biddy_opcache_overwrite] \
  ]

  set biddyINFO [join $list]

  #puts "DEBUG update_info FINISH"
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

# this should be compatible with MNGACTIVE in bddscout.c */
set ACTIVEBDDTYPE "BIDDYTYPEOBDD"

wm deiconify .
update

set BDDNAME ""
update_info

if { $argc == 0 } {
  $selectwin.browser selection set "0"
} else {
  constructBDD
}

#update_info
puts "Ready!"

after 200
set SPLASHTEXT "READY!"

after 500
for {set a 1.} {$a>0.} {set a [expr $a-0.01]} {
 wm attributes .splash -alpha $a; update idletasks; after 10
}
destroy .splash
