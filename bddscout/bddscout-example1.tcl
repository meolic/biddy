# Example script for Bdd Scout
# Robert Meolic, 2017

set robdd "BIDDYTYPEOBDD"
set robddce "BIDDYTYPEOBDDC"
set zbddce "BIDDYTYPEZBDDC"
set tzbdd "BIDDYTYPETZBDD"

changetype $robdd
set expr "ROBDD = a1 + b1"
bddscout_parse_input_infix $expr

changetype $robddce
set expr "ROBDDCE = a2 + b2"
bddscout_parse_input_infix $expr

changetype $zbddce
set expr "ZBDDCE = a3 + b3"
bddscout_parse_input_infix $expr

changetype $tzbdd
set expr "TZBDD = a4 + b4"
bddscout_parse_input_infix $expr

update_info
