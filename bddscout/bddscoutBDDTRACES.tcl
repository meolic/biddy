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

menubutton $menubar.bddtraces -text "BDDTRACES" -menu $menubar.bddtraces.menu -pady 6 -bg $COLORFRAME \
                          -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]
pack $menubar.bddtraces -side left -before $menubar.help

menu $menubar.bddtraces.menu -bg $COLORFRAME -relief groove -tearoff false \
                             -font [list -family $FONTFAMILYMENU -size $FONTSIZEMENU -weight normal -slant roman]

$menubar.bddtraces.menu add command -command menu_bddtraces_run -label "Run BDD Trace"

proc menu_bddtraces_run {  } {
  global mainwin
  global BDDSCOUT_PATH_BIN

  toplevel .warning
  wm title .warning "WARNING"
  wm iconname .warning "WARNING"
  grab set .warning

  message .warning.m -width 200 -text "All functions will be deleted and BDD package will be reset."
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

  set fname [tk_getOpenFile -title "Select BDD Trace file" -initialdir "$BDDSCOUT_PATH_BIN" -parent $mainwin]
  if {[string length $fname] != 0} {
    puts "Reading file $fname ..."
    set et [time {BDDTRACES_run $fname}]
    puts $et
    update_info
  }
}

proc BDDTRACES_run { fname } {
  global tcl_platform
  global tcl_patchLevel
  global tk_patchLevel

  toplevel .dialogBddTrace
  wm title .dialogBddTrace "BDD Trace"
  wm iconname .dialogBddTrace "BDD Trace"
  grab set .dialogBddTrace

  message .dialogBddTrace.m -width 200 -text "Bdd Trace in progress ..."
  pack .dialogBddTrace.m -fill both -expand yes

  set x [expr {([winfo screenwidth .]-[.dialogBddTrace.m cget -width])/2}]
  set y [expr {([winfo screenheight .])/2}]
  wm geometry .dialogBddTrace +$x+$y

  tkwait visibility .dialogBddTrace.m

  bddscout_clear

  text .dialogBddTrace.f -font REPORTFONT -width 160 -height 40 -tabs {750 left 850 left 950 left 1050 left 1150 left 1250 left} -yscrollcommand ".dialogBddTrace.ys set"
  .dialogBddTrace.f tag configure bg0 -background azure1
  .dialogBddTrace.f tag configure bg1 -background azure2
  .dialogBddTrace.f tag configure bg2 -background azure3
  .dialogBddTrace.f tag configure err -foreground red
  .dialogBddTrace.f tag configure bold -font BOLDFONT

  set OS "$tcl_platform(machine) ($tcl_platform(os), $tcl_platform(osVersion)) with tcl $tcl_patchLevel and tk $tk_patchLevel"

  .dialogBddTrace.f insert end "Bdd Trace report generated by BDD Scout\n" {bg2 bold}
  .dialogBddTrace.f insert end "$OS\n" {bg2 bold}
  .dialogBddTrace.f insert end "[clock format [clock seconds] -format {%d.%m.%Y %H:%M:%S}]\n" {bg2 bold}
  .dialogBddTrace.f insert end "\n" {bg2 bold}

  set et [time {set report [bddscout_runBddTrace $fname]}]
  set et [expr [scan $et %i] / 1000]

  #set host [lindex $report 0]
  set host ""
  set report [split $report "\t\n"]
  #set report [lreplace $report 0 0] #remove first element
  #set report [lreplace $report end end] #remove last element

  .dialogBddTrace.f insert end "Command" bg2
  .dialogBddTrace.f insert end "\t" bg2
  .dialogBddTrace.f insert end "Reported" bg2
  .dialogBddTrace.f insert end "\t" bg2
  .dialogBddTrace.f insert end "Time" bg2
  .dialogBddTrace.f insert end "\t" bg2
  .dialogBddTrace.f insert end "Stat time" bg2
  .dialogBddTrace.f insert end "\t" bg2
  .dialogBddTrace.f insert end "Counted" bg2
  .dialogBddTrace.f insert end "\t" bg2
  .dialogBddTrace.f insert end "Counted plain" bg2
  .dialogBddTrace.f insert end "\n" bg2

  set j 0
  set i 0
  set cmdlist [list]
  foreach cmd $report {
    lappend cmdlist $cmd
    incr j
    if {$j == 6} {
      if {[lindex $cmdlist 1] == [lindex $cmdlist 5]} {
        .dialogBddTrace.f insert end [lindex $cmdlist 0] bg$i
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 1] bg$i
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 2] bg$i
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 3] bg$i
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 4] bg$i
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 5] bg$i
        .dialogBddTrace.f insert end " \n" bg$i
      } else {
        .dialogBddTrace.f insert end [lindex $cmdlist 0] {bg$i err}
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 1] {bg$i err bold}
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 2] {bg$i err bold}
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 3] {bg$i err bold}
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 4] {bg$i err bold}
        .dialogBddTrace.f insert end "\t" bg$i
        .dialogBddTrace.f insert end [lindex $cmdlist 5] {bg$i err bold}
        .dialogBddTrace.f insert end " \n" bg$i

  destroy .dialogBddTrace.m

      }
      set j 0
      set cmdlist [list]
      set i [expr 1 - $i]
    }
  }

  .dialogBddTrace.f insert end "\n" bg2
  .dialogBddTrace.f insert end "Host: $host" bg2
  .dialogBddTrace.f insert end "\n" bg2
  .dialogBddTrace.f insert end "File: $fname" bg2
  .dialogBddTrace.f insert end "\n" bg2
  .dialogBddTrace.f insert end "Total time: $et ms" bg2
  .dialogBddTrace.f insert end "\n" bg2

  .dialogBddTrace.f configure -state disabled
  scrollbar .dialogBddTrace.ys -borderwidth 0 -command ".dialogBddTrace.f yview" -highlightthickness 0 -orient vertical
  pack .dialogBddTrace.ys -fill y -side right
  pack .dialogBddTrace.f -expand yes -fill both

  set x [expr {([winfo screenwidth .] - [.dialogBddTrace.f cget -width] * [font measure [.dialogBddTrace.f cget -font] "0"] - [.dialogBddTrace.ys cget -width]) / 2}]
  set y 0
  wm geometry .dialogBddTrace +$x+$y

  update_info

  frame .dialogBddTrace.buttons -relief raised

  button .dialogBddTrace.buttons.ok -borderwidth 2 -command {
    destroy .dialogBddTrace
  } -relief raised -text "OK" -width 6
  pack .dialogBddTrace.buttons.ok -padx 10 -side left

  button .dialogBddTrace.buttons.print -borderwidth 2 -command {
    not_implemented_yet "Printing is not implemented, yet"
  } -relief raised -text "Print" -width 6
  pack .dialogBddTrace.buttons.print -padx 10 -side left

  pack .dialogBddTrace.buttons
}
