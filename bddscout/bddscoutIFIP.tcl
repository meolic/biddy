#  Authors     [Robert Meolic (robert.meolic@um.si)]
#  Revision    [$Revision: 289 $]
#  Date        [$Date: 2017-07-13 12:27:36 +0200 (čet, 13 jul 2017) $]
#
#  Copyright   [This file is part of Bdd Scout package.
#               Copyright (C) 2008, 2017 UM-FERI
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

menubutton .menuFrame.ifip -menu .menuFrame.ifip.menu -pady 6 -font MENUFONT -text "IFIP"
pack .menuFrame.ifip -side left

menu .menuFrame.ifip.menu -font MENUFONT -relief groove -tearoff false

.menuFrame.ifip.menu add command -command menu_ifip_read -label "Read IFIP File ..."
.menuFrame.ifip.menu add separator
.menuFrame.ifip.menu add command -command menu_ifip_benchmark -label "IFIP benchmark"

proc menu_ifip_read {  } {
  global mainwin

  set filename [tk_getOpenFile -title "Select IFIP file" -parent $mainwin]
  if {[string length $filename] != 0} {
    puts "Reading file $filename ..."
    set et [time {bddscout_parseIFIP $filename [file tail [file rootname $filename]]}]
    puts $et
    update_info
  }
}

proc menu_ifip_benchmark {  } {
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
  IFIP_benchmark
}

proc IFIP_benchmark {  } {
  global tcl_platform
  global tcl_patchLevel
  global tk_patchLevel
  global STATUSBAR

  if {[file exists "/usr/share/IFIP"]} {
    cd "/usr/share/IFIP"
  } elseif {[file exists "./IFIP"]} {
    cd "./IFIP"
  } elseif {[file exists "../IFIP"]} {
    cd "../IFIP"
  } elseif {[file exists "./bddscout/IFIP"]} {
    cd "./bddscout/IFIP"
  } else {
    not_implemented_yet "IFIP files not found!"
    return
  }

  set status $STATUSBAR
  set STATUSBAR "IFIP benchmark in progress ..."

  toplevel .dialogIFIPbench
  wm title .dialogIFIPbench "IFIP Benchmark"
  wm iconname .dialogIFIPbench "IFIP Benchmark"
  grab set .dialogIFIPbench

  text .dialogIFIPbench.f -font REPORTFONT -width 120 -height 40 -tabs {250 center 370 center 490 center 610 center 730 center 850 center} -yscrollcommand ".dialogIFIPbench.ys set"
  .dialogIFIPbench.f tag configure bg0 -background azure1
  .dialogIFIPbench.f tag configure bg1 -background azure2
  .dialogIFIPbench.f tag configure bg2 -background azure3
  .dialogIFIPbench.f tag configure bold -font BOLDFONT

  set OS "$tcl_platform(machine) ($tcl_platform(os), $tcl_platform(osVersion)) with tcl $tcl_patchLevel and tk $tk_patchLevel"

  .dialogIFIPbench.f insert end "IFIP benchmark report generated by BDD Scout\n" {bg2 bold}
  .dialogIFIPbench.f insert end "$OS\n" {bg2 bold}
  .dialogIFIPbench.f insert end "[clock format [clock seconds] -format {%d.%m.%Y %H:%M:%S}]\n" {bg2 bold}
  .dialogIFIPbench.f insert end "\n" {bg2 bold}
  .dialogIFIPbench.f insert end "Filename\tElapsed.Time\tNum.Variables\tNum.Nodes\tNum.GC\tResult\n" {bg2 bold}

  set list [list \
    "cath/add1.be" \
    "cath/add2.be" \
    "cath/add3.be" \
    "cath/add4.be" \
    "cath/addsub.be" \
    "cath/alu.be" \
    "" \
    "ex/ex2.be" \
    "ex/mul03.be" \
    "ex/mul04.be" \
    "ex/mul05.be" \
    "ex/mul06.be" \
    "ex/mul07.be" \
    "ex/mul08.be" \
    "ex/rip02.be" \
    "ex/rip04.be" \
    "ex/rip06.be" \
    "ex/rip08.be" \
    "ex/transp.be" \
    "ex/ztwaalf1.be" \
    "ex/ztwaalf2.be" \
    "" \
    "hachtel/alupla20.be" \
    "hachtel/alupla21.be" \
    "hachtel/alupla22.be" \
    "hachtel/alupla23.be" \
    "hachtel/alupla24.be" \
    "hachtel/dc2.be" \
    "hachtel/dk17.be" \
    "hachtel/dk27.be" \
    "hachtel/f51m.be" \
    "hachtel/misg.be" \
    "hachtel/mlp4.be" \
    "hachtel/rd73.be" \
    "hachtel/risc.be" \
    "hachtel/root.be" \
    "hachtel/sqn.be" \
    "hachtel/vg2.be" \
    "hachtel/x1dn.be" \
    "hachtel/x6dn.be" \
    "hachtel/z4.be" \
    "hachtel/z5xpl.be" \
    "hachtel/z9sym.be" \
    "" \
    "plasco/counter.be" \
    "plasco/d3.be" \
    "plasco/hostint1.be" \
    "plasco/in1.be" \
    "plasco/mp2d.be" \
    "plasco/mul.be" \
    "plasco/pitch.be" \
    "plasco/rom2.be" \
    "plasco/table.be" \
    "plasco/werner.be" \
  ]

  set maxVar 0
  set maxNodes 0
  set totalT 0
  
# TO DO: CHANGE THIS
  set totalFOA 0
  set totalCMP 0
  set totalADD 0

  bddscout_clearPkg

  set i 0
  foreach name $list  {

    if  {$name == ""} {
    } else {

      set et [time {set report [bddscout_parseIFIP $name [file tail [file rootname $name]]]}]
      set et [expr [scan $et %i] / 1000]

      set totalT [expr $totalT + $et]
      set totalFOA [expr $totalFOA + 0]
      set totalCMP [expr $totalCMP + 0]
      set totalADD [expr $totalADD + 0]

      set nameX [string map {"/" "_" "\\" "_" "." "_" ":" "_"} $name]

      set host [lindex $report 0]

      if {[lindex $report 1] == "0"} {
        set r "FALSE"
        set fgc "red"
      } elseif {[lindex $report 1] == "1"} {
        set r "TRUE"
        set fgc "black"
      } else {
        set r "???"
        set fgc "red"
      }

      set numvar [lindex $report 2]
      set numnodes [lindex $report 3]
      set garbage [lindex $report 4]

      if {$numvar > $maxVar} {
        set maxVar $numvar
      }

      if {$numnodes > $maxNodes} {
        set maxNodes $numnodes
      }

      .dialogIFIPbench.f insert end "$name" bg$i
      .dialogIFIPbench.f insert end "\t" bg$i
      .dialogIFIPbench.f insert end "$et ms" bg$i
      .dialogIFIPbench.f insert end "\t" bg$i
      .dialogIFIPbench.f insert end "$numvar" bg$i
      .dialogIFIPbench.f insert end "\t" bg$i
      .dialogIFIPbench.f insert end "$numnodes" bg$i
      .dialogIFIPbench.f insert end "\t" bg$i
      .dialogIFIPbench.f insert end "$garbage" bg$i
      .dialogIFIPbench.f insert end "\t" bg$i

      set report [lrange $report 5 end]
      menubutton .dialogIFIPbench.f.$nameX -menu .dialogIFIPbench.f.$nameX.list -text $r -bg azure3 -fg $fgc -relief groove -borderwidth 1
      menu .dialogIFIPbench.f.$nameX.list -tearoff false
      .dialogIFIPbench.f.$nameX.list add command -label $name
      foreach {fun r} $report {
        if {$r == "0"} {
          .dialogIFIPbench.f.$nameX.list add command -label "$fun FALSE" -foreground red
        } else {
          .dialogIFIPbench.f.$nameX.list add command -label "$fun TRUE" -foreground green3
        }
      }

      .dialogIFIPbench.f window create end -window .dialogIFIPbench.f.$nameX
      .dialogIFIPbench.f insert end "\n" bg$i

      set i [expr 1 - $i]
    }

  }

  .dialogIFIPbench.f insert end "\n" bg2
  .dialogIFIPbench.f insert end "$host\n" {bg2 bold}
  .dialogIFIPbench.f insert end "total time: $totalT ms\n" bg2
  .dialogIFIPbench.f insert end "max variables: $maxVar\n" bg2
  .dialogIFIPbench.f insert end "max nodes: $maxNodes\n" bg2
  .dialogIFIPbench.f insert end "FOA number: $totalFOA\n" bg2
  .dialogIFIPbench.f insert end "FOA compare: $totalCMP\n" bg2
  .dialogIFIPbench.f insert end "FOA add: $totalADD\n" bg2

  .dialogIFIPbench.f configure -state disabled

  set STATUSBAR $status

  scrollbar .dialogIFIPbench.ys -borderwidth 0 -command ".dialogIFIPbench.f yview" -highlightthickness 0 -orient vertical
  pack .dialogIFIPbench.ys -fill y -side right
  pack .dialogIFIPbench.f -expand yes -fill both

  set x [expr {([winfo screenwidth .] - [.dialogIFIPbench.f cget -width] * [font measure [.dialogIFIPbench.f cget -font] "0"] - [.dialogIFIPbench.ys cget -width]) / 2}]
  set y 0
  wm geometry .dialogIFIPbench +$x+$y

  update_info

  frame .dialogIFIPbench.buttons -relief raised

  button .dialogIFIPbench.buttons.ok -borderwidth 2 -command {
    destroy .dialogIFIPbench
  } -relief raised -text "OK" -width 6
  pack .dialogIFIPbench.buttons.ok -padx 10 -side left

  button .dialogIFIPbench.buttons.print -borderwidth 2 -command {
    not_implemented_yet "Printing is not implemented, yet"
  } -relief raised -text "Print" -width 6
  pack .dialogIFIPbench.buttons.print -padx 10 -side left

  pack .dialogIFIPbench.buttons
}
