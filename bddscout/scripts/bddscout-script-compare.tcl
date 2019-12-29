# Example script for Bdd Scout
# Robert Meolic, 2019
#
# BDD Scout supports 4 different BDD managers:
# * ROBDD - Reduced Ordered Binary Decision Diagrams
# * ROBDDCE - Reduced Ordered Binary Decision Diagrams with Complemented Edges
# * ZBDD - 0-sup Binary Decision Diagrams
# * ZBDDCE - 0-sup Binary Decision Diagrams with Complemented Edges
# * TZBDD - Tagged 0-sup Binary Decision Diagrams
#
# BDDs created in one BDD manager are not automaticaly copied to the other ones.
# For efficiency, constructed functions are not automaticaly shown.
#
# This script creates 2 LaTeX documents:
# * comparison of BDD types for all Boolean functions with 2 variables
# * comparison of BDD types for all Boolean functions with 3 variables

proc create_nvar_all {OUTPUTDIR var} {
  global BDDSCOUT_PATH_CREATE

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  bddscout_change_bddtype "ROBDD"
  source [string cat $BDDSCOUT_PATH_CREATE "/bddscout-create-fun" $var "var" ".tcl"]
  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set bddname "F$var\[[format "%02d" $i]\]"
    } else {
      set bddname "F$var\[[format "%03d" $i]\]"
    }
    bddscout_copy_formula $bddname "ROBDD" "ROBDDCE"
    bddscout_copy_formula $bddname "ROBDD" "ZBDD"
    bddscout_copy_formula $bddname "ROBDD" "ZBDDCE"
    bddscout_copy_formula $bddname "ROBDD" "TZBDD"
  }

  set outputdir "$OUTPUTDIR/TEX$var"
  file mkdir $outputdir

  bddscout_change_bddtype "ROBDD"
  puts "Creating TeX files for ROBDDs with $var variables..."
  export_tex_nvar "$outputdir" $var

  bddscout_change_bddtype "ROBDDCE"
  puts "Creating TeX files for ROBDDCEs with $var variables..."
  export_tex_nvar "$outputdir" $var

  bddscout_change_bddtype "ZBDD"
  puts "Creating TeX files for ZBDDs with $var variables..."
  export_tex_nvar "$outputdir" $var

  bddscout_change_bddtype "ZBDDCE"
  puts "Creating TeX files for ZBDDCEs with $var variables..."
  export_tex_nvar "$outputdir" $var

  bddscout_change_bddtype "TZBDD"
  puts "Creating TeX files for TZBDDs with $var variables..."
  export_tex_nvar "$outputdir" $var
}

proc export_tex_nvar {OUTPUTDIR var} {
  global ACTIVEBDDTYPE

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set bddname "F$var\[[format "%02d" $i]\]"
      set filename "$OUTPUTDIR/F$var-[format "%02d" $i]-$ACTIVEBDDTYPE.tex"
    } else {
      set bddname "F$var\[[format "%03d" $i]\]"
      set filename "$OUTPUTDIR/F$var-[format "%03d" $i]-$ACTIVEBDDTYPE.tex"
    }
    bddscout_show $bddname
    #puts "Creating TeX file \"$filename\""
    bddview_export_tex $filename "" 0
  }
}

proc create_tex_document {OUTPUTDIR var} {
  global BIDDYVERSION
  global BDDVIEWVERSION

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  set filename "$OUTPUTDIR/TEX$var/F$var-comparison.tex"
  set f [open $filename w]
  puts $f "%prepared using bddview v$BDDVIEWVERSION (biddy.meolic.com)"
  puts $f "%date: [clock format [clock seconds] -format {%d.%m.%Y %H:%M:%S}]"
  puts $f "%use 'pdflatex $filename' to produce pdf"
  puts $f ""
  puts $f "\\documentclass\[a4paper,twoside\]\{report\}"
  puts $f "\\usepackage\[margin=24mm\]\{geometry\}"
  puts $f "\\usepackage\{pbox\}"
  puts $f "\\usepackage\{tikz\}"
  puts $f "\\usetikzlibrary\{arrows,shapes,decorations.markings\}"
  puts $f "\\renewcommand\{\\thepage\}\{Comparison-\\arabic\{page\}\}"
  puts $f "\\raggedbottom"
  puts $f ""
  puts $f "\\begin\{document\}"
  puts $f "\\section*\{Comparison of BDDs with $var variables\}"
  puts $f "Generated on [clock format [clock seconds] -format {%d.%m.%Y}] with Bdd Scout v$BIDDYVERSION using bddview v$BDDVIEWVERSION.\\\\"
  puts $f "If you find this document usable please cite the research paper ``Robert Meolic. The Biddy BDD package. Journal of Open Source Software, 4(34), 1189, 2019.''"
  puts $f "\\bigskip"
  puts $f ""
  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set bddname "F$var\[[format "%02d" $i]\]"
      set filerobdd "F$var-[format "%02d" $i]-BIDDYTYPEOBDD.tex"
      set filerobddce "F$var-[format "%02d" $i]-BIDDYTYPEOBDDC.tex"
      set filezbdd "F$var-[format "%02d" $i]-BIDDYTYPEZBDD.tex"
      set filezbddce "F$var-[format "%02d" $i]-BIDDYTYPEZBDDC.tex"
      set filetzbdd "F$var-[format "%02d" $i]-BIDDYTYPETZBDD.tex"
    } else {
      set bddname "F$var\[[format "%03d" $i]\]"
      set filerobdd "F$var-[format "%03d" $i]-BIDDYTYPEOBDD.tex"
      set filerobddce "F$var-[format "%03d" $i]-BIDDYTYPEOBDDC.tex"
      set filezbdd "F$var-[format "%03d" $i]-BIDDYTYPEZBDD.tex"
      set filezbddce "F$var-[format "%03d" $i]-BIDDYTYPEZBDDC.tex"
      set filetzbdd "F$var-[format "%03d" $i]-BIDDYTYPETZBDD.tex"
    }
    set caption $bddname
    set caption [string map {"_" "\\_"} $caption]
    puts $f "\\begin\{center\}"
    puts $f "\\vbox\{"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$filerobdd\}\\vspace\{4mm\}\}\\hspace\{4mm\}"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$filerobddce\}\\vspace\{4mm\}\}\\hspace\{4mm\}"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$filezbdd\}\\vspace\{4mm\}\}\\hspace\{4mm\}"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$filezbddce\}\\vspace\{4mm\}\}\\hspace\{4mm\}"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$filetzbdd\}\\vspace\{4mm\}\}"
    puts $f ""
    puts $f "ROBDD, ROBDDCE, ZBDD, ZBDDCE, and TZBDD for Boolean function $bddname"
    puts $f "\}"
    puts $f "\\end\{center\}"
    puts $f ""
  }
  puts $f "\\cleardoublepage"
  puts $f "\\end\{document\}"
  close $f

  puts "Main TeX file: use 'cd [file dirname $filename]; pdflatex [file tail $filename]' to produce pdf."
}

# ####################################################################
# main program
# do not define global variables here
# ####################################################################

global BDDSCOUT_PATH_BIN
global BDDSCOUT_PATH_CREATE

bddscout_clear
bddscout_message "Click OK to start" "BDD Scout will be unresponsive until all LaTeX documents are generated."

# comparison of BDDs with 2 variables

create_nvar_all "$BDDSCOUT_PATH_BIN" 2
create_tex_document "$BDDSCOUT_PATH_BIN" 2
bddscout_clear

# comparison of BDDs with 3 variables

create_nvar_all "$BDDSCOUT_PATH_BIN" 3
create_tex_document "$BDDSCOUT_PATH_BIN" 3
bddscout_clear

# finished - change back to default BDD manager

bddscout_change_bddtype "ROBDD"
bddscout_message "SCRIPT FINISHED" "LaTeX documents succesfully generated."
