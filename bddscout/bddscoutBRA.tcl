#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 444 $]
#  Date        [$Date: 2018-05-27 22:55:44 +0200 (ned, 27 maj 2018) $]
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

menubutton $menubar.bra -text "BRA" -menu $menubar.bra.menu -pady 6 -bg $COLORFRAME \
                          -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.bra -side left -before $menubar.help

menu $menubar.bra.menu -bg $COLORFRAME -relief groove -tearoff false \
                        -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

$menubar.bra.menu add command -command menu_bra_swap_with_higher -label "Swap with higher"
$menubar.bra.menu add command -command menu_bra_swap_with_lower -label "Swap with lower"
$menubar.bra.menu add separator
$menubar.bra.menu add command -command menu_bra_sifting -label "Sifting"
$menubar.bra.menu add command -command menu_bra_sifting_on_function -label "Sifting on function"
$menubar.bra.menu add command -command menu_bra_alphabetic -label "Set alphabetic ordering"
$menubar.bra.menu add separator
$menubar.bra.menu add command -command menu_bra_minimize -label "Minimize BDD"
$menubar.bra.menu add command -command menu_bra_maximize -label "Maximize BDD"
$menubar.bra.menu add separator
$menubar.bra.menu add command -command menu_bra_exhaustive -label "Exhaustive statistic"

proc menu_bra_swap_with_higher {  } {
  bddscout_swap_with_higher [browse_variables_byName]
}

proc menu_bra_swap_with_lower {  } {
  bddscout_swap_with_lower [browse_variables_byName]
}

proc menu_bra_sifting {  } {
  global BDDNAME
  global selectwin

  biddy_sifting
  set name $BDDNAME
  set BDDNAME ""
  update_info
  $selectwin.browser selection set $name
}

proc menu_bra_sifting_on_function {  } {
  global BDDNAME
  global selectwin

  if {$BDDNAME != ""} {
    biddy_sifting_on_function $BDDNAME
  } else {
    biddy_sifting_on_function [browse_formulae_byNodeNumber]
  }
  set name $BDDNAME
  set BDDNAME ""
  update_info
  $selectwin.browser selection set $name
}

proc menu_bra_alphabetic {  } {
  global BDDNAME
  global selectwin

  biddy_set_alphabetic_ordering
  set name $BDDNAME
  set BDDNAME ""
  update_info
  $selectwin.browser selection set $name
}

proc menu_bra_minimize {  } {
  global BDDNAME
  global selectwin

  if {$BDDNAME != ""} {
    biddy_minimize $BDDNAME
  } else {
    biddy_minimize [browse_formulae_byNodeNumber]
  }
  set name $BDDNAME
  set BDDNAME ""
  update_info
  $selectwin.browser selection set $name
}

proc menu_bra_maximize {  } {
  global BDDNAME
  global selectwin

  if {$BDDNAME != ""} {
    biddy_maximize $BDDNAME
  } else {
    biddy_maximize [browse_formulae_byNodeNumber]
  }
  set name $BDDNAME
  set BDDNAME ""
  update_info
  $selectwin.browser selection set $name
}

proc menu_bra_exhaustive {  } {
  global mainwin

  toplevel .warning
  wm title .warning "WARNING"
  wm iconname .warning "WARNING"
  grab set .warning

  message .warning.m -width 200 -text "Exhaustive statistic may take some time!"
  pack .warning.m -fill both -expand yes

  set x [expr {([winfo screenwidth .]-[.warning.m cget -width])/2}]
  set y [expr {([winfo screenheight .])/2}]
  wm geometry .warning +$x+$y

  frame .warning.buttons -relief raised

  button .warning.buttons.cancel -borderwidth 2 -command {
    destroy .warning
  } -relief raised -text "Cancel" -width 6
  pack .warning.buttons.cancel -padx 10 -side left

  button .warning.buttons.ok -borderwidth 2 -command {
    destroy .warning
  } -relief raised -text "OK" -width 6
  pack .warning.buttons.ok -padx 10 -side left

  pack .warning.buttons

  tkwait window .warning
  BRA_exhaustiveStatistic
}

proc BRA_exhaustiveStatistic {  } {
  global tcl_platform
  global tcl_patchLevel
  global tk_patchLevel

  set fname [browse_formulae_byNodeNumber]

  toplevel .dialogBRAexhaustive
  wm title .dialogBRAexhaustive "BRA Exhaustive statistic"
  wm iconname .dialogBRAexhaustive "BRA Exhaustive statistic"
  grab set .dialogBRAexhaustive

  message .dialogBRAexhaustive.m -width 200 -text "BRA Exhaustive statistic in progress ..."
  pack .dialogBRAexhaustive.m -fill both -expand yes

  set x [expr {([winfo screenwidth .]-[.dialogBRAexhaustive.m cget -width])/2}]
  set y [expr {([winfo screenheight .])/2}]
  wm geometry .dialogBRAexhaustive +$x+$y

  tkwait visibility .dialogBRAexhaustive.m

  text .dialogBRAexhaustive.f -font REPORTFONT -width 120  -height 40 -tabs {250 center 370 center 490 center 610 center 730 center} -yscrollcommand ".dialogBRAexhaustive.ys set"
  .dialogBRAexhaustive.f tag configure bg0 -background azure1
  .dialogBRAexhaustive.f tag configure bg1 -background azure2
  .dialogBRAexhaustive.f tag configure bg2 -background azure3
  .dialogBRAexhaustive.f tag configure bold -font BOLDFONT

  set OS "$tcl_platform(machine) ($tcl_platform(os), $tcl_platform(osVersion)) with tcl $tcl_patchLevel and tk $tk_patchLevel"

  set report [bddscout_bra_exhaustive $fname]
  set totalVar [biddy_variabletable_num]

# TO DO: UPDATE THIS
  set totalFOA 0
  set totalCMP 0
  set totalADD 0

  .dialogBRAexhaustive.f insert end "BRA Exhaustive statistic report generated by BDD Scout\n" {bg2 bold}
  .dialogBRAexhaustive.f insert end "$OS\n" {bg2 bold}
  .dialogBRAexhaustive.f insert end "[clock format [clock seconds] -format {%d.%m.%Y %H:%M:%S}]\n" {bg2 bold}
  .dialogBRAexhaustive.f insert end "\n" bg2
  .dialogBRAexhaustive.f insert end "Name-Order\tNum.Nodes\tMax.Level\tAvg.Level\n" {bg2 bold}

  set i 0
  foreach {name numnodes maxlevel avglevel} $report  {
    .dialogBRAexhaustive.f insert end "$name" bg$i
    .dialogBRAexhaustive.f insert end "\t" bg$i
    .dialogBRAexhaustive.f insert end "$numnodes" bg$i
    .dialogBRAexhaustive.f insert end "\t" bg$i
    .dialogBRAexhaustive.f insert end "$maxlevel" bg$i
    .dialogBRAexhaustive.f insert end "\t" bg$i
    .dialogBRAexhaustive.f insert end "$avglevel" bg$i
    .dialogBRAexhaustive.f insert end "\n" bg$i
    set i [expr 1 - $i]
  }

  .dialogBRAexhaustive.f insert end "\n" bg0
  .dialogBRAexhaustive.f insert end "Num Variables: $totalVar\n" bg0
  .dialogBRAexhaustive.f insert end "FOA number: $totalFOA\n" bg0
  .dialogBRAexhaustive.f insert end "FOA compare: $totalCMP\n" bg0
  .dialogBRAexhaustive.f insert end "FOA add: $totalADD\n" bg0

  .dialogBRAexhaustive.f configure -state disabled

  destroy .dialogBRAexhaustive.m

  scrollbar .dialogBRAexhaustive.ys -borderwidth 0 -command ".dialogBRAexhaustive.f yview" -highlightthickness 0 -orient vertical
  pack .dialogBRAexhaustive.ys -fill y -side right
  pack .dialogBRAexhaustive.f -expand yes -fill both

  set x [expr {([winfo screenwidth .] - [.dialogBRAexhaustive.f cget -width] * [font measure [.dialogBRAexhaustive.f cget -font] "0"] - [.dialogBRAexhaustive.ys cget -width]) / 2}]
  set y 0
  wm geometry .dialogBRAexhaustive +$x+$y

  update_info

  frame .dialogBRAexhaustive.buttons -relief raised

  button .dialogBRAexhaustive.buttons.ok -borderwidth 2 -command {
    destroy .dialogBRAexhaustive
  } -relief raised -text "OK" -width 6
  pack .dialogBRAexhaustive.buttons.ok -padx 10 -side left

  button .dialogBRAexhaustive.buttons.print -borderwidth 2 -command {
    not_implemented_yet "Printing is not implemented, yet"
  } -relief raised -text "Print" -width 6
  pack .dialogBRAexhaustive.buttons.print -padx 10 -side left

  pack .dialogBRAexhaustive.buttons
}
