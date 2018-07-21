# BDD SCOUT EXTENSION
# NAME: Boolean functions with 2 variables
# AUTHOR: Robert Meolic
# VERSION: 2018-01-29

proc create_fun2var_support {ch var} {
  set support "1"
  set seq 1
  for {set i 1} {$i <= $var} {incr i} {
    set support "$support*$ch$i"
  }
  return $support
}

proc create_fun2var {} {
  set var 2
  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  set support [create_fun2var_support "x" $var]
  bddscout_parse_input_infix "SUPPORT$var=$support"

  for {set i 0} {$i < $n} {incr i} {
    biddy_create_function "F$var\[[format "%02d" $i]\]" "SUPPORT$var" $i
  }
}

# ####################################################################
# main program
# do not define global variables here
# ####################################################################

create_fun2var
update_info
