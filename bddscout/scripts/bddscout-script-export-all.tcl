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
#
# This script creates 16 LaTeX documents, for each supported BDD type:
# * all Boolean functions with 2 variables ordered by truth table
# * all Boolean functions with 2 variables ordered by node number
# * all Boolean functions with 3 variables ordered by truth table
# * all Boolean functions with 3 variables ordered by node number

proc create_nvar {OUTPUTDIR BDDTYPE var} {
  global BDDSCOUT_PATH_CREATE

  bddscout_change_bddtype $BDDTYPE
  source [string cat $BDDSCOUT_PATH_CREATE "/bddscout-create-fun" $var "var" ".tcl"]

  set outputdir "$OUTPUTDIR/TEX-$BDDTYPE$var"
  file mkdir $outputdir
  puts "Creating TeX file for [string cat $BDDTYPE s] with $var variables..."
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
      set filename "$OUTPUTDIR/F$var-[format "%02d" $i].tex"
    } else {
      set bddname "F$var\[[format "%03d" $i]\]"
      set filename "$OUTPUTDIR/F$var-[format "%03d" $i].tex"
    }
    bddscout_show $bddname
    #puts "Creating TeX file \"$filename\""
    bddview_export_tex $filename "" 0
  }
}

proc create_tex_document {OUTPUTDIR BDDTYPE var} {
  global BIDDYVERSION
  global BDDVIEWVERSION

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  set filename "$OUTPUTDIR/TEX-$BDDTYPE$var/F$var-$BDDTYPE.tex"
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
  puts $f "\\renewcommand\{\\thepage\}\{$BDDTYPE-\\arabic\{page\}\}"
  puts $f "\\raggedbottom"
  puts $f ""
  puts $f "\\begin\{document\}"
  puts $f "\\section*\{$BDDTYPE with $var variables\}"
  puts $f "Generated on [clock format [clock seconds] -format {%d.%m.%Y}] with Bdd Scout v$BIDDYVERSION using bddview v$BDDVIEWVERSION.\\\\"
  puts $f "If you find this document usable please cite the research paper ``Robert Meolic. The Biddy BDD package. Journal of Open Source Software, 4(34), 1189, 2019.''"
  puts $f "\\bigskip"
  puts $f ""
  puts $f "\\begin\{center\}"
  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set fileone "F$var-[format "%02d" $i].tex"
    } else {
      set fileone "F$var-[format "%03d" $i].tex"
    }
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$fileone\}\\vspace\{4mm\}\}\\hfil"
  }
  puts $f "\\end\{center\}"
  puts $f ""
  puts $f "\\cleardoublepage"
  puts $f "\\end\{document\}"
  close $f

  puts "Created TeX file (use 'cd [file dirname $filename]; pdflatex [file tail $filename]' to produce pdf)."
}

proc create_tex_document_ordered {OUTPUTDIR BDDTYPE var} {
  global BIDDYVERSION
  global BDDVIEWVERSION

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  set ordered [bddscout_list_formulae_by_node_number]

  set filename "$OUTPUTDIR/TEX-$BDDTYPE$var/F$var-$BDDTYPE-ordered.tex"
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
  puts $f "\\renewcommand\{\\thepage\}\{$BDDTYPE-byNumberOfNodes-\\arabic\{page\}\}"
  puts $f "\\raggedbottom"
  puts $f ""
  puts $f "\\begin\{document\}"
  puts $f "\\section*\{$BDDTYPE with $var variables ordered by number of nodes\}"
  puts $f "Generated on [clock format [clock seconds] -format {%d.%m.%Y}] with Bdd Scout v$BIDDYVERSION using bddview v$BDDVIEWVERSION.\\\\"
  puts $f "If you find this document usable please cite the research paper ``Robert Meolic. The Biddy BDD package. Journal of Open Source Software, 4(34), 1189, 2019.''"
  puts $f "\\bigskip"
  puts $f ""
  set n 0
  foreach item $ordered {
    if {[string index $item 0] == "F"} {
      set num [string range $item [expr [string first "(" $item] +1] end-1]
      if {($num != $n) && ($n != 0)} {
        puts $f "\\end\{center\}"
        puts $f ""
      }
      if {$num != $n} {
        set n $num
        puts -nonewline $f "\\subsection*\{$BDDTYPE with $n node"
        if {$n == 1} {
          puts $f "\}"
        } else {
          puts $f "s\}"
        }
        puts $f "\\begin\{center\}"
      }
      set fileone [string range $item 0 [expr [string first "(" $item] -1]]
      set fileone [string map {"[" "-" "]" ""} $fileone]
      set fileone [string cat $fileone ".tex"]
      puts $f "\\pbox\{\\linewidth\}\{\\input\{$fileone\}\\vspace\{4mm\}\}\\hfil"
    }
  }
  puts $f "\\end\{center\}"
  puts $f ""
  puts $f "\\cleardoublepage"
  puts $f "\\end\{document\}"
  close $f

  puts "Created TeX file (use 'cd [file dirname $filename]; pdflatex [file tail $filename]' to produce pdf)."
}

# ####################################################################
# main program
# do not define global variables here
# ####################################################################

global BDDSCOUT_PATH_BIN
global BDDSCOUT_PATH_CREATE

bddscout_clear
bddscout_message "Click OK to start" "BDD Scout will be unresponsive until all LaTeX documents are generated."

# all Boolean functions with 2 variables

create_nvar "$BDDSCOUT_PATH_BIN" "ROBDD" 2
create_tex_document "$BDDSCOUT_PATH_BIN" "ROBDD" 2
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ROBDD" 2
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ROBDDCE" 2
create_tex_document "$BDDSCOUT_PATH_BIN" "ROBDDCE" 2
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ROBDDCE" 2
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ZBDD" 2
create_tex_document "$BDDSCOUT_PATH_BIN" "ZBDD" 2
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ZBDD" 2
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ZBDDCE" 2
create_tex_document "$BDDSCOUT_PATH_BIN" "ZBDDCE" 2
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ZBDDCE" 2
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "TZBDD" 2
create_tex_document "$BDDSCOUT_PATH_BIN" "TZBDD" 2
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "TZBDD" 2
bddscout_clear

# all Boolean functions with 3 variables

create_nvar "$BDDSCOUT_PATH_BIN" "ROBDD" 3
create_tex_document "$BDDSCOUT_PATH_BIN" "ROBDD" 3
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ROBDD" 3
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ROBDDCE" 3
create_tex_document "$BDDSCOUT_PATH_BIN" "ROBDDCE" 3
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ROBDDCE" 3
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ZBDD" 3
create_tex_document "$BDDSCOUT_PATH_BIN" "ZBDD" 3
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ZBDD" 3
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "ZBDDCE" 3
create_tex_document "$BDDSCOUT_PATH_BIN" "ZBDDCE" 3
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "ZBDDCE" 3
bddscout_clear

create_nvar "$BDDSCOUT_PATH_BIN" "TZBDD" 3
create_tex_document "$BDDSCOUT_PATH_BIN" "TZBDD" 3
create_tex_document_ordered "$BDDSCOUT_PATH_BIN" "TZBDD" 3
bddscout_clear

# finished - change back to default BDD manager

bddscout_change_bddtype "ROBDD"
bddscout_message "SCRIPT FINISHED" "LaTeX documents succesfully generated."
