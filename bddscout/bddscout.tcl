#!/bin/sh
# the next line executes the program\
exec wish "$0" "$@"

# MAYBE YOU MUST USE ONE OF THE FOLLOWING LINES
# exec /usr/local/bin/wish "$0" "$@"
# exec /usr/bin/wish8.4-X11 "$0" "$@"
# exec /home/meolic/ActiveTcl/bin/wish "$0" "$@"

#  Authors     [Robert Meolic (robert@meolic.com)]
#  Revision    [$Revision: 694 $]
#  Date        [$Date: 2024-06-30 20:28:55 +0200 (ned, 30 jun 2024) $]
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

# USAGE:
# ./bddscout.tcl
# ./bddscout.tcl example.bddview
#
# DATA FORMATS:
#
# BDD Scout is builded on bddview thus the native
# data format used in Open/Save is bddview format.
# See bddview.tcl for details.
#
# Example (example-robdd.bddview):
# ---------------------------------------------------
#type ROBDD
#var "B" "i" "d" "y"
#label 0 "Biddy" 40.0 1.0
#node 1 "B" 40.0 73.0
#terminal 2 "0" 9.0 145.0
#node 3 "i" 72.0 145.0
#node 4 "d" 45.0 217.0
#terminal 5 "0" 9.0 289.0
#node 6 "y" 72.0 289.0
#terminal 7 "0" 36.0 361.0
#terminal 8 "1" 108.0 361.0
#node 9 "d" 99.0 217.0
#terminal 10 "1" 135.0 289.0
#connect 0 1 s
#connect 1 2 l
#connect 1 3 r
#connect 3 4 l
#connect 3 9 r
#connect 4 5 l
#connect 4 6 r
#connect 6 7 l
#connect 6 8 r
#connect 9 6 l
#connect 9 10 r
# ---------------------------------------------------
#
# BDD Scout can import BDD given in prefix form:
# 1. the first word is the name of BDD
# 2. the second word is the name of top variable
# 3. any non-constant variable is followed by two
#    supgraphs in parenthesis, e.g. VAR (...) (...)
# 4. the name of constant variables are "0" and "1"
# 5. symbol * is used to denote complement edges.
#
# TO DO: CHANGE THIS FORMAT TO AN XML-BASED DESCRIPTION
#
# Example (example.bdd):
# ---------------------------------------------------
# Biddy
# B (* i (d (1) (y (* 1) (1))) (d (y (* 1) (1)) (1)))
#   (  i (d (1) (y (* 1) (1))) (d (y (* 1) (1)) (1)))
# ---------------------------------------------------
#
# BDD Scout can import Boolean function given in the
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
# TO DO: ADD SUPPORT FOR INFIX FORM
#
# BDD TYPE NAMES:
# ROBDD - use BIDDYTYPEOBDD
# ROBDDCE - use BIDDYTYPEOBDDC
# ZBDD - use BIDDYTYPEZBDD
# ZBDDCE - use BIDDYTYPEZBDDC
# TZBDD - use BIDDYTYPETZBDD
# TZBDDCE - use BIDDYTYPETZBDDC (not implemented, yet)
#
# API functions defined in bddscout.tcl (see near the end of file):
#
# proc bddscout_message {text1 text2}
# proc bddscout_clear {}
# proc bddscout_show {fname}
# proc bddscout_open {filename}
# proc bddscout_change_bddtype {type}
# proc bddscout_update {}
# proc bddscout_copy_formula {fname type1 type2}
# proc bddscout_count_nodes {fname}
# proc bddscout_reset_all_values {}
# proc bddscout_set_values {list}
# proc bddscout_get_value {v}
# proc bddscout_reset_all_probabilities {}
# proc bddscout_set_probabilities {list}
# proc bddscout_get_probability {v}
# proc bddscout_eval_probability {fname}
# proc bddscout_swap_with_higher {varname}
# proc bddscout_swap_with_lower {varname}
#
# API functions from bddview which can be used:
#
# proc bddview_bendsOnOff {}
# proc bddview_arrowsOnOff {}
# proc bddview_gridsOnOff {}
# proc bddview_save {filename}
# proc bddview_export_tex {filename caption document}
# proc bddview_print {filename}
#
# API functions from bddview which should NOT be used:
#
# proc bddview_message - use bddscout_message instead
# proc bddview_clear - use bddscout_clear instead
# proc bddview_open - use bddscout_open instead
#
# API functions defined in bddscoutTcl.c:
# if used from script these functions does not update GUI
# if used from input line these functions update list of Boolean functions and list of variables
#
# bddscout_read_bdd
# bddscout_read_bf
# bddscout_parse_input_infix
#
# Some other helpful functions defined in bddscout.tcl and bddscoutTcl.c:
#
# browse_variables_byName
# browse_formulae_byName
# browse_formulae_byNodeNumber
# browse_formulae_byNodeMaxLevel
# browse_formulae_byNodeAvgLevel
# browse_formulae_byPathNumber
# browse_formulae_byMintermNumber
# bddscout_list_variables_by_positon
# bddscout_list_variables_by_name
# bddscout_list_variables_by_order
# bddscout_list_variables_by_number
# bddscout_list_formulae_by_name
# bddscout_list_formulae_by_node_number
# bddscout_list_formulae_by_node_max_level
# bddscout_list_formulae_by_node_avg_level
# bddscout_list_formulae_by_path_number
# bddscout_list_formulae_by_minterm_number
#
# Biddy's functions (wrappers are defined in bddscoutTcl.c):
# if used from script these functions does not update GUI
# if used from input line these functions update list of Boolean functios and list of variables, only
#
# biddy_get_then
# biddy_get_else
# biddy_reset_variables_value
# biddy_set_variable_value
# biddy_get_variable_value
# biddy_clear_variables_data
# biddy_set_variable_data_int
# biddy_set_variable_data_float
# biddy_set_variable_data_string
# biddy_get_variable_data_int
# biddy_get_variable_data_float
# biddy_get_variable_data_string
# biddy_eval
# biddy_eval_probability
# biddy_set_alphabetic_ordering
# biddy_swap_with_higher
# biddy_swap_with_lower
# biddy_sifting
# biddy_sifting_on_function
# biddy_minimize
# biddy_maximize
# biddy_not
# biddy_ite
# and many others (see bddscoutTcl.c)
#
# TRICKY FEATURES:
#
# - BDD construction does not preserve variable ordering
#   therefore, the constructed BDD may not be identical
#   to the one being given in the file (e.g. different node number).
#
# - If "Export to PNG" and/or "Export to PDF" create an image with wrong
#   dimensions you can try to change the global variable DPI

# ####################################################################
# Tcl/Tk
# ####################################################################

# HELP ON Tcl/Tk + BWidget
# http://docs.activestate.com/activetcl/8.6/full_toc.html

# Here, version 1.0 should be given due to the problems with
# "package ifneeded" in the packages that use this.
package provide bddscout 1.0

# Use this if you start the program using wish
lappend auto_path .
lappend auto_path /usr/lib/bddscout

# DEBUGGING
# if {$tcl_platform(platform) == "windows"} {console show}

# ####################################################################
# Biddy
# ####################################################################

# on Linux, this file is manipulated from sed in Makefile
# on Windows, you have to cet this manual
# in any case, you should not forget to change BIDDYVERSION in biddy.h, too
set BIDDYVERSION "THEBIDDYVERSION"
if {$tcl_platform(platform) == "windows"} {set BIDDYVERSION "2.4.1"}

set BFCMDLIST [list \
  "bddview_save" \
  "bddview_export_tex" \
  "biddy_print_table" \
  "biddy_print_sop" \
  "biddy_print_minterms" \
  "biddy_dependent_variable_number" \
  "biddy_count_minterms" \
  "biddy_density_of_function" \
  "bddscout_reset_all_values" \
  "bddscout_set_values" \
  "biddy_eval" \
  "biddy_support" \
]

set CACMDLIST [list \
  "bddview_save" \
  "bddview_export_tex" \
  "biddy_change" \
  "biddy_varsubset" \
  "biddy_subset0" \
  "biddy_subset1" \
  "biddy_quotient" \
  "biddy_remainder" \
  "biddy_element_abstract" \
  "biddy_product" \
  "biddy_selective_product" \
  "biddy_supset" \
  "biddy_subset" \
  "biddy_permitsym" \
  "biddy_stretch" \
]

set BDDLCMDLIST [list \
  "bddview_save" \
  "bddview_export_tex" \
  "biddy_read_bddl_file" \
]

set CMDLIST [list \
  "bddview_save" \
  "bddview_export_tex" \
  "biddy_get_then" \
  "biddy_get_else" \
  "biddy_count_nodes" \
  "biddy_count_nodes_plain" \
  "biddy_count_complemented_edges" \
  "biddy_count_paths" \
  "biddy_max_level" \
  "biddy_avg_level" \
  "biddy_density_of_bdd" \
  "biddy_min_nodes" \
  "biddy_max_nodes" \
  "bddscout_count_nodes" \
  "bddscout_reset_all_probabilities" \
  "bddscout_set_probabilities" \
  "bddscout_eval_probability" \
]

# proc bddscout_clear {}
# proc bddscout_show {fname}
# proc bddscout_open {filename}
# proc bddscout_change_bddtype {type}
# proc bddscout_update {}
# proc bddscout_copy_formula {fname type1 type2}
# proc bddscout_count_nodes {fname}
# proc bddscout_reset_all_values {}
# proc bddscout_set_values {list}
# proc bddscout_get_value {v}
# proc bddscout_reset_all_probabilities {}
# proc bddscout_set_probabilities {list}
# proc bddscout_get_probability {v}
# proc bddscout_eval_probability {fname}
# proc bddscout_swap_with_higher {varname}
# proc bddscout_swap_with_lower {varname}

# ####################################################################
# Show splash screen
# ####################################################################

wm minsize . 1280 800
wm geometry . [winfo screenwidth .]x[winfo screenheight .]+[expr {([winfo screenwidth .]-1280)/2}]+[expr {([winfo screenheight .]-800)/2}]
set SPLASHBG "#FAF8F4"

puts "$tcl_platform(platform) / $tcl_platform(os)"
puts "Press CTRL if BDD Scout is freezed during loading!"

if {($tcl_platform(platform) == "unix")} {
  wm attributes . -zoomed yes
}
if {($tcl_platform(platform) == "windows") || ($tcl_platform(os) == "Darwin")} {
  wm state . zoomed
}

# we have noticed strange wm deadlocks on Ubuntu
# simply press CTRL if BDD Scout is freezed during loading
# moreover, you can disable splash to prevent these strange wm deadlock
if { $tcl_platform(platform) == "windows" } {
  set USESPLASH 1
} else {
  set USESPLASH 0
}

set SPLASHTEXT ""
set BDDVIEWTEXT ""

if { $USESPLASH == 1 } {

  toplevel .splash
  frame .splash.f -width 800 -height 400 -bg $SPLASHBG
  pack propagate .splash.f 0
  wm geometry .splash +[expr {([winfo screenwidth .]-800)/2}]+[expr {([winfo screenheight .]-400)/2}]
  wm attributes .splash -topmost yes
  wm overrideredirect .splash 1
  update idletasks

  label .splash.f.l -text "BDD Scout v$BIDDYVERSION" -font [list TkHeadingFont 36] -fg $SPLASHBG -bg BLACK
  pack .splash.f.l -expand 1
  label .splash.f.m1 -text "Copyright (C) 2008, 2019 UM FERI, Maribor, Slovenia" -font [list TkFixedFont 12] -fg BLACK -bg $SPLASHBG
  pack .splash.f.m1 -fill x -expand 0
  label .splash.f.m2 -text "Copyright (C) 2019, 2024 Robert Meolic, Slovenia" -font [list TkFixedFont 12] -fg BLACK -bg $SPLASHBG
  pack .splash.f.m2 -fill x -expand 0
  label .splash.f.m3 -text "Robert Meolic (robert@meolic.com)" -font [list TkFixedFont 12] -fg BLACK -bg $SPLASHBG
  pack .splash.f.m3 -fill x -expand 0
  label .splash.f.m4 -text "This is free software!" -font [list TkFixedFont 12 bold] -fg BLACK -bg $SPLASHBG
  pack .splash.f.m4 -fill x -expand 0
  label .splash.f.t -textvariable SPLASHTEXT -font [list TkCaptionFont 12] -fg BLACK -bg $SPLASHBG
  pack .splash.f.t -fill x -expand 1
  label .splash.f.m5 -textvariable BDDVIEWTEXT -font [list TkFixedFont 12] -fg BLACK -bg $SPLASHBG
  pack .splash.f.m5 -fill x -expand 0
  pack .splash.f -expand 1
  update idletasks

}

# ####################################################################
# Start Bdd View script
# ####################################################################

set bddscout__argc $argc
package require bddview
set argc $bddscout__argc
set BDDVIEWTEXT "Using bddview v$BDDVIEWVERSION"
wm title . "BDD Scout v$BIDDYVERSION"
wm iconname . "bddscout"
update idletasks

# ####################################################################
# OS specific settings
# bddview has:
#   set OS $tcl_platform(platform)
#   set OS1 $tcl_platform(os)
# ####################################################################

if {($OS == "unix") && !($OS1 == "Darwin")} {

  set GHOSTSCRIPT_EXE "/usr/bin/gs"
  set DOT_EXE "/usr/bin/dot"

  if {[file executable $GHOSTSCRIPT_EXE] != 1} {
    set GHOSTSCRIPT_EXE ""
  }

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
  }

} elseif {($OS == "unix") && ($OS1 == "Darwin")} {

  set GHOSTSCRIPT_EXE ""
  set DOT_EXE "/usr/local/bin/dot"

  if {[file executable $GHOSTSCRIPT_EXE] != 1} {
    set GHOSTSCRIPT_EXE ""
  }

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
  }

} elseif {$OS == "windows"} {

  set GHOSTSCRIPT_EXE [lindex [glob -nocomplain "C:/Program Files/gs/*/bin/gswin64c.exe"] 0]
  set DOT_EXE [lindex [glob -nocomplain "C:/Program Files/Graphviz/bin/dot.exe"] 0]
  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE [lindex [glob -nocomplain "C:/Program Files (x86)/*/bin/dot.exe"] 0]
  }

  if {[file executable $GHOSTSCRIPT_EXE] != 1} {
    set GHOSTSCRIPT_EXE ""
  }

  if {[file executable $DOT_EXE] != 1} {
    set DOT_EXE ""
  }

} else {

  set GHOSTSCRIPT_EXE ""
  set DOT_EXE ""

}

# ####################################################################
# Default folders
# ####################################################################

set BDDSCOUT_PATH_BIN "[file dirname [info nameofexecutable]]/../lib/bddscout"
if {[file exists "$BDDSCOUT_PATH_BIN/bddscout.tcl"] != 1} {
  set BDDSCOUT_PATH_BIN "[pwd]/bddscout"
  if {[file exists "$BDDSCOUT_PATH_BIN/bddscout.tcl"] != 1} {
    set BDDSCOUT_PATH_BIN [pwd]
  }
}
#bddview_message "BDDSCOUT_PATH_BIN" "$BDDSCOUT_PATH_BIN"
#puts $BDDSCOUT_PATH_BIN
set BDDSCOUT_PATH_CREATE "$BDDSCOUT_PATH_BIN/create"
#puts $BDDSCOUT_PATH_CREATE
set BDDSCOUT_PATH_SCRIPTS "$BDDSCOUT_PATH_BIN/scripts"
#puts $BDDSCOUT_PATH_SCRIPTS
set BDDSCOUT_PATH_EXAMPLES "$BDDSCOUT_PATH_BIN"
#puts $BDDSCOUT_PATH_EXAMPLES

# ####################################################################
# Create and add new window (Input Boolean function etc.)
# ####################################################################

set INPUT ""
set INPUTTYPE 0
set inputwin [frame $verticalwindow.inputline -relief flat -highlightthickness 1 -highlightcolor $COLORFRAME -bg $COLORBG]
$verticalwindow add $inputwin -after $horizontalwindow -height 40 -stretch never
entry $inputwin.entry -font [list $FONTFAMILYLABEL 12] -relief solid -bd 1 -bg $COLORBG -exportselection yes -textvariable INPUT

if {($OS == "unix")} {
  frame $inputwin.label -width 300 -height 32 -bg $COLORBG
}
if {($OS == "windows")} {
  frame $inputwin.label -width 436 -height 40 -bg $COLORBG
}
if {($OS == "Darwin")} {
  frame $inputwin.label -width 300 -height 32 -bg $COLORBG
}

# INPUTTYPE 0: Boolean expression
# INPUTTYPE 1: Unate cube set algebra
# INPUTTYPE 2-5: not used, yet
# INPUTTYPE 6: Knuth's BDDL command
# INPUTTYPE 7: Tcl command
# change also in parseinput{}
pack propagate $inputwin.label 0
# variable bddscout__selectedInputType is not used
set inputtype [tk_optionMenu $inputwin.label.menu bddscout__selectedInputType \
     "Boolean expression (infix *+!>^-())" \
     "Unate cube set algebra (+cube, -cube)" \
     "Knuth's BDDL command (infix &|~><^_?.)" \
     "Tcl command" \
]

frame $inputwin.cmd
set inputcmd [menubutton $inputwin.cmd.menu -text "f()"]

menu $inputcmd.tclcmd -tearoff 0
menu $inputcmd.bftclcmd -tearoff 0
menu $inputcmd.catclcmd -tearoff 0
menu $inputcmd.bddltclcmd -tearoff 0

$inputtype entryconfigure 0 -command {set INPUTTYPE 0; $bddtype invoke 0; $inputcmd configure -menu $inputcmd.bftclcmd}
$inputtype entryconfigure 1 -command {set INPUTTYPE 1; $bddtype invoke 2; $inputcmd configure -menu $inputcmd.catclcmd}
$inputtype entryconfigure 2 -command {set INPUTTYPE 6; $bddtype invoke 0; $inputcmd configure -menu $inputcmd.bddltclcmd}
$inputtype entryconfigure 3 -command {set INPUTTYPE 7; $inputcmd configure -menu $inputcmd.tclcmd}

set bddscout__wheel 0
# necessary for GNU/Linux, not needed on MS Windows
bind $inputcmd.tclcmd <4> {set bddscout__wheel 1}
bind $inputcmd.tclcmd <5> {set bddscout__wheel 1}
bind $inputcmd.bftclcmd <4> {set bddscout__wheel 1}
bind $inputcmd.bftclcmd <5> {set bddscout__wheel 1}
bind $inputcmd.catclcmd <4> {set bddscout__wheel 1}
bind $inputcmd.catclcmd <5> {set bddscout__wheel 1}
bind $inputcmd.bddltclcmd <4> {set bddscout__wheel 1}
bind $inputcmd.bddltclcmd <5> {set bddscout__wheel 1}
# necessary for MS Windows, not needed on GNU/Linux
bind $inputcmd.tclcmd <MouseWheel> {set bddscout__wheel 1}
bind $inputcmd.bftclcmd <MouseWheel> {set bddscout__wheel 1}
bind $inputcmd.catclcmd <MouseWheel> {set bddscout__wheel 1}
bind $inputcmd.bddltclcmd <MouseWheel> {set bddscout__wheel 1}

# this is executed when a command is selected (Boolean expression)
proc BFONECMD { i } {
  global BFCMDLIST
  global INPUT
  global bddscout__wheel
  if {$bddscout__wheel == 1} {
    set bddscout__wheel 0
    return
  }
  set INPUT [lindex $BFCMDLIST $i]
  parseinput 0
}
set i 0
foreach bddscout__cmd $BFCMDLIST {
  $inputcmd.bftclcmd add command \
    -label "Invoke command $bddscout__cmd" \
    -command [list BFONECMD $i]
  incr i
}

# this is executed when a command is selected (Unate cube set algebra)
proc CAONECMD { i } {
  global CACMDLIST
  global INPUT
  global bddscout__wheel
  if {$bddscout__wheel == 1} {
    set bddscout__wheel 0
    return
  }
  set INPUT [lindex $CACMDLIST $i]
  parseinput 0
}
set i 0
foreach bddscout__cmd $CACMDLIST {
  $inputcmd.catclcmd add command \
    -label "Invoke command $bddscout__cmd" \
    -command [list CAONECMD $i]
  incr i
}

# this is executed when a command is selected (Knuth's BDDL command)
proc BDDLONECMD { i } {
  global BDDLCMDLIST
  global INPUT
  global bddscout__wheel
  if {$bddscout__wheel == 1} {
    set bddscout__wheel 0
    return
  }
  set INPUT [lindex $BDDLCMDLIST $i]
  parseinput 0
}
set i 0
foreach bddscout__cmd $BDDLCMDLIST {
  $inputcmd.bddltclcmd add command \
    -label "Invoke command $bddscout__cmd" \
    -command [list BDDLONECMD $i]
  incr i
}

# this is executed when a command is selected (Tcl command)
proc ONECMD { i } {
  global CMDLIST
  global INPUT
  global bddscout__wheel
  if {$bddscout__wheel == 1} {
    set bddscout__wheel 0
    return
  }
  set INPUT [lindex $CMDLIST $i]
  parseinput 0
}
set i 0
foreach bddscout__cmd $CMDLIST {
  $inputcmd.tclcmd add command \
    -label "Invoke command $bddscout__cmd" \
    -command [list ONECMD $i]
  incr i
}

if {($OS == "unix")} {
  $inputtype configure -relief flat -bd 0 -font [list $FONTFAMILYINFO 12] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -anchor e -font [list $FONTFAMILYINFO 10] -activebackground $COLORBG -direction above -indicatoron 0
  $inputcmd configure -relief groove -bd 1 -font [list $FONTFAMILYINFO 10 italic] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID -direction above -indicatoron 0
  $inputcmd.bftclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.catclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.bddltclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.tclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
}
if {($OS == "windows")} {
  $inputtype configure -relief flat -bd 0 -font [list $FONTFAMILYINFO 12] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -anchor e -font [list $FONTFAMILYINFO 12] -activebackground $COLORBG -direction above -indicatoron 0
  $inputcmd configure -relief groove -bd 1 -font [list $FONTFAMILYINFO 12 italic] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID -direction above -indicatoron 0
  $inputcmd.bftclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 12] -activebackground $COLORGRID
  $inputcmd.catclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 12] -activebackground $COLORGRID
  $inputcmd.bddltclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 12] -activebackground $COLORGRID
  $inputcmd.tclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 12] -activebackground $COLORGRID
}
if {($OS == "Darwin")} {
  $inputtype configure -relief flat -bd 0 -font [list $FONTFAMILYINFO 12] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID
  $inputwin.label.menu configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -highlightthickness 0 -anchor e -font [list $FONTFAMILYINFO 10] -activebackground $COLORBG -direction above -indicatoron 0
  $inputcmd configure -relief groove -bd 1 -font [list $FONTFAMILYINFO 12 italic] \
      -fg black -bg $COLORBG -activeforeground black -activebackground $COLORGRID -direction above -indicatoron 0
  $inputcmd.bftclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.catclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.bddltclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputcmd.tclcmd configure -relief flat -bd 0 -fg black -bg $COLORBG \
      -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
}

$inputcmd configure -menu $inputwin.cmd.menu.bftclcmd

pack $inputwin.label.menu -fill both -expand yes
pack $inputwin.label -side left -fill y -expand no
pack $inputwin.cmd.menu -fill both -expand yes
pack $inputwin.cmd -side left -expand no
pack $inputwin.entry -side right -fill x -expand yes -padx 2
bind $inputwin.entry <Return> {parseinput 1}
update idletasks

# ####################################################################
# Create and add new window (Select Boolean function)
# ####################################################################

set selectwin [frame $mainframe.verticalwindow.horizontalwindow.form -relief flat -highlightthickness 1 -highlightcolor $COLORFRAME -bg $COLORBG]
$mainframe.verticalwindow.horizontalwindow add $selectwin -before $mainwin -width 300 -stretch never
Tree $selectwin.browser -relief flat -highlightthickness 0 -bg $COLORBG -deltay 24 -selectcommand changeform
pack $selectwin.browser -fill both -expand yes -padx 0 -pady 0
update idletasks

# ####################################################################
# Create and add new window (Variables)
# ####################################################################

set varwin [frame $mainframe.verticalwindow.horizontalwindow.var -relief flat -highlightthickness 1 -highlightcolor $COLORFRAME -bg $COLORBG]
$mainframe.verticalwindow.horizontalwindow add $varwin -after $mainwin -width 150 -stretch never
ListBox $varwin.browser -multicolumn no -selectmode none -relief flat -highlightthickness 0 -fg $COLORFG -bg $COLORBG -deltay 24
pack $varwin.browser -fill both -expand yes -padx 0 -pady 0
update idletasks

$varwin.browser bindImage <ButtonPress-1> {bddscout_swap_with_higher}
$varwin.browser bindImage <ButtonPress-3> {bddscout_swap_with_lower}

# ####################################################################
# Load functions written in C
# ####################################################################

if { $USESPLASH == 1 } {

  after 800
  set SPLASHTEXT "Importing BDD Scout library... "
  update idletasks
  after 400
  .splash.f configure -bg $COLORBG
  .splash.f.l configure -fg $COLORBG
  .splash.f.m1 configure -fg BLACK -bg $COLORBG
  .splash.f.m2 configure -fg BLACK -bg $COLORBG
  .splash.f.m3 configure -fg BLACK -bg $COLORBG
  .splash.f.m4 configure -fg BLACK -bg $COLORBG
  .splash.f.m5 configure -fg $COLORBG -bg BLACK
  .splash.f.t configure -fg BLACK -bg $COLORBG
  update idletasks

}

puts -nonewline "Importing BDD Scout library... "
package require bddscout-lib
bddscout_initPkg
puts "OK"

# ####################################################################
# Add new butons to the toolbar
#
# THEY ARE BASE64 ENCODED GIFS (32x32)
# - create 32x32 PNG
# - use command "base64 filename.png"
# - use color: #49B2C2
#
# Icons made by Freepik (www.freepik.com)
# Downloaded from Flaticon (www.flaticon.com)
# License: Flaticon Basic License (free license with attribution)
# Obtained from:
#
# <Bitmap>,<Pdf>
# https://www.flaticon.com/packs/file-formats-icons
#
# <Formula>
# https://www.flaticon.com/packs/maths-symbols
#
# Icons made by Good Ware (www.flaticon.com/authors/good-ware)
# Downloaded from Flaticon (www.flaticon.com)
# License: CC 3.0 BY
#
# <New>
# https://www.flaticon.com/packs/document-3
# ####################################################################

set bddscout_icon_New {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAADh0lEQVRYw+2XXUhbZxjHf+fknETT
NiMQlkTNpR/gheDV2Ac03caY+4DZrZkBizIyE1eJtnNI0d14s6sEJm1sAwqVrV1hGwym2ehS1u5i
ozBDFEEvVhVlcbodWsXqOSc5u3ArVtEZm+yqz937Af/f87z/9znnFdgRtbW1ZkVRzptMpqCu604O
GRaLJTE7O/sGoO+3T9g59ng8Y7Isv9ja2ipVVVUhimLe4sPDw4yPjyPL8pX5+fkWwNhrr7R94HK5
TuZyuZdHRkbEurq6wyZPIpEgk8mwvLzcXF5ePre4uPjxXnsfSU8UxRPV1dXa44j/G5WVlQSDQcEw
jD632x04EIBhGEecTqdEgcLr9eL3+xFF8ZLb7X7rPwGKEY2NjTQ0NAgmk+l6WVnZs/8bgKIopNNp
0uk09fX1OJ1OyWw2JyoqKir3NGGhwuFwMDo6ysTExM6lY4Zh9ADvFRWgt7eXcDi8a767uzubTCal
oldAEARsNtuueVmWH8+EiqLgP+VjYGBg11o8HudU40lWVlbygpXyET/d5Oev3+b4NZUiq+t0dnUB
EIvFiEaiPC1INL/bxMi1qzgcjsIChD84w5935/hy1c4vJpWe2OBWzy8pIRqJ0r9hw6tZeHt+kfbA
+1z/+qvCArzgPc7Pd+5wS9rEp1phHXpig+QMg/4NGz7VyjfyA/4gyzsnvIU/gkAggK7r9EWiAPhU
K8fWRXTD4BW9hG/lDT6y3iPU3k5HR0fhAQBCoRAAfZEotpzAq1opAD9Jm3x45B6hUDvhzs68THjo
TqgJe3/Ti3ILtru9b8PGm1opP0qbrAoGr2slRNZsnI3FtgybRxUODBCPxx+KN6tWRqUHnDt6n2zO
QBG25ow1OBeLIZpMB/bBgY/gVvImTwkmjusWxv4RbwsG6TrbRX/JfUbM6zyTtVCGzO3kzcJX4NOL
Fzjd5Md3dw6FLG3B4MNGBNAfiXK5dJ2jnnIuxi8XHsBut3Pl6uecCYZoev65R0ocCoWQJIkbie+4
cGnwwF0wbxPa7XY+++Lann0iEAjkfQuK/kf0BOAJQF4AgiCsLS0t6cUSy2QymiAIq/s9TH6Ynp6W
U6lUwcWnpqaYnJw0Azf2e5zi8XjGJEl6qaWlRaqpqTnU43RHUszMzDA0NKSrqvr9wsLCa/sCALLL
5TpvNpvbNE1zFyJ7WZZ/V1U1lslkPgG07Wt/Awd1QlpuRfNDAAAAAElFTkSuQmCC
}

set bddscout_icon_Bitmap {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAE6klEQVRYw92XXWxUZRCGnzlnKYW2
YJECImIsTYyKJE1BEdhTuAC6pbvbaGy8MCaoSKCJCTeoIV5pBKIXGkhEuMDEBExqlN3CbktIKqeV
RAU04QZtNILSShEQSv/YPd94sWdpXeSnWwiJc7fvmZ2Z877zzZlPyLHaeHudonuBYvK395IRZ9Pt
OEouEIq5OxHmGCOb88lsWfoJUC7CukTY2XEr/0AuoIII8mdrffBQPgXUxt1eBVTZXht3uxMRJ3bT
grl7ZivsCcU7Ft6rAgAmgomvjLVV3LYEuVYTc+cJlHiWd/5geNnJTJ8crgSZaFvasz9c3Zn1s20Z
px4XEB0ZoswSO76y5ciS1ppFF0bNgCXsQuiwsbcOt67sRehIq7w9DJE0Ro96wnf/EeYxe8jbt7St
rfCuS2CL/mdMFQ1O6LXfGrUE2PZaSacnqcq82li7C6AENohe7QMWZDHPTr9CKpAWWysFzfipfAzi
ZRjS1xVmjpqBxKrFPyai1a6I9qhoUEWDRgc7E9FqF+RyFrPEPtZaHzwkRg2IA+KUDN3/ZUs02NQS
DTaBnM6rCUPxjoWieh9qHgBaM4IXPFEba69QtBTNYEa9qpX72tMIhSJ6COBcWY8Z9SC6flSaj1R4
CpF4MuzUZIpyT6rwqMJnLZFr2BksnQlsS4Sd5bc9ObnHdksGjFovqWWKRHVBKO4ez8x7a11azSVL
cbIYSoNly6B6uiIUd48C9HRPfObY2vmpMTHQEl3yU2vYOW7BRaASqEx7qd9bw85xlL4sxjh+PlAX
PKboVaAKqCovnSBjZmBV7OtlHvZUI/qwGL7wGaiqibVXIlqGZjBNydKaWDtqUWyhTXesCY1Y7wi6
GCWRjDqrMg3XfkJE5yJ8now4z/tNeAqYjWFHIuo0/H+akID3gu1ZE9KGRaG42wkQEFmtmLNptZZn
MTRQb9neJWPMs6G4+wtA8WBgblPDooExMZCsXfbH/nB1J5Z1GagAKoZI/ZX5Cmp/Fktjzh6oC/4q
SgooB8ovTe61xs5AdiAZ8xsiuwECFn8DiNKJsBugyKT6/Y3qBOhOANvz0qPeCWvi7i6BFSJ8eye1
VuVphYMtEWfN7TAwW5WZwEWEy6gWg0z3nw0A3aBFw5ieycS65gPgAacEROGhG+W6mUYfJiPOtGTY
qUhGqmdYlswHuoAjyYgzJyBmLmSkQAmrWOv8//WjrC8eDJQkI86cRMQpV4u6/FeyZvcbUa72FaWX
T+i1twBRgK6ukkvTZvS9i8gH/9JUdE0iUr0nFG9/IxR3nwP1VDF5F2CpTFF0BhBQ6M02zfSZAy8W
DUzd1ld4vhG1CjJboJ5N1Dl7Q81uPapbgH6QIX9FNHkdQyPeyybF42XnxovAq8MPWD00vrvUoFuN
rf4tSk4joijzfDY2JiPOlGTEmQJ6Lq8CRO0nJSCbrhSmTwKLR+x4gZQUvCmBvk9Rc9mHH/F1+MFf
yTbWNh9uHNMgEtggoo3A7FxygPUMTZ7VEln6vY9NDTUfXpEMB5tV2AicUbVSd2QQ3cAKJOBtBhpG
rL97a2JuY8lAYHtTw6L3AUKJtlmkpWA0l9M1wKB/5icB9giXNNALlPgvoP5xHJdzo1bQLpASPwYo
u5JR57WbSmAgBlwBCoHSnORZ1kpHsCf+7+LrX04evJYcriDWV7n5/gEvsA4Y0gbt4gAAAABJRU5E
rkJggg==
}

set bddscout_icon_Pdf {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAEHElEQVRYw7WXbWhVdRzHP79ztrlr
SqGZppL5BIklFGri3N2kwXamu9uMlgbVi0iIgl4ISRSIEZpBGBWRpZCRlTNo92zsbibMe4/Y06uU
wh6kRzSNUnP5sO2cby+2qV23ud3r/nBf3P/ve87/y+/h+/8eI2tV+8FKoQ+BceS+NqUS8eeGA7Ts
DS+ZeRtjdhTZ5lxOdhxtA2aZ8URrTfyta+ELsjdkmGF/tNeV7suFQLWfOStA4o1qP3O8NRFPDkmY
0Vuu4APPP7DkuhDwmoMazw92VSUzC0ZAYixEfmWyY05eBB5obHSR3gMO4dirI8zEJMdcv7Lt4ISc
CexpaAgNfpHxvUkzcijHPPdi2FTe0VGccwkiosct0ms2wOT8r4lR94D7ptLYWffZnAm0Jcq/Atsg
mDhUH4QWPSxZQ/YPOCCYes0xHFI0HBVJHDZTa2XL/jXtK8uDbMzemuVHgCNXNbEf1IHyG0NBmcHW
yGyVEzm7qv3M/fnO6sh0QCrt6XGD9pr4l67pPrAXPT+zEclGncDK5vRcsDN7V5WcBGipKfuhQLYY
dLfXHLRUNH56Yy4Eht0DPSJuItP/v7Lt4ISeC+FcHOdjpC1FxcWfe366P34K7ByQTiXi6euSARNx
HP7x/MxWz89863R1H5ITbQDuQWyVou1g9SZnnuAnpH+BN6v89GN5Z6A6eWChLPKQnTBF20OiRHvt
8h+zcYnkvve7rGibYQ2hEz7pqOA8sATYkTMBzw/Wi+h5sJ9TidK7hsL6tRUngLqqZPpRF7cJFFmk
B3POgOcHazHVm2gS0dfDFq3asp3AzvynQFGJxCmwhWPCaPto3NnO0MLjbjJsf4EuljfVLz89GgQK
hk7lsu+ALaNoWgYjoGXVzZnG63mQpHuBvcPNwG0SE4HjwBnDbhCacwX+GNjfoC7QZLApgNsX6wIO
Zx0/BWzayEogXkjVxl++NBGtHdMVurtNLB13YeLMPQ3zu/pjK1qCWWHEZkMNwIlUIr4wy2k/gw1c
ymHoQKYDI9bdebGysLhwHUSfAVQ2ZxY7YrcZ30TdztNtdUtXe83BLcDsXvEKqnC0rjf9zMhHiich
FhUWjr2Z0P689KCIAbdLrMDteR0zSXr3koOyaJpEhUQFMDdnApJtlDllqfqSo7jR6kFuijt7X6Zj
lx1U2Y5UIm6pRNwwPsr9LnDC3x1spucHu0BrBkEd7cW605H6616C6ZG++KKcCZicdwTzB7JTfR93
x5HTZzb10BUm9A7D1vZbqbz9QPY6Pz78Ina6aM7Jk2N+7fuo3SypAvgtLyEyIQwwPM9PTwAmZ2M6
Y3+9Eut0T+GGsclTz80QLJO4tS98k+enXwIWDPjuaxGIIOnAGqAcrHwQjXjqcpNeFR0Ptn6Apzox
55Pszf8Ar4urohcepZMAAAAASUVORK5CYII=
}

set bddscout_icon_Formula {
}

# used for variable ordering
set bddscout_icon_Up {
R0lGODlhEAAQAIABAAAAAP///yH5BAEKAAEALAAAAAAQABAAAAIajI+py+0P
GwCRBisRVtt0z3SfdoyJGaWqWgAAOw==
}

# used for variable ordering
set bddscout_icon_Down {
R0lGODlhEAAQAIABAAAAAP///yH5BAEKAAEALAAAAAAQABAAAAIZjI+py+0O
XoiGMlvvwlkfrlDgNkrmiaZOAQA7
}

image create photo icon.new.small
icon.new.small put $bddscout_icon_New

image create photo icon.bitmap.small
icon.bitmap.small put $bddscout_icon_Bitmap

image create photo icon.pdf.small
icon.pdf.small put $bddscout_icon_Pdf

image create photo icon.up
icon.up put $bddscout_icon_Up

image create photo icon.down
icon.down put $bddscout_icon_Down

$bb0 insert 0 -image icon.new.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Clear all" -command {bddscout_clear}

$bb4 add -image icon.bitmap.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Export to PNG" -command {export_by_gs $mainwin png png16m}

$bb4 add -image icon.pdf.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Export to PDF" -command {export_by_gs $mainwin pdf pdfwrite}

# ####################################################################
# Change and convert BDD type
# ####################################################################

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
set bddtype [tk_optionMenu $toolbar.type.menu bddscout__selectedBddType \
    "ROBDD" \
    "ROBDD with CE" \
    "ZBDD" \
    "ZBDD with CE" \
    "TZBDD" \
]
$bddtype entryconfigure 0 -command {changetype "BIDDYTYPEOBDD"}
$bddtype entryconfigure 1 -command {changetype "BIDDYTYPEOBDDC"}
$bddtype entryconfigure 2 -command {changetype "BIDDYTYPEZBDD"}
$bddtype entryconfigure 3 -command {changetype "BIDDYTYPEZBDDC"}
$bddtype entryconfigure 4 -command {changetype "BIDDYTYPETZBDD"}

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
  global bddscout__selectedBddType
  global BDDNAME
  global bddtype

  set type -1
  if {$bddscout__selectedBddType == "ROBDD"} {
    set type 0
  } elseif {$bddscout__selectedBddType == "ROBDD with CE"} {
    set type 1
  } elseif {$bddscout__selectedBddType == "ZBDD"} {
    set type 2
  } elseif {$bddscout__selectedBddType == "ZBDD with CE"} {
    set type 3
  } elseif {$bddscout__selectedBddType == "TZBDD"} {
    set type 4
  }
  while { $type != -1 } {
    set type [expr $type - 1]
    if { $type == 0 } {
      if { [bddscoutCheckFormula "BIDDYTYPEOBDD" $BDDNAME] == 1 } {
        $bddtype invoke 0
        set type -1
      }
    } elseif { $type == 1 } {
      if { [bddscoutCheckFormula "BIDDYTYPEOBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 1
        set type -1
      }
    } elseif { $type == 2 } {
      if { [bddscoutCheckFormula "BIDDYTYPEZBDD" $BDDNAME] == 1 } {
        $bddtype invoke 2
        set type -1
      }
    } elseif { $type == 3 } {
      if { [bddscoutCheckFormula "BIDDYTYPEZBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 3
        set type -1
      }
    } elseif { $type == 4 } {
      # this is not possible
    }
  }
}

proc bddtypeDown {} {
  global bddscout__selectedBddType
  global BDDNAME
  global bddtype

  set type 4
  if {$bddscout__selectedBddType == "ROBDD"} {
    set type 0
  } elseif {$bddscout__selectedBddType == "ROBDD with CE"} {
    set type 1
  } elseif {$bddscout__selectedBddType == "ZBDD"} {
    set type 2
  } elseif {$bddscout__selectedBddType == "ZBDD with CE"} {
    set type 3
  } elseif {$bddscout__selectedBddType == "TZBDD"} {
    set type 4
  }
  while { $type != 4 } {
    set type [expr $type + 1]
    if { $type == 0 } {
      # this is not possible
    } elseif { $type == 1 } {
      if { [bddscoutCheckFormula "BIDDYTYPEOBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 1
        set type 4
      }
    } elseif { $type == 2 } {
      if { [bddscoutCheckFormula "BIDDYTYPEZBDD" $BDDNAME] == 1 } {
        $bddtype invoke 2
        set type 4
      }
    } elseif { $type == 3 } {
      if { [bddscoutCheckFormula "BIDDYTYPEZBDDC" $BDDNAME] == 1 } {
        $bddtype invoke 3
        set type 4
      }
    } elseif { $type == 4 } {
      if { [bddscoutCheckFormula "BIDDYTYPETZBDD" $BDDNAME] == 1 } {
        $bddtype invoke 4
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
  global bddscout__selectedBddType
  global bddtype

  if {$bddscout__selectedBddType == "ROBDD"} {
  } elseif {$bddscout__selectedBddType == "ROBDD with CE"} {
    converttype "BIDDYTYPEOBDD" false
    $bddtype invoke 0
  } elseif {$bddscout__selectedBddType == "ZBDD"} {
    converttype "BIDDYTYPEOBDDC" false
    $bddtype invoke 1
  } elseif {$bddscout__selectedBddType == "ZBDD with CE"} {
    converttype "BIDDYTYPEZBDD" false
    $bddtype invoke 2
  } elseif {$bddscout__selectedBddType == "TZBDD"} {
    converttype "BIDDYTYPEZBDDC" false
    $bddtype invoke 3
  }
}

proc bddconvertDown {} {
  global bddscout__selectedBddType
  global bddtype

  if {$bddscout__selectedBddType == "ROBDD"} {
    converttype "BIDDYTYPEOBDDC" false
    $bddtype invoke 1
  } elseif {$bddscout__selectedBddType == "ROBDD with CE"} {
    converttype "BIDDYTYPEZBDD" false
    $bddtype invoke 2
  } elseif {$bddscout__selectedBddType == "ZBDD"} {
    converttype "BIDDYTYPEZBDDC" false
    $bddtype invoke 3
  } elseif {$bddscout__selectedBddType == "ZBDD with CE"} {
    converttype "BIDDYTYPETZBDD" false
    $bddtype invoke 4
  } elseif {$bddscout__selectedBddType == "TZBDD"} {
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

# you should call "add command" as many times as you have entryconfigures
set bddscout__formulamenu [menu .formulamenu -tearoff false -relief solid -bd 1 -bg $COLORBG]
$bddscout__formulamenu add command
$bddscout__formulamenu add command
$bddscout__formulamenu add command
$bddscout__formulamenu add command
$bddscout__formulamenu add command

$selectwin.browser bindText <ButtonPress-3> {showformulamenu %X %Y}

proc showformulamenu { x y fname } {
  global bddscout__formulamenu
  global selectwin
  global BDDNAME
  if {$fname == $BDDNAME} {
    $bddscout__formulamenu entryconfigure 0 -command {converttype "BIDDYTYPEOBDD" false}
    $bddscout__formulamenu entryconfigure 1 -command {converttype "BIDDYTYPEOBDDC" false}
    $bddscout__formulamenu entryconfigure 2 -command {converttype "BIDDYTYPEZBDD" false}
    $bddscout__formulamenu entryconfigure 3 -command {converttype "BIDDYTYPEZBDDC" false}
    $bddscout__formulamenu entryconfigure 4 -command {converttype "BIDDYTYPETZBDD" false}
  } else {
    $selectwin.browser selection set $fname
    $bddscout__formulamenu entryconfigure 0 -command {converttype "BIDDYTYPEOBDD" true}
    $bddscout__formulamenu entryconfigure 1 -command {converttype "BIDDYTYPEOBDDC" true}
    $bddscout__formulamenu entryconfigure 2 -command {converttype "BIDDYTYPEZBDD" true}
    $bddscout__formulamenu entryconfigure 3 -command {converttype "BIDDYTYPEZBDDC" true}
    $bddscout__formulamenu entryconfigure 4 -command {converttype "BIDDYTYPETZBDD" true}
  }
  $bddscout__formulamenu entryconfigure 0 -label "Draw ROBDD for $fname" -font [list TkHeadingFont 10]
  $bddscout__formulamenu entryconfigure 1 -label "Draw ROBDD with CE for $fname" -font [list TkHeadingFont 10]
  $bddscout__formulamenu entryconfigure 2 -label "Draw ZBDD for $fname" -font [list TkHeadingFont 10]
  $bddscout__formulamenu entryconfigure 3 -label "Draw ZBDD with CE for $fname" -font [list TkHeadingFont 10]
  $bddscout__formulamenu entryconfigure 4 -label "Draw TZBDD for $fname" -font [list TkHeadingFont 10]
  tk_popup $bddscout__formulamenu [expr $x+16] [expr $y+8]
}

# ####################################################################
# Entry for variable's probability
# ####################################################################

$varwin.browser bindText <Double-Button-1> {showentryprobability %X %Y}

proc showentryprobability { x y varname } {
  global bddscout__entry

  #puts "showentryprobability: $x, $y, $varname"

  set bddscout__entry [bddscout_get_probability $varname]
  if {$varname == ".c."} {set varname "c"}
  set varname [string map {".AND." "&"} $varname]

  toplevel .entryProbability
  wm transient .entryProbability .
  #wm overrideredirect .entryProbability 1
  #wm attributes .entryProbability -topmost yes
  wm geometry .entryProbability "+$x+$y"
  wm title .entryProbability "$varname"
  wm iconname .entryProbability "Probability for $varname"
  pack [entry .entryProbability.e -textvariable bddscout__entry]
  bind .entryProbability <Return> {destroy .entryProbability}

  focus .entryProbability
  focus .entryProbability.e

  tkwait window .entryProbability

  bddscout_set_probabilities [list $varname $bddscout__entry]

  # is this needed ??
  update_info
  focusform
}

# ####################################################################
# Create menus
# ####################################################################

set FONTFAMILYMENU Helvetica
set FONTSIZEMENU 11
set FONTFAMILYREPORT Courier
set FONTSIZEREPORT 11

set menubar [frame .menuFrame -bg $COLORFRAME -highlightthickness 0]
pack $menubar -fill x -before $mainframe

menubutton $menubar.file -text "File" -menu $menubar.file.menu -pady 6 -bg $COLORFRAME \
                         -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.file -side left

menu $menubar.file.menu -bg $COLORFRAME -relief groove -tearoff false \
                        -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

$menubar.file.menu add command -command {menu_file_open} -label "Open"
$menubar.file.menu add command -command {menu_file_saveas} -label "Save As"
$menubar.file.menu add separator
$menubar.file.menu add command -command {menu_file_read_BF} -label "Import Boolean function"
$menubar.file.menu add separator
$menubar.file.menu add command -command {menu_file_read_BDDL} -label "Read BDDL File"
$menubar.file.menu add separator
$menubar.file.menu add command -command {export_by_gs $mainwin png png16m} -label "Export to PNG"
$menubar.file.menu add command -command {export_by_gs $mainwin pdf pdfwrite} -label "Export to PDF"
$menubar.file.menu add separator
$menubar.file.menu add command -command {bddview_print $mainwin} -label "Print to File"
$menubar.file.menu add separator
$menubar.file.menu add command -command {menu_system_info} -label "System Info"
$menubar.file.menu add command -command {menu_file_run_tclscript} -label "Run tcl script"
$menubar.file.menu add command -command {menu_options} -label "Options"
$menubar.file.menu add separator
$menubar.file.menu add command -command {exit} -label "Exit"

menubutton $menubar.view -text "View" -menu $menubar.view.menu -pady 6 -bg $COLORFRAME \
                         -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.view -side left

menu $menubar.view.menu -bg $COLORFRAME -relief groove -tearoff false \
                        -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

$menubar.view.menu add command -command menu_view_formulae_byName -label "Browse by name"
$menubar.view.menu add command -command menu_view_formulae_byNodeNumber -label "Browse by node number"
$menubar.view.menu add command -command menu_view_formulae_byNodeMaxLevel -label "Browse by graph depth"
$menubar.view.menu add command -command menu_view_formulae_byNodeAvgLevel -label "Browse by node avg depth"
$menubar.view.menu add command -command menu_view_formulae_byPathNumber -label "Browse by path number"
$menubar.view.menu add command -command menu_view_formulae_byMintermNumber -label "Browse by minterm number"

menubutton $menubar.create -text "Create" -menu $menubar.create.menu -pady 6 -bg $COLORFRAME \
                           -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.create -side left

menu $menubar.create.menu -bg $COLORFRAME -relief groove -tearoff false \
                          -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

proc createCreateMenu {} {
  global menubar
  global BDDSCOUT_PATH_CREATE
  foreach script [glob -nocomplain -directory "$BDDSCOUT_PATH_CREATE" *.tcl] {
    set sf [open "$script"]
    if {[gets $sf line] >= 0} {
      if {$line == "# BDD SCOUT EXTENSION"} {
        if {[gets $sf line] >= 0} {
          if {[string range $line 0 6] == "# NAME:"} {
            set line [string range $line 8 end]
            $menubar.create.menu add command -command [list source "$script"] -label $line
          }
        }
      }
    }
    close $sf
  }
}

createCreateMenu

# ####################################################################
# Implement commands assigned to menu buttons
# ####################################################################

proc menu_file_open {  } {
  global mainwin
  global selectwin
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getOpenFile -title "Select bddview file" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -parent $mainwin]
  if {[string length $filename] != 0} {
    bddscout_open $filename

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }
}

proc menu_file_saveas {  } {
  global mainwin
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getSaveFile -title "Save As" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -initialfile "$BDDNAME.bddview" -parent $mainwin]
  if {[string length $filename] != 0} {
    bddview_save $filename

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }
}

proc menu_file_run_tclscript {  } {
  global mainwin
  global BDDSCOUT_PATH_SCRIPTS

  set filename [tk_getOpenFile -title "Select tcl script" -initialdir "$BDDSCOUT_PATH_SCRIPTS" -parent $mainwin]
  if {[string length $filename] != 0} {
    puts "Script $filename started."
    source $filename
    puts "Script $filename finished."

    #remember last path
    set BDDSCOUT_PATH_SCRIPTS [file dirname $filename]
  }
}

proc menu_file_read_BDD {  } {
  global mainwin
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getOpenFile -title "Import BDD" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -parent $mainwin]
  if {[string length $filename] != 0} {

    # for OBDDs, create and store tmp file for current formula
    # for ZBDDs and TZBDDs, importing new variables may change all existing formula
    if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDD") || ($ACTIVEBDDTYPE == "BIDDYTYPEOBDDC")} {
      createTreeItemData $selectwin.browser $BDDNAME
    } else {
      clearTreeItemData $selectwin.browser
    }

    set BDDNAME [bddscout_read_bdd $filename]
    if {[string length $BDDNAME] != 0} {
      update_info
      focusform
    }

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }

}

proc menu_file_read_BF {  } {
  global mainwin
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getOpenFile -title "Import Boolean functions" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -parent $mainwin]
  if {[string length $filename] != 0} {

    # for OBDDs, create and store tmp file for current formula
    # for ZBDDs and TZBDDs, importing new variables may change all existing formula
    if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDD") || ($ACTIVEBDDTYPE == "BIDDYTYPEOBDDC")} {
      createTreeItemData $selectwin.browser $BDDNAME
    } else {
      clearTreeItemData $selectwin.browser
    }

    set BDDNAME [bddscout_read_bf $filename]
    if {[string length $BDDNAME] != 0} {
      update_info
      focusform
    }

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }

}

proc menu_file_read_BDDL {  } {
  global mainwin
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getOpenFile -title "Read BDDL file" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -parent $mainwin]
  if {[string length $filename] != 0} {

    # for OBDDs, create and store tmp file for current formula
    # for ZBDDs and TZBDDs, importing new variables may change all existing formula
    if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDD") || ($ACTIVEBDDTYPE == "BIDDYTYPEOBDDC")} {
      createTreeItemData $selectwin.browser $BDDNAME
    } else {
      clearTreeItemData $selectwin.browser
    }

    set BDDNAMELIST [biddy_read_bddl_file $filename]
    set BDDNAME [lindex [split $BDDNAMELIST] end]
    if {[string length $BDDNAME] != 0} {
      update_info
      focusform
    }

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }

}

proc menu_file_write_BDD {  } {
  global mainwin
  global BDDNAME
  global BDDSCOUT_PATH_EXAMPLES

  set filename [tk_getSaveFile -title "Export BDD" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -parent $mainwin]
  if {[string length $filename] != 0} {

    biddy_write_bdd $filename $BDDNAME

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]
  }
}

proc menu_system_info {  } {
  global bddscout__info
  global bddscout__biddyInfo
  global FONTFAMILYMENU
  global FONTSIZEMENU
  global FONTFAMILYREPORT
  global FONTSIZEREPORT

  toplevel .dialogInfo
  wm title .dialogInfo "Info"
  wm iconname .dialogInfo "Info"

  frame .dialogInfo.f -relief raised

  label .dialogInfo.f.text1 -relief sunken -anchor w -justify left -text "BDD Scout Info" -bg azure3 \
                            -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight bold -slant roman]
  label .dialogInfo.f.text2 -relief flat -anchor w -justify left -textvariable bddscout__info -bg azure1 \
                            -font [list -family $FONTFAMILYREPORT -size $FONTSIZEREPORT -weight normal -slant roman]
  label .dialogInfo.f.text3 -relief flat -text "" -bg azure1
  label .dialogInfo.f.text4 -relief sunken -anchor w -justify left -text "Biddy Info" -bg azure3 \
                            -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight bold -slant roman]
  label .dialogInfo.f.text5 -relief flat -anchor w -justify left -textvariable bddscout__biddyInfo -bg azure1 \
                            -font [list -family $FONTFAMILYREPORT -size $FONTSIZEREPORT -weight normal -slant roman]
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
    bddview_message "NOTE" "Printing is not implemented, yet"
    focus .dialogInfo
  } -relief raised -text "Print" -width 6
  pack .dialogInfo.f.buttons.print -padx 10 -side left

  button .dialogInfo.f.buttons.update -borderwidth 2 -command {
    update_info
    focusform
  } -relief raised -text "Update" -width 6
  pack .dialogInfo.f.buttons.update -padx 10 -side left

  button .dialogInfo.f.buttons.ok -borderwidth 2 -command {
    destroy .dialogInfo
  } -relief raised -text "OK" -width 6
  pack .dialogInfo.f.buttons.ok -padx 10 -side left

  pack .dialogInfo.f.buttons

}

proc menu_view_formulae_byName {  } {
  global selectwin

  set fname [browse_formulae_byName]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

proc menu_view_formulae_byNodeNumber {  } {
  global selectwin

  set fname [browse_formulae_byNodeNumber]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

proc menu_view_formulae_byNodeMaxLevel {  } {
  global selectwin

  set fname [browse_formulae_byNodeMaxLevel]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

proc menu_view_formulae_byNodeAvgLevel {  } {
  global selectwin

  set fname [browse_formulae_byNodeAvgLevel]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

proc menu_view_formulae_byPathNumber {  } {
  global selectwin

  set fname [browse_formulae_byPathNumber]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

proc menu_view_formulae_byMintermNumber {  } {
  global selectwin

  set fname [browse_formulae_byMintermNumber]
  if {$fname != ""} {
    $selectwin.browser selection set $fname
  }
}

# ####################################################################
# Options
# ####################################################################

set bddscout__options 0

proc CHANGE { name item } {
  global $name
  set value [$item get]
  eval [list set $name $value]
  update_gui
}

proc menu_options {  } {
  global COLORFRAME
  global bddscout__options

  if {$bddscout__options == 1} {
    return
  }
  set bddscout__options 1

  set OPTIONS ""
  set TCLTK_INTERNAL [list \
    "argc" \
    "argv" \
    "argv0" \
    "auto_path" \
    "errid" \
    "errorInfo" \
    "errorCode" \
  ]
  set BDDVIEW_INTERNAL [list \
    "filename" \
    "mainwin" \
    "mainframe" \
    "bb0" \
    "bb1" \
    "bb2" \
    "bb3" \
    "bb4" \
    "bb5" \
    "sep1" \
    "sep2" \
    "sep3" \
    "sep4" \
    "verticalwindow" \
    "horizontalwindow" \
    "toolbar" \
    "toolbarsize" \
    "BDDVIEWVERSION" \
    "ACTIVEBDDTYPE" \
    "USEBDDTYPE" \
    "TITLE" \
    "OS" \
    "OS1" \
    "STATE" \
    "STATUSBAR" \
    "XWIN" \
    "YWIN" \
    "ZOOM" \
    "DOUBLELINE" \
    "ARROWSIZE" \
    "INVSIZE" \
    "LABELOFFSETX" \
    "LABELOFFSETY" \
    "GRID" \
    "GRIDRESOLUTION" \
    "GRIDOFFSETX" \
    "GRIDOFFSETY" \
    "GRIDON" \
    "XMAX" \
    "YMAX" \
    "BDDNAME" \
    "BDDNAMELIST" \
    "BDDVARIABLES" \
    "BDDNODES" \
    "TERMINALS" \
    "SELECTED" \
    "TERMSELECTED" \
    "LISTF" \
    "LISTT" \
    "DEPTH" \
    "TERMDEPTH" \
    "XARROW" \
    "YARROW" \
    "ARROWSON" \
    "BENDSON" \
  ]
  set BDDSCOUT_INTERNAL [list \
    "bddtype" \
    "inputwin" \
    "inputtype" \
    "selectwin" \
    "varwin" \
    "menubar" \
    "SPLASHTEXT" \
    "BIDDYVERSION" \
    "INPUT" \
    "INPUTTYPE" \
  ]

  foreach OPT [lsort -ascii [info globals]] {
    # variable i is used for loops
    if {$OPT == "i"} {
    # tcl/tk internal variables
    } elseif {[string range $OPT 0 3] == "tcl_"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[string range $OPT 0 2] == "tk_"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[lsearch $TCLTK_INTERNAL $OPT] != -1} {
      #puts "Variable $OPT is not user modifiable."
    # bddview internal variables
    } elseif {[string range $OPT 0 12] == "bddview_icon_"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[string range $OPT 0 8] == "bddview__"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[lsearch $BDDVIEW_INTERNAL $OPT] != -1} {
      #puts "Variable $OPT is not user modifiable."
    # bddscout internal variables
    } elseif {[string range $OPT 0 13] == "bddscout_icon_"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[string range $OPT 0 9] == "bddscout__"} {
      #puts "Variable $OPT is not user modifiable."
    } elseif {[lsearch $BDDSCOUT_INTERNAL $OPT] != -1} {
      #puts "Variable $OPT is not user modifiable."
    } else {
      global $OPT
      if {[array exists $OPT]} {
        #puts "Variable $OPT is not user modifiable."
      } else {
        lappend OPTIONS [list $OPT [subst $$OPT]]
      }
    }
  }

  toplevel .options -bg $COLORFRAME
  wm title .options "Options"
  wm iconname .options "Options"

  ScrolledWindow .options.w
  ScrollableFrame .options.w.f -width 960 -height 640
  .options.w setwidget .options.w.f
  set sf [.options.w.f getframe]

  foreach OPT $OPTIONS {
    set name [lindex $OPT 0]
    set value [lindex $OPT 1]
    label $sf.name_$name -text $name -width 32
    entry $sf.value_$name -width 60
    $sf.value_$name insert 0 $value
    button $sf.set_$name -relief raised -text "SET" -width 6 -command [list CHANGE $name $sf.value_$name]
    grid $sf.name_$name $sf.value_$name $sf.set_$name -sticky nsew
  }

  button .options.close -borderwidth 2 -command {destroy .options} -relief raised -text "CLOSE" -width 6

  grid .options.w
  grid .options.close

  tkwait window .options
  set bddscout__options 0
}

# ####################################################################
# Help
# ####################################################################

set bddscout__helpTutorial 0
set bddscout__helpBF 0
set bddscout__helpCA 0
set bddscout__helpBDDL 0
set bddscout__helpAbout 0

proc menu_help_tutorial {} {
  global COLORFRAME
  global COLORMENU
  global bddscout__helpTutorial

  if {$bddscout__helpTutorial == 1} {
    return
  }
  set bddscout__helpTutorial 1

  toplevel .helpTutorial -bg $COLORFRAME
  wm title .helpTutorial "Tutorial"
  wm iconname .helpTutorial "Tutorial"

  text .helpTutorial.txt -bg $COLORMENU -height 20 -width 100 -xscroll {.helpTutorial.h set} -yscroll {.helpTutorial.v set}
  scrollbar .helpTutorial.h -orient horizontal -command {.helpTutorial.txt xview}
  scrollbar .helpTutorial.v -orient vertical -command {.helpTutorial.txt yview}

  .helpTutorial.txt insert 1.0 "\
  Tutorial\n\n\
  Biddy is an academic BDD package.\n\
  Biddy supports ROBDDs without and with complemented edges.\n\
  Biddy supports 0-sup-BDDs without and with complemented edges.\n\
  Biddy supports TZBDDs (tagged form of 0-sup-BDDs) without complemented edges.\n\
  Biddy supports automatic garbage collection and sifting algorithm.\n\n\
  bddview is a pure Tcl/Tk script for visualization of BDDs.\n\
  bddview is a graph viewer only and does not use a BDD package.\n\n\
  BDD Scout is a demo application demonstrating the capability of Biddy and bddview.\n\
  "
  .helpTutorial.txt configure -state disabled

  button .helpTutorial.close -borderwidth 2 -command {destroy .helpTutorial} -relief raised -text "CLOSE" -width 6

  grid .helpTutorial.txt .helpTutorial.v -sticky nsew
  grid .helpTutorial.h -sticky nsew
  grid .helpTutorial.close
  grid rowconfigure .helpTutorial .helpTutorial.txt -weight 1
  grid columnconfigure .helpTutorial .helpTutorial.txt -weight 1
  tkwait window .helpTutorial
  set bddscout__helpTutorial 0
}

proc menu_help_bf {} {
  global COLORFRAME
  global COLORMENU
  global bddscout__helpBF

  if {$bddscout__helpBF == 1} {
    return
  }
  set bddscout__helpBF 1

  toplevel .helpBF -bg $COLORFRAME
  wm title .helpBF "Help on Boolean functions"
  wm iconname .helpBF "Help on Boolean functions"

  text .helpBF.txt -bg $COLORMENU -height 20 -width 100 -xscroll {.helpBF.h set} -yscroll {.helpBF.v set}
  scrollbar .helpBF.h -orient horizontal -command {.helpBF.txt xview}
  scrollbar .helpBF.v -orient vertical -command {.helpBF.txt yview}

  .helpBF.txt insert 1.0 "\
  Help on specification of Boolean functions\n\n\
  Parenthesis are implemented.\n\
  Operators' priority is implemented.\n\
  Boolean constants are 0 and 1.\n\
  Boolean operators are:\n
    NOT (~! before variable)
    AND (&*)
    OR (|+)
    XOR (^%)
    XNOR (-)
    IMPLIES (><)
    NAND (@)
    NOR (#)
    BUTNOT (\)
    NOTBUT (/)\n\
  "
  .helpBF.txt configure -state disabled

  button .helpBF.close -borderwidth 2 -command {destroy .helpBF} -relief raised -text "CLOSE" -width 6

  grid .helpBF.txt .helpBF.v -sticky nsew
  grid .helpBF.h -sticky nsew
  grid .helpBF.close
  grid rowconfigure .helpBF .helpBF.txt -weight 1
  grid columnconfigure .helpBF .helpBF.txt -weight 1
  tkwait window .helpBF
  set bddscout__helpBF 0
}

proc menu_help_ca {} {
  global COLORFRAME
  global COLORMENU
  global bddscout__helpCA

  if {$bddscout__helpCA == 1} {
    return
  }
  set bddscout__helpCA 1

  toplevel .helpCA -bg $COLORFRAME
  wm title .helpCA "Help on Unate Cube Set Algebra"
  wm iconname .helpCA "Help on Unate Cube Set Algebra"

  text .helpCA.txt -bg $COLORMENU -height 20 -width 100 -xscroll {.helpCA.h set} -yscroll {.helpCA.v set}
  scrollbar .helpCA.h -orient horizontal -command {.helpCA.txt xview}
  scrollbar .helpCA.v -orient vertical -command {.helpCA.txt yview}

  .helpCA.txt insert 1.0 "\
  Help on operations in Unate Cube Set Algebra\n\n\
  1. Cube is one word including only symbols 0 and 1\n\
  2. Word \"0\" is used for empty set if not preceeded by + or -\n\
  3. Word \"0\" is a cube without elements (an empty cube) if preceeded by + or -\n\
  4. Word \"1\" is used for universal set over existing variables if not preceeded by + or -\n\
  5. Word \"1\" is a cube with one element if preceeded by + or -\n\
  6. Create new set:\n\
       0 (create new empty set, NEWSET = {})\n\
       0 + 0 (create new set with one element, an empty cube, NEWSET = {{}})\n\
       1 (create new universal set, NEWSET = {...all elements...})\n\
       0 + 1 (create new set with one element that is a cube with one element, NEWSET = {{x1}}\n\
       cube (create new set, NEWSET = {cube})\n\
       SET1 + cube (create new set, add cube, NEWSET = SET1 + {cube})\n\
       SET1 - cube (create new set, remove cube, NEWSET = SET1 - {cube})\n\
  7. Create or change the set:\n\
       SET1 = 0 (create or change the set, SET1 = {})\n\
       SET1 = 0 + 0 (create or change the set, SET1 = {{}})\n\
       SET1 = 1 (create or change the set, SET1 = {...all elements...})\n\
       SET1 = 0 + 1 (create or change the set, SET1 = {{x1}})\n\
       SET1 = cube (create or change the set, SET1 = {cube})\n\
       SET2 = SET1 + cube (create or change the set, add cube, SET2 = SET1 + {cube})\n\
       SET2 = SET1 - cube (create or change the set, remove cube, SET2 = SET1 - {cube})\n\
  8. Add/Remove a cube to/from the current set:\n\
       +cube\n\
       -cube\n\
  9. Add/Remove a cube to/from the existing set:\n\
       SET2 += cube (add cube, change the existing set)\n\
       SET2 -= cube (remove cube, change the existing set)\n\
  "
  .helpCA.txt configure -state disabled

  button .helpCA.close -borderwidth 2 -command {destroy .helpCA} -relief raised -text "CLOSE" -width 6

  grid .helpCA.txt .helpCA.v -sticky nsew
  grid .helpCA.h -sticky nsew
  grid .helpCA.close
  grid rowconfigure .helpCA .helpCA.txt -weight 1
  grid columnconfigure .helpCA .helpCA.txt -weight 1
  tkwait window .helpCA
  set bddscout__helpCA 0
}

proc menu_help_bddl {} {
  global COLORFRAME
  global COLORMENU
  global bddscout__helpBDDL

  if {$bddscout__helpBDDL == 1} {
    return
  }
  set bddscout__helpBDDL 1

  toplevel .helpBDDL -bg $COLORFRAME
  wm title .helpBDDL "Help on Knuth's BDDL"
  wm iconname .helpBDDL "Help on Knuth's BDDL"

  text .helpBDDL.txt -bg $COLORMENU -height 20 -width 100 -xscroll {.helpBDDL.h set} -yscroll {.helpBDDL.v set}
  scrollbar .helpBDDL.h -orient horizontal -command {.helpBDDL.txt xview}
  scrollbar .helpBDDL.v -orient vertical -command {.helpBDDL.txt yview}

  .helpBDDL.txt insert 1.0 "\
  Help on Knuth's BDDL\n\n\
  Using implementation from https://www-cs-faculty.stanford.edu/~knuth/programs/bdd14.w\n\
  PDF: https://github.com/shreevatsa/knuth-literate-programs/blob/master/programs/bdd14.pdf, pp. 47 - 53 \n\n\
  "
  .helpBDDL.txt configure -state disabled

  button .helpBDDL.close -borderwidth 2 -command {destroy .helpBDDL} -relief raised -text "CLOSE" -width 6

  grid .helpBDDL.txt .helpBDDL.v -sticky nsew
  grid .helpBDDL.h -sticky nsew
  grid .helpBDDL.close
  grid rowconfigure .helpBDDL .helpBDDL.txt -weight 1
  grid columnconfigure .helpBDDL .helpBDDL.txt -weight 1
  tkwait window .helpBDDL
  set bddscout__helpBDDL 0
}

proc menu_help_about {} {
  global COLORFRAME
  global COLORMENU
  global bddscout__helpAbout
  global BIDDYVERSION

  if {$bddscout__helpAbout == 1} {
    return
  }
  set bddscout__helpAbout 1

  toplevel .helpAbout -bg $COLORFRAME
  wm title .helpAbout "About"
  wm iconname .helpAbout "About"

  text .helpAbout.w -bg $COLORMENU -height 20 -width 100
  .helpAbout.w insert 1.0 "\
  BDD Scout v$BIDDYVERSION\n\n\
  Author: Robert Meolic (robert@meolic.com)\n\n\
  Copyright (C) 2008, 2019 UM FERI, Koroska cesta 46, SI-2000 Maribor, Slovenia\n\
  Copyright (C) 2019, 2024 Robert Meolic, SI-2000 Maribor, Slovenia\n\n\
  Biddy is free software; you can redistribute it and/or modify it under the terms\n\
  of the GNU General Public License as published by the Free Software Foundation;\n\
  either version 2 of the License, or (at your option) any later version.\n\n\
  Biddy is distributed in the hope that it will be useful, but WITHOUT ANY\n\
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A\n\
  PARTICULAR PURPOSE. See the GNU General Public License for more details.\n\n\
  You should have received a copy of the GNU General Public License along with\n\
  this program; if not, write to the Free Software Foundation, Inc., 51 Franklin\n\
  Street, Fifth Floor, Boston, MA 02110-1301 USA.\
  "
  pack .helpAbout.w -fill both -expand yes

  button .helpAbout.close -borderwidth 2 -command {destroy .helpAbout} -relief raised -text "CLOSE" -width 6
  pack .helpAbout.close -padx 10 -fill y

  pack .helpAbout.close
  tkwait window .helpAbout
  set bddscout__helpAbout 0
}

menubutton $menubar.help -text "Help" -menu $menubar.help.menu -pady 6 -bg $COLORFRAME \
                         -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.help -side left

menu $menubar.help.menu -bg $COLORFRAME -relief groove -tearoff false \
                        -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

$menubar.help.menu add command -command menu_help_tutorial -label "Tutorial"
$menubar.help.menu add command -command menu_help_bf -label "Help on Boolean formulae"
$menubar.help.menu add command -command menu_help_ca -label "Help on Unate Cube Set Algebra"
$menubar.help.menu add command -command menu_help_bddl -label "Help on Knuth's BDDL"
$menubar.help.menu add command -command menu_help_about -label "About"

# ###############################################################
# Internal procedures
# ###############################################################

proc addList {mylist t root} {
  global icon.edit.small
  global FONTFAMILYLABEL

  set first [lindex $mylist 0]
  while { [llength $first] == 1 } {
    set nameX [join $first]
    set data [bddscout_get_probability $nameX]
    if {$data == ""} {
      set nameY $nameX
    } else {
      set nameY "$nameX ($data)"
    }
    if {$nameX == "c"} {set nameX ".c."}
    set nameX [string map {"&" ".AND."} $nameX]
    $t insert end $nameX -text $nameY -font [list $FONTFAMILYLABEL 10] -image icon.down
    set mylist [lrange $mylist 1 end]
    set first [lindex $mylist 0]
  }
}

proc addTree {mylist t root} {
  global FONTFAMILYLABEL

  set first [lindex $mylist 0]
  while { [llength $first] == 1 } {
    set nameX [string map {"&" ".AND."} [join $first]]
    $t insert end $root $nameX -text [join $first] -font [list $FONTFAMILYLABEL 10]

    #puts "DEBUG (addTree): $nameX"

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

    $t insert end $root $nameX -text "$firstname..." -fill darkblue -font [list $FONTFAMILYLABEL 10]
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

proc getTreeAsList {t} {
  set treelist [$t nodes root]
  #puts "DEBUG (getTree): $treelist"
  return $treelist
}

proc setTreeItemData {t n d} {
  if {[$t exists $n]} {
    $t itemconfigure $n -data $d
  } else {
    puts "WARNING (bddscout.tcl): setTreeItemData <$n> does not exist"
  }
}

proc createTreeItemData {t n} {
  if {(n != "") && [$t exists $n]} {
    file mkdir "/tmp/bddscout"
    close [file tempfile tmpname "/tmp/bddscout/${n}_[clock seconds].bddview"]
    #puts "DEBUG (createTreeItemData): store <$n>, <$tmpname>"
    bddview_save $tmpname
    $t itemconfigure $n -data $tmpname
  } else {
    puts "WARNING (bddscout.tcl): createTreeItemData <$n> does not exist"
  }
}

proc getTreeItemData {t n} {
  set d ""
  #puts "DEBUG (getTreeItemData): search for $n"
  if {[$t exists $n]} {
    set d [$t itemcget $n -data]
  } else {
    puts "WARNING (bddscout.tcl): getTreeItemData <$n> does not exist"
  }
  #puts "DEBUG (getTreeItemData): return $d"
  return $d
}

proc clearTreeItemData {t} {
  foreach n [$t nodes root] {
    #puts "DEBUG (clearTreeItemData): clear <$n>"
    $t itemconfigure $n -data ""
  }
}

# draw graph for the given Boolean formula (BDD) using the current bdd type
# it returns 0 if no new BDD has been created (fname == "")
# it returns 1 if new BDD has been created (fname != "")
proc drawbdd { fname } {
  global DOT_EXE
  global STATUSBAR

  if {[file executable $DOT_EXE] != 1} {
    set STATUSBAR "Cannot run dot from Graphviz (DOT_EXE = \"$DOT_EXE\")"
    return 0
  }

  #puts "DEBUG: drawbdd: <$fname>"

  set redraw 0
  if {$fname != ""} {
    set redraw 1
    set tmpfile "tmp.bddview"
    set tmpfile [bddscoutWriteBddview $fname $tmpfile $DOT_EXE]

    #DEBUGGING
    #set fp [open $tmpfile r]
    #set file_data [read $fp]
    #close $fp
    #set data [split $file_data "\n"]
    #foreach line $data {
    #  puts $line
    #}

    bddview_open $tmpfile
    file delete $tmpfile
  }

  return $redraw
}

# create Boolean formula (BDD) from the current graph
# only one BDD is constructed, the one pointed by label specified with variable BDDNAME
# TO DO: this will change if BiddyManagedConstructBDD returns an array of BDDs
proc constructBDD { } {
  global ACTIVEBDDTYPE
  global BDDNAME
  global BDD
  global BDDVARIABLES
  global BDDNODES

  #this must be compatible with function Biddy_Managed_ConstructBDD in biddyMain.c
  #
  #node type
  #type == t : terminal
  #type == l : label
  #type == n : node
  #
  #successors
  #terminal node 0:           type = 0, l = -1, r = -1
  #terminal node 1:           type = 1, l = -1, r = -1
  #regular label:             type = 2, r = -1
  #complemented label:        type = 3, r = -1
  #regular node:              type = 4
  #complemented successor(s): type = 5

  if {$BDDNODES == ""} return;

  if {$ACTIVEBDDTYPE == "BIDDYTYPEOBDD"} {set USETAGS false}
  if {$ACTIVEBDDTYPE == "BIDDYTYPEOBDDC"} {set USETAGS false}
  if {$ACTIVEBDDTYPE == "BIDDYTYPEZBDD"} {set USETAGS false}
  if {$ACTIVEBDDTYPE == "BIDDYTYPEZBDDC"} {set USETAGS false}
  if {$ACTIVEBDDTYPE == "BIDDYTYPETZBDD"} {set USETAGS true}

  #graph is a string representing the result graph

  #the first word in the string is the name of the BDD
  #while BiddyManagedConstructBDD returns only one BDD, this is used to choose the created formula

  set graph $BDDNAME

  #DEBUGGING
  #puts $ACTIVEBDDTYPE
  #puts [join $BDDNODES]

  foreach {num name type} [join $BDDNODES] {

    #DEBUGGING
    #puts "($num $name $type) "
    #if {$type != "t"} {puts $BDD(n$num)}

    set tag ""
    set tag1 ""
    set tag2 ""

    if {$type == "t"} {
      if {$name == "0"} {
        if {!$USETAGS} {
          #terminal node 0: type = 0, l = -1, r = -1
          lappend graph [list $num $name 0 -1 -1]
        } else {
          #terminal node 0 with tag: type = 0, l = {-1 -1}, r = {-1 -1}
          lappend graph [list $num $name 0 -1 -1 -1 -1]
        }
      } else {
        if {!$USETAGS} {
          #terminal node 1: type = 1, l = -1, r = -1
          lappend graph [list $num $name 1 -1 -1]
        } else {
          #terminal node 1 with tag: type = 1, l = {-1 -1}, r = {-1 -1}
          lappend graph [list $num $name 1 -1 -1 -1 -1]
        }
      }
    } elseif {$type == "l"} {
      # puts "constructBDD: label $num $name $type"
      if {[lindex $BDD(n$num) 0] == "i"} {
        if {!$USETAGS} {
          #complemented label: type = 3, r = -1
          set succ [string range [lindex $BDD(n$num) 1] 1 end]
          lappend graph [list $num $name 3 $succ -1]
        } else {
          #complemented label with tag: type = 3, r = {-1 -1}
          set succ [string range [lindex $BDD(n$num) 1] 1 end]
          set tag [string range [lindex $BDD(n$num) 2] 1 end]
          lappend graph [list $num $name 3 $succ $tag -1 -1]
        }
      } else {
        if {!$USETAGS} {
          #regular label: type = 2, r = -1
          set succ [string range [lindex $BDD(n$num) 0] 1 end]
          lappend graph [list $num $name 2 $succ -1]
        } else {
          #regular label with tag: type = 2, r = {-1 -1}
          set succ [string range [lindex $BDD(n$num) 0] 1 end]
          set tag [string range [lindex $BDD(n$num) 1] 1 end]
          lappend graph [list $num $name 2 $succ $tag -1 -1]
        }
      }
    } else {
      if {[lindex $BDD(n$num) 0] == "i"} {
        if {!$USETAGS} {
          #node with complemented successor: type = 5
          #for OBDDC, left successor is inverted
          #for ZBDDC, right successor is inverted
          set succ1 [string range [lindex $BDD(n$num) 1] 1 end]
          set succ2 [string range [lindex $BDD(n$num) 2] 1 end]
          if {$succ2 == ""} {
            lappend graph [list $num $name 5 $succ1 $succ1]
          } else {
            lappend graph [list $num $name 5 $succ1 $succ2]
          }
        } else {
          #node with complemented successor: type = 5
          #for TZBDDC, right successor is inverted
          set succ1 [string range [lindex $BDD(n$num) 1] 1 end]
          set tag1 [string range [lindex $BDD(n$num) 2] 1 end]
          set tag2 [string range [lindex $BDD(n$num) 4] 1 end]
          if {$tag2 == ""} {
            set succ2 $succ1
            set tag2 [string range [lindex $BDD(n$num) 3] 1 end]
          } else {
            set succ2 [string range [lindex $BDD(n$num) 3] 1 end]
          }
          lappend graph [list $num $name 5 $succ1 $tag1 $succ2 $tag2]
        }
      } else {
        if {!$USETAGS} {
          #regular node: type = 4
          set succ1 [string range [lindex $BDD(n$num) 0] 1 end]
          set succ2 [string range [lindex $BDD(n$num) 1] 1 end]
          if {$succ2 == ""} {
            lappend graph [list $num $name 4 $succ1 $succ1]
          } else {
            lappend graph [list $num $name 4 $succ1 $succ2]
          }
        } else {
          #regular node with tags: type = 4
          set succ1 [string range [lindex $BDD(n$num) 0] 1 end]
          set tag1 [string range [lindex $BDD(n$num) 1] 1 end]
          set tag2 [string range [lindex $BDD(n$num) 3] 1 end]
          if {$tag2 == ""} {
            set succ2 $succ1
            set tag2 [string range [lindex $BDD(n$num) 2] 1 end]
          } else {
            set succ2 [string range [lindex $BDD(n$num) 2] 1 end]
          }
          lappend graph [list $num $name 4 $succ1 $tag1 $succ2 $tag2]
        }
      }
      if {[lsearch -exact $BDDVARIABLES $name] == -1} {
        puts "ERROR (tcl proc constructBDD): undeclared variable: $name"
        return
      }
      if {($tag != "") && ($tag != "0") && ($tag != "1") && ([lsearch -exact $BDDVARIABLES $tag] == -1)} {
        puts "ERROR (tcl proc constructBDD): undeclared tag variable: $tag"
        return
      }
      if {($tag1 != "") && ($tag1 != "0") && ($tag1 != "1") && ([lsearch -exact $BDDVARIABLES $tag1] == -1)} {
        puts "ERROR (tcl proc constructBDD): undeclared tag variable: $tag1"
        return
      }
      if {($tag2 != "") && ($tag2 != "0") && ($tag2 != "1") && ([lsearch -exact $BDDVARIABLES $tag2] == -1)} {
        puts "ERROR (tcl proc constructBDD): undeclared tag variable: $tag2"
        return
      }
    }
  }

  #puts "constructBDD graph: $graph"

  bddscoutConstruct [llength $BDDVARIABLES] [join $BDDVARIABLES] [expr [llength $graph] -1] [join [join $graph]]
}

# formulae tree $selectwin.browser needs to be refreshed separately, use update_info if needed
proc focusform {} {
  global selectwin
  global BDDNAME

  set name $BDDNAME
  set name [string map {"&" ".AND."} $name]
  set BDDNAME ""
  
  $selectwin.browser selection set $name
  $selectwin.browser see $BDDNAME
}

# keep the existing bdd type, changes formula
# used when clicked in GUI and by "$selectwin.browser selection set $name" statement
# parameter tree is not used (it is given by "Tree -selectcommand" default behaviour)
# if special characters are used, select command will use list instead of a plain string
# changeform will redraw graph but it will use tmp file if it exist
proc changeform { tree fname } {
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME

  #puts "DEBUG (changeform): BDDNAME = <$BDDNAME> / change to / fname = <$fname>"

  if {$fname == ""} return

  set fname [string map {".AND." "&" } $fname]
  set fname [lindex $fname 0]

  if {$BDDNAME != $fname} {
    if {$BDDNAME != ""} {
      #create and store tmp file for current formula
      createTreeItemData $selectwin.browser $BDDNAME
    }
    set BDDNAME $fname
    if {[bddscoutCheckFormula $ACTIVEBDDTYPE $BDDNAME] == 1} {
      #puts "DEBUG (changeform): search  $BDDNAME, <$tmpname>"
      set tmpname [getTreeItemData $selectwin.browser $BDDNAME]
      if {($tmpname != "") && [file exist $tmpname]} {
        #reuse tmp file
        #puts "DEBUG (changeform): tmp file for <$BDDNAME> exists: $tmpname"
        bddview_open $tmpname
      } else {
        #tmp file does not exist
        #puts "DEBUG (changeform): tmp file for <$BDDNAME> does not exist"
        drawbdd $fname
      }
    }
  }

  #puts "DEBUG changeform finished BDDNAME = <$BDDNAME>"
}

# changes bdd type
# this is used by invoke command for bdd type
proc changetype { type } {
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME

  # puts "DEBUG changetype IN: type = <$type>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"

  if {$type != $ACTIVEBDDTYPE} {
    bddscoutChangeType $type
    set ACTIVEBDDTYPE $type
    clearTreeItemData $selectwin.browser
    update_info
    if {$BDDNAME != ""} {
      if {[bddscoutCheckFormula $ACTIVEBDDTYPE $BDDNAME] == 0} {
        set BDDNAME "0"
      }
      focusform
    }
  }

  # puts "DEBUG changetype OUT: type = <$type>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"
}

# converttype changes bdd type and show formula specified with BDDNAME
# if type is changed and formula does not exists in new type then the formula is constructed
# if type is not changed and (fchange == true) then the selected formula is redrawn using the last tmp file
proc converttype { type fchange} {
  global bddtype
  global ACTIVEBDDTYPE
  global BDDNAME

  # puts "DEBUG converttype IN: type = <$type>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"

  if {$type == $ACTIVEBDDTYPE} {
    if {$fchange == true} {
      if { [bddscoutCheckFormula $ACTIVEBDDTYPE $BDDNAME] == 0 } {
        puts "Formula $BDDNAME does not exist!"
      } else {
        focusform
      }
    }
  } else {
    if { [bddscoutCheckFormula $type $BDDNAME] == 0 } {
      bddscoutCopyFormula $BDDNAME $ACTIVEBDDTYPE $type
    }
    if {$type == "BIDDYTYPEOBDD"} {$bddtype invoke 0}
    if {$type == "BIDDYTYPEOBDDC"} {$bddtype invoke 1}
    if {$type == "BIDDYTYPEZBDD"} {$bddtype invoke 2}
    if {$type == "BIDDYTYPEZBDDC"} {$bddtype invoke 3}
    if {$type == "BIDDYTYPETZBDD"} {$bddtype invoke 4}
  }

  # puts "DEBUG converttype OUT: type = <$type>, ACTIVEBDDTYPE = <$ACTIVEBDDTYPE>"
}

proc parseinput_after { name } {
  global selectwin
  global ACTIVEBDDTYPE
  global BDDNAME

  # refresh formulae tree $selectwin.browser
  # for OBDD, remove the current tmp file for the resulting formula, only ...
  # ... and if not modifying the current formula, then also create and store tmp file for current formula
  # for ZBDD and TZBDD, remove tmp file for all formulae, adding variables may change them
  if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDD") || ($ACTIVEBDDTYPE == "BIDDYTYPEOBDDC")} {
    if {[$selectwin.browser exists $name]} {
      setTreeItemData $selectwin.browser $name ""
    }
    if {$BDDNAME != $name} {
      createTreeItemData $selectwin.browser $BDDNAME
    }
  } else {
    clearTreeItemData $selectwin.browser
  }
  update_info
  set BDDNAME $name
  focusform
}

proc parseinput { force } {
  global selectwin
  global ACTIVEBDDTYPE
  global INPUT
  global INPUTTYPE
  global BDDNAME
  global BFCMDLIST
  global CACMDLIST
  global BDDLCMDLIST
  global CMDLIST

  set INPUT [string trimleft $INPUT]
  set INPUT [string trimright $INPUT]
  if {$INPUT != ""} {

    set CMD $INPUT

    # INPUTTYPE 0: Boolean expression
    # INPUTTYPE 1: Unate cube set algebra
    # INPUTTYPE 2-5: not used, yet
    # INPUTTYPE 6: Knuth's BDDL command
    # INPUTTYPE 7: Tcl command

    if {$INPUTTYPE == 7} {
      set TRYCMD 2
    } else {
      set TRYCMD 0
    }

    # check if the user enter a tcl command instead of Boolean function / Unate cube set algebra / Knuth's BDDL command
    if {($INPUTTYPE == 0) || ($INPUTTYPE == 1) || ($INPUTTYPE == 6)} {
      #get first word
      set CMD [string range $INPUT 0 [expr [string wordend $INPUT 0] - 1]]
      set ARGS [string range $INPUT [string wordend $INPUT 0] end]
      if {[lsearch [concat $BFCMDLIST $CACMDLIST $BDDLCMDLIST $CMDLIST] $CMD] != -1} {
        if {$CMD == "biddy_nop"} {
          #NOTHING HERE
          set TRYCMD 1
        #
        # GENERAL COMMANDS
        # bddview_save, bddview_export_tex
        } elseif {$CMD == "bddview_save"} {
          if {$ARGS == ""} {
            set INPUT [string cat $CMD " " $BDDNAME ".bddview"]
            bddview_message "NOTE" "Saved as $BDDNAME.bddview"
          } else {
            bddview_message "NOTE" "Saved as $ARGS"
          }
          set TRYCMD 1
        } elseif {$CMD == "bddview_export_tex"} {
          if {$ARGS == ""} {
            set INPUT [string cat $CMD " " $BDDNAME ".tex"]
            bddview_message "NOTE" "Exported as $BDDNAME.tex"
          } else {
            bddview_message "NOTE" "Exported as $ARGS"
          }
          set TRYCMD 1
        #
        # BFCMDLIST
        # these are special tcl commands for BF (Boolean functions)
        #
        # biddy_print_table, biddy_print_sop, biddy_print_minterms, biddy_dependent_variable_number, biddy_count_minterms,
        # biddy_density_of_function, bddscout_reset_all_values, bddscout_set_values, biddy_eval, biddy_support
        } elseif {$CMD == "biddy_set_alphabetic_ordering"} {
          if {$ARGS == ""} {
            #input is already OK
          } else {
            set INPUT $CMD
          }
          set TRYCMD 1
        } elseif {$CMD == "bddscout_reset_all_values"} {
          if {$ARGS == ""} {
            #input is already OK
          } else {
            #extra arguments are ignored
            set INPUT $CMD
          }
          set TRYCMD 1
        } elseif {$CMD == "bddscout_set_values"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_support"} {
          if {$ARGS == ""} {
            set INPUT [string cat $CMD " " $BDDNAME "_SUPPORT " $BDDNAME]
          } else {
            #input should be OK
          }
          set TRYCMD 1
        #
        # CACMDLIST
        # these are special tcl commands for CA (cube algebra)
        #
        # biddy_change, biddy_varsubset, biddy_subset0, biddy_subset1, biddy_quotient,
        # biddy_remainder, biddy_element_abstract, biddy_product, biddy_selective_product, biddy_supset,
        # biddy_subset, biddy_permitsym, biddy_stretch
        } elseif {$CMD == "biddy_change"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_varsubset"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_subset0"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_subset1"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "quotient"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_remainder"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_element_abstract"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_product"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_selective_product"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_supset"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_subset"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_permitsym"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        } elseif {$CMD == "biddy_stretch"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            #input should be OK
            set TRYCMD 1
          }
        #
        # BDDLCMDLIST
        # no special tcl commands for BDDL implemented, yet
        #
        # CMDLIST
        # biddy_get_then, biddy_get_else, biddy_count_nodes, biddy_count_nodes_plain, biddy_count_complemented_edges,
        # biddy_count_paths, biddy_max_level, biddy_avg_level, biddy_density_of_bdd, biddy_min_nodes,
        # biddy_max_nodes, bddscout_count_nodes, bddscout_reset_all_probabilities, bddscout_set_probabilities, bddscout_eval_probability
        } elseif {$CMD == "biddy_get_then"} {
          if {$ARGS == ""} {
            set INPUT [string cat $CMD " " $BDDNAME "_T " $BDDNAME]
          }
          set TRYCMD 1
        } elseif {$CMD == "biddy_get_else"} {
          if {$ARGS == ""} {
            set INPUT [string cat $CMD " " $BDDNAME "_E " $BDDNAME]
          }
          set TRYCMD 1
        } elseif {$CMD == "bddscout_reset_all_probabilities"} {
          if {$ARGS == ""} {
            #input is already OK
          } else {
            #extra arguments are ignored
            set INPUT $CMD
          }
          set TRYCMD 1
        } elseif {$CMD == "bddscout_set_probabilities"} {
          if {$ARGS == ""} {
            #missing arguments
            set TRYCMD 2
          } else {
            set TRYCMD 1
          }
        } else {
          if {$ARGS == ""} {
            set INPUT "$INPUT $BDDNAME"
          }
          set TRYCMD 1
        }
      } elseif {$CMD == "bddscout_message"} {
        set OKINPUT [string map {"[" "\\[" "]" "\\]"} $INPUT]
        set TRYCMD 2
        eval $OKINPUT
      } elseif {[string range $INPUT 0 4] == "biddy"} {
        set TRYCMD 2
      } elseif {[string range $INPUT 0 7] == "bddscout"} {
        set TRYCMD 2
      }
    }

    # if (INPUTTYPE == 0) and the input was not recognized as a tcl command then go with a Boolean function
    if {($INPUTTYPE == 0) && ($TRYCMD == 0) && ($force != 0)} {

      set force 0
      set INPUT [string map {"\"" ""} $INPUT]
      set name [bddscout_parse_input_infix $INPUT]

      # puts "DEBUG parseinput: <$name>"
      # bddscout_parse_input_infix returns name=="" for unsuccessful calls

      if {$name != ""} {
        # refresh formulae tree $selectwin.browser
        # for OBDD, remove the current tmp file for the resulting formula, only ...
        # ... and if not modifying the current formula, then also create and store tmp file for current formula
        # for ZBDD and TZBDD, remove tmp file for all formulae, adding variables may change them
        parseinput_after $name
      } else {
        set TRYCMD 2
      }

    }

    # if (INPUTTYPE == 1) and the input was not recognized as a tcl command then go with a Unate cube set algebra
    if {($INPUTTYPE == 1) && ($TRYCMD == 0) && ($force != 0)} {

      set force 0
      set INPUT [string map {"\"" ""} $INPUT]
      set name [bddscout_parse_cube $BDDNAME $INPUT]

      if {$name != ""} {
        # refresh formulae tree $selectwin.browser
        # for OBDD, remove the current tmp file for the resulting formula, only ...
        # ... and if not modifying the current formula, then also create and store tmp file for current formula
        # for ZBDD and TZBDD, remove tmp file for all formulae, adding variables may change them
        parseinput_after $name
      }
    }

    # if (INPUTTYPE == 6) and the input was not recognized as a tcl command then go with a Knuth's BDDL command
    if {($INPUTTYPE == 6) && ($TRYCMD == 0) && ($force != 0)} {

      set force 0
      set INPUT [string map {"\"" ""} $INPUT]
      set name [bddscout_parse_bddl $INPUT]

      # bddview_message "Knuth's BDDL command" "<$name> <$INPUT>"

      if {$name != ""} {
        # refresh formulae tree $selectwin.browser
        # for OBDD, remove the current tmp file for the resulting formula, only ...
        # ... and if not modifying the current formula, then also create and store tmp file for current formula
        # for ZBDD and TZBDD, remove tmp file for all formulae, adding variables may change them
        parseinput_after $name
      }
    }

    # if the command was recognized as a full tcl command or the execution is forced
    if {($TRYCMD == 1) || ($force != 0)} {

      set OKINPUT [string map {"[" "\\[" "]" "\\]"} $INPUT]
      if {[string range $OKINPUT 0 15] == "bddscout_message"} {
        eval $OKINPUT
        set result ""
      } else {
        set result [eval $OKINPUT]
        # refresh formulae tree $selectwin.browser
        update_info
        #is this needed ??
        #if {$BDDNAME != ""} {
        #  focusform
        #}
      }

      if {$result != ""} {
        bddview_message "$INPUT" "$result"
      }
    }

    #puts "TRYCMD: $TRYCMD"
    #puts "force: $force"
    #puts "INPUT: $INPUT"

    # this clears the input line, partial command must remain if constructed from the menu
    if {$TRYCMD != 2} {
      set INPUT ""
    }

    # use this if you prefer keeping tcl commands without arguments when in Tcl command mode
    if {($INPUTTYPE == 7) && ($force != 0)} {
      set INPUT $CMD
    }
  }
}

# use ghostscript to export bitmap (png) or pdf
proc export_by_gs {win type gstype} {
  global OS
  global DPI
  global GHOSTSCRIPT_EXE
  global BDDSCOUT_PATH_EXAMPLES
  global ZOOM
  global XMAX
  global YMAX
  global BDDNAME

  set dpi 300

  if {$BDDNAME == ""} return;

  if {[file executable $GHOSTSCRIPT_EXE] != 1} {
    bddview_message "ERROR" "Cannot run Ghostscript ($GHOSTSCRIPT_EXE)"
    return
  }

  puts "Exporting $BDDNAME.$type ..."

  set filename [tk_getSaveFile -title "Export to [string toupper $type]" -initialdir "$BDDSCOUT_PATH_EXAMPLES" -initialfile "$BDDNAME.$type" -parent $win]

  if {[string length $filename] != 0} {

    #remember last path
    set BDDSCOUT_PATH_EXAMPLES [file dirname $filename]

    set koord [$win bbox all]
    if {$koord == ""} {set koord [list 0 0 0 0]}
    set x1 [lindex $koord 0]
    set y1 [lindex $koord 1]
    set x2 [lindex $koord 2]
    set y2 [lindex $koord 3]

    if {$OS == "unix"} {

      set gs [open "|$GHOSTSCRIPT_EXE -q -sDEVICE=$gstype -g[expr round(($x2-$x1)*($dpi/$DPI))]x[expr round(($y2-$y1)*($dpi/$DPI))] -r$dpi -dNOPAUSE -dBATCH -dSAFER -sOutputFile=$filename -" w]

      $win postscript -channel $gs -pageanchor sw -pagex 0 -pagey 0 -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1]

      close $gs

    } elseif {$OS == "windows"} {

      $win postscript -file tmp.ps -pageanchor sw -pagex 0 -pagey 0 -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1]

      exec $GHOSTSCRIPT_EXE -q -sDEVICE=$gstype -g[expr round(($x2-$x1)*($dpi/$DPI))]x[expr round(($y2-$y1)*($dpi/$DPI))] -r$dpi -dNOPAUSE -dBATCH -dSAFER -sOutputFile=$filename tmp.ps

      file delete tmp.ps

    }

  }
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

# ###############################################################
# API: update_info and update_gui
# ###############################################################

# update list of variables, list of formulae, and statistical info
proc update_info {  } {
  global varwin
  global selectwin
  global bddscout__info
  global bddscout__biddyInfo

  #global BDDNAME
  #puts "DEBUG (update_info start): BDDNAME = <$BDDNAME>"

  # variables

  set listName [bddscout_list_variables_by_order]
  set num [llength $listName]

  $varwin.browser delete [$varwin.browser items]
  if {$num > 0} {
    addList $listName $varwin.browser root
  }

  # formulae

  set currentTree [getTreeAsList $selectwin.browser]
  set listName [bddscout_list_formulae_by_name]
  set num [llength $listName]

  #puts "DEBUG (update_info): listName = $listName"
  #puts "DEBUG (update_info): currentTree = $currentTree"

  # tmpfiles is a dict of tmpfile names
  set tmpfiles [dict create]
  foreach n $currentTree {
    #puts "DEBUG (update_info): currentTreeItem = $n, <[getTreeItemData $selectwin.browser $n]>"
    dict append tmpfiles $n [getTreeItemData $selectwin.browser $n]
  }

  # selectwin.browser delete will invoke changeform with fname == ""
  $selectwin.browser delete [$selectwin.browser nodes root]
  if {$num > 0} {
    addTree $listName $selectwin.browser root
  }

  # restore existing tmpfile names
  foreach n $listName {
    set d ""
    if {[dict exists $tmpfiles $n]} {
      set d [dict get $tmpfiles $n]
      setTreeItemData $selectwin.browser $n $d
    }
    #puts "DEBUG (update_info): listItem = $n, <$d>"
  }

  set maxnodes "-"
  set maxdepth "-"
  set avgdepth "-"

  if {($num > 0) && ($num < 100)} {

    # maxlevel and avglevel are disabled because the problem with efficiency

    set listNumber [bddscout_list_formulae_by_node_number]
    #set listMax [bddscout_list_formulae_by_node_max_level]
    #set listAvg [bddscout_list_formulae_by_node_avg_level]

    set f [join [lindex $listNumber end]]
    set maxnodes [string range $f 0 [expr [string last "(" $f] -1]]
    set nodes [findData $maxnodes $listNumber]
    #set max [findData $maxnodes $listMax]
    set max "-"
    #set avg [findData $maxnodes $listAvg]
    set avg "-"
    set maxnodes "$maxnodes (nodes = $nodes, depth = $max, avg depth = $avg)"

    #set f [join [lindex $listMax end]]
    #set maxdepth [string range $f 0 [expr [string last "(" $f] -1]]
    #set nodes [findData $maxdepth $listNumber]
    #set max [findData $maxdepth $listMax]
    #set avg [findData $maxdepth $listAvg]
    #set maxdepth "$maxdepth (nodes = $nodes, depth = $max, avg depth = $avg)"

    #set f [join [lindex $listAvg end]]
    #set avgdepth [string range $f 0 [expr [string last "(" $f] -1]]
    #set nodes [findData $avgdepth $listNumber]
    #set max [findData $avgdepth $listMax]
    #set avg [findData $avgdepth $listAvg]
    #set avgdepth "$avgdepth (nodes = $nodes, depth = $max, avg depth = $avg)"

  }

  set list [list \
    "Number of functions = $num" \
    "\nMax nodes = $maxnodes" \
    "\nMax depth = $maxdepth" \
    "\nMax avg depth = $avgdepth" \
  ]

  set bddscout__info [join $list]

  set list [list \
    "biddy_variabletable_num = " [biddy_variabletable_num] \
    "\nbiddy_nodetable_size = " [biddy_nodetable_size] \
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

  set bddscout__biddyInfo [join $list]

  #puts "DEBUG (update_info finish): BDDNAME = <$BDDNAME>"
}

# update gui colors, fonts, etc.
proc update_gui {  } {
  global mainwin
  global toolbar
  global bddtype
  global bb0
  global bb1
  global bb2
  global bb3
  global bb4
  global bb5
  global sep1
  global sep2
  global sep3
  global sep4
  global verticalwindow
  global horizontalwindow
  global inputwin
  global inputtype
  global selectwin
  global varwin
  global menubar
  global ZOOM
  global DPI
  global FONTFAMILYLABEL
  global FONTSIZELABEL
  global FONTFAMILYTAG
  global FONTSIZETAG
  global FONTFAMILYINFO
  global FONTSIZEINFO
  global FONTFAMILYMENU
  global FONTSIZEMENU
  global FONTFAMILYREPORT
  global FONTSIZEREPORT
  global GRIDON
  global ARROWSON
  global COLORFRAME
  global COLORMENU
  global COLORBUTTON
  global COLORBG
  global COLORFG
  global COLORNODE
  global COLORGRID
  global COLORLEFT
  global COLORRIGHT

  $mainwin configure -highlightcolor $COLORMENU -bg $COLORBG
  $toolbar configure -bg $COLORMENU
  $bb0 configure -bg $COLORMENU
  $bb0 itemconfigure 0 -bg $COLORMENU
  $bb0 itemconfigure 1 -bg $COLORMENU
  $bb0 itemconfigure 2 -bg $COLORMENU
  $bb1 configure -bg $COLORMENU
  $bb1 itemconfigure 0 -bg $COLORMENU -highlightbackground $COLORBUTTON
  $bb1 itemconfigure 1 -bg $COLORMENU -highlightbackground $COLORBUTTON
  $bb1 itemconfigure 2 -bg $COLORMENU -highlightbackground $COLORBUTTON
  $bb1 itemconfigure 3 -bg $COLORMENU -highlightbackground $COLORBUTTON
  $bb2 configure -bg $COLORMENU
  $bb2 itemconfigure 0 -bg $COLORMENU
  $bb2 itemconfigure 1 -bg $COLORMENU
  $bb2 itemconfigure 2 -bg $COLORMENU
  $bb2 itemconfigure 3 -bg $COLORMENU
  $bb3 configure -bg $COLORMENU
  $bb3 itemconfigure 0 -bg $COLORFRAME -activebackground $COLORFRAME -highlightbackground $COLORBUTTON
  $bb3 itemconfigure 1 -bg $COLORFRAME -activebackground $COLORFRAME -highlightbackground $COLORBUTTON
  $bb3 itemconfigure 2 -bg $COLORFRAME -activebackground $COLORFRAME -highlightbackground $COLORBUTTON
  $bb3 itemconfigure 3 -bg $COLORFRAME -activebackground $COLORFRAME
  $bb3 itemconfigure 4 -bg $COLORFRAME -activebackground $COLORFRAME
  $bb4 configure -bg $COLORMENU
  $bb4 itemconfigure 0 -bg $COLORMENU
  $bb4 itemconfigure 1 -bg $COLORMENU
  $bb4 itemconfigure 2 -bg $COLORMENU
  $bb5 configure -bg $COLORMENU
  $bb5 itemconfigure 0 -bg $COLORMENU
  if {[wm attributes . -fullscreen]} {
    $bb5 itemconfigure 1 -bg $COLORMENU
    $bb5 itemconfigure 2 -bg $COLORMENU
  }
  $sep1 configure -bg $COLORMENU
  $sep2 configure -bg $COLORMENU
  $sep3 configure -bg $COLORMENU
  $sep4 configure -bg $COLORMENU
  $verticalwindow configure -bg $COLORBG
  $horizontalwindow configure -bg $COLORBG
  $inputwin configure -highlightcolor $COLORFRAME -bg $COLORBG
  $inputwin.entry configure -font [list $FONTFAMILYLABEL 10] -bg $COLORBG
  $inputwin.label configure -bg $COLORBG
  $inputwin.label.menu configure -bg $COLORBG -font [list $FONTFAMILYINFO 10] -activebackground $COLORBG
  $inputwin.label.menu.tclcmd configure -bg $COLORBG -font [list $FONTFAMILYINFO 10] -activebackground $COLORGRID
  $inputtype configure -font [list $FONTFAMILYINFO 12] -bg $COLORBG -activebackground $COLORGRID
  $selectwin configure -highlightcolor $COLORFRAME -bg $COLORBG
  $selectwin.browser configure -bg $COLORBG
  $varwin configure -highlightcolor $COLORFRAME -bg $COLORBG
  $varwin.browser configure -fg $COLORFG -bg $COLORBG
  $toolbar.type configure -bg $COLORMENU
  $menubar configure -bg $COLORFRAME
  $menubar.file configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.file.menu configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.view configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.view.menu configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.create configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.create.menu configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.help configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $menubar.help.menu configure -bg $COLORFRAME -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
  $toolbar.type.menu configure -bg $COLORMENU
  $bddtype configure -bg $COLORMENU -activebackground $COLORGRID
  foreach n [$mainwin find withtag {outline && label}] {
    $mainwin itemconfigure $n -outline $COLORBG -fill $COLORBG
  }
  foreach n [$mainwin find withtag {tekst && label}] {
    $mainwin itemconfigure $n -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]]
  }
  foreach n [$mainwin find withtag {outline && terminal}] {
    $mainwin itemconfigure $n -fill $COLORNODE
  }
  foreach n [$mainwin find withtag {tekst && terminal}] {
    $mainwin itemconfigure $n -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]]
  }
  foreach n [$mainwin find withtag {outline && internal}] {
    $mainwin itemconfigure $n -fill $COLORNODE
  }
  foreach n [$mainwin find withtag {tekst && internal}] {
    $mainwin itemconfigure $n -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]]
  }
  foreach n [$mainwin find withtag {left || leftline}] {
    $mainwin itemconfigure $n -fill $COLORLEFT
  }
  foreach n [$mainwin find withtag {right || rightline}] {
    $mainwin itemconfigure $n -fill $COLORRIGHT
  }
  foreach n [$mainwin find withtag {leftmark}] {
    $mainwin itemconfigure $n -outline $COLORLEFT -fill $COLORLEFT
  }
  foreach n [$mainwin find withtag {rightmark}] {
    $mainwin itemconfigure $n -outline $COLORRIGHT -fill $COLORRIGHT
  }
  foreach n [$mainwin find withtag {lefttag}] {
    $mainwin itemconfigure $n -fill $COLORLEFT -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]]
  }
  foreach n [$mainwin find withtag {righttag}] {
    $mainwin itemconfigure $n -fill $COLORRIGHT -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]]
  }
}

# ###############################################################
# API: browse variables
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
# API: browse formulae
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

# ####################################################################
# API: general functions
# ####################################################################

proc bddscout_message {text1 text2} {
  bddview_message $text1 $text2
}

proc bddscout_clear { } {
  global selectwin
  global BDDNAME
  global ACTIVEBDDTYPE

  set t $ACTIVEBDDTYPE

  # reset bddview
  bddview_clear

  # reset and init Biddy package
  # this should be compatible with initialization in bddscout.c */
  bddscout_exitPkg
  bddscout_initPkg
  set ACTIVEBDDTYPE ""
  set BDDNAME "0"
  changetype $t
}

proc bddscout_show {fname} {
  global BDDNAME

  set BDDNAME $fname
  focusform
}

proc bddscout_open {filename} {
  global selectwin
  global ACTIVEBDDTYPE
  global bddtype
  global BDDNAME
  global BDDNAMELIST

  if {([string length $filename] != 0) && [file exists $filename]} {

    #check BDD type, only the first specification is used
    #check, if there are all needed coordinates for label and nodes
    #file without coordinates is called an "incomplete" bddview format
    #if coordinates are present (1) then bddview_open will be used
    #if coordinates are not present (2) then biddy_read_bddview will be used

    set type ""
    set coordinates 0
    set f [open $filename r]
    set code 0
    set code [gets $f line]
    while {$code >= 0} {
      if {($type == "") && ([string range $line 0 3] == "type")} {set type [string trim [string range $line 5 end]]}
      if {($coordinates != 2) && ([string range $line 0 4] == "label")} {
        set line [string trimleft [string range $line 6 end]]
        set line [string trim [string range $line [string first " " $line] end]]
        if {[string first " " $line] != -1} {
          set coordinates 1
        } else {
          set coordinates 2
        }
      }
      if {($type != "") && ($coordinates == 2)} {
        #type is specified and at least some coordinates are missing
        set code -1
      } else {
        #type is not specified, yet, or not the whole file has been checked for coordinates
        set code [gets $f line]
      }
    }
    close $f

    if {$type == ""} {
      return 0
    }
    if {$coordinates == 0} {
      return 0
    }

    set currentbddname $BDDNAME
    set BDDNAME ""

    # use invoke instead of changetype to also change the label of the button
    if {(($type == "ROBDD") && ($ACTIVEBDDTYPE != "BIDDYTYPEOBDD")) ||
        (($type == "ROBDDCE") && ($ACTIVEBDDTYPE != "BIDDYTYPEOBDDC")) ||
        (($type == "ZBDD") && ($ACTIVEBDDTYPE != "BIDDYTYPEZBDD")) ||
        (($type == "ZBDDCE") && ($ACTIVEBDDTYPE != "BIDDYTYPEZBDDC")) ||
        (($type == "TZBDD") && ($ACTIVEBDDTYPE != "BIDDYTYPETZBDD"))
    } {
      if {$type == "ROBDD"} {
        $bddtype invoke 0
      } elseif {$type == "ROBDDCE"} {
        $bddtype invoke 1
      } elseif {$type == "ZBDD"} {
        $bddtype invoke 2
      } elseif {$type == "ZBDDCE"} {
        $bddtype invoke 3
      } elseif {$type == "TZBDD"} {
        $bddtype invoke 4
      } else {
        #no BDD type or an unknown BDD type - keep the existing type
      }
    } else {
      # for OBDDs, create and store tmp file for current formula
      # for ZBDDs and TZBDDs, importing new variables may change all existing formula
      if {($type == "ROBDD") || ($type == "ROBDDCE")} {
        createTreeItemData $selectwin.browser $currentbddname
      } else {
        clearTreeItemData $selectwin.browser
      }
    }

    #adapt if BiddyManagedConstructBDD returns an array of BDDs
    if {$coordinates == 1} {
      #bddview_open returns bdd type and also set BDDNAME ad BDDNAMELIST
      #puts "bddview_open $filename"
      bddview_open $filename
      set ROOTNAME $BDDNAME
      #puts "constructBDD: $BDDNAME, $BDDNAMELIST"
      foreach BDDNAME $BDDNAMELIST {
        if {[$selectwin.browser exists $BDDNAME]} {
          setTreeItemData $selectwin.browser $BDDNAME ""
        }
        constructBDD
      }
      # for OBDDs, create and store tmp file for current formula
      # for ZBDDs and TZBDDs, importing new variables may change all existing formula
      set BDDNAME $ROOTNAME
      update_info
      #focusform will redraw graph, we do not need this
      $selectwin.browser see $BDDNAME
    } else {
      #biddy_read_bddview returns the list of BDDs
      #puts "biddy_read_bddview $filename"
      set BDDNAMELIST [biddy_read_bddview $filename]
      #puts "biddy_read_bddview returned: $BDDNAME, $BDDNAMELIST"
      foreach BDDNAME $BDDNAMELIST {
        if {[$selectwin.browser exists $BDDNAME]} {
          setTreeItemData $selectwin.browser $BDDNAME ""
        }
      }
      set BDDNAME [lindex [split $BDDNAMELIST] 0]
      update_info
      focusform
    }

    return 1
  } else {
    return 0
  }
}

proc bddscout_change_bddtype { type } {
  if {$type == "ROBDD"} {
    changetype "BIDDYTYPEOBDD"
  } elseif {$type == "ROBDDCE"} {
    changetype "BIDDYTYPEOBDDC"
  } elseif {$type == "ZBDD"} {
    changetype "BIDDYTYPEZBDD"
  } elseif {$type == "ZBDDCE"} {
    changetype "BIDDYTYPEZBDDC"
  } elseif {$type == "TZBDD"} {
    changetype "BIDDYTYPETZBDD"
  } else {
    #no BDD type or an unknown BDD type - keep the existing type
  }
}

proc bddscout_update {} {
  global bddtype
  global ACTIVEBDDTYPE

  if {$ACTIVEBDDTYPE == "ROBDD"} {
    $bddtype invoke 0
  } elseif {$ACTIVEBDDTYPE == "ROBDDCE"} {
    $bddtype invoke 1
  } elseif {$ACTIVEBDDTYPE == "ZBDD"} {
    $bddtype invoke 2
  } elseif {$ACTIVEBDDTYPE == "ZBDDCE"} {
    $bddtype invoke 3
  } elseif {$ACTIVEBDDTYPE == "TZBDD"} {
    $bddtype invoke 4
  } else {
    #no BDD type or an unknown BDD type - keep the existing type
  }
}

proc bddscout_copy_formula { fname type1 type2 } {
  set t1 ""
  if {$type1 == "ROBDD"} {
    set t1 "BIDDYTYPEOBDD"
  } elseif {$type1 == "ROBDDCE"} {
    set t1 "BIDDYTYPEOBDDC"
  } elseif {$type1 == "ZBDD"} {
    set t1 "BIDDYTYPEZBDD"
  } elseif {$type1 == "ZBDDCE"} {
    set t1 "BIDDYTYPEZBDDC"
  } elseif {$type1 == "TZBDD"} {
    set t1 "BIDDYTYPETZBDD"
  }
  set t2 ""
  if {$type2 == "ROBDD"} {
    set t2 "BIDDYTYPEOBDD"
  } elseif {$type2 == "ROBDDCE"} {
    set t2 "BIDDYTYPEOBDDC"
  } elseif {$type2 == "ZBDD"} {
    set t2 "BIDDYTYPEZBDD"
  } elseif {$type2 == "ZBDDCE"} {
    set t2 "BIDDYTYPEZBDDC"
  } elseif {$type2 == "TZBDD"} {
    set t2 "BIDDYTYPETZBDD"
  }
  if {($t1 != "") && ($t2 != "")} {
    bddscoutCopyFormula $fname $t1 $t2
  }
}

# ###############################################################
# API: operations on formulae and BDDs
# ###############################################################

proc bddscout_count_nodes {fname} {
  return [biddy_count_nodes $fname]
}

proc bddscout_reset_all_values {} {
  biddy_reset_variables_value
}

proc bddscout_set_values {list} {
  foreach {var value} $list {
    biddy_set_variable_value $var $value
  }
}

proc bddscout_get_value {v} {
  return [biddy_get_variable_value $v]
}

proc bddscout_reset_all_probabilities {} {
  biddy_clear_variables_data
}

proc bddscout_set_probabilities {list} {
  foreach {var value} $list {
    biddy_set_variable_data_float $var $value
  }
}

proc bddscout_get_probability {v} {
  set p [biddy_get_variable_data_float $v]
  set p [string trim $p]
  set p [string trimright $p "0"]
  return $p
}

proc bddscout_eval_probability {fname} {
  set p [biddy_eval_probability $fname]
  set p [string trim $p]
  set p [string trimright $p "0"]
  return $p
}

proc bddscout_swap_with_higher {varname} {
  global selectwin
  global BDDNAME
  global BDDVARIABLES

  # puts "DEBUG bddscout_swap_with_higher <$varname>"
  # puts "DEBUG bddscout_swap_with_higher: <$BDDNAME>"
  # puts $BDDVARIABLES

  if {$varname == ".c."} {set varname "c"}
  set varname [string map {".AND." "&"} $varname]
  biddy_swap_with_higher $varname
  clearTreeItemData $selectwin.browser
  update_info
  focusform
}

proc bddscout_swap_with_lower {varname} {
  global selectwin
  global BDDNAME
  global BDDVARIABLES

  # puts "DEBUG bddscout_swap_with_lower <$varname>"
  # puts "DEBUG bddscout_swap_with_lower: <$BDDNAME>"
  # puts $BDDVARIABLES

  if {$varname == ".c."} {set varname "c"}
  set varname [string map {".AND." "&"} $varname]
  biddy_swap_with_lower $varname
  clearTreeItemData $selectwin.browser
  update_info
  focusform
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

#update

# this should be compatible with initialization in bddscout.c */
set BDDNAME ""
set ACTIVEBDDTYPE "BIDDYTYPEOBDD"

update_info

after 100
if { $DOT_EXE == "" } {
  puts "ERROR: Cannot run dot from Graphviz (DOT_EXE = \"$DOT_EXE\")"
  set SPLASHTEXT "ERROR: Cannot run dot from Graphviz (DOT_EXE = \"$DOT_EXE\")"
} else {
  puts "Ready!"
  set SPLASHTEXT "READY!"
}

$selectwin.browser selection set "0"

if { $USESPLASH == 1 } {

  after 100
  for {set i 1.} {$i>0.} {set i [expr $i-0.01]} {
    wm attributes .splash -alpha $i; update idletasks; after 10
  }
  destroy .splash

}

# if argument is given it is considered to be the name of a tcl script
if { $argc == 0 } {
} elseif { $argc == 1 } {
  set filename [lindex $argv 0]
  if {[string length $filename] != 0} {
    puts "Script $filename started."
    source $filename
    puts "Script $filename finished."
    #remember last path
    set BDDSCOUT_PATH_SCRIPTS [file dirname $filename]
  }
} else {
}
