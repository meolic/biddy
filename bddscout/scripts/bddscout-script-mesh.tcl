# Example script for Bdd Scout
# Robert Meolic, 2019
#
# This is from L. Xing, M. Tannous, V. M. Vokkarane, H. Wang and J. Guo:
# "Reliability Modeling of Mesh Storage Area Networks for Internet of Things,"
# in IEEE Internet of Things Journal, vol. 4, no. 6, pp. 2047-2057, Dec. 2017.
#
# this is a tcl script and thus syntax of tcl language must be respected

# show console - needed for MS Windows
catch {console show}

# to make script more readable
proc EVAL { s } {bddscout_parse_input_infix $s}

# choose BDD type
bddscout_change_bddtype "ROBDD"

# for efficiency, calculated functions are not shown automaticaly
# use bddscout_update and the end of calculations
# the first evaluation is used to setup the initial variable ordering, only

EVAL "0 = 0 * SRa * SRb * SAa * SAb * SWa1 * SWa2 * SWa3 * SWa4 * SWb1 * SWb2 * SWb3 * SWb4"

# generate all paths from src to dst
# sw is tcl list describing the set of switches
# m is tcl array describing the mesh topology
proc generatePaths {src dst sw m} {
  upvar $m mesh
  set numPaths 0
  set paths [list $src]
  while {$numPaths != [llength $paths]} {
    set numPaths [llength $paths]
    set newPaths ""
    foreach item $paths {
      set last [lindex $item end]
      if {[lsearch [concat $src $sw] $last] == -1} {lappend newPaths $item} else {
      foreach next [set mesh($last)] { if {
        ($next == $dst) || (([lsearch $sw $next] != -1) && ([lsearch $item $next] == -1))
      } { lappend newPaths [concat $item $next] }}}
    }
    set paths $newPaths
  }
  return $newPaths
}

# generate failure function for the set of paths
proc generateF {name paths} {
  EVAL "$name = 0"
  foreach p $paths { EVAL "$name = $name + !([join $p " + "])" }
  EVAL "$name = !$name"
}

# set of switches and mesh network from the paper
set sw {SWa1 SWa2 SWa3 SWa4 SWb1 SWb2 SWb3 SWb4}
array set mesh {
  SRa {SWa2 SWb2} SRb {SWa3 SWb3}
  SAa {SWa1 SWb1} SAb {SWa4 SWb4}
  SWa1 {SAa SWa2 SWa3} SWa2 {SRa SWa1 SWa3 SWa4}
  SWa3 {SRb SWa1 SWa2 SWa4} SWa4 {SAb SWa2 SWa3}
  SWb1 {SAa SWb2 SWb3} SWb2 {SRa SWb1 SWb3 SWb4}
  SWb3 {SRb SWb1 SWb2 SWb4} SWb4 {SAb SWb2 SWb3}
}

generateF FAA [generatePaths SRa SAa $sw mesh]
generateF FAB [generatePaths SRa SAb $sw mesh]
generateF FBA [generatePaths SRb SAa $sw mesh]
generateF FBB [generatePaths SRb SAb $sw mesh]

EVAL "FAILURE = FAA * FAB * FBA * FBB"

# set failure probabilities
bddscout_reset_all_probabilities
bddscout_set_probabilities {SRa 0.0219 SRb 0.0219}
bddscout_set_probabilities {SAa 0.00001 SAb 0.00001}
foreach n {SWa1 SWa2 SWa3 SWa4 SWb1 SWb2 SWb3 SWb4} {
  bddscout_set_probabilities [list $n 0.00001]
}

# update GUI
# needed to refresh list of Boolean functions and list of variables
bddscout_update

# show resulting function
bddscout_show FAILURE

# write results to console
puts "Number of nodes: [bddscout_count_nodes FAILURE]"
puts "Number of minterms: [biddy_count_minterms FAILURE]"
puts "Function FAILURE has [bddscout_count_nodes FAILURE] nodes"

# bddscout_eval_probability - write to console
puts "Function FAA is evaluated to probabilty [bddscout_eval_probability FAA]"
puts "Function FAB is evaluated to probabilty [bddscout_eval_probability FAB]"
puts "Function FBA is evaluated to probabilty [bddscout_eval_probability FBA]"
puts "Function FBB is evaluated to probabilty [bddscout_eval_probability FBB]"
puts "Probability: [bddscout_eval_probability FAILURE]"
puts "Probability of FAILURE is [bddscout_eval_probability FAILURE]"

# bddscout_eval_probability - write to message window
bddscout_message "RESULT" "Function FAILURE is evaluated to probabilty [bddscout_eval_probability FAILURE]"
