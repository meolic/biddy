# Example script for Bdd Scout
# Robert Meolic, 2018
#
# BDD Scout supports 5 different BDD managers:
# * ROBDD - Reduced Ordered Binary Decision Diagrams
# * ROBDDCE - Reduced Ordered Binary Decision Diagrams with Complemented Edges
# * ZBDD - 0-sup Binary Decision Diagrams
# * ZBDDCE - 0-sup Binary Decision Diagrams with Complemented Edges
# * TZBDD - Tagged 0-sup Binary Decision Diagrams
#
# BDDs created in one BDD manager are not automaticaly copied to the other ones.
# For efficiency, constructed functions are not automaticaly shown.

# show console - needed for MS Windows
catch {console show}

# to make script more readable
proc EVAL { s } {bddscout_parse_input_infix $s}

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ROBDD"

# this is used to setup the initial variable ordering, only
EVAL "0 = 0 * a * b * c"

set expr "F = ~a * ~b * c + a * ~c + b * ~c"
set bdd [EVAL $expr]

# copy function F from manager ROBDD into another managers
bddscout_copy_formula "F" "ROBDD" "ROBDDCE"
bddscout_copy_formula "F" "ROBDD" "ZBDD"
bddscout_copy_formula "F" "ROBDD" "ZBDDCE"
bddscout_copy_formula "F" "ROBDD" "TZBDD"

# update GUI and show the constructed function in the current BDD manager
# please note that bddscout_copy_formula is not changing BDD manager
bddscout_show $bdd

# set Boolean values
bddscout_reset_all_values
bddscout_set_values {"a" 1 "b" 0 "c" 1}

# biddy_eval - write to console
puts "Function F(1,0,1) is evaluated to [biddy_eval F]"

# biddy_eval - write to message window
bddscout_message "RESULT" "Function F(1,0,1) is evaluated to [biddy_eval F]"
