# Example script for Bdd Scout
# Robert Meolic, 2019
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

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ROBDD"

set expr "F1 = a1 + b1"
bddscout_parse_input_infix $expr

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ROBDDCE"
set expr "F2 = a2 + b2"
bddscout_parse_input_infix $expr

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ZBDD"
set expr "F3 = a3 + b3"
bddscout_parse_input_infix $expr

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ZBDDCE"
set expr "F3 = a3 + b3"
bddscout_parse_input_infix $expr

# switch BDD manager (if neccessary)
bddscout_change_bddtype "TZBDD"
set expr "F4 = a4 + b4"
bddscout_parse_input_infix $expr

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ROBDD"

# update GUI - refresh list of known Boolean functions in the current BDD manager
# this will not select/show any of the constructed function
bddscout_update

# show constructed function in the current BDD manager
# this will also update GUI making the previous call to bddscout_update redundant
# this is commented out to demonstrate the effect of not using this command

#bddscout_show "F1"
