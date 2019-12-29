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

# change BDD type (if neccessary)
bddscout_change_bddtype "ROBDD"

# for efficiency, calculated functions are not shown automaticaly
# use bddscout_update and the end of calculations
# the first evaluation is used to setup the initial variable ordering, only

EVAL " \
  0 = 0 * SRA * SRB * SAA * SAB * SWA1 * SWA2 * SWA3 * SWA4 * SWB1 * SWB2 * SWB3 * SWB4 \
"

EVAL " \
F_SAA_SRA = \
  SAA + \
  SRA + \
  (SWA1 + SWA2) * (SWB1 + SWB2) \
"

EVAL " \
F_SAA_SRB = \
  SAA + \
  SRB + \
  (SWA1 + SWA3) * (SWB1 + SWB3) \
"

EVAL " \
F_SAB_SRA = \
  SAB + \
  SRA + \
  (SWA2 + SWA4) * (SWB2 + SWB4) \
"

EVAL " \
F_SAB_SRB = \
  SRB + \
  SAB + \
  (SWA3 + SWA4) * (SWB3 + SWB4) \
"

EVAL " \
FDEP_SAA = \
  SWA1 * SWB1 \
"

EVAL " \
FDEP_SAB = \
  SWA4 * SWB4 \
"

EVAL " \
FDEP_SRA = \
  SWA2 * SWB2 \
"

EVAL " \
FDEP_SRB = \
  SWA3 * SWB3 \
"

EVAL " \
FAILURE = \
  (F_SAA_SRA + FDEP_SAA + FDEP_SRA) * \
  (F_SAA_SRB + FDEP_SAA + FDEP_SRB) * \
  (F_SAB_SRA + FDEP_SAB + FDEP_SRA) * \
  (F_SAB_SRB + FDEP_SAB + FDEP_SRB)
"

# set failure probabilities
bddscout_reset_all_probabilities
bddscout_set_probabilities {"SAA" 0.00001 "SAB" 0.00001}
bddscout_set_probabilities {"SRA" 0.0219 "SRB" 0.0219}
bddscout_set_probabilities {"SWA1" 0.00001 "SWA2" 0.00001 "SWA3" 0.00001 "SWA4" 0.00001}
bddscout_set_probabilities {"SWB1" 0.00001 "SWB2" 0.00001 "SWB3" 0.00001 "SWB4" 0.00001}

# update GUI
# needed to refresh list of Boolean functions and list of variables
bddscout_update

# show resulting function
bddscout_show "FAILURE"

# bddscout_count_nodes - write to console
puts "Function FAILURE has [bddscout_count_nodes FAILURE] nodes"

# bddscout_eval_probability - write to console
puts "Function F_SAA_SRA is evaluated to probabilty [bddscout_eval_probability F_SAA_SRA]"
puts "Function F_SAA_SRB is evaluated to probabilty [bddscout_eval_probability F_SAA_SRB]"
puts "Function F_SAB_SRA is evaluated to probabilty [bddscout_eval_probability F_SAB_SRA]"
puts "Function F_SAB_SRB is evaluated to probabilty [bddscout_eval_probability F_SAB_SRB]"
puts "Function FAILURE is evaluated to probabilty [bddscout_eval_probability FAILURE]"

# bddscout_eval_probability - write to message window
bddscout_message "RESULT" "Function FAILURE is evaluated to probabilty [bddscout_eval_probability FAILURE]"
