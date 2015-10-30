#!/bin/sh
# the next line executes the program\
exec wish "$0" "$@"

# use this if your BWidget package is installed in user's space
# lappend auto_path /$env(HOME)/BWidget-1.8.0/

package require BWidget
package provide bddview 1.0

# ############################################
# $Revision: 104 $
# $Date: 2015-10-23 17:13:04 +0200 (pet, 23 okt 2015) $
#
# This file (bddview.tcl) is a Tcl/Tk script
# Author: Robert Meolic (robert.meolic@um.si)
#
# This file has been released into the public domain by
# the author.
#
# The author grants any entity the right to use this work
# for any purpose, without any conditions, unless such
# conditions are required by law.
#
# YOU NEED Tcl/Tk AND BWidget PACKAGE
# Both are included in ActiveTcl distribution
# Mac OS X: You can also use TclTk Aqua Batteries-Included
#
# http://www.tcl.tk/
# http://www.activestate.com/Products/activetcl/
# http://tcltkaqua.sourceforge.net/
# http://sourceforge.net/projects/tcllib/
# http://users.pandora.be/koen.vandamme1/tcl_tools/bwidgets/bwidgets.html
#
# bddview is used in Biddy project
# Homepage: http://lms.uni-mb.si/biddy/
#
# bddview is used in EST project
# Homepage: http://lms.uni-mb.si/EST/
#
# Updates of bddview from Robert Meolic
# are available in SVN repository:
# http://altair.uni-mb.si:8080/viewvc/
#
# USAGE:
# ./bddview.tcl example.bddview
#
# FORMAT:
# bddview reads BDD from the file with the
# following format:
#
# label <n> <name> <x> <y>
# node <n> <name> <x> <y>
# terminal <n> 1 <x> <y>
# connect <n1> <n2> <type>
#
# <n> is the unique number (integer)
# <name> is a string
# <x> and <y> are coordinates (integer)
# <type> is one of the following:
# s : single line
# si : inverted single line
# r : line to 'then' succesor
# l : line to 'else' successor
# li : inverted line to 'else' successor
# d : double line
#
# REMARKS:
# 1. (0,0) is top left corner.
# 2. Only one label is supported.
# 3. Single line (or inverted single line) should be
#    used to connect a label and a node.
# 4. Line to the right successor can not be inverted.
# 5. When using double line, the line to the left succesor
#    is always inverted.
#
# API (see near the end of file):
# proc not_implemented_yet { t }
# proc bddview_clear {}
# proc bddview_draw {name}
# proc bddview_saveas {win}
# proc bddview_print {win}
#
# Example (example.bddview):
# ---------------------------------------------------
# label 0 "Biddy" 100.0 10
# node 1 "B" 100 60
# node 2 "i" 100 125
# node 3 "d" 50 175
# terminal 4 1 50 240
# node 5 "y" 100 225
# terminal 6 1 100 290
# node 7 "d" 150 175
# terminal 8 1 150 240
# connect 0 1 s
# connect 1 2 d
# connect 2 3 l
# connect 2 7 r
# connect 3 4 l
# connect 3 5 r
# connect 5 6 d
# connect 7 5 l
# connect 7 8 r
#
# ---------------------------------------------------
# KNOWN BUGS:
# 1. none
#
# ############################################

set TITLE "BDDview, Robert Meolic"
set OS $tcl_platform(platform)

# DEBUGGING

# if {$tcl_platform(platform) == "windows"} {console show}
puts -nonewline "Starting $TITLE ... "

# ####################################################################
# IMAGES FOR TOOLBAR ICONS
#
# THEY ARE BASE64 ENCODED GIFS (32x32)
#
# --------------------------------------------------------------------
#
# Some images are from Kids Icons by Everaldo Coelho
#
# Obtained from:
# http://www.iconarchive.com/category/system/kids-icons-icons-by-everaldo.html
#
# License: Free for non-commercial use
#
# --------------------------------------------------------------------
#
# Some images are from Boomy toolbar icon set by Milosz Wlazlo
#
# Obtained from:
# http://www.iconarchive.com/category/application/boomy-icons-by-milosz-wlazlo.html
#
# License: Free for non-commercial use
#
# Details:
# All icons in the Boomy toolbar package are for personal use only.
# You may also use the icons in open source and freeware projects as
# long as you give credits to the author of the icons and link to his
# website (miloszwl.deviantart.com).
#
# The image data may be changed by You in a limited way, only by
# combining the included icons and adjusting hue, saturation, brightness.
#
# --------------------------------------------------------------------
#
# ####################################################################

set iconSave {
R0lGODlhIAAgAOf/AEhIeExLfE9SiFNSg1BTiVVUhVdVh1NWjVlXiVRailpY
illahV5YhVZcjFpbh1ddjV1bjlxdiF1eirNGIl9dkFpgkF5fi19gjGFfkWBh
jV1ik2JgkmFijmNhlGJjj7RNK2NkkGdki2RlkbVPMmVmkmdkmGZnlGlmmm1m
lGhplmprl2xpnWtsmWluoG9soLlZPnJulm9vnLtaP2xxpHVunXJvo3BxnnFy
n25zpXdwn3lyoXl1nXV2o3d4pXl5p394qHt7qXd8r3x9q4F9poV+rYODsYSE
s4mCsoiErYqDs4WJsI2GtpODto2JsoiMs4+IuJWGuYuLuo2Mu46OsZaKtpGN
t5yIto6OvY2RuZKRtJCQv8OCgKCMupKSwZOTwpyQvMSIf5SUw5aWxZWZwaWS
wJiYyKGVwZaaz5ubvpqayqOXw6SYxJyczKCcxp2dzaaaxquYxsOUnp+fz66a
yKKhxKCg0MmWlqKi0qufy86YkqGlzaOnw6ejzrKezaujyKqmxKinyqam1q+j
0KOn3LWh0Kin2Kerx8agrqmo2bej0rCoza2txKqt1qys3LOvzbur066x28On
2Lay0LWx3MCs27Oy48er3LG067a22rO34N+tn7i4z7m50Mqu38Oz2six27i3
6Ly34syw4tqzrrq567+62ce3382138+z5dW14b7C3tK26MTA3s666cPC59i5
5cPC9MrB6MjE48TH5MbF6sXE9snE8Ny96eO86tLG5+HB7s/L6tHN38zL/crN
+MvP7NTM5svO+drJ8s7N/+rC8dTQ4tDP9M7S79XQ79TP/M/S/OrH7tbR/tLV
/9PW/9nZ5ODV6dba6uXR9NnY/eXW8dvb8tjc+dva//vL/Nzb/+na6PHV++Le
/eHi7OHg+N7h/9/i///V/+Hl/+bl/efo8urm+Ofn/ujp8/zd/evn+ujo/+zq
7unp/+rq//ri/+zs9+/r/frn/evv//nq+O3x/+7y//Hy/fP0///x//b2//r3
/Pz6/v78//n///7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXMiwocOBJTZQgAABA4YODwu2OFEihhIsdAwZQjMFiIoMHDA07DGjxY8x
s6qNu+evZj953oxl0tIjhggTCH3UoIEgSrZs4Mxpk1dv1IdR8spB4wbOn6MC
HrIW9NFDzKwpFUgdBTPBjroXE16AszNhi753LvH00CqwRw024/yRu1Aj2LQJ
abmxtfMN7QR/Uw7gQvfKRwYPAoPcwOTvHrcrBc44izPikDNnuZgNOzRiFC8B
RFZhC9fEAsEYUvQZuwRpRQlfztTV05cvXz11yrz5A9KhT6RVxGJcKBjD1a9K
pLoIAOSP37pu0aB1O5eP36YBRz7+Ubo1RwJkgiyMVCM1CVEJFXSMQDCg4MAB
Ckr2tEDRJ1WnVzBkcFALmByDyE4dmABEFGF4UQUWRdTQQQRJWGIJMV9EcF5B
LCDhzSSFDHIGIoXcIYccZZRRRRhffEFIJKjccgMHCN3Qgiy73IEIH3Ww4cYY
YpQRhhZXePHGi8s8omFCLejhTR133FGHHnLo0QYWY3ShxRdr4EEJOm0siZAJ
Y4zDCBt1jFHEChd1gEEMC5oBxyfoUGFeQihgUQ4mbXyBAQAsZJHFFFNYEIAH
T8zxiTtNSEDjmGOAg0kUCSSwiT3+7LOPPufskQADS5jiThURCHgQCygA0s4k
EITwjD7+sICiTDDNcINPMSFQ4Ak+fixgqkEpzEBLO2YwwIo8g0SQwCTcXOOM
L+aossAc+OSigWsHmRAFN9ws8UAvvfDAwAA8mAPON84oA8sCTMQzzxALLFeQ
CSZ4Yo4wVjyQjC8SLLCAB/n489s1zDxgxTL+SCIBtgSZ0Nw0qFiRQC/KPJDA
v39Iosou4CSTgBWr4AOMCBFIUFAINAhiCyVkMKAFEB6EYIIHCxjAQAxIlBEB
FZ9QAwgHC5hMkAgp/GBGH0lw4EEGKPwkwtMeXGAeCkwQ8gYMEYQQgUEmqLBD
DvSGQFdBGVjgAQw7oMAAA1sfhAJKGy60QAjxMpBRQ/LerfcE3g4FBAA7
}

set iconPrint {
R0lGODlhIAAgAOf/AD1CREFDQUBERkJGSERISkpMSk5QTU1RU05TVVFTUE9T
YVNUUlFWWFNXWlNWZFhWWlRYW1VZXFZaXFhaV1VZZ1NbYldbXXRZH1xaXVhc
X1tcWlpeYF5cYFpfYXNeKFtgYl5gXVtfbGFeYlxhY4JeDl1iZGBiX15jZWNh
ZV9kZmJkYYxhAGBlZ2VjZ2BkcmFmaI5jAJJhAGJnaWVlbmdlaWNoapFlA2Rp
a2lna2VqbGhqZ2ZrbWtpbWdrbmhsb2ltcJlrAGpucWxtdmtvcmxwc21xdKJt
AGtzem5zdZ9wAHVzd3F2eJ92AKV1AJ92DHV6fKp5AHh9f3p/gpKAQLN8AJB+
XXyBhLF/AK6BAKuADqSAIpWCQ36DhYOBhaCDMn+Eh5mFP4CFiJuGOrKFCIOI
irCJC4SJjImHi6iKMoaLjbOMEKCMTLiLEpuPU8GMAImOkIqPkriQF46PmIyR
k8WQAI6SlbyTG5SSlsOYApGWmLuYH5KXmb6aFZiWmr2aIbCXXsybAJWanKOe
ZcGdGqefWpecnsmeDsKeJpidoMObNsWgHsSgKZ+docikIqWhk8OlLKymZ8ak
PaWip8ynJqKnqaWmr8usKqSprNKsIqesr62rr9GxMNmxHNCxONuzK9e2Na6z
tbOxtda3R9q5OdO6R9S5Xdm+PLS5vLy9lNi+RNe+TLe8v8q7o7+9wb7DxeDI
XMXDx8HGyejMSsjFyuLLZu3PPMTJzMrHy+bRVenPW8fMzvDSSM7L0MnO0O7W
StDN0fHUUsrQ0uvWWfPVS+jZW8zR1M3S1dPQ1e7ZXM/U19TS1vLcX9fV2dLY
2tbb3fHjbPniXdzZ3t3a39jd4N/c4drf4vfoaeDe4tvh4/fqef7sXt7j5uXi
5uDl6OLn6vbprujl6uTp7Orn7Pn0h/jxmubs7ujt8Pz0lv73g/vyr+/s8f/3
kvjv1O3y9ffz5PTy9vL08fz6yfL3+f/+svb49P7/v/T5/P3/x//+zf3/2/f9
//z/7//8+v78//3//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBNSoXLlCpQkQBJKJEgH0CZVr2jRElUm4sSEgHLF25fv3rxz60QBsWHjo0E3
r/SxK3WIzyFLsiwBgQHDZUE+39zpgaLGThw1ih45WbnCp0AtiuJ9IzZuXLZn
z4ThUvUpCwmncrx4wkfOlJ0xWBrG6YTLFxYPM1zKmSP2XqQkdvgssrRokZpN
n5pUOSLjo5xQiRbNA7YLGDBfu3xBs6bt0wpWknBM5CLn1581qAQRWrNly5Qp
awRBAnPBUawlJySmecNlCZEeLnLnpkDBgQIFDihYoMFDM8I7d7rwUKJER5Eg
PGjQqFFDhIgNICxMSJCiR46Dc97+vCGDpAgRJTx86NBBo4UKFSA2SJjw4AGP
4jcMnrkTqs96JDqgx0MONbSHgnUiSGBBAWlQwkMNBaWRhiTXFCICczzosEMO
OLx3wggjWCCBBhoE0MU1c6BQ2EBnkLEKPHts4Nx9OOiAAw0HgvDBBhjUZyI3
oOjQAkFcvBGMOn2IkGGGQrbg3gkiaKDdBAsEwEU5tyjxAkFd3IGNOoikkF57
NMigAgomjBAlBmw+EEAf3CTTRWwDRfEGN/BogkILOhR4JprW8QhBAwscUIAm
5VzDxZADdfGGN/C4ogMGNKjgXgkijCAlmxo8UKUGwSRKRgoELXFHOeX8IgkG
EKAApQj+H1yHgQQMeDrAAppwcw02Z6BQ6hniwFNMMqE8gUEDECywwAMMKHsA
ARh0EQo22OxKBgsEKUEGNvA084u3qxTCRRdDrKdEl5K0wkszyjBzDTNdMCqQ
Elwwww82vxjTrjLG/OJvL7fookssrgSsTDPO6BIFqQMF6Eo//cBTzjTMeGtM
L7rcYgsst3DsyrfXcNOKkAQBiAg29PDDjz/82AOPONxgM40zzlDLTTnucOMO
M4zosGW2AZ6hiSvG1BwOPPbI00/O1zSjSyuhUFIHGUXQYFCGPbDQAgo6dNEF
GXP0EUgfe6ThdXotLEkdQhzWSEOa1n2AQawigPAeDTaW6dMSjTmogEMOePuN
Aw4FOmW4RAEBADs=
}

set iconPan {
R0lGODlhIAAgAOf/AAAEMQIFNwAHSQAKUgANXAgKagAOegASZwgNcwAShQUS
fwAcWAAZgwAXkAoB9gIalBcA/wAM/wsegQAjjAAhmAweiQAV8RgP3w8sZhAj
lQMvfwAa/w0okQMvlwcwkQAvrAYvngAj/xgZ/xEziwI4pgAr/wI4tbkAAAAy
/wBAtcQAAAQ26xcq/wA3/7MMAMwAAq4PCAA/4iM+jQA8/9oAAgBG3h5CpNMG
ABtEreYAAN8EABVF0swQABpIuwBJ/+oDABBQt7UaGBNC/8kVArwaDRNM0x5J
xwlM/wBS//8AAPcFAB1PyxRO6u0MANoWA8AgFwVX8QBX/xdXxSBT2BNa1xVU
9i1Wq94cAABf/wBk/yVdxdkkDwxi9QBo/7M2LA5j/b0yLB9b/wBs//8aAKM9
QC9X9TFb280xHbM6NcczJQdt/9gvGQBy/yxj4TdjxgB3/y9j8L4+NAB6/yVp
9ihr8LRERUFpzqpLUQCD/0hrvLxHQgqA/yR0/wCI/59WYI1ahEBu7sVLSUNx
6rJVWTt19QCT/z52/Ud25TR7/8xQSNJORBmK/wCZ/zh+/QCd/0V89b9cWa5j
ZwCg/qpmbSCR/0CC/8NgY2R/x4lzpkSG/wCq/wum/r1oaliH57dsdVqF7wCy
/wC1/rJ0eCqg/8VvcFCQ/9FuaVSU/zKm/wu8/gPC/3WV0ADI/2OY/22Y6sp/
gbaIksOLk2qn/9GJin+k8Hil/72VnY+h57WZq4qq7rWgsJCs5tiWmoay/9Se
ocyjrKiw3pq255227qG26JW8+b2yxaa45dmps5i8/5HC/eSoqtCwvKe/66TB
8rG95fKxqOi1tKnL/7DL+rDO9bbS6bPT8MDP8NLM2OvGx8TT+9/V27Xn9NHb
8/DS0Mve+MXj8N7Y59vc5sjk/Mvj/+7b29Hk/vfZ19Pn9MDy/9vm/8ny/9Dx
//ni4cf3/t7u/9L3/+Py/8v//9n6/dX+//zu7+n2/dv9//Ty9uP8/+H//Or/
/v/5+PH///n///7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
Bgk1etQok8NKEBEZMsRnjkU4XLJ8KVOlCpMpNgiWGpUKlCZJhQr1kcNHzJcq
SHwIaYGihIgQEXLmXNFDoJtTqlSFOsmIEh45bLpgiXJEyIwWJULgzAmhKoQa
C/4RGsWqpCZHhRYdZfNlaVMhNaWKoGr1goZ/griW3ARW7Bs1XcIw9UEzqogN
ERxYhbDBwz8ZqLqCohv2qJosZWI6hRpibU4HmB1cMIwBlarFYPvgacRGTJcv
UJikrrGj9ZQiRpb0kOIGCAiBpyo1QsTnDaJGdOgAEnToUKdOrlzd2iUMGDNr
4+Dx89eMA+5avZAhu4XpT7Zz8Or+9fO3r/y+fv3oEbmy5swZUv2YWf8ny585
U2l00AiCvry/8eP9xw8/kCiRRBJjgNFPdQLJwgsPOSDIwzH97OPPhRheiB44
deQwhoFxxDcBbsfoMMYYTUDyX4YZjufLCUqcOEYSITIz4j+ZgOPEiU0McUYi
7LCIXj/8hKPCjCcMMYYeC0ogUCPlrIHiiUqcYE6G5xHJjz6x0HADLLYMoYc/
w1QgECHrKKKCHjokcQI2AWpIpD505uMFGvbYI8ok/DRj5j9tiPOMMv4kQgQ2
FmKoJT/16IOPPuecM88786jjTzAMCNSGNOiRR895imqJTz744GNPPpO+8446
/BiT6T/+SyCjYaKhDkhnqfi8I4+q6qDDqqsC4dDLhbTKaaujpqpKaTzrtINO
qwYINEItK7Y4J7Kl2oPPPN/coUs821ADrbStkAcNef1AQo6o2eY6TzyD3ACD
NWT4wU8wCAikwSf9sOMCKfvMokIgW96aa6nveEKDEk3AQAMa+uAr0AKV+MOO
EzoMQkQSNPyyZZ6lrhNPMS7IeCAR+BgTrUCA9NPNFWPoEGMSQeCqLKXi4DIJ
DE18+EI4zKz8Dx38dHMDlTo88QquufLaDrhPyEiDL9MoMJAZ7nRDg4xDXJPs
O02r02s7d/yghA48eIFN1QMtMQ42LtxwwxUqeKKtqSDjs2VuPXEkMssy3Iwj
zipWC7RENv/dw0430HRDZD3wlDONM7usckkeVgABBAkTZNDAAwkMZAQt2UjT
TC+0uNKJHVr0YIIJFHyQQAMJKGAAAggUUAABAwggwEAk9AACCB1IcAAByA8Q
AAAAIOT889A/HxAAOw==
}

set iconZoom {
R0lGODlhIAAgAOf/AARGfwBIhQBLggBPhwBSfg5KgwBShABTfxBMfwFUgARV
gQBYgwBYiRJTdBJVcABchwtYhABcjQBejwBfijFSTxBZhgNfkQBikgBlnAlj
jwBmmApilABnnw9jlgBqohRlmABsqSRifQRvmgBurABxogBxqAttpX5RAwBy
sH1VABtrlwB3qAB1s35WBlxeOhNvp4BXAIRVAAB5qjZuYYNaAAB6vVtmM4Vb
AQB8wA14txxzrAB/wwCBvoVhBoheEQCByotgAJBfABR/sAiHvhqBsw2EyJJl
BJNmAACLz41nEJBpBkV5fg6LuxeHyyOFtz9/j5psABWNvgWUyx2JzgqT0Jpy
BR2QwSGNy6FyAB+RwgCc3xWT15x0Fgie4pl4Kap5ACmWx557G517JAuk2i6W
zhKj4Caa3wCs6LKAAa+CAiqe3Bmn3q2CEaWCKzab1Bym47qBAC2g3reEAC+h
3yOo5keb1TOj4baIDrmKAECf3yWs4z+h2jal5Ba167+LBDim5cGMACqv5Tuo
5y6u6zur4x647S+x6D+q6SO670Ks7DS060Kv6DS350Su7k6s5saaAC2+9CvA
8FGv6bKbTlqu4omni1Sx6z297TDD8l2x5TPE9Fez7b6eRUC/72Cw61m170HD
7WK06Vy38jnI99OmDGW08EbG8MimP1+69VTB82i38zzN9knI8teqImy69tSt
I0zL9UHQ+dqxCXS68U3P8m2/9Hi+9XfA8FHT9tu4FHrA94C/8HTF++C8BVXW
+dG1Wty6MVjY+1ba9tu6Olva/YjG+Nm8Wd/CLd/HFt7CN+TFF+DIJ+fEO+HG
ROTEU+fSANzGaOLIVdzSOerWAOjLQZ7R+ObUJ+7ZCqHU++nSRqjW9+rUUPDg
AOrdMKzW/6zZ++/YTOTgRerXaO3VcP3lAPDbXrbb/uveZu7ZgLzc+fPhS/Hb
gsHh//Lhf8nl/fLlkPjohv/nh/fplPv0UdPr/dbu//v2if31l/n4l+v0/O73
//7/rP/+4fj9//7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CA+iQPHChAcMCSMWzJEDxQgQHDhgwNBBYsIdNWrsqCOJkic1PDhYsBDBY8Ei
O5Dc+kavHj133oo14tEhQkuX/4r82FKtn7963mZ58iSpFioqGhgwALqjSdF+
7HapspToT548cQZRyVBBgceQofL1MzfLVahSjhJ10aLmjZ4hD6ZKrHHFnD52
tnS5EuUpkac8ZtScYXQmgwICESvmcUev7axQnizVsaNGS5xAkQwJeRyRxQhP
9LR5cqXqk6dGib5qKcOIViQmCyAnfAGCkjulTAsL4jO3DqFLtDo5OaAbIe86
7EIZFpQoNmK6jGDh0sSE9G4MZLz+ifoj6GueOV3UdAkEStiqVCKYR9RYotYt
QYQE2fmqhpAhU7gIY8oeE0DQHEIYcEBGLaks4uAZgRhSmy/E0MKIELl5pEEH
JBDSSSSRmAIKK9oJIwwskGQxgQEOuMTRCpRcYgortASICy6sXJJFBg0UYAMc
LkVwgQpZ6AGJKZ3A0kkhYwgxAQMBPLGMH3J4FAEDEVigAhFSgCGFFSLweAAA
M4SzTzd4VCkRAw9U4OYEE7RZAQQIALBENtzMww8ycqDhEgMHBEpAoAYMEEIl
8KhzDTf23PPIF0D9E2igDVDgwiTwyHPOMtNYg08yX2ARKUExGNEGNPLIk40y
vYBTThpmUIxKUAs+TLLOO+Mckwsw5eBxhBGyCkSDD1wYg047zrRCSjl3AAFE
sALR6oUx40TzCjDUVEEDDdBG64MXpwwzTDNsxBBDtwPR6kMYYSRxAgwwoDsQ
vC28m0K88g50QgstnJsvUAEBADs=
}

set iconMove {
R0lGODlhIAAgAOeYAD09PT4+Pj8/P0BAQEFBQUJCQkNDQ0dHR0hISEtLS0xM
TE9PT1FRUVJSUlNTU1VVVVZWVlhYWFlZWVxcXF5eXl9fX2BgYGFhYQCmAGJi
YgCpAGRkZGZmZmhoaGtra21tbQO6C3JycnNzc1hu2QDEAHR0dHZ2dnd3d3p6
ent7e3x8fADSA319fX5+fiu3NYCAgIGBgYKCgg7SFGmFxQDgAIWFhVaXpYeH
h4iIiIqKiouLi4yMjEW/Uo+PjzvFVJOTk5SUlJWVlZeXl5mZmZqampycnETZ
TZ2dnWTCc56enp+fn6CgoKGhoaOjo6amplvcYaioqKmpqaurq4jEn66urq+v
r4jIl7CwsLGxsbKysrOzs7e3t5bMqLi4uLKx/7q6ury8vL29vb6+vr+/v5rX
psLCwqDZrsTExMXFxcbGxsfHx8jIyMzMzM3Nzc7OztHR0dLS0tPT07rlxLnn
vtbW1tfX19nZ2dra2tvb287n2d7e3uDg4OHh4eLi4uPj4+Xl5dbv3ubm5ufn
5+Pn+Ojo6Orq6t7y4+Hy5+zs7O3t7e7u7u/v7/Hx8fLy8vT09PX19ff39/j4
+Pn5+fr6+vT++Pz8/Pn+/P7+/v//////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////////////////////yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXPgPggkcOmCoaMGQ4YUiNSj8+4CiRo8eLCoedLAkxsEXRK5IiZJljch/
HZxEeCnSQpOZNCv+gJGz4oQcFXoynKCISQEKJ05M3IEDhVCBL5isuYSpqiM7
bd5cQtTjQggGLyHYqUp2kiNGaoAogQIJ06IkGxhW0MMGjJg6jiL5UVRIShZF
ZMnaKSIhYQqyjdJUAVPoT5M+gSNjgnQEwUEdkgJLesOEySOyeQ5JruoGbEEl
dDBVcoRI0KQmjMgCWiFjDlkuZMiiMXC6DCNGfuosKvMmMCUjGqZUNYPBhSWy
IQp6+HPlD5w3f6hIHjRjhI8nIHj+GKpK9UrBAGX6HEHzaAtkyV5s0CCBJPBq
OgkKSpDkZ0iSJqORJYcVkUVCyB4NGKSCaljo0EWAASrShx0JFgSAE1bdAEQh
ZFEFYR98fHHAQQdocUklmIxRgxOIVFXJi5fEGCMmjtRRxwkEIHQAE4h8xggV
MCxBxySqTWKkkZXoAYcQDyg0AAp1KNIWIljggAMWe0QSWCBg6GCCAgsBkMEY
fyTySCWQxMGElUE4gUUSLaAARAYVBQBBFngIskgjkFTSCB5OvCBCCUKEkYIA
LyWQwxl03KEHH3vY8QYaZaSBxQcF5DRABT9c8cUYY4CxxRVJqMAAoj0NUMAC
HJjAAgoRH0RwwABPEUSAALjSWutTAQEAOw==
}

set iconInfo {
R0lGODlhIAAgAOf/AEJ6Ykl4YUV5Z0N7Y0p5YkR8ZEt6Y0V9ZUx7ZEZ+Zkp/
YUSCYk19ZUt+bEyAYkaDZE9+Z0mBaEqCakSHYU2AbkiFZU6DZVGBaU2GYUmH
Z0eLWVOCa0eKY1CDcUWNYEGQW0iLZEuJaVKGaEyLX1WEbTaaUEOSXUqNZlGK
ZTScWEiQY0STXkyOaD2ZVi+jSkaUYEKXWyuoSEuTZTyeVDiiUUWaXi6qSRK9
MUqYY06ZXkSfXE+XaViSbCO1Q0yaZUidYTepUCuxRzKtTD2nVUqeYjetRkah
XiO6OEKkWk6cZ0ufYzqrUkeiXz2rSz+oVlWaZkuiWVKdYkygZC23PkijYCm4
RkWmXDutU02hZUGqWCK+QwDROlGfahXIMiDCNzuxSU6iZgrPMDiyUTK2TCy6
SFybbj6vVTG6QUKvTgDXLVahZjS3TVCkaEynY1SkYh/FQhvKNUCxV0mrYDC8
Sj+0TCjBRjq4R0muVVWkbmeYf0WyUR7LNlKpXzW9Q0+qZkOzWRXSMijHOyDM
N0mxXlSrYTHDQD+4Vk2vYyvIPTPEQkG5Vzu9UinNL0+wZSXOOS3JPke3XGGm
cWydhEy0YVeuZCLUKlawbFKzZ062Y2yhgkW9Wku6X0i7Z1C4ZB7YQlmzb028
YVG8W1K6ZkrBXk++Y0bFWkvCX0rFU1C/ZFTAXjvSRk3EYUrIXVu8b0bNWlnA
bFm+eEvKXlzAZmW5e0TPYk3LYH2rjEnPXHyviVTKZkXVWV3FcFrGd1DSV03T
X2DLfIO+lpixqX3IimzRiXjNjX3SknPag4TQkaS9tIjUlYLajHzdjoDbk4Ha
oI7UnYvYmXjmh4nblIzco6jPsJ7XqIzioYbnl4rlnZDlpK7Vt5riqZLnpqTi
q6rpsqLtuqXttKPwsMTazMLcx6vtvMvY2bzkxb7mxrXvv9Pc18fnyb7vwbT3
xdji3b7zysnt1d7o4sr30OXn5NH52eLx5Ovu6ujy7PDt8u7w7ej26uf65uX8
7fP9+Pz++//9+/7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXLiQRIcNFyBGYLiwwogWMzKm+IAhAgOKBjHkgPVMHTx457b9ujNjRISJ
IEV8aHVuX75x15gxqzbO2zAnORx8ZCjiSbJ++aCpetTlxpEpdF41a9aoRoWh
CS284NYPHCNAggS9SdSnTxUhaD4Ra5MjwgGFHor5gwaoriNBiBLNmSFHjBka
lmYpcWkAYYVP+azVZeTI0aOxLYK9O2bDihE2YJJYaIAwx7h2ghozfvQo0ZlM
/fpRq0zlR5IkIxgULujAj7xbgVQ5DlT6TZ9X+9z9uWM5Rw4fOyIMMJih2Dgv
XhKRzpuoSI07o8RccQIFi5QoXJ7+iDAQgGCDHNqMHUmUaGyhQjTyoKsnqwgQ
J1R0RMGiJgkIAQKYB4M3qfRRSFl9VJfHPf74I0oTWVihBBZYUMEGFycQEOBA
DcCwzSJz2DFHH2TMMUE8/PjjjBARUkFEFGD40YgaLABIEAM1bEPHGnaAaEcJ
yPTDTzlCmOEEEm74gEUbflyixggBlMdhDsyIoYgYYtAhRhn38OMOGkvQAIUS
PhDhBiWdWMJGBjYOxIAKv+iBZRxiAGFLP/UMEkMLPACDBYyNdLLJJVxE0KZA
DVQwCChy6mGGC7bwgw8XGuRBzzQ1gGFJJ6OIQokMAGxIUARUbLIJJH9kMQQe
9vgTDzrf/ISjQxuNgDLKKpi4wUEAog7kgAp/KCLsJoNYIQ4/QmZjRaCjsJIL
KpbIYECvBFXgxiijlFLKKIq0Yk46wvyxSbO11JLKJUlEAICUBkXAAR+olOKK
K6ycUooppdRCCy2x0DLKJTksEKpCCYzgByir0HJLL7ro4ksvvvjy7CU7VDCA
AOwilAAHYHQSSi78KlxLLqlM4sYICwCYcUIKVCADE43I0oksnFziRxQqVKDc
oQwVkMACE6jwwgtPaADCAgdgvDJIBhSgQAEFvHQAAQODhJAABFAdJbVWd+31
1wYFBAA7
}

set iconLeft {
R0lGODlhIAAgAOf/AAANgQAPhAAWhgEbiBAUiQMbjQAhmAAklAAnmwAqkQAs
mg4okQUvlwA0mhUpogA4qQE9qABCqCcxqwBHrABJryE/nig6tApJtwBUswxP
sCdGqRNRuC1GvzdDvgBeywBhvShOvyxPyEdFxDhNyEFJxkhHvwBo3EpHxjVY
pRtd0zpVrABs2UZMykVPvgB0zAB12kxR0Bxp1w1zywB6zEdW0yBtxEBirTNh
4UpZ1gR91kNd2D1nt1RaywCD2kZh1UVqtUJk7SJ7zgKN2BiD1k9vrkdo6wiI
72BlvRqF0Uxwuz9s9ACT42Zj0Sh+8Dt19BWO5Fh0u1R3twqW3jN7/z9661h5
sl5w00p09nJn0SeN0zeB6kJ5+UR5/0iBygCi5TCH9wCi7DyA/wCj7QCj/wCl
9mF73gCr6xya/0SG/y2S/2172QCs/06I7D2O/wCu+1iH50qM/jWX/wCy/356
2Rmq/wC3/yyh/3OF1mmPvHCOvHaH0nSF4y+l/h+t/4WB1z6e/3WN0HSN14CI
1ICL0VeY/V6b3zaq/VKd+he+/yy1/32Q6QDM/2+b4o6N3ISR5ADS/4Way0Gz
/wDX/yLI/VCx/4Ojxm2l/gDd/wrZ/0e6/16w/zPG/16z/ADk/1e5/5Ck1nKs
/yzQ/muw/wDp/xbf/nix8G62/wDy/1fG/kXP/2+6/me+/x3m/jfZ/3q6/4O3
/pK6yTje/q+m5mzF/0zY/yTt/p250HfE/2TO/4K//23K/o27/FrV/0/e/2fU
/3HR/o7E/nzN/1/d/3vQ/jH4/7q37IbP/3XY/47M/2zg/oDX/3be/YrX/5rP
/77B60f6/pnS/pPV/4Tg/6LR/37k/bHS1Y3e/5bc/57a/6XX/47j/rvS5MPO
6Zfi/8jL9a3X/7Td0aHh/6ng/7bb/rDe/73Z/qzn/7Pl/8fg5M/a9rvk/8Hi
/8Pp173q/sPo/srm/sfz2d/h+9Ts/s7v/9jy6d3x/uHy+dv73eX/5e7/5Oz/
6vD/8vf/9/j9//7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoEAS
JEQYXMiwoIgSLEpARKiwocV/EmEUucJlyxYlRWAgvGhQxAkcW+C0IQTqlUtL
cIqcqEgSY4sradpYylWu3Dt5QMuJ2gKDpkURNNDEIWSq2bdv46K+WyevHrAw
LUhcLHGjTZwzuZo1g5Yt6rh179LWy3VlZkMROgidsZMLGTCxZLORI6cO7bt6
pm6UYFgCx5m5s2YFC2asmTFs2PSSO+dO3Tx1cFhodcgijR07nkApUvRqWLBo
qCGLm1x5niodgwuWoDLGDiVKe4o5a3Qr2DBmzFCLwybunDp316gY/QfXDh9D
mdTI8tevkapfvoFfY0ZcXHF1djj+WCBY4gsdPpm0zPHnz9meWbh+/VL22xqz
cMXPtfMUQsJAuHTQEckfWMTTTzxzUMILLr74csyDylhzjTXhmHMOLik48B8V
cqzRBw/T+eMHG5lEcpspqyymjDTaSBjOOceYYMB/ZyRShxZY9OOPNzxYoUYj
ggwCiB53BBIII6UYI0044fiyQgECWcABHXW4EUIx1B1ozz5c6oPPPfewU40t
kAASjDbdHLMCAQJJAIIbiBjRgo7ssadjl/rks08++eBTSSEs0uKBAAIhAIIc
m6xwhJ39fAJFEj/Y8AMRUeBRCSzgwJNHIdpIs8kFAAjkAAh1bPKCBnX68wMB
BETQAxL+QyARRA017LBDF7wks0wmChD6jwEP1LGIGweowE2d20RRwQRgsOJs
L7SkgsoxvQjTyhO+/nOAAm5MsggZEBRQwSc69kMPHgyAQUortLQSy7vVhrJB
qAMR0EMoizwiSR0fHLBAFOiwh4IMo1xCSizu9tILJmIMkO0/AEzwyCSPLILJ
JYsg8YAAKFSxgBedXBzLwe8uQgG9AwkggBiYPOLyxaN0YgYGEQjRCSk44/wu
KTI8PBAAETzSMiYXk3IJK7WQkvTNN+PcyRICoEzQAADUwIrRBpNy89VM59zJ
KEs43JDKM8TcydmksNIJ11p3cgopLqh8UdQfPFLLKM7mzQo43qeMYkYEAEg9
NgAMCBFKLc+c8szitcTiRQZU10RQ4AdkEAQSWWTRBQYLUO2z5BBHHUAAgctN
UkAAOw==
}

set iconRight {
R0lGODlhIAAgAOf/AAAAKgECOgADQQcCSAAHUAEKWgIJYQANcAoJcgANeQoL
awsIeQMQhQcQjgAUjAAXhwEalBoZVwAflQkdjwAkmxsgah4jZhAomQAvoR4m
iwAzpwwxmSkqdQA4oS4sfxYysi4uiBw3mgNCtDgvkjIzlApEqwBLsjY6sABW
thdOqzA+vUY4pDs/riBKwQBcui9LrwBgyklBs0REu01CvABrxElGxildsk1J
yQBu10VN0hdm3DBb1wxx0VVN1R5rzAB7zUResQB62VBS2Qp40UpX2wh/0U9c
2kdf4gx86SJy40Bl3wCH3ltgtwCG5Udqu0Jm6Dxq8Uls4FNvtgCW4D556guU
5SGN1zOG0xCb30J//juD/UyEzj6F8l5+xF+Auhab9XB2yDWM9ASk/3V11kWH
/y+T/yCb/wCs/3R61kOM/yuf3T+Q/xOo/wCy/yaf/jWY/0GU93WA2XKHxxus
/UeW/32F0VSU/3+F2zGm/xy08Qq8/nqPv3aQxXWM40Oj/gDF/3uRzlSc/4uH
3jG0/wDN/xvC/kau/4qWyADS/3ac8TW8/wDX/yHH/IOd1GGo+Fur/5GX5gDd
/wnZ/0y5/3Cq/VK79ADk/3Wt7ETE/2O3/xXf/kDJ/07C/wDr/4uzwV2//zXU
/5+k5gDx/3i0/4iw613E/h3m/pqq7TfZ/5ywyJ2t2nO9/6Wp7FzL/Z6y123E
/zrf/0vX/7Kq6SXu/3XG/G3K/q6u5YTB/z7m/07e/4+9/6K59oPJ/3LS/2De
/7K56DD3/6bGyZHH/7u374bP/3La/7656WLk/5DO/5vK/oDY/0X5/Hjf/pnS
/pTX/6PS/6vQ/53Z/43k/6XY/67hxq/Y/83M8Zrk/6Dh/6je/sjR8KHm/LDe
/7fc/6rl/rPl/8bd/bvk/8Li/9zY977r/8rm/sTp/97i+83u/tft6uLm9sf+
yNXt/9L23uLs99vx/eLw8NH/0dL/2Nj/y9v/2uP/x+j/5PH5//D7/PT/7PX/
+v37///+7///9v7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwIIgZ
N2QkBFGwoUOCHmIIIXLkyZEjRG4IifGw478KMogoyUKGzJpAdtZogUKkBweP
BVcIMUImjR1KwqCBEwcO2igyO3qQgCnwhhEtdso8SlatWzdx4tixc1ct0BOh
MG88ofPGjK5nzaqJdSquXLl35QLtEBKh4w0lb974WYWsbrNp07h1Cxeu7Dtx
dIzIeNhDiRm5r3jxquvMGd5s3PiGM8euGRkhFhreOPLGzdxXoIkJ05XMWbRo
kPmaM/cu05HBBEcYeYMHT6ZPoHmtgsQq1Clh0bBhy/btWzhy0ajcKCiEiyE3
cz59KlWqVSJj+u5Zk8VKV7Ns2Iz+k/vmJ0eFgSuUsMHDZpIiTpyqJ7Klz5+/
feeMydoFDZu3b+hkskMGA91AxxlzDKIIJphs0sompfQByS/u7GPhPuMM8wt4
5PSiwwUCefAEG23gQUUUXATihiG3VUJJH4KEcs59Fg6TzDbe9JIEBQKdQMUg
c0QBRiqH1IHGGEfGEQeMd9hin33WkLLNNsVwMUGPYehhRg3B1DPPPPHI0046
ZMKjj4VP+vMLKdJgo0wQV/6jghl6DKIEE55Qs4489NiTDz/9pPnkPXK8Io00
vSDxgEAqsKFHnTi0IMILQEjhxR578MGHHF04oc19qmzBDDPSxAJDAgJ9gMej
hfxRSBv+VSwRBA8++MDDD1U4kYI6/pzjRCXHjPqHCAak+oWrhbRKCCKISKKJ
JJFoYgohKbiyjz9dWBFsMb7MQUEAAknQBCGtMkLIuecyKwkqkrggxX18ABEL
LLn4gkoQDAz0gAvoosusuqBEkkcI8PhzSAd5HFMvKnpgUKxABmBQCLqgIMKu
JItEkvEfG7jiSghYWAILLOzykC9BDiwRCSKgLNIytJHAbIkVG4BsiiYkR3KF
BAMUlIDEGsMcSSSWED10J4gQ0okmljQ9hQQHOOQAChkPrQnTTDetdSdNa9JJ
FRBE7RABDZiACNeWZJ211tKKYkkREjz8UAEOIDxLJ6bknTdKLnoDY0oeJjBA
AFEBMDABDYyYAswywDS+zCyoYGECBAkAQNRABjAgwQs0XIGFGlb84AIGDhwA
7uUFEYBAAgy0zsACCPSM+uyzBwQAOw==
}

set iconUp {
R0lGODlhIAAgAOf/AAAALgEANAEDOwADQQAHTwAIVwAIYAEIaAAMbwgJcQwI
eQAQfAMQhQQPjQAWhgAXkBkZWQYblQAhkAAkjAAimSMfZQclnRckgAAroCMk
dSgncBYqmyEmkwc2oi8sfywthhMysQRDqghDsgBKsDg0lh1AsBZKlzk3oAhO
ryBAvys8uTw3pwBWthtQqgBbuRZSuQBbxT1BsEY/sQRkw0dEvARi3kBIyABr
wiVW1DtN0BRpwlBLywRu3wBx2hhozzhdtCVi0QB20VNM1AJ4yU1Q1lBS2Uda
yEpX2zth4UZe4QCD4UVssEhoxEttpTpp8Ct05EVn6Utn4z1x1SGB0UBt7kp2
owCV4ltq10Zz7TV99SiI4mB1t2Zu00N5/SeJ+C6I8Fl5zSGX3ACk9QCj/0CD
/xGd/CqU9Bqc9Wp9wwWm/QCs73Z210aH/zCT/yGb/12Pp1CJ7gCw/z+Q/w2w
7GOG6DeZ/02O/wC3/zuc/gC98jGl/yGu/4iA3kuZ/0qa/XSM2X2LyoOE4G6a
rQ68/y+s/wDG/yiz/nuO53SV4YaO2xfB/QDN/1Oj/4qO416g/gDS/y68/06t
/wDX/0q1/2in/HavribJ/gDd/1yv/pGc2Q3Z/52Z5ADk/yzQ/nKw+XSu/5mi
3RXf/new707C/wDr/6ic40TL/wDy/5qo70fQ/2PC/zjZ/x/n/zjd/o2+vlvM
/ou1/nTC/qWs703Z/ybu/4vJs6yv5Uzc/lrV/52196+v7IjE/UDr/V3c/27U
/5TC/XLW/KW+9jH4/2vb/2zg/nfe/b+98aPJ/5DT/kf6/rrC8ZrS/4Pe/43d
/8bD76PS/6PfvK7YzKzS/6bY/4/k/7LT/pnj/6/Z/6je/tHM8qLi/7De/7bb
/6rl/tLV87Pl/7vk/8Hh/9bd88Xs2rvr/8Xp/8vn/+Lh/+Hl9eTj+9Ts/s7v
/+Dt7t3u9Nj059D/ytzw/eT27+7u/+D/zeT/6ez/1vH+3u//5ff4//P+6/r4
/Pb7/vX/+vz/9f7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwYIUT
NGic0FCwocOCGmQQOZKk4hEiRGI83Ciwwo4jULqQsWNHDpsuTi5+4NhQYhQ2
cvp8+lWNW7Vjn9gkIUKC5UAiSNi06QNLGrdwSNXBg8fNURQiHnzugNKnDaNd
y6Zdm3Y0nDml6xhBEcJSCJY+fSjBQsX22LRt27iBAxcOXjk/SWRsJIKljJtJ
njZlO+dMV65n2MBtC1dunbplWYhAcEgjit9IsQ4Z87fPnz5nso5hw7aNbmNM
SPQW/HDEDSM9qj4FwufPM+dzuoJpG/0NnDpkWXY03CFnjJ5Jquhs6ly7tj58
xnQt6/btWzlybnJMHkgiip40eyD+RVqTzrY/ecb26duXTdczbdXJYcJxgSAR
L3siTYIEZxPnRsro400jm8jTmTO/WNMNNuS8UgMHA2mQBHhx3GHIFd744w0X
tviDziGi/OGMOKAcQ4023ZDjCw8YDLTCE5PEEccgZnDBjz+ZGAHKPuyA4Ysq
iCAiSjPAMKMgME9IMJAMX9xRISRAgOIPO0xIAUg/8YBhSi+9AFPMMEY2Yw0w
PSgpkAplDHLHIKPAgEY+gMCgBRr9uMPEK73MMsswfBZjJC4+ODAQmoNAAoki
epTQgQh3aLFFPu78YMksveCCC5/MMENNKkAkMBAIYygyiCKKWDKIGINcEsQS
94zTgiX+q9ySJ5fE9FKMIiIY8OkZpCpSSCGLdPLIJTM0YU80LXSyyrK3yGpr
L2lEEMBADyixyK+LWLLItpK8UEU9rqDwSCut6BnrLcSs0oMCBC0Aw7XbLvLI
vJKM8EY9grDQyiWrXBJKK81qMsgGBRBUwAaDbDvvI5pI8kgHlcxTxQykhBLK
KuQCrMkMghb0QBCXLKyJJpcoMgE085gwBS0Wt9wKJ1ZQMEBDB2CQ6sgkc7LI
BLVAM8EcFXMSCisWD0KBrg4xgIImoTDNydM3THCBC08LLTQtamCAwEYDNMBC
KKRUzQkpaqjBCStns3IKKVZIsDVHBDwQgiK00MLK3XXfzctC2mqw8EDBPgXA
AAU65BEKLckkngwvq4SBQgMJAOCTwQw80EELQ0wxhQ4jWPDAAdNO3tAABSSw
gAIKJHDAzKK37npAADs=
}

set iconDown {
R0lGODlhIAAgAOf/AAABJQEALwABNAAEOgADQQMGOAMGUAAHVgAGXQMKcQAO
aQAOeg0JegAPgwkQjgAXiAAWjhcYVQAZkwAhkAAglwkdjwgmjyUgZgAtlAAr
oQAsqSUnciIpiBAxqQA5qhMznC4sfxQzoyAuoTMtiAs6txE8nRM9pQBIrQhD
sjI0lDg2ngBLxwBUtABWvStJpkM8rkA/qxpUriFPzQBhyUREuz1GxTVRqABp
wgBqyQxlxSRevB9gyE1JySNkuDtbr09N1EFW1Rtn5E1S0Q90xlZP0VBS2UpX
21RV3ACD0wh67wCA5hKB0zlo8ExqujtxzkRp6wCQ2khq5RyG0k91uhKJ90Vy
7EVx8jN89z92/QCd5gCe4VF31yqK4CyF+ACe9ReS+F96wi+S2E+E1QKm+keC
/wCp8hme72OBwUOG/gik/y2S/x6a/0KN/ziU/2CG4Caf/wCz/wC19wG38Dib
/AC3/1OU3z6Z/3aG0Tal5HSLyG6OySCu/zGm/3eRxw68/2yQ7ADH/1qb/yq0
/2iZ7ADN/wDQ/xzD/wDS/zS7/0qx/4SdzQDX/3qc7yLI/Vys/wDd/wrZ/0m7
/1C684ykzinO/ADj/z3E/1W4/2uu/2+v85Sk0Iuj703B/2uz/kDJ/wDr/xbg
/5il35yh6lq//wDw/3yz/zXY/13E/pqt05et3B3m/lTK/3e6+Hi7/5ut9GrE
/wD6/6Wq7Tne/0nW/2HL/iHs/XfE/6ey4Dzl/nLK/03d/7Cz6orD/6252oLI
/6218UDr/aS5+S32/1/d/5rC/l/i/mzc/3XZ/7m85o/N/2vg/bLB4rTE14HY
/5jN/5jQ/Xjg/5HU/6HM/0f6/rjI3ITg/6rP/47e/5Xb/8DL5rzP6abZ/6zW
/5Xi/sLT4KHh/6jf/8bX3cbW6rHe/6rl/rne/7Pl/8zh4Lvk/8Hi/77s/9nh
98Xq/8vn/8Tt/Nbn7dPt5NXu/9Xy/dzw/d/75OL39tz/6Oz1/ef/4+7/5Pr3
/Pb/5vv/7v//9v7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiw4L8N
NBJeMMiw4UCERI4ImXiEBwyHGP+BoHHkCRYyaEJiYWKkSIqMBnkcscKGTSBM
xKxJI1aKzZMfF1FqLPKk5ZxW3NClW8cu3rx0mKwcUYFyxJErgdR0krZtHLdx
5cqla2e0VZUfGzCCMIJmzZpOyZ45Cxbsajl07YjGw/SEh8MLRdC8+WkrmS9X
yLL94gYO3dui5toYGdHwxxU+by7ZepWLlyh6+X5JA8cZnTl283xVsWvwRRTI
iUa9otwpFD16v6xh4zwOnbp45tQIiWDQiB0+fEYJP5Vr0y188FxFw7YN27dx
5D4nksGh4IsqY/jsiTTq1KlVg5b+4Tsnqlmza86/FVaXKwhTgr4TJRIUiRMn
S6sYZcvnbZP5ZtikZ446zSTRAUEqRDHGGNshEgl+nrjhDT/LDHLMMdVUc003
33RDDjZUHDgQDVTQAQccdPjhhyUsuhEOP70MMowtndiiIYfkdNNGBgTV8AYc
gtDnhyGWeOLJFhPeYgYrd4RyByvQbNhNN3PwOFANewiSIh2WAAJII4044U0/
mnAhRi/+LCMGNNBU0w00XFAwUAEywKEiHYYYwgcchRCSAzP+KKJDE/UEWscw
x7RpjBIPDDTACimuaMgMIYSwgx84oOLPJB/o4Y83PkhSDJvQzDLDAgMJQAIc
hnjpxQf+73hjgwcm9LEpBtT4k0cPw/Q6jDGGeHAAQRqMQQgghYxhgTj++NNH
A1P4o80Z/txjAx64mKKLMqZ4AQEABFHAxbGEHHLCss2ikk2zzd4SAySyyKKL
LqacWtACLRxCSLmHsDBBHuwG3MQSqsQbbxkUEFDQARXQAckhh0ACChIVuLBO
wOuU0Ago8YJyCAoJMNSADo9AYvIjtfT7rz7NghHDJ6qAAgokOjTQkAESQFEJ
JI88UkkltWjxAQY+2GCBHLWAEjMOEATgUAIUaFFLJaD8rAoplWhxwxBywExK
vw0MkFECECDxCSmqpA20MGyTIgwkUITQALgoKeDACVqAUgsz29P0DQwhSHjg
wLA6CTTAAhCEwMIQUkAhxQ0nQMAA4YUTFAACCSzAwAMMLICAAJWHXnhAADs=
}

set iconCenter {
R0lGODlhIAAgAOf/AAAAGAABJQAALQ4BCQABNAADSQADUgUCTRkFCQAHXjIF
AA8QMAMMawAPdz0HAEUIAQQPjS0RHQAUiwAYiAAahFoLADEaMhEjZGcRAAUo
nBAokgAxjwAymxElvkMqIXcYAgY6jXoaAB4rtG4mCQBBqhYxyIcdABY3tipA
Xxg/oZEgABpNbzVHUSNFoVg+ORBXjKErAABZxQBlnZc3AHRCKWdLJa8xACRW
tgBqrSxVqo1DAzFNypE/FkhWZ1FXTSRct0RA/DpK4wdqzSpubDdWz8w1AblC
AKZMAcY8AEZsXAB8xmJkPx5ysZpTCVRL/8BFAHljLSh/gQCE1Nw8AFFU/yx0
yMBPAMdNABWLmQCRs+09AACN3UtswtpNAExyvS6B1ZtnOv9CAFlx4NNdAP9L
AhOhxACi8/9RAORfAwep0m1u/2R/x/1YAGKE0R+k6FiNzv9hANt1AL1+TQa7
/v9sAPpxBHiOzTup5YWB/ye26QDG//91AHyS4+6EAADO/wLS//+BAJGN/42T
9v+HANyMUoOj1f+MAv+HIO6WAADk//+SAG+x5BjZ/46l2UDJ+f+PL/+TG/+a
AFjH8vSlAP6dEv+WO6Gi/ZCw45iw2/+dKf+lAP6lA/mdWP+fPf2tAP+nIv+h
SHnG7zbm/0Ph//+nQP+zAP+sNv+pTJfF5f+rWP6wQP60K/+3Gf+8AP+yUVLq
/Uvw/v/EAP+0YP67Pv++Mv+9TIjZ+/3HJ//OA/+9X/+/WP+8bWzt//7VAMPE
///IR/7EbP/NO4Po/8DN6//Ff3/v///OVf/YKv/Je//RZP/VT63f+Xj5/6Pl
+f7YSP/Ra//XYIz1//zeVJjy///Xef/Uiv3def7fbI/9///hZP/Zmf7eh8Pn
/f/rSNzb/6P4///ilv/oev/gpv/nh/3nqLL9///oo/3tkf3wjL38///up/7x
nufs/v/uuP/wwsz//v/4rv/3tOz0/db///jx/v75zeH//v/61/r3/Or//vL/
//v/7v//6Pn+//7//P///yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXGjwAQwrECNKvGKk4hOKKhjCmNSrI66PsVq1KlXKkyZNkTQNsqFAoQpE
3dCFy5ZNmbNgt2LRKolSkc8rFRJ+iNMN3jh017JBUxbsF6tSJ30aGmQITQiE
FcYcW7du3Dho0H79okVrlSdKkQYNqtMFEB0YCI3ciqdunbW70IzNmrXq06dI
gADV0XFkDx0kDgzC0ASvHTl126g9S6Zrr99Mg/bsMZIFChw6XTAUNNFn3b12
5sBVk6xLVy1SpjBrnjLkzxI2cLRkHJj13D53qFUje6bLlSpSmSDRocOmhh5G
SciwOWNj4IMr1vzdq9dOnLZtyHb+AXOFvJNbNjPS+PkTJQyZM0UeCJzxzN++
e8C9VwsvKxUo82ewUQQWf+jxBxZakEGGFldZ0EQy9dwjoTviiEMMMbv0B8oj
bLAxRRKM+OGIH2VMoSAZcP2zgAc0gCEHIZykIssurqRySiWH0EHGFD4wwsgf
QOZhxIkpCiTABkzI8MIKLLhAAw9GFKFFGDD08McoPv7BiB48hHEGEiMUFAMs
2IjyhyNzmLGFEji8cAETvMDyiiiiYPkHChEgMIBBMWDTjz3lRMPLnIkkokcG
oTAjyR15jIglCAAgJEQ5+sxTzjfF8DLKpm5oIEw0P3RQgggZjEgBQgJM2s88
6XwzTTH+xYwCixQpuMoBEFQAsUMxdzSAahXv+GPPO+W8miksQuQwTzMnUJFr
Fdh8cQBCBghhDz72zEOsq5m24IU/qJyghhpBLFLODwQgVACi/eSjbTqXfiMM
B5jgUwgRgeBBRDPpcKDQBBksoo8+2b5DrC0asOOPHWJYEggX9ixzqkIMQPCG
PQRrO88iG/jjjxeW+CKIHfhcwgBDBUhwAzf4DOzPGyl47IUv3vAxjD9tFMCQ
QA1kUEg/Mt/gDzskEy1PPzcEsLNAKZNQiDze+jNMI/30Q3UjEyxNUAMQaCAB
F+wU0gg7l/xwgwQCaF1QAg00QMEEFFDAwAFpq2333XgLFBAAOw==
}

set iconExit {
R0lGODlhIAAgAOf9AIAAA4IAAIEABIMCAIgAAYkAAoQDAIsAAIwAAIsAA40B
AJMAAJQAAZUAApcAAJgAAJkCAJ8AAKAAAaEAAqIAAJoFAKQBAJsHAKkAAKwA
AaUEAK0AAa4AAqYGALQAALUAALADALcAALgAAKcJALkAALEGALsCAcEAAMIA
AMMAAMQAALwFAsYBAL0IAMcEANAAANEAANIDAMAOANsAAtwAA9MGAN4BAMoL
AeYAAN8FAOkAAdYOAOAJAOoEAvMAAPQAANcRAfYCAOENAOwIAOsIBPYCDP8A
AP8AAOMQAPcGAO0MAOQTAO4QAPgLAuUWAPATAOcYAfoPA/sSAPIYAP0VAP4Y
AP4YB/8bAP8bCP8dAP8dCvkjAP8eFf8gC/8gFv4qCv8sC/MsLf8sFv8uDf8u
F/8wIf83DP43GP85Gf86Gv85If86Iv85KP88I/87Kf88Kv9CD/FBPf9CGv5C
Iv9DG/5CKv9DI/9DK/9FJP9EMf9FLPZGQf9GM/9FOf9LJf5LLP9NH/9MJv5M
M/9OJ/9NNP9NOv9ONf9PNv9PPP9PQv9VKP9WKf9VL/9VNv9VPP9WN/ZXS/9W
Pf9XOP9WQ/9XPv9YRP9YSv9dMv9dP/9dRf9eQP9eRv9fQf9fR/9eUv5jOv9k
O/9kSP5kTv9lSP9mSf9nVf9rRP9oXP9tTP9tUv5wRf9yTf5yU/9zTv90W/91
Vvx0a/91Yf93aP16Yv96Xf99a/6AX/+BYP6BbP+Iaf+Jav+ObP+QaP+Qbv+R
df+Vdv+XeP+Yf/+dgf+fg/6ejf+fjv+jhP6kiv6lkf6ohv6pjf+rj/+ooP+s
lv+tl/+vnv+ymf++tP7Br/nDwvrEw//Fs//Jr/3RwP/Twv/Uw//Uyf/VxP/W
y/3ZzP/bz//d0P/e0f3h0v/f2Pzi2f/i0//j1P3j2v/k2//l3P/n3f/p2P/o
3vzq3v7r3/zs5//s4f/t4v/u4/7u6fzx5P/v6v/w6/7y5f/y7P727v/37//4
8f/5+P7//P///////////yH5BAEKAP8ALAAAAAAgACAAAAj+AP8JHEiwoMGD
CBMqXMiwocOCJThk0KBhgsWHBFOsEMERBIYMICZCGNmgZEMWGlO4ENIEDJqX
ZrpkmbJDhoYGChoodKEChxlTvYwpazb0GDBdtlA1UqNlxs2ELF6g4WVtm7Zq
x3KxahTIjh0yYL2qETJB58EVWZKhOwfNliRFgei8BAMmSxYtWsio+SLCrEEW
dLJ1u7VoERwzgOwEMgMlS13HaNqouQHhIIkbrbT9AgTKyYpJdOiQ4VBET5Uq
Rrqo+WMHx4SDIoDskqbqkhMZ1PgVyzIiTj5yk4ZM6YKGD58pGhQYFEEk2DNF
ZkTk5scP1p587tixKzVcDSFCVTD+KC8ooouwX3O+eOKXjx69ffnklTs3Lp6o
H2oaEWoyYvzADB6YMUwqZtBhRzT80OPOPe+sc84354TDRxJlRNKIFuIVBIIH
ZAgjCRl0ecFMPvG4g85a6GyThxBZqCEJJbwlUNAIK5ABDB0ubTECJPmgY+I5
57yTyA5MdGEHJZFo0V9BGtS4CxlddDFCGPL8SN835CAzBQ5a8IFJJEZgQACT
HmjBikxCEGHPO0CKI8433pDDzjNedEHIJo38kFxBF3CQBSZZbCFEIWy+SY46
5HjTTTfsSKMGJagQAgUEBhRkERGSzPQDEbXEA6E2slxzDjnhJLrJJqnosUMD
lRYEQQ7pdnTxwxRTEFGMPd/0MUMepXoTTi2bvDKKGio0AIBBDdSIRhdaZPFD
F8WIgoMPPpwyjjizUEKLKzCCMKZBEIzggxpdgAWlEUaApYYas8ySyiu0bOLG
DBB8W1ADEKigmhp66MGHIP82gsmpo7DCyih6MJFcAAhBoIELUpThBiGNFBLJ
wJuMojElenThRAYNCKBQAxN8UMMPeanhxr98uEGGF1MAQYIEChDAsEIKQHDB
CCK4AAQQOeSwQw0reGCBAwoYQMCxDCWQgAI5TXDBBBRccAFOCRBgANMYKeB0
0kkbIIDYGJVt9tkEBQQAOw==
}

image create photo icon.save.small
icon.save.small put $iconSave

image create photo icon.print.small
icon.print.small put $iconPrint

image create photo icon.pan.small
icon.pan.small put $iconPan

image create photo icon.zoom.small
icon.zoom.small put $iconZoom

image create photo icon.move.small
icon.move.small put $iconMove

image create photo icon.info.small
icon.info.small put $iconInfo

image create photo icon.left.small
icon.left.small put $iconLeft

image create photo icon.right.small
icon.right.small put $iconRight

image create photo icon.up.small
icon.up.small put $iconUp

image create photo icon.down.small
icon.down.small put $iconDown

image create photo icon.center.small
icon.center.small put $iconCenter

image create photo icon.exit.small
icon.exit.small put $iconExit

# ############################################
# INITIALIZATION
# ############################################

#STATE: pan, zoom, move, info
set STATE ""

#STATUSBAR: text in statusbar
set STATUSBAR ""

#XWIN,YWIN: initial size of canvas
set XWIN 1000
set YWIN 420

#ZOOM: initial 16
set ZOOM 16
set DOUBLELINE [expr $ZOOM/4.0]
set ARROWSIZE [expr round(2*sqrt($ZOOM))]
set INVSIZE [expr $ZOOM/8.0 + 1]

#XMAX,YMAX: $XMAX*$ZOOM and $YMAX*$ZOOM are max coordinates
#these are used to determine the size of box and grid
#min coordinates are always 0,0
#XMAX and YMAX are set in boxit, call boxit after every change
set XMAX [expr round($XWIN /$ZOOM)]
set YMAX [expr round($YWIN /$ZOOM)]

#SCROLL: x and y scroll increment
set SCROLL 8

#GRID
set GRIDON 0
set GRIDRESOLUTION 400
set GRID [expr $ZOOM * 8]

#ARROW
set ARROWSON 0
set ARROWSIZE 7

#BDD
set BDDNAME ""
array unset BDD
set BDDNODES ""
set TERMINALS ""

#these are used internal
set SELECTED ""
set TERMSELECTED ""
set LISTF ""
set LISTT ""
set DEPTH ""
set TERMDEPTH 0

# ############################################
# GUI
# ############################################

set COLORMENU gray80
set COLORTEXT "#90F0FF"
set COLORHIGH "#ffAA88"
set COLORBG "#FFE8C0"

set COLORGRID "#90A8CF"
set COLORNODE "#F8F8F0"
set COLORLEFT "#006828"
set COLORRIGHT "#002868"

wm title . $TITLE
wm iconname . BDDview

set mainframe [MainFrame .mainframe -bg $COLORMENU -textvariable STATUSBAR]
$mainframe.status.label configure -fg black
$mainframe showstatusbar status
pack $mainframe -fill both -expand yes

# CREATE TOOLBAR

set toolbarsize "small"
set toolbar [$mainframe addtoolbar]
$toolbar configure -bg $COLORMENU

set bb0 [ButtonBox $toolbar.bb0 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb0 add -image icon.save.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Save As" -command {bddview_saveas $mainwin}
$bb0 add -image icon.print.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Print to File" -command {bddview_print $mainwin}

pack $bb0 -side left -anchor w

set sep1 [Separator $toolbar.sep1 -bg $COLORMENU -orient vertical]
pack $sep1 -side left -fill y -padx 4 -anchor w

set bb1 [ButtonBox $toolbar.bb1 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb1 add -image icon.pan.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -background $COLORMENU -helptext "Pan" -command {state pan $mainwin $bb1}
$bb1 add -image icon.zoom.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -background $COLORMENU -helptext "Zoom" -command {state zoom $mainwin $bb1}
$bb1 add -image icon.move.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -background $COLORMENU -helptext "Move" -command {state move $mainwin $bb1}
$bb1 add -image icon.info.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 2 \
    -background $COLORMENU -helptext "Info" -command {state info $mainwin $bb1}

pack $bb1 -side left -anchor w

set sep2 [Separator $toolbar.sep2 -bg $COLORMENU -orient vertical]
pack $sep2 -side left -fill y -padx 4 -anchor w

set bb2 [ButtonBox $toolbar.bb2 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb2 add -image icon.left.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Left" -command {compass "w" $mainwin}
$bb2 add -image icon.right.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Right" -command {compass "e" $mainwin}
$bb2 add -image icon.up.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Up" -command {compass "n" $mainwin}
$bb2 add -image icon.down.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Down" -command {compass "s" $mainwin}
$bb2 add -image icon.center.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Center" -command {compass "c" $mainwin}

pack $bb2 -side left -anchor w

set sep3 [Separator $toolbar.sep3 -bg $COLORMENU -orient vertical]
pack $sep3 -side left -fill y -padx 4 -anchor w

set bb3 [ButtonBox $toolbar.bb3 -bg $COLORMENU -homogeneous 0 -spacing 8]
$bb3 add -text "Arrows On/Off" -fg black -bg $COLORTEXT \
    -highlightbackground snow -highlightthickness 1 \
    -padx 4 -pady 2 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Arrows On/Off" -command {arrowsOnOff $mainwin $bb3}
$bb3 add -text "Grid On/Off" -fg black -bg $COLORTEXT \
    -highlightbackground snow -highlightthickness 1 \
    -padx 4 -pady 2 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Grid On/Off" -command {gridOnOff $mainwin $bb3}
$bb3 add -text "Grid +" -fg black -bg $COLORTEXT \
    -highlightbackground snow -highlightthickness 1 \
    -padx 4 -pady 2 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Grid +" -command {gridPlus $mainwin}
$bb3 add -text "Grid -" -fg black -bg $COLORTEXT \
    -highlightbackground snow -highlightthickness 1 \
    -padx 4 -pady 2 -takefocus 0 -relief link -borderwidth 2 \
    -helptext "Grid -" -command {gridMinus $mainwin}

pack $bb3 -side left -anchor w

set bb4 [ButtonBox $toolbar.bb4 -bg $COLORMENU -homogeneous 0 -spacing 2]
$bb4 add -image icon.exit.$toolbarsize \
    -highlightthickness 0 -takefocus 0 -relief link -borderwidth 1 \
    -helptext "Exit" -command {exit}

pack $bb4 -side right -anchor w

# CREATE SCROLLED CANVAS

set sc [ScrolledWindow $mainframe.scrolledcanvas -bg $COLORMENU -scrollbar both -auto none]
pack $sc -in [$mainframe getframe] -fill both -expand yes

set mainwin [$sc getframe].canvas
canvas $mainwin -bg $COLORBG -xscrollincrement $SCROLL -yscrollincrement $SCROLL -width $XWIN -height $YWIN
$sc setwidget $mainwin

# ENABLE MOUSE WHEEL
bind $mainwin <4> "%W yview scroll -5 units"
bind $mainwin <5> "%W yview scroll  5 units"
bind $mainwin <MouseWheel> {%W yview scroll [expr {-%D/120}] units}

set x [expr {([winfo screenwidth .] - [$mainwin cget -width]) / 2 - 8}]
set y 16
wm geometry . +$x+$y

# ############################################
# MOUSE ACTIONS
# ############################################

bind $mainwin <ButtonPress-1> {
  global X
  global Y
  global STATE
  global SELECTED
  global LISTF

  if {$STATE == "pan"} {
  }

  if {$STATE == "zoom"} {
    zoomIn $mainwin [$mainwin canvasx %x] [$mainwin canvasy %y]
  }

  if {$STATE == "move"} {
    set item [$mainwin gettags current]
    if {$item != ""} {
      set item [lindex $item 0]
      if {$item != "line"} {
        selectgraph $mainwin $item
        hidearrows $mainwin
        $mainwin itemconfigure "current&&outline" -activeoutline red
        foreach item $SELECTED {$mainwin itemconfigure $item -state disabled}
        foreach item $LISTF {
          $mainwin itemconfigure $item -state disabled
        }
      }
    }
  }

  if {$STATE == "info"} {
    set item [$mainwin gettags current]
    if {$item != ""} {
      set item [lindex $item 0]
      if {$item != "line"} {
        $mainwin itemconfigure "$item&&outline" -activeoutline red
        $mainwin itemconfigure "$item&&tekst" -state disabled
        $mainwin itemconfigure "$item&&outline" -state disabled
        drawinfo $mainwin $item [$mainwin canvasx %x] [$mainwin canvasy %y] 1
        foreach item $SELECTED {$mainwin itemconfigure $item -state disabled}
        foreach item $LISTF {
          $mainwin itemconfigure $item -state disabled
          $mainwin itemconfigure "i$item" -state disabled
          $mainwin itemconfigure "p$item" -state disabled
        }
      }
    }
  }

  set X %x
  set Y %y
}

bind $mainwin <ButtonRelease-1> {
  global STATE
  global SELECTED
  global LISTF
  global LISTT

  if {$STATE == "pan"} {
  }

  if {$STATE == "zoom"} {
  }

  if {$STATE == "move"} {
    boxit $mainwin
    fixedges $mainwin
    $mainwin itemconfigure "outline" -activeoutline black
    foreach item $SELECTED {$mainwin itemconfigure $item -state normal}
    foreach item $LISTF {
      $mainwin itemconfigure $item -state normal
    }
  }

  if {$STATE == "info"} {
    removeinfo $mainwin
    $mainwin itemconfigure "outline" -activeoutline black
    $mainwin itemconfigure "tekst" -state normal
    $mainwin itemconfigure "outline" -state normal
    foreach item $SELECTED {$mainwin itemconfigure $item -state normal}
    foreach item $LISTF {
      $mainwin itemconfigure $item -state normal
      $mainwin itemconfigure "i$item" -state normal
      $mainwin itemconfigure "p$item" -state normal
    }
  }

  set SELECTED ""
  set TERMSELECTED ""
  set LISTF ""
  set LISTT ""
}

bind $mainwin <B1-Motion> {
  global X
  global Y
  global STATE

  set xd [expr %x-$X]
  set yd [expr %y-$Y]

  if {$STATE == "pan"} {
    pan $mainwin $xd $yd
  }

  if {$STATE == "zoom"} {
  }

  if {$STATE == "move"} {
    if {[llength $SELECTED] > 0} {motion $mainwin $xd $yd}
  }

  if {$STATE == "info"} {
    $mainwin move info $xd $yd
  }

  set X %x
  set Y %y
}

bind $mainwin <ButtonPress-3> {
  global X
  global Y
  global STATE
  global SELECTED
  global LISTF

  if {$STATE == "pan"} {
    centerpoint $mainwin [$mainwin canvasx %x] [$mainwin canvasy %y]
  }

  if {$STATE == "zoom"} {
    zoomOut $mainwin [$mainwin canvasx %x] [$mainwin canvasy %y]
  }

  if {$STATE == "move"} {
    set item [$mainwin gettags current]
    if {$item != ""} {
      set item [lindex $item 0]
      if {$item != "line"} {
        selectnode $mainwin $item
        hidearrows $mainwin
        $mainwin itemconfigure "current&&outline" -activeoutline red
        foreach item $SELECTED {$mainwin itemconfigure $item -state disabled}
        foreach item $LISTF {
          $mainwin itemconfigure $item -state disabled
        }
      }
    }
  }

  if {$STATE == "info"} {
    set item [$mainwin gettags current]
    if {$item != ""} {
      set item [lindex $item 0]
      if {$item != "line"} {
        $mainwin itemconfigure "$item&&outline" -activeoutline red
        $mainwin itemconfigure "$item&&tekst" -state disabled
        $mainwin itemconfigure "$item&&outline" -state disabled
        drawinfo $mainwin $item [$mainwin canvasx %x] [$mainwin canvasy %y] 2
        foreach item $SELECTED {$mainwin itemconfigure $item -state disabled}
        foreach item $LISTF {
          $mainwin itemconfigure $item -state disabled
          $mainwin itemconfigure "i$item" -state disabled
          $mainwin itemconfigure "p$item" -state disabled
        }
        drawborder $mainwin
      }
    }
  }

  set X %x
  set Y %y
}

bind $mainwin <ButtonRelease-3> {
  global STATE
  global SELECTED
  global LISTF
  global LISTT

  if {$STATE == "pan"} {
  }

  if {$STATE == "zoom"} {
  }

  if {$STATE == "move"} {
    boxit $mainwin
    fixedges $mainwin
    $mainwin itemconfigure "outline" -activeoutline black
    foreach item $SELECTED {$mainwin itemconfigure $item -state normal}
    foreach item $LISTF {
      $mainwin itemconfigure $item -state normal
      $mainwin itemconfigure "i$item" -state normal
    }
  }

  if {$STATE == "info"} {
    removeinfo $mainwin
    removeborder $mainwin
    $mainwin itemconfigure "outline" -activeoutline black
    $mainwin itemconfigure "tekst" -state normal
    $mainwin itemconfigure "outline" -state normal
    foreach item $SELECTED {$mainwin itemconfigure $item -state normal}
    foreach item $LISTF {
      $mainwin itemconfigure $item -state normal
      $mainwin itemconfigure "i$item" -state normal
      $mainwin itemconfigure "p$item" -state normal
    }
  }

  set SELECTED ""
  set TERMSELECTED ""
  set LISTF ""
  set LISTT ""
}

bind $mainwin <B3-Motion> {
  global X
  global Y

  set xd [expr %x-$X]
  set yd [expr %y-$Y]

  if {$STATE == "pan"} {
  }

  if {$STATE == "zoom"} {
  }

  if {$STATE == "move"} {
    if {[llength $SELECTED] > 0} {motion $mainwin $xd $yd}
  }

  if {$STATE == "info"} {
    $mainwin move info $xd $yd
  }

  set X %x
  set Y %y
}

# ############################################
# CANVAS PROCEDURES
# ############################################

proc boxit {win} {
  global SELECTED
  global LISTF
  global LISTT
  global ZOOM
  global GRIDON
  global GRID
  global GRIDRESOLUTION
  global XMAX
  global YMAX

  removebox $win
  if {$GRIDON == 1} {removegrid $win}

  # REPAIR NEGATIVE COORDINATES (min = 2)

  set koord [$win bbox all]
  set x1 [lindex $koord 0]
  set y1 [lindex $koord 1]

  set nok 0
  if {$x1>2} {set x1 2} else {set nok [expr $nok + 1]}
  if {$y1>2} {set y1 2} else {set nok [expr $nok + 1]}

  if {$nok != 0} {motion $win [expr - $x1 + 2] [expr - $y1 + 2]}

  if {[llength $SELECTED] > 0} {

    # IF SOMETHING IS SELECTED THEN
    # SNAP TO GRID THE FIRST NODE IN THE SELECTION

    if {$GRIDON == 1} {

      set igrid [expr round($GRIDRESOLUTION*$ZOOM/$GRID)]
      set igrid2 [expr round($igrid/2)]

      set item [lindex $SELECTED 0]
      set koord [$win coords $item]
      set x1 [lindex $koord 0]
      set y1 [lindex $koord 1]
      set x2 [lindex $koord 2]
      set y2 [lindex $koord 3]

      set xc [expr round(($x1+$x2)/2)]
      set yc [expr round(($y1+$y2)/2)]

      if {$xc < $igrid} {
        set xg [expr $xc - $igrid]
      } else {
        set xg [expr $xc % $igrid]
        if {$xg > $igrid2} {set xg [expr $xg-$igrid]}
      }

      if {$yc < $igrid} {
        set yg [expr $yc - $igrid]
      } else {
        set yg [expr $yc % $igrid]
        if {$yg > $igrid2} {set yg [expr $yg-$igrid]}
      }

      # MOVE SELECTED NODES
      motion $win [expr -$xg] [expr -$yg]
    }

  }

  # set XMAX and YMAX

  set koord [$win bbox all]
  if {$koord == ""} {set koord [list 0 0 0 0]}
  set x2 [lindex $koord 2]
  set y2 [lindex $koord 3]
  set XMAX [expr round(1+(1.0*$x2)/$ZOOM)]
  set YMAX [expr round(1+(1.0*$y2)/$ZOOM)]

  $win configure -scrollregion [list 0 0 [expr 1+$XMAX*$ZOOM] [expr 1+$YMAX*$ZOOM]]

  drawbox $win
  if {$GRIDON == 1} {drawgrid $win}
}

proc centerpoint {win x y} {
  global ZOOM
  global XMAX
  global YMAX
  global SCROLL

  $win configure -xscrollincrement 1 -yscrollincrement 1

  if {$x > [expr $XMAX*$ZOOM/2.0]} {
    set x [expr $x - $XMAX*$ZOOM]
    $win xview moveto 1
    set offset [expr round($x + [$win canvasx 0])]
    if {$offset > 0} {set offset 0}
    $win xview scroll [expr round($x)] units
    set koord [$win xview]
    set x [expr ([lindex $koord 0]+[lindex $koord 1])/2.0]
    if {$x > 1} {set x 1}
    $win xview moveto $x
    $win xview scroll $offset units
  } else {
    $win xview moveto 0
    $win xview scroll [expr round($x)] units
    set offset [expr round($x - [$win canvasx 0])]
    set koord [$win xview]
    set x [expr (3*[lindex $koord 0]-[lindex $koord 1])/2.0]
    if {$x < 0} {set x 0}
    $win xview moveto $x
    $win xview scroll $offset units
  }

  if {$y > [expr $YMAX*$ZOOM/2.0]} {
    set y [expr $y - $YMAX*$ZOOM]
    $win yview moveto 1
    set offset [expr round($y + [$win canvasy 0])]
    if {$offset > 0} {set offset 0}
    $win yview scroll [expr round($y)] units
    set koord [$win yview]
    set y [expr ([lindex $koord 0]+[lindex $koord 1])/2.0]
    if {$y > 1} {set y 1}
    $win yview moveto $y
    $win yview scroll $offset units
  } else {
    $win yview moveto 0
    $win yview scroll [expr round($y)] units
    set offset [expr round($y - [$win canvasy 0])]
    set koord [$win yview]
    set y [expr (3*[lindex $koord 0]-[lindex $koord 1])/2.0]
    if {$y < 0} {set y 0}
    $win yview moveto $y
    $win yview scroll $offset units
  }

  $win configure -xscrollincrement $SCROLL -yscrollincrement $SCROLL
}

proc zoomIn {win px py} {
    global ZOOM
    global DOUBLELINE
    global ARROWSIZE
    global INVSIZE
    global GRIDON
    global GRID
    global LISTF
    global LISTT

    if {$ZOOM < 64} {

      removebox $win
      if {$GRIDON == 1} {removegrid $win}

      set LISTF [$win find withtag left]
      set LISTT [$win find withtag right]
      hidearrows $win

      set ZOOM [expr $ZOOM * 2]
      set DOUBLELINE [expr $ZOOM/4.0]
      set ARROWSIZE [expr round(2*sqrt($ZOOM))]
      set INVSIZE [expr $ZOOM/8.0 + 1]

      if {$GRIDON == 1} {
        gridPlus $win
      } else {
         if {$GRID<[expr $ZOOM*32]} {
           set GRID [expr $GRID*2]
         }
      }

      $win scale all 0 0 2 2
      $win itemconfigure tekst -font [list Courier $ZOOM]
      $win itemconfigure "left||right" \
        -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \

      boxit $win
      fixedges $win

      set LISTF ""
      set LISTT ""

      centerpoint $win [expr 2 * $px] [expr 2 * $py]

    }
}

proc zoomOut {win px py} {
    global ZOOM
    global DOUBLELINE
    global ARROWSIZE
    global INVSIZE
    global GRIDON
    global GRID
    global LISTF
    global LISTT

    if {$ZOOM > 1} {

      removebox $win
      if {$GRIDON == 1} {removegrid $win}

      set LISTF [$win find withtag left]
      set LISTT [$win find withtag right]
      hidearrows $win

      set ZOOM [expr $ZOOM / 2]
      set DOUBLELINE [expr $ZOOM/4.0]
      set ARROWSIZE [expr round(2*sqrt($ZOOM))]
      set INVSIZE [expr $ZOOM/8.0 + 1]

      if {$GRIDON == 1} {
        gridMinus $win
      } else {
         if {$GRID>[expr $ZOOM*4]} {
           set GRID [expr $GRID/2]
         }
      }

      $win scale all 0 0 0.5 0.5
      $win itemconfigure tekst -font [list Courier $ZOOM]
      $win itemconfigure "left||right" \
        -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \

      boxit $win
      fixedges $win

      set LISTF ""
      set LISTT ""

      centerpoint $win [expr $px/2.0] [expr $py/2.0]

    }
}

proc compass {direction win} {
  global ZOOM
  global SCROLL
  global XMAX
  global YMAX
  global XWIN
  global YWIN

  if {($direction == "w")} {
    $win xview moveto 0.0
  }

  if {($direction == "e")} {
    $win xview moveto 1.0
  }

  if {($direction == "n")} {
    $win yview moveto 0
  }

  if {($direction == "s")} {
    $win yview moveto 1
  }

  if {($direction == "c")} {
    set koord [$win bbox all]
    if {$koord == ""} {set koord [list 0 0 0 0]}
    set x [expr round(([lindex $koord 0]+[lindex $koord 2])/2.0)]
    set y [expr round(([lindex $koord 1]+[lindex $koord 3])/2.0)]
    centerpoint $win $x $y
  }

}

proc gridOnOff {win bb} {
  global GRIDON
  global COLORTEXT
  global COLORHIGH

  if {$GRIDON == 1} {
    $bb itemconfigure 1 -relief link -bg $COLORTEXT
    removegrid $win
    set GRIDON 0
  } else {
    $bb itemconfigure 1 -relief sunken -bg $COLORHIGH
    drawgrid $win
    set GRIDON 1
  }
}

proc gridPlus {win} {
  global ZOOM
  global GRID
  global GRIDON

  if {$GRIDON == 1} {
    if {$GRID<[expr $ZOOM*32]} {
      set GRID [expr $GRID*2]
      removegrid $win
      drawgrid $win
    }
  }
}

proc gridMinus {win} {
  global ZOOM
  global GRID
  global GRIDON

  if {$GRIDON == 1} {
    if {$GRID>[expr $ZOOM*4]} {
      set GRID [expr $GRID/2]
      removegrid $win
      drawgrid $win
    }
  }
}

proc drawbox {win} {
  global ZOOM
  global XMAX
  global YMAX

  $win create rectangle \
       0 0 [expr $XMAX*$ZOOM] [expr $YMAX*$ZOOM] \
       -outline blue -state normal \
       -tags [list line box]
}

proc removebox {win} {
  $win delete box
}

proc drawgrid {win} {
  global COLORGRID
  global GRIDRESOLUTION
  global ZOOM
  global XMAX
  global YMAX
  global GRID

  set igrid [expr round($GRIDRESOLUTION*$ZOOM/$GRID)]

  for {set i $igrid} {$i < [expr $YMAX*$ZOOM]} {set i [expr $i + $igrid]} {
    $win create line 0 $i [expr $XMAX*$ZOOM] $i \
      -fill $COLORGRID -dash "2 4" -state normal \
      -tags [list line grid]
  }

  for {set i $igrid} {$i < [expr $XMAX*$ZOOM]} {set i [expr $i + $igrid]} {
    $win create line $i 0 $i [expr $YMAX*$ZOOM] \
      -fill $COLORGRID -dash "2 4" -state normal \
      -tags [list line grid]
  }

  $win lower grid
}

proc removegrid {win} {
  $win delete grid
}

proc drawinfo {win item x y mode} {
  global SELECTED
  global TERMSELECTED
  global DEPTH
  global TERMDEPTH
  global BDD
  global LISTF
  global LISTT

  # mode 0: show info only
  # mode 1: show info + depth
  # mode 2: show info + coordinates

  set koord [$win bbox $item]
  set x [lindex $koord 2]
  set y [lindex $koord 3]

  set TERMDEPTH 0
  if { [lsearch -exact [$win gettags $item] "label"] != -1 } {
    foreach node [lindex [array get BDD $item] 1] {
      if {$node != "i"} {set variable [$win itemcget "$node && tekst" -text]}
    }
    addSelectedDepth $item 0
    set nodes -1
  } else {
    set variable [$win itemcget "$item && tekst" -text]
    addSelectedDepth $item 1
    set nodes 0
  }

  foreach item $TERMSELECTED {
    lappend SELECTED $item
    lappend DEPTH $item
    lappend DEPTH $TERMDEPTH
  }

  foreach item $SELECTED {
    set LISTF [concat $LISTF [$win find withtag [string replace $item 0 0 "f"]]]
    set LISTT [concat $LISTT [$win find withtag [string replace $item 0 0 "t"]]]
  }

  #puts "SELECTED"
  #puts $SELECTED
  #puts "TERMSELECTED"
  #puts $TERMSELECTED
  #puts "DEPTH"
  #puts $DEPTH

  set nodes [expr $nodes + [llength $SELECTED] - [llength $TERMSELECTED] + 1]

  set info1 "Root variable: "
  set info2 "BDD nodes: "
  set info3 "BDD terminal depth: "
  set info "$info1$variable\n$info2$nodes\n$info3$TERMDEPTH"

  $win create text $x $y \
     -text $info -font [list Helvetica 12 bold] -anchor w \
     -fill darkblue -state normal \
     -tags [list info]

  foreach item $SELECTED {
    set koord [$win bbox $item]
    set x1 [lindex $koord 0]
    set y1 [lindex $koord 1]
    set x2 [lindex $koord 2]
    set y2 [lindex $koord 3]

    if { $mode == 1 } {
      set infodepth [lindex $DEPTH [expr [lsearch -exact $DEPTH $item] + 1]]
      if {$infodepth != 0} {

        set infonum [string replace $item 0 0 "#"]
        $win create text $x2 [expr $y1-4] \
          -text $infonum -font [list Courier 10] -anchor e \
          -fill black -state normal \
          -tags [list infonum]

        $win create text [expr round($x1+0.95*($x2-$x1))] [expr round($y1+1.15*($y2-$y1))] \
          -text $infodepth -font [list Helvetica 10 bold] -anchor e \
          -fill black -state normal \
          -tags [list infonum]

      }
    }

    if { $mode == 2 } {

      set infonum [string replace $item 0 0 "#"]
      $win create text $x2 [expr $y1-4] \
        -text $infonum -font [list Courier 10] -anchor e \
        -fill black -state normal \
        -tags [list infonum]

      set infocoords "([expr round($x1+$x2)/2],[expr round($y1+$y2)/2])"
      $win create text $x1 [expr round($y1+1.15*($y2-$y1))] \
         -text $infocoords -font [list Helvetica 10] -anchor w \
         -fill black -state normal \
         -tags [list infonum]
    }

  }

  set DEPTH ""
}

proc removeinfo {win} {
  $win delete info
  $win delete infonum
}

proc drawborder {win} {
  global SELECTED

  foreach item $SELECTED {
    $win addtag selection withtag $item
  }

  set koord [$win bbox selection]
  set x1 [lindex $koord 0]
  set y1 [lindex $koord 1]
  set x2 [lindex $koord 2]
  set y2 [lindex $koord 3]

  $win create rectangle $x1 $y1 $x2 $y2 \
       -outline red -dash "2 4" -state normal \
       -tags [list line border]

  $win create rectangle \
       [expr $x1-2] [expr $y1-2] [expr $x1+2] [expr $y1+2]\
       -outline red -fill red -state normal \
       -tags [list line border]

  $win create rectangle \
       [expr $x2-2] [expr $y1-2] [expr $x2+2] [expr $y1+2]\
       -outline red -fill red -state normal \
       -tags [list line border]

  $win create rectangle \
       [expr $x1-2] [expr $y2-2] [expr $x1+2] [expr $y2+2]\
       -outline red -fill red -state normal \
       -tags [list line border]

  $win create rectangle \
       [expr $x2-2] [expr $y2-2] [expr $x2+2] [expr $y2+2]\
       -outline red -fill red -state normal \
       -tags [list line border]

  $win dtag selection
}

proc removeborder {win} {
  $win delete border
}

# ############################################
# DRAWING PROCEDURES
# ############################################

proc addTerminal {win num label x y} {
    global ZOOM
    global COLORNODE
    global BDDNODES
    global TERMINALS

    lappend BDDNODES [list $num $label "t"]
    lappend TERMINALS "n$num"

    set len [string length $label]
    set sizex [expr 7+$len*$ZOOM/2]
    set sizey [expr 7+$ZOOM/2]

    $win create rectangle \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline black -activeoutline black -disabledoutline red -fill $COLORNODE -state normal \
       -tags [list "n$num" outline node]

    $win create text $x [expr $y+1] \
       -text $label -font [list Courier $ZOOM] -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node]
}

proc addNode {win num label x y} {
    global ZOOM
    global COLORNODE
    global BDDNODES

    lappend BDDNODES [list $num $label "n"]

    set len [string length $label]
    set sizex [expr 7+$len*$ZOOM/2]
    set sizey [expr 6+$ZOOM/2]

    $win create oval \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline black -activeoutline black -disabledoutline red -fill $COLORNODE -state normal \
       -tags [list "n$num" outline node]

    $win create text $x [expr $y+1]\
       -text $label -font [list Courier $ZOOM] -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node]
}

proc addLabel {win num label x y} {
    global ZOOM
    global COLORBG
    global BDDNAME
    global BDDNODES

    set BDDNAME $label
    lappend BDDNODES [list $num $label "l"]

    set len [string length $label]
    set sizex [expr 7+$len*$ZOOM/2]
    set sizey [expr 6+$ZOOM/2]

    $win create oval \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline $COLORBG -fill $COLORBG -state normal \
       -tags [list "n$num" outline node label]

    $win create text $x [expr $y+1]\
       -text $label -font [list Courier $ZOOM] -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node label]
}

proc addPoint {win x y tag} {
    $win create oval \
       [expr $x] [expr $y] [expr $x+1] [expr $y+1] \
       -outline black -activeoutline red -disabledoutline red -state normal \
       -tags [list line $tag node]
}

proc connect {win num1 num2 type} {
    global ZOOM
    global BDD
    global ARROWSIZE
    global COLORLEFT
    global COLORRIGHT

    set item [lindex [array get BDD "n$num1"] 1]

    set koord1 [$win bbox "n$num1"]
    set koord2 [$win bbox "n$num2"]

    set center1x [expr ([lindex $koord1 0]+[lindex $koord1 2])/2]
    set center1y [expr ([lindex $koord1 1]+[lindex $koord1 3])/2]

    set center2x [expr ([lindex $koord2 0]+[lindex $koord2 2])/2]
    set center2y [expr ([lindex $koord2 1]+[lindex $koord2 3])/2]

    if {$type == "l"} {
      set item [linsert $item 0 "n$num2"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left node]
    }

    if {$type == "li"} {
      set item [linsert $item 0 "n$num2"]
      set item [linsert $item 0 "i"]
      $win create line $center1x $center1y $center2x $center2y \
       -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
       -width 1 -fill $COLORLEFT -disabledfill red -state normal \
       -tags [list line "f$num1" "t$num2" left inverted node]
    }

    if {$type == "r"} {
      set item [linsert $item end "n$num2"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right node]
    }

    if {$type == "s"} {
      set item [linsert $item 0 "n$num2"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill black -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left right node]
    }

    if {$type == "si"} {
      set item [linsert $item 0 "n$num2"]
      set item [linsert $item 0 "i"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill black -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left right inverted node]
    }

    if {$type == "d"} {
      set item "n$num2"
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double inverted node]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double node]
    }

    if {$type == "dequal"} {
      set item "n$num2"
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double node]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double node]
    }

    $win lower line

    array set BDD [list "n$num1" $item]
}

proc parsefile {win filename} {
  set f [open $filename r]

  while {[gets $f line] >= 0} {
    if {$line != ""} {
      set i 0
      set end 0
      while {$end < [string length $line]} {
        set line [string range $line $end end]
        set line [string trim $line]
        set end [string first " " $line 0]
        if {$end == -1} {
          set end [string length $line]
        }
        set word [string range $line 0 [expr $end -1]]
        set "arg$i" $word
        set i [expr $i+1]
      }

      if {$arg0 == "node"} {
        set arg2 [string map {\" ""} $arg2]
        addNode $win $arg1 $arg2 $arg3 $arg4
      }

      if {$arg0 == "terminal"} {
        set arg2 [string map {\" ""} $arg2]
        addTerminal $win $arg1 $arg2 $arg3 $arg4
      }

      if {$arg0 == "label"} {
        set arg2 [string map {\" ""} $arg2]
        addLabel $win $arg1 $arg2 $arg3 $arg4
      }

      if {$arg0 == "connect"} {
        connect $win $arg1 $arg2 $arg3
      }
    }
  }

  close $f
}

# ############################################
# USER PROCEDURES
# ############################################

proc state {s win bb} {
  global STATE
  global STATUSBAR
  global COLORMENU
  global COLORHIGH

  if {$s != $STATE} {
    switch $STATE {
      pan {$bb itemconfigure 0 -relief link -background $COLORMENU}
      zoom {$bb itemconfigure 1 -relief link -background $COLORMENU}
      move {$bb itemconfigure 2 -relief link -background $COLORMENU}
      info {$bb itemconfigure 3 -relief link -background $COLORMENU}
    }
    switch $s {
      pan {
             $win itemconfigure "tekst" -activefill black
             $win itemconfigure "outline" -activeoutline black
             $bb itemconfigure 0 -relief sunken -background $COLORHIGH
             $win configure -cursor hand2
             set STATUSBAR "\[Pan\]  Left button + drag = scroll  /  Right button = center"
           }
      zoom {
             $win itemconfigure "tekst" -activefill black
             $win itemconfigure "outline" -activeoutline black
             $bb itemconfigure 1 -relief sunken -background $COLORHIGH
             $win configure -cursor target
             set STATUSBAR "\[Zoom\]  Left button = zoom in  /  Right button = zoom out"
           }
      move {
             $win itemconfigure "tekst" -activefill red
             $win itemconfigure "outline" -activeoutline black
             $bb itemconfigure 2 -relief sunken -background $COLORHIGH
             $win configure -cursor fleur
             set STATUSBAR "\[Move\]  Left button + drag = move subgraph  /  Right button + drag = move node"
           }
      info {
             $win itemconfigure "tekst" -activefill red
             $win itemconfigure "outline" -activeoutline black
             $bb itemconfigure 3 -relief sunken -background $COLORHIGH
             $win configure -cursor arrow
             set STATUSBAR "\[Info\]  Left button = subgraph info /  Right button = subgraph bounding box"
           }
    }
    set STATE $s
  }
}

proc selectnode {win item} {
  global SELECTED
  global LISTF
  global LISTT

  set SELECTED [list $item]
  set LISTF [$win find withtag [string replace $item 0 0 "f"]]
  set LISTT [$win find withtag [string replace $item 0 0 "t"]]
}

proc addSelected {root} {
  global SELECTED
  global TERMSELECTED
  global BDD
  global TERMINALS

  if { [lsearch -exact $SELECTED $root] == -1 } {
    lappend SELECTED $root
    if { [lsearch -exact $TERMINALS $root] != -1 } {
      lappend TERMSELECTED $root
    } else {
      foreach item [lindex [array get BDD $root] 1] {
        if {$item != "i"} {addSelected $item}
      }
    }
  }
}

proc addSelectedDepth {root num} {
  global SELECTED
  global TERMSELECTED
  global DEPTH
  global TERMDEPTH
  global BDD
  global TERMINALS

  if { [lsearch -exact $TERMINALS $root] != -1 } {
    if {$TERMDEPTH == 0} {
      set TERMDEPTH $num
    } elseif {$num > $TERMDEPTH} {
      set TERMDEPTH $num
    }
    if { [lsearch -exact $TERMSELECTED $root] == -1 } {
      lappend TERMSELECTED $root
    }
  } elseif { [lsearch -exact $SELECTED $root] == -1 } {
    lappend SELECTED $root
    lappend DEPTH $root
    lappend DEPTH $num
    foreach item [lindex [array get BDD $root] 1] {
      if {$item != "i"} {addSelectedDepth $item [expr $num + 1]}
    }
  } else {
    set i [expr [lsearch -exact $DEPTH $root] + 1]
    set inum [lindex $DEPTH $i]
    if {$num > $inum} {
      set DEPTH [lreplace $DEPTH $i $i $num]
      foreach item [lindex [array get BDD $root] 1] {
        if {$item != "i"} {addSelectedDepth $item [expr $num + 1]}
      }
    }
  }
}

proc selectgraph {win item} {
  global SELECTED
  global LISTF
  global LISTT

  addSelected $item
  foreach item $SELECTED {
    set LISTF [concat $LISTF [$win find withtag [string replace $item 0 0 "f"]]]
    set LISTT [concat $LISTT [$win find withtag [string replace $item 0 0 "t"]]]
  }
}

proc pan {win xd yd} {
  global SCROLL

  $win xview scroll [expr round((-1.0*$xd)/(2.0*$SCROLL))] units
  $win yview scroll [expr round((-1.0*$yd)/(2.0*$SCROLL))] units
}

proc motion {win xd yd} {
  global SELECTED
  global LISTF
  global LISTT

  if {[llength $SELECTED] > 0} {

    foreach item $SELECTED {
      $win move $item $xd $yd
    }

    foreach item $LISTF {
      set koord [$win coords $item]
      set x1 [lindex $koord 0]
      set y1 [lindex $koord 1]
      set x2 [lindex $koord 2]
      set y2 [lindex $koord 3]

      set x1 [expr $x1+$xd]
      set y1 [expr $y1+$yd]

      $win coords $item $x1 $y1 $x2 $y2
    }

    foreach item $LISTT {
      set koord [$win coords $item]
      set x1 [lindex $koord 0]
      set y1 [lindex $koord 1]
      set x2 [lindex $koord 2]
      set y2 [lindex $koord 3]

      set x2 [expr $x2+$xd]
      set y2 [expr $y2+$yd]

      $win coords $item $x1 $y1 $x2 $y2
    }

  } else {

    foreach item [$win find withtag node] {
      $win move $item $xd $yd
    }

  }
}

proc fixedgenode {win item tags dir a b dx dy} {
  global XARROW #used as parameter called by reference
  global YARROW #used as parameter called by reference
  global DOUBLELINE

  if {[lsearch -exact $tags "double"] == -1} {
    set diff [expr sqrt($dy*$dy*$a*$a+$dx*$dx*$b*$b)]
    set S [expr ($dy*$a)/$diff]
    set C [expr ($dx*$b)/$diff]
    set XARROW [expr $XARROW + $dir*$a*$C]
    set YARROW [expr $YARROW - $dir*$b*$S]
  } else {
    set diff [expr sqrt($dy*$dy+$dx*$dx)]
    set S [expr $dy/$diff]
    set C [expr $dx/$diff]
    set pa [expr $b*$b*$C*$C-$DOUBLELINE*$DOUBLELINE]
    set pb [expr $a*$b*$S*$C]
    set pd [expr $a*$a*$S*$S+$pa]
    if {[lsearch -exact $tags "left"] != -1} {
      set sign -1
      if {([expr $dir*$dx] < 0)&&($pa < 0)&&([expr $dir*$pb] < 0)} {set sign +1}
      if {([expr $dir*$dx] >= 0)&&!(($pa < 0)&&([expr $dir*$pb] < 0))} {set sign +1}
      if {([expr $dy] < 0)&&($pa < 0)&&($pb == 0)} {set sign -1}
      set angle [expr atan((-$pb+$dir*$DOUBLELINE*sqrt($pd))/$pa)]
      set XARROW [expr $XARROW + $sign*$a*cos($angle)]
      set YARROW [expr $YARROW + $sign*$b*sin($angle)]
    }
    if {[lsearch -exact $tags "right"] != -1} {
      set sign -1
      if {([expr $dir*$dx] < 0)&&($pa < 0)&&([expr $dir*$pb] >= 0)} {set sign +1}
      if {([expr $dir*$dx] >= 0)&&!(($pa < 0)&&([expr $dir*$pb] >= 0))} {set sign +1}
      if {([expr $dy] < 0)&&($pa < 0)&&($pb == 0)} {set sign +1}
      set angle [expr atan((-$pb-$dir*$DOUBLELINE*sqrt($pd))/$pa)]
      set XARROW [expr $XARROW + $sign*$a*cos($angle)]
      set YARROW [expr $YARROW + $sign*$b*sin($angle)]
    }
  }
}

proc fixedgeterminal {win item tags dir a b dx dy} {
  global XARROW #used as parameter called by reference
  global YARROW #used as parameter called by reference
  global DOUBLELINE

  if {$dx == 0} {
    if {[lsearch -exact $tags "double"] == -1} {
      set n 0
    } else {
      if {[lsearch -exact $tags "left"] == -1} {
        set n [expr -$DOUBLELINE]
      }
      if {[lsearch -exact $tags "right"] == -1} {
        set n [expr +$DOUBLELINE]
      }
    }
    if {$dy < 0} {set sign -1} else {set sign +1}
    set XARROW [expr $XARROW + $sign*$n]
    set YARROW [expr $YARROW + $sign*$b]
  }

  if {$dy == 0} {
    if {[lsearch -exact $tags "double"] == -1} {
      set n 0
    } else {
      if {[lsearch -exact $tags "left"] == -1} {
        set n [expr -$DOUBLELINE]
      }
      if {[lsearch -exact $tags "right"] == -1} {
        set n [expr +$DOUBLELINE]
      }
    }
    if {$dx < 0} {set sign -1} else {set sign +1}
    set XARROW [expr $XARROW - $sign*$a]
    set YARROW [expr $YARROW + $sign*$n]
  }

  if {($dx != 0) && ($dy != 0)} {

    set k [expr ($dy/$dx)]

    if {[lsearch -exact $tags "double"] == -1} {
      set n 0
    } else {
      if {[lsearch -exact $tags "left"] == -1} {
        set n [expr -$DOUBLELINE*sqrt(1+$k*$k)]
      }
      if {[lsearch -exact $tags "right"] == -1} {
        set n [expr +$DOUBLELINE*sqrt(1+$k*$k)]
      }
    }

    if {$dx < 0} {
      set n [expr -$n]
      set a [expr -$a]
    }
    if {$dy < 0} {
      set b [expr -$b]
    }

    set xdiff [expr abs(($b-$n)/$k)]
    set ydiff [expr abs($a*$k+$n)]

    if {([expr abs($b)] > $ydiff) || ([expr abs($a)] < $xdiff)} {
      set XARROW [expr $XARROW - $a]
      set YARROW [expr $YARROW + $a*$k+$n]
    } else {
      set XARROW [expr $XARROW - ($b-$n)/$k]
      set YARROW [expr $YARROW + $b]
    }
  }
}

# THIS IS USED TO RESTORE EDGES BETWEEN NODES
# FUNCTION ALSO CREATES INVERTERS
proc fixedges {win} {
  global XARROW #used as parameter called by reference
  global YARROW #used as parameter called by reference
  global LISTF
  global LISTT
  global COLORLEFT
  global COLORRIGHT
  global INVSIZE

  foreach item $LISTF {$win addtag selection withtag $item}
  foreach item $LISTT {$win addtag selection withtag $item}
  $win itemconfigure "selection&&double&&left" -stipple ""
  $win itemconfigure "selection&&double" -width 1
  $win itemconfigure "selection&&inverted" -width 1
  set list [$win find withtag selection]
  $win dtag selection

  set allitem ""
  while {$list != ""} {

  set allitem [concat $allitem $list]
  set otherlist ""

  foreach item $list {
    $win delete "p$item"
    $win delete "i$item"

    set tags [$win gettags $item]
    set node1 [string replace [lindex $tags 1] 0 0 "n"]
    set node2 [string replace [lindex $tags 2] 0 0 "n"]

    set koord1 [$win bbox $node1]
    set koord2 [$win bbox $node2]

    set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
    set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
    set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
    set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]

    set dx [expr $xarrow2 - $xarrow1]
    set dy [expr $yarrow1 - $yarrow2]
    if {($dx == 0)&&($dy == 0)} {set dx 1}

    if {([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "left"] != -1)} {

      set oline [$win find withtag "[lindex $tags 1]&&right"]
      if {[lsearch -exact $allitem $oline] == -1} {
        lappend otherlist $oline
      }
    }

    if {([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "right"] != -1)} {

      set oline [$win find withtag "[lindex $tags 1]&&left"]
      if {[lsearch -exact $allitem $oline] == -1} {
        lappend otherlist $oline
      }
    }

    # edge's start

    set point 0

    if {([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "left"] != -1) && ($dy < 0)} \
    {
      set other [$win gettags "[lindex $tags 1]&&right"]
      set onode [$win bbox [string replace [lindex $other 2] 0 0 "n"]]
      set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
      set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
      set odx [expr $oxarrow2 - $xarrow1]
      set ody [expr $yarrow1 - $oyarrow2]
      if {($odx == 0)&&($ody == 0)} {set odx 1}

      if {([expr $dy*$odx] > [expr $dx*$ody]) && ($ody < 0)} {

        set point [expr abs([lindex $koord1 1]-[lindex $koord1 3])]
#        addPoint $win [expr $yarrow1+$point] [expr $xarrow1-$point] "p$item"

        $win create line $xarrow1 $yarrow1 [expr $xarrow1-$point] [expr $yarrow1+$point] \
           -arrow none -width 1 -fill $COLORLEFT -disabledfill red -state normal \
           -tags [list line "p$item" node]

        $win lower "p$item"

        if {([lsearch -exact $tags "inverted"] != -1)} {
          $win create oval \
            [expr (8*$xarrow1-5*$point)/8-$INVSIZE] [expr (8*$yarrow1+5*$point)/8-$INVSIZE] \
            [expr (8*$xarrow1-5*$point)/8+$INVSIZE] [expr (8*$yarrow1+5*$point)/8+$INVSIZE] \
            -outline $COLORLEFT -disabledoutline red \
            -fill $COLORLEFT -disabledfill red -state normal \
            -tags [list line "i$item" node]
        }

        set xarrow1 [expr $xarrow1-$point]
        set yarrow1 [expr $yarrow1+$point]

        set dx [expr $xarrow2 - $xarrow1]
        set dy [expr $yarrow1 - $yarrow2]
        if {($dx == 0)&&($dy == 0)} {set dx 1}
      }
    }

    if {([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "right"] != -1) && ($dy < 0)} \
    {

      set other [$win gettags "[lindex $tags 1]&&left"]
      set other [$win bbox [string replace [lindex $other 2] 0 0 "n"]]
      set oxarrow2 [expr ([lindex $other 0] + [lindex $other 2])/2.0]
      set oyarrow2 [expr ([lindex $other 1] + [lindex $other 3])/2.0]
      set odx [expr $oxarrow2 - $xarrow1]
      set ody [expr $yarrow1 - $oyarrow2]
      if {($odx == 0)&&($ody == 0)} {set odx 1}

      if {([expr $dy*$odx] < [expr $dx*$ody]) && ($ody < 0)} {

        set point [expr abs([lindex $koord1 1]-[lindex $koord1 3])]
#        addPoint $win [expr $yarrow1+$point] [expr $xarrow1+$point] "p$item"

        $win create line $xarrow1 $yarrow1 [expr $xarrow1+$point] [expr $yarrow1+$point] \
           -arrow none -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
           -tags [list line "p$item" node]

        $win lower "p$item"

        set xarrow1 [expr $xarrow1+$point]
        set yarrow1 [expr $yarrow1+$point]

        set dx [expr $xarrow2 - $xarrow1]
        set dy [expr $yarrow1 - $yarrow2]
        if {($dx == 0)&&($dy == 0)} {set dx 1}
      }
    }

    if {$point == 0} {
      set XARROW $xarrow1
      set YARROW $yarrow1
      set a [expr ([lindex $koord1 2] - [lindex $koord1 0])/2.0]
      set b [expr ([lindex $koord1 3] - [lindex $koord1 1])/2.0]
      if {[$win type $node1] == "oval"} {
        fixedgenode $win $item $tags 1 $a $b $dx $dy
      }
      if {[$win type $node1] == "rectangle"} {
        fixedgeterminal $win $item $tags 1 $a $b $dx $dy
      }
      set xarrow1 $XARROW
      set yarrow1 $YARROW
    }

    # edge's end

    set XARROW $xarrow2
    set YARROW $yarrow2
    set a [expr ([lindex $koord2 2] - [lindex $koord2 0])/2.0]
    set b [expr ([lindex $koord2 3] - [lindex $koord2 1])/2.0]
    if {[$win type $node2] == "oval"} {
      fixedgenode $win $item $tags -1 $a $b $dx $dy
    }
    if {[$win type $node2] == "rectangle"} {
      fixedgeterminal $win $item $tags -1 $a $b $dx $dy
    }
    set xarrow2 $XARROW
    set yarrow2 $YARROW

    $win coords $item $xarrow1 $yarrow1 $xarrow2 $yarrow2

    if {$point == 0} {
      if {([lsearch -exact $tags "inverted"] != -1)} {
        $win create oval \
           [expr (5*$xarrow1+3*$xarrow2)/8-$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8-$INVSIZE] \
           [expr (5*$xarrow1+3*$xarrow2)/8+$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8+$INVSIZE] \
           -outline $COLORLEFT -disabledoutline red \
           -fill $COLORLEFT -disabledfill red -state normal \
           -tags [list line "i$item" node]
      }
    }
  }

  set list $otherlist
  }
}

# THIS IS USED TO CREATE SIMPLE EDGES BETWEEN NODES CENTER
# POINTS ARE DELETED, USE FIXEDGES TO RESTORE THEM
proc hidearrows {win} {
  global LISTF
  global LISTT
  global DOUBLELINE

  foreach item $LISTF {$win addtag selection withtag $item}
  foreach item $LISTT {$win addtag selection withtag $item}
  $win itemconfigure "selection&&double&&left" \
    -width [expr 2*$DOUBLELINE+2] -stipple gray50
  $win itemconfigure "selection&&double&&right" -width 2
  $win itemconfigure "selection&&!double&&inverted" -width 2
  set list [$win find withtag "selection"]
  $win dtag selection

  foreach item $list {
    set tags [$win gettags $item]

    set koord1 [$win bbox [string replace [lindex $tags 1] 0 0 "n"]]
    set koord2 [$win bbox [string replace [lindex $tags 2] 0 0 "n"]]

    set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
    set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
    set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
    set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]

    $win delete "p$item"
    $win delete "i$item"
    $win coords $item $xarrow1 $yarrow1 $xarrow2 $yarrow2
  }
}

proc noarrows {win} {
  $win itemconfigure "left||right" -arrow none
}

proc showarrows {win} {
  $win itemconfigure "left||right" -arrow last
}

proc arrowsOnOff {win bb} {
  global ARROWSON
  global COLORTEXT
  global COLORHIGH

  if {$ARROWSON == 1} {
    $bb itemconfigure 0 -relief link -bg $COLORTEXT
    noarrows $win
    set ARROWSON 0
  } else {
    $bb itemconfigure 0 -relief sunken -bg $COLORHIGH
    showarrows $win
    set ARROWSON 1
  }
}

# ############################################
# API
# ############################################

proc not_implemented_yet { t } {
  tk_messageBox -icon info -message "$t" -type ok
}

proc bddview_clear {} {
  global mainwin

  global BDDNAME
  global BDD
  global BDDNODES
  global TERMINALS

  global X
  global Y

  global SELECTED
  global TERMSELECTED
  global LISTF
  global LISTT
  global DEPTH
  global TERMDEPTH

  $mainwin delete all

  set BDDNAME ""
  array unset BDD
  set BDDNODES ""
  set TERMINALS ""

  set SELECTED ""
  set TERMSELECTED ""
  set LISTF ""
  set LISTT ""
  set DEPTH ""
  set TERMDEPTH 0
}

proc bddview_draw {name} {
  global mainwin
  global ZOOM
  global ARROWSON
  global LISTF
  global LISTT

  bddview_clear

  set activezoom $ZOOM

  while {$ZOOM > 16} {
    zoomOut $mainwin 0 0
  }

  while {$ZOOM < 16} {
    zoomIn $mainwin 0 0
  }

  parsefile $mainwin $name
  boxit $mainwin

  set LISTF [$mainwin find withtag left]
  set LISTT [$mainwin find withtag right]
  fixedges $mainwin

  set LISTF ""
  set LISTT ""

  while {$ZOOM > $activezoom} {
    zoomOut $mainwin 0 0
  }

  while {$ZOOM < $activezoom} {
    zoomIn $mainwin 0 0
  }

  compass "c" $mainwin
  compass "n" $mainwin

  if {$ARROWSON == 1} {
    showarrows $mainwin
  }

}

proc bddview_saveas {win} {
  global mainwin

  global BDD
  global ZOOM
  global BDDNAME

  set filename [tk_getSaveFile -title "Save As" -initialfile "$BDDNAME.bddview" -parent $win]
  if {[string length $filename] != 0} {

    #remember last path
    cd [file dirname $filename]

    while {$ZOOM > 16} {
      zoomOut $mainwin 0 0
    }

    while {$ZOOM < 16} {
      zoomIn $mainwin 0 0
    }

    set f [open $filename w]

    set i 0
    set llist [$win find withtag "label && tekst"]
    foreach item $llist {
      set name [string range [lindex [$win gettags $item] 0] 1 end]
      set koord [$win coords $item]
      set x [expr floor([lindex $koord 0]) - 18]
      set y [expr floor([lindex $koord 1]) - 18]
      set t [$win itemcget $item -text]
      puts $f "label $name \"$t\" $x $y"
    }

    set nlist [$win find withtag "node && !label && tekst"]
    foreach item $nlist {
      set name [string range [lindex [$win gettags $item] 0] 1 end]
      set koord [$win coords $item]
      set x [expr floor([lindex $koord 0]) - 18]
      set y [expr floor([lindex $koord 1]) - 18]
      set t [$win itemcget $item -text]
      if {$t == "1"} {
        puts $f "terminal $name 1 $x $y"
      } else {
        puts $f "node $name \"$t\" $x $y"
      }
    }

    foreach item $llist {
      set name [lindex [$win gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      if {[lindex $succ 0] == "i"} {set succ [lrange $succ 1 end]}
      set linef [string replace $name 0 0 "f"]
      set linet [string replace $succ 0 0 "t"]
      if {[$win find withtag "$linef && $linet && inverted"] == ""} {
        set inv ""
      } else {
        set inv "i"
      }
      puts $f "connect [string range $name 1 end] [string range $succ 1 end] s$inv"
    }

    foreach item $nlist {
      set name [lindex [$win gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      if {[lindex $succ 0] == "i"} {set succ [lrange $succ 1 end]}
      if {[llength $succ] == 1} {
        set then [lindex $succ 0]
        puts $f "connect [string range $name 1 end] [string range $then 1 end] d"
      }
      if {[llength $succ] == 2} {
        set else [lindex $succ 0]
        set then [lindex $succ 1]
        set linef [string replace $name 0 0 "f"]
        set linet [string replace $else 0 0 "t"]
        if {[$win find withtag "$linef && $linet && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        puts $f "connect [string range $name 1 end] [string range $else 1 end] l$inv"
        puts $f "connect [string range $name 1 end] [string range $then 1 end] r"
      }
    }

    close $f

    compass "c" $mainwin
    compass "n" $mainwin
  }

}

proc bddview_print {win} {
  global ZOOM
  global XMAX
  global YMAX
  global BDDNAME

  if {$BDDNAME == ""} return
  set koord [$win bbox all]
  if {$koord == ""} return

  set filename [tk_getSaveFile -title "Print to File (postscript)" -initialfile "$BDDNAME.ps" -parent $win]
  if {[string length $filename] != 0} {

    #remember last path
    cd [file dirname $filename]

    set x1 [lindex $koord 0]
    set y1 [lindex $koord 1]
    set x2 [lindex $koord 2]
    set y2 [lindex $koord 3]

    if {[expr (1.0*($y2-$y1))/($x2-$x1)] > 1.415125082} {

      $win postscript -file $filename -pageanchor nw -pagex 0m -pagey 297.039m -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1] -pageheight 297.039m

    } else {

      $win postscript -file $filename -pageanchor nw -pagex 0m -pagey 297.039m -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1] -pagewidth 209.903m

    }

  }
}

# ############################################
# FINAL INITIALIZATION
# ############################################

state pan $mainwin $bb1
puts "OK"

if { $argc != 0 } {

  set filename [lindex $argv 0]

  if {[file exists $filename]} {
    bddview_draw $filename
  } else {
    puts "Warning: $filename does not exists"
  }
}

