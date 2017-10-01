# Example script for Bdd Scout
# Robert Meolic, 2017

set robdd "BIDDYTYPEOBDD"
set robddce "BIDDYTYPEOBDDC"
set zbddce "BIDDYTYPEZBDDC"
set tzbdd "BIDDYTYPETZBDD"

changetype $robdd
set expr "F = ~a * ~b * c + a * ~c + b * ~c"
set bdd [bddscout_parse_input_infix $expr]
bddscout_copy_formula "F" $robdd $robddce
bddscout_copy_formula "F" $robdd $zbddce
bddscout_copy_formula "F" $robdd $tzbdd
drawbdd $bdd
update_info
