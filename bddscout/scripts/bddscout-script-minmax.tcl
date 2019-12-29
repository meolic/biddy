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
# This script creates 8 LaTeX documents, for each supported BDD type:
# * minmax for all Boolean functions with 2 variables
# * minmax for all Boolean functions with 3 variables

# this is from Glenn Jackman, thanks!
# https://stackoverflow.com/questions/29289360/tcl-check-for-same-content-between-two-files
proc comp_file {file1 file2} {
  # optimization: check file size first
  set equal 0
  if {[file size $file1] == [file size $file2]} {
    set fh1 [open $file1 r]
    set fh2 [open $file2 r]
    set equal [string equal [read $fh1] [read $fh2]]
    close $fh1
    close $fh2
  }
  return $equal
}

proc create_nvar_minmax {OUTPUTDIR BDDTYPE var} {
  global BDDSCOUT_PATH_CREATE

  bddscout_change_bddtype $BDDTYPE
  source [string cat $BDDSCOUT_PATH_CREATE "/bddscout-create-fun" $var "var" ".tcl"]

  set outputdir "$OUTPUTDIR/TEX-$BDDTYPE$var"
  file mkdir $outputdir
  puts "Creating TeX file for [string cat $BDDTYPE s] with $var variables..."
  export_tex_nvar_minmax "$outputdir" $var
}

proc export_tex_nvar_minmax {OUTPUTDIR var} {
  global ACTIVEBDDTYPE

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  biddy_set_alphabetic_ordering

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

  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set bddname "F$var\[[format "%02d" $i]\]"
      set minfilename "$OUTPUTDIR/F$var-[format "%02d" $i]-min.tex"
      set maxfilename "$OUTPUTDIR/F$var-[format "%02d" $i]-max.tex"
    } else {
      set bddname "F$var\[[format "%03d" $i]\]"
      set minfilename "$OUTPUTDIR/F$var-[format "%03d" $i]-min.tex"
      set maxfilename "$OUTPUTDIR/F$var-[format "%03d" $i]-max.tex"
    }

    biddy_set_alphabetic_ordering
    biddy_minimize $bddname
    bddscout_show $bddname
    #puts "Creating TeX file \"$filename-min\""
    bddview_export_tex $minfilename "" 0

    biddy_set_alphabetic_ordering
    biddy_maximize $bddname
    bddscout_show $bddname
    #puts "Creating TeX file \"$filename-max\""
    bddview_export_tex $maxfilename "" 0
  }
}

proc create_tex_document_minmax {OUTPUTDIR BDDTYPE var} {
  global BIDDYVERSION
  global BDDVIEWVERSION

  set m 1
  for {set i 0} {$i < $var} {incr i} {set m [expr 2 * $m]}
  set n 1
  for {set i 0} {$i < $m} {incr i} {set n [expr 2 * $n]}

  set outputdir "$OUTPUTDIR/TEX-$BDDTYPE$var"
  set filename "$outputdir/F$var-$BDDTYPE-minmax.tex"
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
  puts $f "\\renewcommand\{\\thepage\}\{$BDDTYPE-minmax-\\arabic\{page\}\}"
  puts $f "\\raggedbottom"
  puts $f ""
  puts $f "\\begin\{document\}"
  puts $f "\\section*\{$BDDTYPE with $var variables - the best and the worst ordering\}"
  puts $f "Generated on [clock format [clock seconds] -format {%d.%m.%Y}] with Bdd Scout v$BIDDYVERSION using bddview v$BDDVIEWVERSION.\\\\"
  puts $f "If you find this document usable please cite the research paper ``Robert Meolic. The Biddy BDD package. Journal of Open Source Software, 4(34), 1189, 2019.''"
  puts $f "\\bigskip"
  puts $f ""
  for {set i 0} {$i < $n} {incr i} {
    if {$var == 2} {
      set fileone "F$var-[format "%02d" $i].tex"
      set fileonemin "F$var-[format "%02d" $i]-min.tex"
      set fileonemax "F$var-[format "%02d" $i]-max.tex"
    } else {
      set fileone "F$var-[format "%03d" $i].tex"
      set fileonemin "F$var-[format "%03d" $i]-min.tex"
      set fileonemax "F$var-[format "%03d" $i]-max.tex"
    }
    puts $f "\\begin\{center\}"
    puts $f "\\pbox\{\\linewidth\}\{\\input\{$fileone\}\\vspace\{4mm\}\}\\hfil"
    if {[comp_file "$outputdir/$fileone" "$outputdir/$fileonemin"]} {
      puts $f "\\pbox\{\\linewidth\}\{*\\vspace\{4mm\}\}\\hfil"
    } else {
      puts $f "\\pbox\{\\linewidth\}\{\\input\{$fileonemin\}\\vspace\{4mm\}\}\\hfil"
    }
    if {[comp_file "$outputdir/$fileone" "$outputdir/$fileonemax"]} {
      puts $f "\\pbox\{\\linewidth\}\{*\\vspace\{4mm\}\}\\hfil"
    } else {
      puts $f "\\pbox\{\\linewidth\}\{\\input\{$fileonemax\}\\vspace\{4mm\}\}\\hfil"
    }
    puts $f "\\end\{center\}"
    puts $f ""
  }
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

# minmax for Boolean functions with 2 variables

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ROBDD" 2
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ROBDD" 2
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ROBDDCE" 2
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ROBDDCE" 2
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ZBDD" 2
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ZBDD" 2
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ZBDDCE" 2
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ZBDDCE" 2
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "TZBDD" 2
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "TZBDD" 2
bddscout_clear

# minmax for Boolean functions with 3 variables

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ROBDD" 3
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ROBDD" 3
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ROBDDCE" 3
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ROBDDCE" 3
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ZBDD" 3
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ZBDD" 3
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "ZBDDCE" 3
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "ZBDDCE" 3
bddscout_clear

create_nvar_minmax "$BDDSCOUT_PATH_BIN" "TZBDD" 3
create_tex_document_minmax "$BDDSCOUT_PATH_BIN" "TZBDD" 3
bddscout_clear

# finished - change back to default BDD manager

bddscout_change_bddtype "ROBDD"
bddscout_message "SCRIPT FINISHED" "LaTeX documents succesfully generated."
