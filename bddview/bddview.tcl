#!/bin/sh
# the next line executes the program\
exec wish "$0" "$@"

# use this if your BWidget package is installed in user's space
# lappend auto_path /$env(HOME)/BWidget-1.8.0/

package require BWidget
package provide bddview 1.0

# ####################################################################
# $Revision: 545 $
# $Date: 2019-02-11 14:07:50 +0100 (pon, 11 feb 2019) $
#
# This file (bddview.tcl) is a Tcl/Tk script
# Author: Robert Meolic (robert@meolic.com)
#
# This file has been released into the public domain by
# the author.
#
# The author grants any entity the right to use this work
# for any purpose, without any conditions, unless such
# conditions are required by law.
#
# bddview script is part of Biddy project (http://biddy.meolic.com/)
# bddview script is used in EST project (http://est.meolic.com/)
#
# INSTALATION:
# You need Tcl/Tk and BWidget.
# For MS Windows, both are included in ActiveTcl distribution.
#
# USAGE:
# ./bddview.tcl example.bddview
#
# BDDVIEW INTERNAL FORMAT:
# bddview reads BDD from the file with the following format:
#
# type <bddtype>
# var <variablelist>
# label <n> <name> <x> <y>
# node <n> <name> <x> <y>
# terminal <n> <name> <x> <y>
# connect <n1> <n2> <class> <tag1> <tag2>
#
# "type" is optional, supported BDD types are ROBDD, ROBDDCE, ZBDD, ZBDDCE, TZBDD, TZBDDCE
# "var" is optional, variables in <variablelist> are separated by space
# <n>, <n1>, and <n2> are the unique sequence numbers (integer)
# <name> is function's, variable's, or terminal's name, respectively (string)
# <x> and <y> are coordinates (float)
# <tag1> and <tag2> are variable's name, only used for TZBDD and TZBDDCE
# <class> is one of the following:
# s : single line
# si : inverted single line
# r : line to 'then' succesor
# ri : inverted line to 'then' succesor
# l : line to 'else' successor
# li : inverted line to 'else' successor
# d : double line ('else' successor inverted)
# di : double line ('then' succesor inverted)
# e : double line (none succesor inverted)
# ei : double line (both succesors inverted)
#
# REMARKS:
# 1. (0,0) is top left corner.
# 2. Only one label is supported.
# 3. If <variablelist> is given it determines the ordering of the listed variables.
# 4. Single line (or inverted single line) must be used to connect a label and a node.
# 5. Terminal's name must be either 0 or 1.
# 6. Function's, variable's, and terminal's name can be given with or without
#    quotation marks (in the last case the name must not include spaces)
# 7. In an "incomplete bddview format" coordinates are omitted,
#    such format cannot be used with bddview but it can be handled by BDD Scout
#
# EXAMPLE (example-robdd.bddview):
# ---------------------------------------------------
#type ROBDD
#var "B" "i" "d" "y"
#label 0 "Biddy" 58.0 18.0
#node 1 "B" 58.0 90.0
#terminal 2 "0" 28.0 162.0
#node 3 "i" 90.0 162.0
#node 4 "d" 64.0 234.0
#terminal 5 "0" 28.0 306.0
#node 6 "y" 90.0 306.0
#terminal 7 "0" 54.0 378.0
#terminal 8 "1" 126.0 378.0
#node 9 "d" 118.0 234.0
#terminal 10 "1" 154.0 306.0
#connect 0 1 s
#connect 1 2 l
#connect 1 3 r
#connect 3 4 l
#connect 3 9 r
#connect 4 5 l
#connect 4 6 r
#connect 6 7 l
#connect 6 8 r
#connect 9 6 l
#connect 9 10 r
# ---------------------------------------------------
#
# EXTERNAL API (see near the end of file):
# proc bddview_message {text1 text2}
# proc bddview_clear {}
# proc bddview_bendsOnOff {}
# proc bddview_arrowsOnOff {}
# proc bddview_gridsOnOff {}
# proc bddview_open {filename}
# proc bddview_save {filename}
# proc bddview_export_tex {filename caption document}
# proc bddview_print {filename}
# ####################################################################

set BDDVIEWVERSION "1.3"
set TITLE "BDDview v$BDDVIEWVERSION, Robert Meolic"
set OS $tcl_platform(platform)
set OS1 $tcl_platform(os)

# DEBUGGING
# if {$tcl_platform(platform) == "windows"} {console show}

puts -nonewline "Starting $TITLE ... "

# ####################################################################
# IMAGES FOR TOOLBAR ICONS
#
# THEY ARE BASE64 ENCODED GIFS (32x32)
# - create 32x32 PNG
# - use command "base64 filename.png"
# - use color: #49B2C2 (COLORBUTTON)
#
# Icons made by Freepik (www.freepik.com)
# Downloaded from Flaticon (www.flaticon.com)
# License: Flaticon Basic License (free license with attribution)
# Obtained from:
#
# <Print>
# https://www.flaticon.com/packs/linear-color-web-interface-elements
#
# <Pan>, <Zoom>, <Edit>, <Info>
# https://www.flaticon.com/packs/web-interface-icons
#
# <GoUp>, <GoLeft>, <GoRight>, <GoDown>
# https://www.flaticon.com/packs/color-arrow-collection
#
# <FullScreen>, <NoFullScreen>, <Iconify>, <Exit>
# https://www.flaticon.com/packs/basic-application
#
# <TeX>
# https://www.flaticon.com/packs/file-formats-icons
#
# Icons made by Good Ware (www.flaticon.com/authors/good-ware)
# Downloaded from Flaticon (www.flaticon.com)
# License: CC 3.0 BY
#
# <Save>
# https://www.flaticon.com/packs/document-3
# ####################################################################

set bddview_icon_GoUp {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAE3ElEQVRYw61XS2xUVRj+zrnPuXfu
PNqBUmgpQycWYTpKsDQFMbqoEhcQCcEHxo2RWtRIjAsMxoakhNpEF0iiCQtiQtSwcKEboItCjLRY
ni00tJ3QgsVCp7TTztx53LkPF22H1pbOHZxvdXPPl/N/5z/f+c/5CWyiuau3ZCKtHVIzRn1My5Qn
dN2hmRYFAJ4SU2LZpJNnhxWOa1M4pqWpNnjfzrwkF+Hgxe76aDpzfDieeMawLFtiGUJQ5pT6fSL/
SXNd9bmnEvDttT7PnUn17GBM3WzaDLwY/C75csDtrP9sY1XUtoCmSzdD4Wj8z/G05kQB4BH4ZIUi
7WzZEmrLKeDgxe7d/dHYL0ndYFBAiAw113ld7x7dEvr5iQK+6uxZ1zseu6nquq3gPKUAAM00bYuo
8rq2z80Enf040tXrGoolOu0GB4Ad/pXY4V9pOwspw6RDU+pvhzp6ihYIGFFTv0eSabfdyZY5BOyt
qsDeqgoscwi2RUxqGXEirZ2dJ6Dp0s3QUEx9KZ89bQxWgqMEHCVoDFbm5YfBKfWFLzt6Xs0KeJhI
/2TkcdSe93mwuSSbRWwuKcJzPk9eIsZS2ncAQJq7eks6Hzx6YFcARwm+f3kTVkgiHqU0AECxyONB
IoXG81eQMe0Xq5rl3jI6kda+yGf1O/2rsEISAQDHbgzg2I0BAMAKScRO/yrb8xiWhVjGOEjVjFGf
r/EA4MroBNoHBtE+MIgroxMAkLchY5lMPY1pmdV2yJYF7K8OgKMEhmWh9Wrf9M+Zb8OywFGC/dUB
2wLiml5GE7ou2SFvW+lDzXIvAOB0eBgjkUh2bCQSwenwMACgZrkXL5b6bAlI6LqDzl6pS8HBMti3
YS0AIJJM40RPeAHnRE8YkWQaALBvw1o42Nz1TDMtSu0o3RMoR5HIAwCOd4ehq/EFHF2N43j3tLAi
kceeQLmtLFCekiULeZlTwq7KaXd3jU6gvf/OE7nt/XfQNWPIXZWrsFqRctwlxKQSyyaWIu0PVoIh
BBnTQuvV2zlX1Hr1NjKmBYYQfLhh6QopsWySKjx370mEraU+hHzT18Opvrt4GBnLKeBhZAyn+u4C
AEI+95KGVHjub9bJsecArF/MeA0zxgOAULEbX9dvsbWv/BxrNQQrcS0SharrC3gyx7SxHoFrYQg5
8N9qONd4ALBp5gjmC6/AYXegDD/eHlpQil0ce5QAQEP75d57scSzs4NunsPnG6vAUoJCQDctfHO9
H9G0lv1Xocj9P7yyqYqdqeONw/HkedOyYFgWhsfG8OmZ0YI9xwhDISkuMGR6QZQQeATu4+x1fLg2
eMGvyB0AkM7osAwThYQgK9ngAOBX5L9mn2VZt5TK4nYAxmJF5v+AER3gmcemLBL4+Fq3/NqCJ9mh
mvVTDpb5gDC0cKlnWcgO8bHrWdbwu+Rtc3uEedF+fX3rydLi4tZCiRAkeV7wgMf5VnNd9fWcjcmb
Zzrev/9o/ISl6099DFjZCYnnZtOeCHicdYdrg922W7OG9svl4Wj8Vio2pSDP1oywLJxOBSwl1hpF
/sMtcG8cqasef6rm9O2znQf+iaktuhq3/dRRPF6sccm3SiThncVWnZeAWbzXdikYTWsnJ1Natalp
vGXO3x7KCYYoCpPFonBhncf5UVNtcMTOvP8C4vDL+KynuZkAAAAASUVORK5CYII=
}

set bddview_icon_GoLeft {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAEvElEQVRYw8VXa0xbZRh+zul3Ti+n
peW2jcGmMGRkhTplWMY2zWKA6Q9NzDQu/jBLdIqbf/RP54hYszlc4iVOExNj1MRgpkajJmbKD4aJ
QMfYZlsZbOg61wSBstKeXs61xx+4yp3Skvj+PN/lefK87/e856WQYbg9/lJeVl28LDfFJKUsoShG
KaXRAMDSVMpESNLCMjc5RteVx5CT7c6asUzupVba0Nbnaw4J0ulgLFGlalpGZHUUhTKz6apNzxzp
aHR0ZUXgWJ+vICxKP12Pxncgy6ApCuUW7nyFlWt56Z6t0xkTcPV6mwLR+PcRSTZgDaJAz8bK87g9
x3fWXl5Acv6Ho73eAyPh6Nm1AgeAW6JkHg7zF1y93v3LKuDq9TaNhKNnBTVF5wpq07OYEkToqP8g
OEJUe2HevW5njXeBAm9fGrHd4BPf5QrOEYKnq+/Ey9urkOCjkNRUei2uKLrR6VjfsT5fwQICo5FY
17QoGXMB311ShA/31uHxyjIQmoKmpiBEI4gnhdnpMEVE+ds5BNr6fM25VPtmiwlvNNTCVVeNfD2z
YF0VkuB5Pq1GgI/vaff4HQBAACAkSKezATYSHZ6o3ITHtpTOyfVioSkKhGgECmeGkWGo8YTYCaCG
uD3+0oGJcFU2ch+yV6DAwAIA5JSGz0duwFFoRd26/CXPKfEYYoQgANjdHn8J4WXVlanDAUCxUY8X
aitRPwtkYCKMUxeHMT4ZwptNjSveoSkK+Okwhg3sB4SX5aZMgBmawqPlpXhq6x1g6Bm5J5Mi3veO
ovvqn1nVzpQgPkBiklK20sa7i2x40VGJDaYZb1I1DV+OBvGRbxRKPJb1qxEE0UoSimJcTu7Wmi24
b3362WJwIoxTF0cwNjmZs0OmZFFHbrfU2cHSNB4p3zhH7ilBwnu/XUP3tevAKmpmxYaF/zlolqZS
8z9KqRS+/iOIQ90XcH78FgCg0MDC7bTjneZdKCkuXjsCJkKSSy1OJkW8PjCEtn4//k7M2Gndunx0
tjjR2rAdhDPnBE7RRKMtLHNzpY2XQ9NoPTeIT68EIKc06CgKB+7ahDMPNWJvVUUO8rMSzTG6rkw2
yyktnZaBiXD6lbiddrzVshvri4tWTcBqYH10vp49sZKPz0/La57f0TE4jFuCBACoX5ePzuYGHNxR
C5bOvK5tevYgBQDPdw+O3ODjq+4HyzUj31QEh3/8ZcmzhDOLPfsfNNAzTJgj9CpUuB1JRcVnwwEc
7rkEbyiyiuqjsNHCudI+0NHo6Cq3cOezLaZgLIFX+n04OSsty4XBksd/0dLw7hwjqrByLQV6NpbL
s/p1LITnzg3iq9EglNTibkkRolXazPZFf0rbPX6HbypyMamoulwNxsoyCIZC0Gb9E1I6GqVFRc+c
2bfz40Wt2O2s8VbZLE9yhKi5EohIMkyWvLRZUToaJYWFp2aDLzmYvNrvqw7wif7JpGhdI8dVjUT3
7DcP7/ok49HsxMBQ3lhc+CHAx+9Xs+x+/45mfSWcYd+x+m3RrIbTdo/fMZ4QO4OxhH01w2mp2Xhl
g8nQ6nbW9OQ0Hd+O4wND68OidDQuq028JG9OKIpp3niesLDMX2aG/GzTMx1t9dvGM7n3HwZ69Iiw
3/GRAAAAAElFTkSuQmCC
}

set bddview_icon_GoRight {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAEv0lEQVRYw61XW0xcVRRd9z2vOzPA
UCShlioplDKkLeWlxrYfqDHRBNNaJY3aGKOJNtFoGmobEUMVf/pjK7U19suYqDHxxxb4INqWMgK2
DgyWOkAtg7RA531n5r79UKdToDAP1ue9Z5+97l7r7Hs2gTTRMThWFBTl1qisPBGR5AdjimKSNJ0E
AJYkNBNNx3iWuWlmqN48jv3kaG3l7XT2JVZb0OYa3XkrluiaicY3q7qeFlmKIFBiMXmKTFxLe32V
OysCxwbHrLNC4vxURGjU0ky8HJFS3vxLsdnwzJHaynDaBJ776dKBuKKeAUBhDVBo5EKlvKnhowbn
tcXvyMUP9p7r/3TO7/8qHAhQMUmGmuXXp2I+LtrG/JHR1n73nhUrsO/85VdnFha+1FXt7gKKRInD
gZAk50zESFOqs8C2PdUXyQq83je0fuaO/0xqcgDQVQ3vbSvHyxWlMNK5KRJXVMobjF46fmXcvoSA
Nxj16IqyrCdoksDeshJ8sasGjxY7ciLhFyXLZEjovofAi90DbyciYX614HwDi8M1Ffi4wYkSiylr
ElMRoa61392UJPB3ROhEBmardthwcue2rGXRdB1BUT4BAORLva4qRYhy6QQOzwVw9o8bkDUdFHFX
lseKHcj0sPiisU0dg2NFZFCUzqYbJGkazg6NoKVnAINzgaQsrTUV+LB+CwqNXNoEVF1HQJSOkKGE
5My0hLfnF/Bu90W0uTyYj4sAgNp1eTi9ewf2PFwChiTS2keQ1SZSkyQ2WzP1XZ/EvnP9+ObPaai6
DoYk8MrmUnTtqsFWh33V+Igkryd1TSFyOVaKEEXXwFW0dLsw/J8sD5gM6Giowge1lSvKElMUI4k1
wuz8PN7puYQ2lwd3EhIAoK4oPykLS5LLeEon14xAtqDXaqPiwkIc2l6OmnV5yWe/3vaja3QiadTF
YElCo0mGU1U1nnWTp80WvOYsw/NlJaCIf+10K5bAZ24vfl8Irhhrouk4bTBwISERz88m+e5ND+Gt
6rKk0WRNx9fjf+HHqRnI2uqdycLSPrrAwP0sAM2ZJC4qdODQ9grUppR7cC6Az0e89y33cuAZppeu
sFveDCSk5mg0Al1RVgxgSRIHdjixv3xDstn4ExJOeyZxcXYh4+saz1CdBAC80Tc8eiMsbInLMhQh
umTxyacfh7PAtqSV/jAxg2+904grasbybeDN10/trimnAaDIxLX4orGrJpYhJMoGMSasWA33Qgin
PBO4GYllfWocBvZg8nfcXl/lLuXNFwCApUjwPA/KYFwSFBBldA5fw/sDIzkl32g1D3U0Onvu6QM2
jmnO59hpvyiZAMBsNEBiWYhCBIqm4zuvD997fRBW8clqsHNsvMxmaVr2UtrmGq323An/JigKlap1
gYFDUJRyblYGitTK86xPdT5S3bvstby9vspdZre8YKZpNdWta5W8Is+6PzX5fQeTo5dHtk6FhQt+
UbKsRZu2sUyi1Gp+dnHyFUez41fG7ZMhoXsqItRpOQwnG63moTyOffJYo9Of1XDa2u9uCoryCV80
tinD4fS6w8Ae/N/tWU/HSX+4RovDsnJYkNWmiCSvjymKcdF4HrewtI9nmF6eoTrb6qtm0tn3H29o
DQnzGHrPAAAAAElFTkSuQmCC
}

set bddview_icon_GoDown {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAE6klEQVRYw61XW2wUVRj+zpnZmb2W
3cK2qRS1ZHux0KJA5RZDmlgwJr4JMYYXTHhDHnwga2zSYEAxMcQEBEENiQka0WB4Eu2bYJtSFN3d
Ekq2lEvX0rJsdzuX3bkeH+huCm2309L/aWbOP//3nXP+853/J3Boh/oSNTez8pePCtr2QkFbZhsa
N32cUJ5RQdCXuYV4UBT2ftexKeEkLpnPoasv0Tqmat/fmVTWSNkJp3zB+/zacwFf9Iedm79YFIGP
euOVOc345Y6kvGbajMiyBGaaWJARAnegQooE/WtOt2+875hAV1+iNZmVezOa7gUAVTdgKjIWa4Tn
2crllft+fGPLt0+P0ac/RHtibw88mvy7CK5b9jOBAwAzTZJKp7/Z9WvPZ2VXoLM3/vLNCemaYpql
BJOkRSz9XCvBUVRVVr534c1tZ2eswLHrg8HhSeXydHAlX1gycABglo28aX19pP9GxQwCt3PKbxlN
9xffdcuGVchjKY33+QGAG1UKl54gEO2JdQxLyqvFjxZj0BRpScEJRyG6eADAsKRs6epLbAcAHgCy
mnHCZqzkvNwt4pPN28BTsiTgps3w+fVB5HQDAGAzhgdq4RSAZv5QX6Kmf3yiYfoPWU1HMidjV6R2
SQj8lBwpgRctJedfOtx/o5qfNMwPrWmzL9rPyRG8vqoaIdEFAPhrfAK6bTsCFCjFhqoQACBT0HE+
OVODLMaQ1YworxhWx2xBFNPE6cQQohuaAACxRzmcvRZ3RGDvxpYSgdMDt5E3rVn9ZMPcQSXdWDVX
oCujacTSOQDAnsYXUB1eMS94dXgF9jS+8Jh0Ooc/R9Nz+kq68TxVTdNTLuBXA0OwGIOLEhxc3zQv
gYPrm+CiBBZjOJkYKuurmqaX6jaj5ZzuSSouDKUAAG1VIbQ3rJ7Tt71hNdqmlv7CUAojslqWgG4z
Sp3s6fnkfWQKOgBgf2ukKCgzRGZ/a6Rs4s1mVKBk3tTOmxbODNwGAIQ9Iva1RGb47GuJIOwRAQBn
yiTek6eF2NTL84709spoGv3jjwuS3ZFa1ITDpbGacBi7pzSjf3wCl/9LO5q9l+dV6hf4EaeCcjKe
hGEzcITg4PpGgBBg6pkjBIbNcDKeBHEooAHBdY8GXK5upwQe5jWcG7wLANhQFUJ7fR3a6+tKZ/7c
4F08zGuOFdLn4rppwMUd5Yhzzb84nMIDtQAAOLCuHgfW1QMAHqgFXBxOOY7DEYKQKHxKuzatTdX6
vbec/mjYDMdjyalLS8BytwAAOBF7vD1OrdbvHehsax6jALDCLby/kMvl33QWV8cypferYxn8k84u
aPbVXvHdUj1weEvL73UVvmsLIXEqMQTDZjBshlPzKN7T9mLA98ehTWtjpXoAAEKisDMj6KmcbrgX
mpALSbywR8zV+NxvzVqURntiHYMTk5cKlu1IIYUpIXV6Tft43mquDKz9eHPLzVnL8qNbW7ubQhV7
3Bx1FFG3bcfgHp6zIkH/O9PB52xMoj2xjruSejGr6Z6lqIgqRUGOBP3bivvuqDU7dn0wmMzJ3cOT
ysbFAlNCUBfwXV29zLfzg1cas4tqTjt74zvSBf34iKw2zFa6zXXMav3eW0HRtf/o1tbuZ+qOp7Xn
KyXDikqG0SHrZq1qmp5iLSFQYnt5Ph8QXPd9Lq47JApHOtuax5zE/R9cgChjVdkJdwAAAABJRU5E
rkJggg==
}

set bddview_icon_Save {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAADNUlEQVRYw+1XXUhTYRh+vvPXXE0S
PfNs/oEbJBpdSBA5I4gIQe2HokATM5wQ4rrRZEJFoNZNFzkCxdJKMeqiMkPWn3d5JejNWGlp0tQ5
c0O3LDtnni4iaf7MsjMj6Ll7fzjv873f+33P+QiWICMjg/P5fDUMw5wRRVGLdSIqKurZ8PBwLgAp
XB5ZaicnJ9s5jttnNpuZ1NTUdRVvbW1Ff38/OI5rGx0dLQYgr5bL/GwIgnB0YWFhf0dHB5Wenr7e
xcNut8Pj8WBycvJkQkLC+7GxsQur5VIhBkXtS0tLk/6k+A8YDAZYLBYiy/J5nU5n/iUCsixv5nme
hkIwmUwoLS0FRVFNOp3uyJoEIoGcnBzk5eURmqbv6/X6rLAzoCSmpqbQ29sLADAajeB5nvF6vU8T
ExMzXS7XUEQJCIKA7u5uOJ3OpaEtsixbAZyOKAGr1Yry8vJl/qqqqmBPTw+9IVsQHR29zMey7MYP
4Vr4rQ4EAgEcP3EMc3OBFeMqlRp3O+4hJiYmMgS8Xi+GBt9h1/EANHwwJPZ5lsKrNg08Hk/kCCze
cru/IN4ohvhmJmm8atP89rf++gz8G0NoszXg+Us7xK/f207I6rpuOVuOTSoOpqy9qD5XrUwHYmPj
4HQMwTf/FqYiP2JTxGU5Gm0Qe07N4hP1Fk7HELS8VrktKCgogMVigW+MxVa9BHqFvhECxBslTL/n
UFZmRklJibKnoKKiAn6/H3eu3sIm9TRSMudD4hNvWHTV8cjPz0dlZVVkhtBqteLQwcN4Uh+Hidfc
ov/jKIPHl7TINu1Ffd1lkJWGRAkChBDU1tYha3c2umq18H5gMOOm0XlRwI7tO9FwzQaGYZQ/BUsF
xWa7juJTRXh0UQbNAikJ29DU2AyO4zbmHlCpVLjR3AIh1oAYdTJabt6GWq2OvBiFHDuNBg8fdEKW
5XWtXJH/gZX0/b8W/BEBQkjA7XZLkSo2Pj4uEkL84R4mLwcHB7m+vj7Fiw8MDMDhcHAAXoR7nCIp
KambEHKgsLCQNhgMihQfGRlBe3u7FAwGn7lcrtywBACwgiDUsCxbJkmSXhHNZ5hxURQb3W73FQAh
UvoNAgsD3voX7AwAAAAASUVORK5CYII=
}

set bddview_icon_Print {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAADKElEQVRYw+2XTUxUVxTHf+e+5wzT
SDEhGUWxMcavFBsXTU1KHYV0mrRRk67cddOFDMSk2qiAC0O6AEdtrQv5cNN93TQxXbRSO3EIJtoP
o7UNBRZSW1siRCjpMMx793QBTTCZGYYZoBvO6uXc887533Pvued/hDxS19SxzVMqKEFcYSLR2TqU
a11yLRxoitdby02WQBRb39fdmsgKMOdPlo1zn/2i/FRUYGE3UGswm3JmaCEnovL5rZ7Tl4sBsL/h
/AcqWpvPxvA/yyqAVQCrAASgLtaxxcecEeU1FV6cW1sLhIExYKJI/xVAJTAKTM2+K0wCdzzXae+/
cvKR1B1t3+Ub5w5QDkwDqWXedAgoA520IntdX5yLs8G1acPY1qvXrh3xlytyJBY/ZsD4iiciV4xy
2QXeQPk12dPStewHrrxtgb6e5kORWPw4sF8isbgCo6J8sUJXDlBUeBcI/9eMwiocZQVSkLUbGscd
2nn4cGYly2/g+vU11ve2uQBiTLp8w8aa+Qbjg4PfDX97YzNKsMSsp7fWRx9Xbt/x6nNqYx7i5+ED
T4cHp9TqeuBJCaUZQqkaGx76pXL7juIICcp7yZ7mb4oqu4b4mwi9eTljoc5qYxfCRry1hdim/PSf
319t+6cg0lqI0b6Gc3sE+wNqCuodL5jQfWDPkgFwnelh3w99LBRI0Q39BdN2IKVoIJ9RorNtCji9
tC+CBoCMC/xoM5m9E7+NPKjY/NIruQj2gYbzUd/IuufUlr+rxrfcWGz/eDYyct9mMjUg92Rf44Wo
qP0a8JxAYMA4zgyAl06H1dpqlKiFCSPczQ7NvNXXdao3XxWIMY/dYHAUwPo24M+kd84OTTbq9nWd
6o3Ezr0DcsmfmdmdbStrqlL3vL9C78/xg3kzg46Xu8Hkgum2tjqTSlXP0/wsxpy41dl60wVIdrd8
Bbz8+olPQs50ugxArDmLcBwg0dbmAZ+VcOCfqrEfAfhlwenblz5MLTgbRhrj7SitKF8K/F5k69mE
cBChI9nVfGZxZWjlCaIgHNTSr/wfi56OQaUudrHGE6+kZuSqm050n3wIknUf/wIW1yLCkjuphAAA
AABJRU5ErkJggg==
}

set bddview_icon_Pan {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAC9ElEQVRYw+1XTU8TURQ9900RxLXu
4EcgFBd0ipiUtqYdWhVQE1f+B/kJ8htwhWAgSNspIVNIpJnWREuQxPgLCDt3Blwg6bsu+sF0PtqZ
yk7epu27r++cue/MufcB//ug1pdksfKegRcARJfVNSjnUSOZvHALT5fLQ8NnislAuAumZGC9lFZf
wQrWE7yxKFz/c+eeV/jWL3GXgYkeDy0IeNn6EbIGbGhHAD51ZoCO9ueip147789FT+N6dYFI3reF
HgE07obVPoJEscK2dF8yaKGUiuT/5Yxn9cpjQdgGMGidN9IqwSPlxyBcgjFA4M34TjXTL3h8p5oR
AnkAgyBcAjh2nIfL/w6kRAbABRgDxLwV1yuLgcF1M0vgTTAGWtkEcOCHAPY0dVcyngC4AKAQYS0I
ibhuZknQhhXc6yg9Vd8viSDg6PXa7WnqLhM9b56fQgKrcd3MXhd4JwFCDQA3P9ujlIrkWfJiW5iC
NtxIOMAlL9rBmemwYTn44iSgnEcRqo8aKfWjffOSFs3ZSSSKnxNXajeTDnAtmnPuE9lCqD6K0PlD
hw8EFhewbaTVpw0br24zONsNvC8NuGUCLDQC5RhyuTVfJ14mUA4stCDgNyOwBtqCk3gtBb3dS6mH
ADC7UwkLyUsgZcVITxlB9hPB7ZUKIMooTG/am0heAlEGJPVuPtGVwHS5PBQrmCMBTObdVR6VlV4+
0RqxgjkyXS4POQgMnymmIugkrlef+TEZa6qN9JThx6ySRXNeEXRy+0yUHQSabRQRcdheUv2ZjNOs
7KWcQRMN3dEDXxpwK6nd3vOSFs0FLeXiOgtLP1VUeLVRRPQhKLgnCYFVr87KjcCMtY3qty/sKOXN
9g7AjB8CY/0+uUMTqUjeqgkAY/Y1Ie87AH8n8GSiaE5eTYlvJS2y1Q00WTTnuQOIAeCHrS13JSA7
M0LjAMY7rwXMsYL51etuECuYIwza8GHx0s0H1q0Bj7TUlFu/f3pFGzGu9QRnXrspw63xF5QpqAmJ
5fMuAAAAAElFTkSuQmCC
}

set bddview_icon_Zoom {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAADhElEQVRYw72WTUwTURDH//No+TDl
otUYuRqDFxPjRzxIoTEqbNmSIOFoohejxgiYGMALB1FMjBoI8eTFcFAkKq3SNmoKbfVA4sWDUW5+
pNjEkx+A7e6OB17Lgk13S7Fz2nn78ub3ZubNDKGAtEzFtpKONgZUMHaCUAdGJQhJBn0mg8PkoKdT
voY5rFMo3+Lx8JvNIp25DNBFANWWhxBeGgYuhds870oG8E1Oew0SjwBsybN/CYAGwJXnnwagJ+T3
jBQDIFa5PBg/aQgRMRlnAj1mps60lnaH/J6akN9Tq1ctugA+wkyjABbkXgeAYSUwMwpmKtoDvslp
ryFEBAynXHonBJ1+3trwttABrcE3dQbrwwxuR46a+8Jq45BtADUYdWtc8SF3c0Zk8Y/eMd3p/WXr
GszU/Cw+SIw+uWIQs3eqrTFmKwQaV1wxuf09nJWdto0vZyGHVU8/ge5lz2Wim3ZCIY49fr0NwNns
XYSgkyHl0I/1PKmFJa0L4JRUDyjBhM8SQDg0FUDVcjzoiVXMC8my12jIlAsdNkJAalYxGA9Qomgs
xgEYMpd8VmEQBOzKKk6hRUsFeNF2OAngo1TdR5682myVhHXZIhNUvd+xIUJfc5eqqKyzAnDI7ww2
SIg4vaIYVYUBCPPyu7ZpPOraCAA2OHdrJzlShQEYn7JKTbU4WKpx/2SiFkT1Uv2dTFbPWwFEVgqa
aC8VICN0JddBGYm3Z/ZnCr8CBz01xe7U0cnEjvUaHxhgAab+3HkCE5Z1QA4TWYhNTuKRYrqZWWb3
JboA7JH+TC249DFbvUAnvU/2czC4vflZfLBY40og3srMN3LhBPVOe71LtttxSyB2AcCwqcPdX/xj
nLdqSgMDLGb3JbqkcYe0PhfyN9SDiIuaiJTAzCiDzpkeVAqgISODh5ETnlXZ3DQeddXUCJ+M+Z48
B9+e8nt6ih7JmoMzvcQ0uGZaMmR5/QIgLavn7n/mRcYcaKW024EQaxfCauMQMXsJmF2zbzeAYwBa
AexdbZxTDJwK+RvqCbi9woNuJRC7VfRUnJ1ylGDCx8QdYPgAuNfs+A1GggQmFlz6mDnhlEDsFgPd
djxh+7mpwajbIGzXuGJTJcS3ZLJ6vlCRsQtB+I+iBOLXGdxrWroT8nu6ywZgB+K/A1hBlAWgEETZ
APIlJoBrZQXIA/Gz7ADLfWfmKkAXiXH3LzCCdaoobeo3AAAAAElFTkSuQmCC
}

set bddview_icon_Edit {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAADJUlEQVRYw+2XPWgUURDHf7MXckTF
WBpiUPETG5FAxGY7yZ7gJhFBLEQQgx+NlWBjp2iRxsZOC4kKVrkV3RMTyb1GkwiiaAqJEbwoaiVe
4scluWdxb+NjzYF3WbHJwLJ38z7+82bm/WcWluU/iyx1g32BEg0rtdbNgGjQCxsL0b8yUMx1udPx
9c5SDdCwFhgUkSkRKTgiU07l95Rg3sIHEe51DgyvSdwAoBnYbc4aPeXo7JZsEMfZE1/ckIAB74Hz
QA8wZ3QrgE1Ak+0s+dOopRnQOaBSoe8WgQvmIROoBqAXuPo3e9QdgkxWtYiw1wvybQu6QK0Gzljg
c8B3K19IxAOZQG0Dbgq0g4x7WdUtwkdz8j4zrQg8Ar4AR6tdOacO8O3ATaDdqHaIcB84a4F/A/pB
HzMJWdUDTh3g/QZcAzNmaLNJRIBpoD/03dMgKSCVSA7EwAEmgYvmHck8MBj67gmL6OYt1puvywAT
cxv8DXAq9N1LWuvDQMHoU0Crl1XbqPh+RsMLoAS8K8NEzVTsBapF4K7l9kkD/nBhTjbfISJ3gPVG
9Vxr9ue63IIXqDSwizKfc93uZE23oDPIp0DvBGm3kuuaDQ5Qmi2NpRvT54DbRrVRhAPAlZzv/gSe
VMNoWKSwrNXQLPA+9N1iJqteIYwDO4CVwHEvq4ZyXe5otC7dmO6wkhDgNTBUczXMBGoVMGi4/Xzo
uxG7bQHum2wHKGjNwdJseSzd6HQAN4CtZuyZCdFIzUxoSmpUWHqsoY+WewHaRLiTbnQOAddj4Cf/
FnyRHBDbI3MWvfZaLp432b4+ZlQEPloLtzix2q5jIWmI0es0lRsxFttnATwTKKnbAAtfm5LaG6PX
W6Hv9mjNEeA58BV4amI+ChD6rq67JfOy+VYRmTIG/LDqefE3vUZzVZu5akOh775MpCf0svl1IlKI
zZkD7lUKi6RMgzfzwHdnkmhKHarEwJLZSkmVPhOOywK93oBKJ2FAnAnLi8xpiuq55baSFkaAx0kb
UAQUsKGKNyJ5h/Dpn3wXdA4MrxHH2RM1kNpu701JLcPEA999u/xZtSxJyC9KxhobQBQu+gAAAABJ
RU5ErkJggg==
}

set bddview_icon_Info {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAi0lEQVRYw+2VoRWDQBAFhzwKwFBC
Ukb8iqSXUERselmBitoeSBFUgUFgzrB5d+aPGrU35r8DIUQC8xjNY8zcuCQen4AVWHevGwC8C14t
4Hfw5eyRPhFwBz67v7SIVjM087BWM3wCMzCbx6PFCq4Hv7UI+AsKUIACMgFdwasFfAte9S8YzGPQ
ryhEhg00ZRzY+bxIcwAAAABJRU5ErkJggg==
}

set bddview_icon_Tex {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAD00lEQVRYw+2XX2xTdRTHv+d3266l
o1EUAhiIAZQQDNE4hcjauYS4/h+EzMQHIRFDEKL4QoohJj6pkQfMQiKgJiYaVIi4e7uud0DScFsW
YzQxgT3wAiJLNpM5B+3azvb+jg+b0HW69Q91PniS+/D73l/u/fzO7/zOOT9Cmfm1ZJDBXwJoRu32
bjzsOVLJRCoXfKpxCoS1UtJ7tfxZCD4JYA0RXusLeU7MN99SLjCBCDTSv919sRYAv2akGQAzjvs1
Y7gv7FHnBEbjTGHgtE9LbVkoAABYBEitQ02sWygAAFgqSNE69IElCwUAABuUSbPn+UTC3nAABhf+
Vid2O9LKW/OeglqsI2o8C5NcQkhZENhlLXIApLgAOcygUQAg4jcYWFkVgD9qvCrBywXTEgbWgxGH
wKp4yBOZsTpbblCZdHwFYPBisO2wX005JUm3HnKfABEDgE9LbgcY1XlAKBf0wNabHaoRIJBN73Qf
D/Rees6rJrtKpy2+PXyuYLW9PKkoA17VOM2QW/Sw51hFiWuul32BrTfLtViwbYBIHiNrWtM73WcF
zPG8Y01Tz472cWJ5kAj9OZf5ScWZs7ZIo4l7PuQr0VBLduprtBSMlD1j2dtYAABkOnd6tUt72LTs
BgBvNNVqEt+C1baHmPf7Y8nH7xuAQuwUxM4ZTlAmvtHDbZ8KITW/mvQS5AEbN/8Y921OE7ifTT7l
/TbxaNXFqNy6zgzasnJ0SAr+PNCbfDAWdP8OuldFn/nBc+37J1OueMjz0t3YAQ5W6sl5Ac6+uPEP
AAN/jQO9iSckI4/i4t1e1ch+h+RTUpgf17qVVSeiWLD9KoBNJdIX9SSxf6sW/A9QbQxwqz9qnLmv
VZJ5M4DzlQbhamas/i9swagQtLb8AfNPJHnTDA10AYzXSaH1AEYA/AJTWQfgSD0ATillxGTeGQu6
r0uJbVLKCIhWWkj5WZq8QkoZkVJGAH6MiA78OuS4AdCHYO42F2VGAN5fTx5wALSXGCkAR4l4GzN1
lXQ/DxHo6enhcgbbl63I7msyzZM5R5MQeXEYxI807BTonW1aPOxpiYc9LQBuTMtv9uxoH3fdGbpN
NPfq627JvGqyi4jfn75jrZpqePijF85dXobiA+NMY93E/E7DAASQBuH61DGDAqDQnH+4O9009nbG
MnbLCsfRIrKvAP98ouoC6Ot06wB0APCrxgcgXM47chZi3gcga885PpuwTxxi0NcVAxCDZ19Z0erT
DOaSnrJA8o5PM0racQDAoSJn72oZ+2+Tpfdf4tld6awglIAKINOAnJNhwT3l4p+Sm4Qnm54iCQAA
AABJRU5ErkJggg==
}

set bddview_icon_FullScreen {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAACCUlEQVRYw+2XsWvUUBzHPy8JVxeP
IuW8FxRO6BYci6tQhC6im4uzODkouDg6ii6tSxdtcRCRbi5aOrgVF/8AkUOveYl6h8MhJ5fk5+BF
ctc7vVyaLt53ennk5fN9v/fl9wj871LZB9d1L4nIDeDcPxcqteX7/nqtVjvtOM4bETk/JbMH3DPG
PAJwMvCrIrIzamqCEmANwLKstRxwgBMicgcYNpAkyV31Ww+A13/7goj89H3/G4DjOLtRFH1QSi1P
XXalFg4dgdb6K7Bk2/aZVqt1kPcsG43GYpIkQ9Xr9XoV27afAxdHXm8bY5aGKpCaUUrJLGFqNpvf
s8+e51X6/f6TMfAhWWUk2/O8SrvdfglcyUzvHIsBz/MqnU7nhVLqcmZ6E7hduoEUPrLzTWPMzUlH
ax0HHJBSMzAr/EgMFIEXNjAu7SKyMQEu48ZWUXg27SKyEQTBrXE7930/AD4Nes1+Ou/MyHfywAfq
J0myopRasW17r5ABrfU1IA8cgDAMvwCvjiIDn1OYiKxPA8/I1lpfL1QBY8zber1+QUROhmG4N+06
13XPish74AfwrEgGCILgXd41lmUtxHF8amCgvMsol6m5gbmBuYFMOz0AiKJodcp/g9ysOI5XB5dR
61AnVEptAQ+Bba31dpm7FpGnf/pyOuh2u/vVarUHLAOLJbE/AveNMY+Za6BfeULw5v++vscAAAAA
SUVORK5CYII=
}

set bddview_icon_NoFullScreen {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAACBElEQVRYw+2Xv4sTQRTHv9/JLuZ+
EEEMu0MwhQbBSrCyEQUbKy3srhIEbUVQ4ZATBblCC7Gz8eCEK46rRMTKztrSP2BlZsOFSxGv2T3m
2SSyl8S4t7dbmdfMr535fPfNzHsM8L8bM3VPa/1ERO6QPDc2NmEi8iyO4xd5Qc1mc7lWq22RvEry
gTFm45CAMAzXSD7PuV6SpunpXq83yAv3PO8zgCvDru/W2ksA4P1xBXl3WK54nvdl1oKDwcD1+/2i
cIjI1sQWaK0dADrnlrvd7v6sRVut1kXn3DbJPefczTiOd/PCAaxba1dHDVXk4IjIfQDnReQyydtF
4YUFOOfqmebC+HgQBEu+73/Mwkm+GocfOgNlWRAES0qpTyJyLQs3xjye9r2qAg4gF7xUAX+Bv54F
L03ADPijf80tQ8BiUXhZh/ApgOytWDfGrOadXIYH6tk/n3bVqvbAyPZEpKG1fjclcPV9338bRZGp
UsApAPemplwSaZpeAHCrrC34ceS8T+4DQKfTOaG1fnisZATAC8NwRSmlc4buX0mSfKjX6wcisgtg
wVqrjrMFB3Ecbx510jBBLQKQSkJxEZsLmAuYC8jGgZ8AziilbrTb7a9VwJIkuT6sRhMCSG6IyBqA
nTRNq3mGkaPk9H7UV8s8Nr41Gg0BcBbAyYo8HpF8Y619CcBhbgB+A/M7yzyOGsNUAAAAAElFTkSu
QmCC
}

set bddview_icon_Iconify {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAkUlEQVRYw+3OuxGDMBCE4b0hVwGq
xz05hcxtOaAWKVdCdmQMjLHQ4+yE3fjm/g/gOI7jOI7j7j7x3j9V9fWL56o6xhin3M2QUpqdcwuA
x7/jADAAgDWiNL4BLBE18QPAAlEb/wD0IFrip4AWRGv8K6AG0RPPAkoQvfFLQA5hES8CnCGs4sWA
HUJE5B1CMIkDwAo3EogqC2dPJAAAAABJRU5ErkJggg==
}

set bddview_icon_Exit {
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAB70lEQVRYw+2Xv24TQRCHvzkfL4CU
RFDQ2efDtCBRgZ3XMTHiRfJHIJ4CngDH0FLj+GJ3KZBiKOjtu6GIHS3x3nlvNxEFTLVa7c7vm9vZ
mT34103WAwXJaPcFHQA7wGlOPOgwvggRGNN51GB5AnSBH4octcneCegfABnJAXB8Y/+8INp/zOSb
j3hGkgAj4IE5r8irlOwtQGTMv7b42I0ohmekT25L/CpqPViPTYD7Jb52G+SfpjRTV/EpzVTQLzbx
le3YAEZlDhXZU2TkAjGlmSoyUmSvYtlwAyAnHgh6GQLhIi7oZU78ZgOgw/hCkRfA9yqIguizLScy
kqQgGm6JfL4k3jdvlpQkzynwsMqReTuqEs7ck9PodTgbW+uAL0REsfAVLwWoc57ro6laI2i3xWxS
WQkDvgQ+kTsBBEJsFXcC8IRwEncGqAnhLH6zEtbqntWLVG7VoeM99+qicgfitSDkjsSdIf56IYrK
IndtLCENrKoZ1artPg3MChDS1XwhxHy9xiy+hpyna94sufds/Sa4zoEGy5MQcYAWs4mg3W0vq5jF
oS0JX4aI14To2QB+VmW7i7gJseV2zDcAFDkKbSymtTk/B3o2CEGPNwCufpe0D8yAX8CHnPipj7gJ
kRM/F/TjyudM0H6L6Xv+28p+A8AyIalgLZ6KAAAAAElFTkSuQmCC
}

image create photo icon.up.small
icon.up.small put $bddview_icon_GoUp

image create photo icon.left.small
icon.left.small put $bddview_icon_GoLeft

image create photo icon.right.small
icon.right.small put $bddview_icon_GoRight

image create photo icon.down.small
icon.down.small put $bddview_icon_GoDown

image create photo icon.save.small
icon.save.small put $bddview_icon_Save

image create photo icon.print.small
icon.print.small put $bddview_icon_Print

image create photo icon.pan.small
icon.pan.small put $bddview_icon_Pan

image create photo icon.zoom.small
icon.zoom.small put $bddview_icon_Zoom

image create photo icon.edit.small
icon.edit.small put $bddview_icon_Edit

image create photo icon.info.small
icon.info.small put $bddview_icon_Info

image create photo icon.tex.small
icon.tex.small put $bddview_icon_Tex

image create photo icon.iconify.small
icon.iconify.small put $bddview_icon_Iconify

image create photo icon.fullscreen.small
icon.fullscreen.small put $bddview_icon_FullScreen
set bddview_icon_FullScreenOn icon.fullscreen.small

image create photo icon.nofullscreen.small
icon.nofullscreen.small put $bddview_icon_NoFullScreen
set bddview_icon_FullScreenOff icon.nofullscreen.small

image create photo icon.exit.small
icon.exit.small put $bddview_icon_Exit

# ############################################
# INITIALIZATION
# ############################################

#DPI - adjust for defaults in different OS
#used for print and export
if {($OS == "unix") && !($OS1 == "Darwin")} {
  set DPI 75
} elseif {($OS == "unix") && ($OS1 == "Darwin")} {
  set DPI 75
} elseif {$OS == "windows"} {
  set DPI 96
} else {
  set DPI 100
}

#STATE: pan, zoom, edit, info
set STATE ""

#STATUSBAR: text in statusbar
set STATUSBAR ""

#XWIN,YWIN: initial size of canvas
set XWIN 1320
set YWIN 800

#ZOOM: initial 16
set ZOOM 16
set DOUBLELINE [expr $ZOOM/4.0]
set ARROWSIZE [expr round(2*sqrt($ZOOM))]
set INVSIZE [expr $ZOOM/8.0 + 1]
set LABELOFFSETX 0
set LABELOFFSETY 0
set GRID [expr $ZOOM * 8]

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
set GRIDRESOLUTION 36
set GRIDOFFSETX 0
set GRIDOFFSETY 0

#ARROWS
set ARROWSON 0

#BENDS
set BENDSON 0

#BDD
set BDDNAME ""
array unset BDD
set BDDVARIABLES ""
set BDDNODES ""
set TERMINALS ""

#these are used internal
#LISTF is sometimes used as a list of left edges, sometimes it is a list of elements reachable from a given item
#LISTT is sometimes used as a list of right edges, sometimes it is a list of elements pointing to a given item
set SELECTED ""
set TERMSELECTED ""
set LISTF ""
set LISTT ""
set DEPTH ""
set TERMDEPTH 0

# ############################################
# GUI
# ############################################

set COLORFRAME "#E8E8E8"
set COLORMENU "#FAF8F4"
set COLORBUTTON "#49B2C2"
set COLORBG "#FFFACA"
set COLORFG "#000737"

set COLORGRID "#90A8CF"
set COLORNODE "#F8F8F0"
set COLORLEFT "#006828"
set COLORRIGHT "#002868"

set FONTFAMILYLABEL TkFixedFont
set FONTSIZELABEL "(\$ZOOM - 1)*75/\$DPI";
set FONTFAMILYTAG TkFixedFont
set FONTSIZETAG "(\$ZOOM - 1)*60/\$DPI";
set FONTFAMILYINFO TkSmallCaptionFont
set FONTSIZEINFO 10

wm title . $TITLE
wm iconname . BDDview

# CREATE AND PREPARE $mainframe

set mainwin ""
set mainframe [MainFrame .mainframe -bg $COLORMENU -textvariable STATUSBAR]
$mainframe.status.label configure -fg black
$mainframe showstatusbar status

# CREATE TOOLBAR

set toolbarsize "small"
set toolbar [$mainframe addtoolbar]
$toolbar configure -bg $COLORMENU -relief flat

set bb0 [ButtonBox $toolbar.bb0 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb0 add -image icon.save.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Save As" -command {savefile}
$bb0 add -image icon.print.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Print to File" -command {printfile}

pack $bb0 -side left -anchor w

set sep1 [Separator $toolbar.sep1 -bg $COLORMENU -orient vertical]
pack $sep1 -side left -fill y -padx 4 -anchor w

set bb1 [ButtonBox $toolbar.bb1 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb1 add -image icon.pan.$toolbarsize \
    -takefocus 0 -relief flat -background $COLORMENU -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -helptext "Pan" -command {state pan $mainwin $bb1}
$bb1 add -image icon.zoom.$toolbarsize \
    -takefocus 0 -relief flat -background $COLORMENU -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -helptext "Zoom" -command {state zoom $mainwin $bb1}
$bb1 add -image icon.edit.$toolbarsize \
    -takefocus 0 -relief flat -background $COLORMENU -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0  \
    -helptext "Edit" -command {state edit $mainwin $bb1}
$bb1 add -image icon.info.$toolbarsize \
    -takefocus 0 -relief flat -background $COLORMENU -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -helptext "Info" -command {state info $mainwin $bb1}

pack $bb1 -side left -anchor w

set sep2 [Separator $toolbar.sep2 -bg $COLORMENU -orient vertical]
pack $sep2 -side left -fill y -padx 4 -anchor w

set bb2 [ButtonBox $toolbar.bb2 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb2 add -image icon.up.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Up" -command {compass "n" $mainwin}
$bb2 add -image icon.left.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Left" -command {compass "w" $mainwin}
$bb2 add -image icon.right.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Right" -command {compass "e" $mainwin}
$bb2 add -image icon.down.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Down" -command {compass "s" $mainwin}

pack $bb2 -side left -anchor w

set sep3 [Separator $toolbar.sep3 -bg $COLORMENU -orient vertical]
pack $sep3 -side left -fill y -padx 4 -anchor w

set bb3 [ButtonBox $toolbar.bb3 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb3 add -text "Bends On/Off" -fg black -bg $COLORFRAME \
    -takefocus 0 -relief flat -padx 4 -pady 4 \
    -activebackground $COLORFRAME -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -font [list TkHeadingFont 10] \
    -helptext "Arrows On/Off" -command {bendsOnOff $mainwin $bb3}
$bb3 add -text "Arrows On/Off" -fg black -bg $COLORFRAME \
    -takefocus 0 -relief flat -padx 4 -pady 4 \
    -activebackground $COLORFRAME -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -font [list TkHeadingFont 10] \
    -helptext "Arrows On/Off" -command {arrowsOnOff $mainwin $bb3}
$bb3 add -text "Grid On/Off" -fg black -bg $COLORFRAME \
    -takefocus 0 -relief flat -padx 4 -pady 4 \
    -activebackground $COLORFRAME -highlightbackground $COLORBUTTON \
    -borderwidth 2 -highlightthickness 0 \
    -font [list TkHeadingFont 10] \
    -helptext "Grid On/Off" -command {gridOnOff $mainwin $bb3}
$bb3 add -text "Grid +" -fg black -bg $COLORFRAME \
    -takefocus 0 -relief flat -padx 4 -pady 4 \
    -activebackground $COLORFRAME \
    -borderwidth 0 -highlightthickness 0 \
    -font [list TkHeadingFont 10] \
    -helptext "Grid +" -command {gridPlus $mainwin}
$bb3 add -text "Grid -" -fg black -bg $COLORFRAME \
    -takefocus 0 -relief flat -padx 4 -pady 4 \
    -activebackground $COLORFRAME \
    -borderwidth 0 -highlightthickness 0 \
    -font [list TkHeadingFont 10] \
    -helptext "Grid -" -command {gridMinus $mainwin}

pack $bb3 -side left -anchor w

set sep4 [Separator $toolbar.sep4 -bg $COLORMENU -orient vertical]
pack $sep4 -side left -fill y -padx 4 -anchor w

set bb4 [ButtonBox $toolbar.bb4 -bg $COLORMENU -homogeneous 0 -spacing 4]
$bb4 add -image icon.tex.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Export to TeX/TikZ" -command {exporttex}

pack $bb4 -side left -anchor w

set bb5 [ButtonBox $toolbar.bb5 -bg $COLORMENU -homogeneous 0 -spacing 2]
$bb5 add -image icon.fullscreen.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "FullScreen" -command {fullscreen}

pack $bb5 -side right -anchor w

# CREATE VERTICAL AND HORIZONTAL PANED WINDOW
# bddview WILL ADD ONLY ONE WINDOW TO THE CREATED FORM
# BUT, USER CAN ADD NEW WINDOWS TO IT

set verticalwindow [panedwindow $mainframe.verticalwindow -orient vertical -relief flat -bg $COLORBG]
set horizontalwindow [panedwindow $mainframe.verticalwindow.horizontalwindow -orient horizontal -relief flat -bg $COLORBG]

# CREATE MAIN CANVAS

set mainwin [canvas $mainframe.verticalwindow.horizontalwindow.scrolledcanvas -relief flat -highlightthickness 1 -highlightcolor $COLORMENU -bg $COLORBG]

# ENABLE MOUSE WHEEL

# necessary for GNU/Linux, not needed on MS Windows
bind $mainwin <4> "%W yview scroll -5 units"
bind $mainwin <5> "%W yview scroll  5 units"
bind $mainwin <Control-4> {
  zoomIn $mainwin [$mainwin canvasx 0] [$mainwin canvasy 0]
}
bind $mainwin <Control-5> {
  zoomOut $mainwin [$mainwin canvasx 0] [$mainwin canvasy 0]
}

# necessary for MS Windows, not needed on GNU/Linux
bind $mainwin <MouseWheel> {%W yview scroll [expr {-%D/120}] units}
bind $mainwin <Control-MouseWheel> {
  if {%D > 0} {zoomIn $mainwin [$mainwin canvasx 0] [$mainwin canvasy 0]}
  if {%D < 0} {zoomOut $mainwin [$mainwin canvasx 0] [$mainwin canvasy 0]}
}

# INITIAL WINDOW POSITION AND SIZE

set bddview__reqX [expr {([winfo screenwidth .]-$XWIN)/2}]
set bddview__reqY [expr {([winfo screenheight .]-$YWIN)/2}]
if {$bddview__reqX < 0} {set bddview__reqX 0}
if {$bddview__reqY < 0} {set bddview__reqY 0}
wm geometry . [expr $XWIN]x[expr $YWIN]+$bddview__reqX+$bddview__reqY
bind . <F11> {fullscreen}

if {($OS == "unix")} {
  wm attributes . -zoomed yes
}
if {($OS == "windows") || ($OS == "Darwin")} {
  wm state . zoomed
}

# SHOW $mainframe

$mainframe.verticalwindow.horizontalwindow add $mainwin -stretch always
$mainframe.verticalwindow add $mainframe.verticalwindow.horizontalwindow -stretch always
pack $mainframe.verticalwindow -in [$mainframe getframe] -fill both -expand yes
pack $mainframe -fill both -expand yes

# ############################################
# WINDOW ACTIONS
# ############################################

# Fullscreen window
proc fullscreen {} {
  global OS
  global COLORMENU
  global bb5
  global toolbarsize
  global bddview_icon_FullScreenOn
  global bddview_icon_FullScreenOff

  if {($OS == "windows") || ($OS == "Darwin")} {
    set oldstate [wm state .]
    wm state . normal
    wm withdraw .
  }

  if {[wm attributes . -fullscreen]} {
    $bb5 delete 2
    $bb5 delete 1
    $bb5 delete 0
    $bb5 add -image icon.fullscreen.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "FullScreen" -command {fullscreen}
    wm attributes . -fullscreen false
  } else {
    $bb5 delete 0
    $bb5 add -image icon.iconify.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Minimize" -command {iconify}
    $bb5 add -image icon.nofullscreen.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Exit FullScreen" -command {fullscreen}
    $bb5 add -image icon.exit.$toolbarsize \
    -takefocus 0 -relief flat -borderwidth 0 \
    -background $COLORMENU -helptext "Exit" -command {exit}
    wm attributes . -fullscreen true
  }

  if {($OS == "windows") || ($OS == "Darwin")} {
    wm state . $oldstate
    wm deiconify .
  }
}

# Minimize window
proc iconify {} {
  wm iconify .
}

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

  if {$STATE == "edit"} {
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

  if {$STATE == "edit"} {
    fixedges $mainwin
    boxit $mainwin
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

  if {$STATE == "edit"} {
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

  if {$STATE == "edit"} {
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

  if {$STATE == "edit"} {
    fixedges $mainwin
    boxit $mainwin
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

  if {$STATE == "edit"} {
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

proc drawbox {win} {
  global COLORFG
  global ZOOM
  global XMAX
  global YMAX

  $win create rectangle \
       0 0 [expr $XMAX*$ZOOM] [expr $YMAX*$ZOOM] \
       -outline $COLORFG -state normal \
       -tags [list line box]
}

proc removebox {win} {
  $win delete box
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

proc compass {direction win} {
  global ZOOM
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

proc zoomIn {win px py} {
    global ZOOM
    global DPI
    global FONTFAMILYLABEL
    global FONTSIZELABEL
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
      $win itemconfigure tekst -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]]
      $win itemconfigure "left||right" \
        -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \

      fixedges $win
      boxit $win

      set LISTF ""
      set LISTT ""

      centerpoint $win [expr 2 * $px] [expr 2 * $py]

    }
}

proc zoomOut {win px py} {
    global ZOOM
    global DPI
    global FONTFAMILYLABEL
    global FONTSIZELABEL
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
      $win itemconfigure tekst -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]]
      $win itemconfigure "left||right" \
        -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \

      fixedges $win
      boxit $win

      set LISTF ""
      set LISTT ""

      centerpoint $win [expr $px/2.0] [expr $py/2.0]

    }
}

proc bendsOnOff {win bb} {
  global OS
  global BENDSON
  global LISTF
  global LISTT

  if {$BENDSON == 1} {
    if {$OS == "windows"} {
      $bb itemconfigure 0 -relief flat
    } else {
      $bb itemconfigure 0 -bd 2 -highlightthickness 0
    }
    set BENDSON 0
  } else {
    if {$OS == "windows"} {
      $bb itemconfigure 0 -relief solid
    } else {
      $bb itemconfigure 0 -bd 0 -highlightthickness 2
    }
    set BENDSON 1
  }
  set LISTF [$win find withtag left]
  set LISTT [$win find withtag right]

  if {$LISTF != ""} {
    fixedges $win
    boxit $win
  }

  set LISTF ""
  set LISTT ""
}

proc noarrows {win} {
  $win itemconfigure "left||right" -arrow none
}

proc showarrows {win} {
  $win itemconfigure "left||right" -arrow last
}

proc arrowsOnOff {win bb} {
  global OS
  global ARROWSON

  if {$ARROWSON == 1} {
    if {$OS == "windows"} {
      $bb itemconfigure 1 -relief flat
    } else {
      $bb itemconfigure 1 -bd 2 -highlightthickness 0
    }
    noarrows $win
    set ARROWSON 0
  } else {
    if {$OS == "windows"} {
      $bb itemconfigure 1 -relief solid
    } else {
      $bb itemconfigure 1 -bd 0 -highlightthickness 2
    }
    showarrows $win
    set ARROWSON 1
  }
}

proc drawgrid {win} {
  global COLORGRID
  global GRIDRESOLUTION
  global GRIDOFFSETX
  global GRIDOFFSETY
  global ZOOM
  global XMAX
  global YMAX
  global GRID

  set igrid [expr round(($GRIDRESOLUTION*$ZOOM*$ZOOM)/($GRID*2))]

  for {set i $GRIDOFFSETY} {$i < [expr $YMAX*$ZOOM]} {set i [expr $i + $igrid]} {
    $win create line 0 $i [expr $XMAX*$ZOOM] $i \
      -fill $COLORGRID -dash "2 4" -state normal \
      -tags [list line grid]
  }

  for {set i $GRIDOFFSETX} {$i < [expr $XMAX*$ZOOM]} {set i [expr $i + $igrid]} {
    $win create line $i 0 $i [expr $YMAX*$ZOOM] \
      -fill $COLORGRID -dash "2 4" -state normal \
      -tags [list line grid]
  }

  $win lower grid
}

proc removegrid {win} {
  $win delete grid
}

proc gridOnOff {win bb} {
  global OS
  global GRIDON
  global COLORFRAME
  global COLORBUTTON

  if {$GRIDON == 1} {
    if {$OS == "windows"} {
      $bb itemconfigure 2 -relief flat
    } else {
      $bb itemconfigure 2 -bd 2 -highlightthickness 0
    }
    removegrid $win
    set GRIDON 0
  } else {
    if {$OS == "windows"} {
      $bb itemconfigure 2 -relief solid
    } else {
      $bb itemconfigure 2 -bd 0 -highlightthickness 2
    }
    removegrid $win
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

# ############################################
# DRAWING PROCEDURES
# ############################################

proc addTerminal {win num label x y} {
    global ZOOM
    global DPI
    global FONTFAMILYLABEL
    global FONTSIZELABEL
    global COLORNODE
    global BDDNODES
    global TERMINALS

    lappend BDDNODES [list $num $label "t"]
    lappend TERMINALS "n$num"

    set len [string length $label]
    set sizex [expr 8+$len*$ZOOM/2]
    set sizey [expr 8+$ZOOM/2]

    if {$label == "0"} {
      set zero "zero"
    } else {
      set zero ""
    }

    $win create rectangle \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline black -activeoutline black -disabledoutline red -fill $COLORNODE -state normal \
       -tags [list "n$num" outline node terminal $zero]

    $win create text $x $y \
       -text $label \
       -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]] \
       -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node terminal $zero]
}

proc addVariable {label} {
  global BDDVARIABLES

  lappend BDDVARIABLES $label
}

proc addNode {win num label x y} {
    global ZOOM
    global DPI
    global FONTFAMILYLABEL
    global FONTSIZELABEL
    global COLORNODE
    global BDDNODES

    lappend BDDNODES [list $num $label "n"]

    set len [string length $label]
    set sizex [expr 9+$len*$ZOOM/2]
    set sizey [expr 7+$ZOOM/2]

    $win create oval \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline black -activeoutline black -disabledoutline red -fill $COLORNODE -state normal \
       -tags [list "n$num" outline node internal]

    $win create text $x $y \
       -text $label \
       -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]] \
       -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node internal]
}

proc addLabel {win num label x y} {
    global ZOOM
    global DPI
    global GRIDOFFSETX
    global GRIDOFFSETY
    global GRIDRESOLUTION
    global FONTFAMILYLABEL
    global FONTSIZELABEL
    global COLORBG
    global BDDNAME
    global BDDNODES

    set BDDNAME $label
    lappend BDDNODES [list $num $label "l"]

    set len [string length $label]
    set sizex [expr 9+$len*$ZOOM/2]
    set sizey [expr 7+$ZOOM/2]

    if {$x > 0} {
      set GRIDOFFSETX [expr round($x) % round($GRIDRESOLUTION)]
    }
    if {$y > 0} {
      set GRIDOFFSETY [expr round($y) % round($GRIDRESOLUTION)]
    }

    $win create oval \
       [expr $x-$sizex] [expr $y-$sizey] \
       [expr $x+$sizex] [expr $y+$sizey] \
       -outline $COLORBG -fill $COLORBG -state normal \
       -tags [list "n$num" outline node label]

    $win create text $x $y\
       -text $label \
       -font [list -family $FONTFAMILYLABEL -size [expr $FONTSIZELABEL]] \
       -anchor center \
       -fill black -activefill black -disabledfill red -state normal \
       -tags [list "n$num" tekst node label]
}

proc addPoint {win x y tag} {
    $win create oval \
       [expr $x] [expr $y] [expr $x+1] [expr $y+1] \
       -outline black -activeoutline red -disabledoutline red -state normal \
       -tags [list line $tag node]
}

# inverters and tags are created in proc fixedges
proc connect {win num1 num2 type tag1 tag2} {
    global ZOOM
    global BDD
    global ARROWSIZE
    global COLORLEFT
    global COLORRIGHT

    set tag1 [string map {\" ""} $tag1]
    set tag2 [string map {\" ""} $tag2]

    #puts "connect $num1 $num2 $type $tag1 $tag2"

    set item [lindex [array get BDD "n$num1"] 1]

    if {$tag1 != ""} {
      set tag1 "#$tag1"
    }

    if {$tag2 != ""} {
      set tag2 "#$tag2"
    }

    set koord1 [$win bbox "n$num1"]
    set koord2 [$win bbox "n$num2"]

    set center1x [expr ([lindex $koord1 0]+[lindex $koord1 2])/2]
    set center1y [expr ([lindex $koord1 1]+[lindex $koord1 3])/2]

    set center2x [expr ([lindex $koord2 0]+[lindex $koord2 2])/2]
    set center2y [expr ([lindex $koord2 1]+[lindex $koord2 3])/2]

# s : single line
# si : inverted single line
# r : line to 'then' succesor
# ri : inverted line to 'then' succesor
# l : line to 'else' successor
# li : inverted line to 'else' successor
# d : double line (inverted left)
# di : double line (inverted right)
# e : double line (none inverted)
# ei : double line (both inverted)

    if {$type == "s"} {
      set item [linsert $item 0 "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill black -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left right node $tag1]
    }

    if {$type == "si"} {
      set item [linsert $item 0 "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      set item [linsert $item 0 "i"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill black -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left right inverted node $tag1]
    }

    if {$type == "l"} {
      set item [linsert $item 0 "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" left node $tag1]
    }

    if {$type == "li"} {
      set item [linsert $item 0 "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      set item [linsert $item 0 "i"]
      $win create line $center1x $center1y $center2x $center2y \
       -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
       -width 1 -fill $COLORLEFT -disabledfill red -state normal \
       -tags [list line "f$num1" "t$num2" left inverted node $tag1]
    }

    if {$type == "r"} {
      set item [linsert $item end "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right node $tag1]
    }

    if {$type == "ri"} {
      set item [linsert $item end "n$num2"]
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      set item [linsert $item 0 "i"]
      $win create line $center1x $center1y $center2x $center2y \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right inverted node $tag1]
    }

    if {$type == "d"} {
      set item "n$num2"
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      if {$tag2 != ""} {set item [linsert $item end $tag2]}
      set item [linsert $item 0 "i"]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double inverted node $tag1]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double node $tag2]
    }

    if {$type == "di"} {
      set item "n$num2"
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      if {$tag2 != ""} {set item [linsert $item end $tag2]}
      set item [linsert $item 0 "i"]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double node $tag1]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double inverted node $tag2]
    }

    if {$type == "e"} {
      set item "n$num2"
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      if {$tag2 != ""} {set item [linsert $item end $tag2]}
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double node $tag1]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double node $tag2]
    }

    if {$type == "ei"} {
      set item "n$num2"
      if {$tag1 != ""} {set item [linsert $item end $tag1]}
      if {$tag2 != ""} {set item [linsert $item end $tag2]}
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORLEFT -disabledfill red -stipple "" -state normal \
         -tags [list line "f$num1" "t$num2" left double inverted node $tag1]
      $win create line [expr $center1x] [expr $center1y] \
                       [expr $center2x] [expr $center2y] \
         -arrow none -arrowshape [list $ARROWSIZE $ARROWSIZE [expr $ARROWSIZE/2]] \
         -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
         -tags [list line "f$num1" "t$num2" right double inverted node $tag2]
    }

    $win lower line

    #DEBUGGING
    #puts "connect <$num1> <$num2> <$type> <$tag1> <$tag2>"
    #puts "n$num1: <$item>"

    array set BDD [list "n$num1" $item]
}

# THIS IS USED TO CALCULATE AND DRAW BOX ARROUND THE WHOLE GRAPH
# IT ALSO PERFORMS SNAP TO GRID
proc boxit {win} {
  global XMAX
  global YMAX
  global SELECTED
  global ZOOM
  global LABELOFFSETX
  global LABELOFFSETY
  global GRIDON
  global GRID
  global GRIDRESOLUTION
  global GRIDOFFSETX
  global GRIDOFFSETY

  removebox $win
  if {$GRIDON == 1} {removegrid $win}

  #DEBUGGING
  #set item [$win find withtag "n1&&outline"]
  #set koord [$win bbox $item]
  #set x1 [lindex $koord 0]
  #set y1 [lindex $koord 1]
  #set x2 [lindex $koord 2]
  #set y2 [lindex $koord 3]
  #puts "boxit start for n1: x1=$x1, y1=$y1, x1=$x2, y1=$y2"

  # REPAIR NEGATIVE COORDINATES (xmin = 8, ymin = 2)
  set koord [$win bbox all]
  set x1 [lindex $koord 0]
  set y1 [lindex $koord 1]

  set repair 0
  if {$x1<8} {
    motion $win [expr - $x1 + 8] 0
    set LABELOFFSETX [expr $LABELOFFSETX - $x1 + 8]
    set repair 1
  }
  if {$y1<2} {
    motion $win 0 [expr - $y1 + 2]
    set LABELOFFSETY [expr $LABELOFFSETY - $x1 + 8]
    set repair 1
  }

  if {[llength $SELECTED] > 0} {

    # IF SOMETHING IS SELECTED THEN
    # SNAP TO GRID THE FIRST NODE IN THE SELECTION
    if {$GRIDON == 1} {

      set igrid [expr round(($GRIDRESOLUTION*$ZOOM*$ZOOM)/($GRID*2))]
      set igrid2 [expr round($igrid/2)]

      set item [lindex $SELECTED 0]
      set koord [$win coords $item]
      set x1 [lindex $koord 0]
      set y1 [lindex $koord 1]
      set x2 [lindex $koord 2]
      set y2 [lindex $koord 3]

      set xc [expr round(($x1+$x2)/2)]
      set yc [expr round(($y1+$y2)/2)]

      if {$xc < $GRIDOFFSETX} {
        set xg [expr $xc-$GRIDOFFSETX]
      } else {
        set xg [expr [expr $xc-$GRIDOFFSETX] % $igrid]
        if {$xg > $igrid2} {set xg [expr $xg-$igrid]}
      }

      if {$yc < $GRIDOFFSETY} {
        set yg [expr $yc-$GRIDOFFSETY]
      } else {
        set yg [expr [expr $yc-$GRIDOFFSETY] % $igrid]
        if {$yg > $igrid2} {set yg [expr $yg-$igrid]}
      }

      # MOVE SELECTED NODES
      motion $win [expr -$xg] [expr -$yg]
      set repair 1
    }

  }

  if {$repair == 1} {
    #fixedges should be used after motion is finished
    fixedges $win
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

proc fixedgenode {win item tags dir a b dx dy} {
  global DOUBLELINE
  global XARROW
  global YARROW

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
  global DOUBLELINE
  global XARROW
  global YARROW

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
# IT FIX ONLY EDGES LISTED IN GLOBAL VARIABLES LISTF AND LISTT
# THIS FUNCTION ALSO CREATES BENDS, INVERTERS, AND TAGS
proc fixedges {win} {
  global LISTF
  global LISTT
  global ZOOM
  global BENDSON
  global DPI
  global FONTFAMILYTAG
  global FONTSIZETAG
  global COLORLEFT
  global COLORRIGHT
  global INVSIZE
  global XARROW
  global YARROW

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
    set edgetag [lsearch -inline $tags "#*"]
    if {$edgetag != ""} {
      set edgetag [string range $edgetag 1 end]
    }
    if {$edgetag == "0"} {
      set edgetag ""
    }

    #do not use bends if they are not required or if the edge is oriented towards up
    #every edge has tag left, right, or both of them (top edge)
    if {($BENDSON == 1) &&
        ([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "left"] != -1) &&
        ([lsearch -exact $tags "right"] == -1) && ($dy <= 0)} \
    {
      if {($dx == 0)&&($dy == 0)} {set dx 8}
      #we have check if left node is on the left side

      # VARIANT 1 - simple
      if {$dx > [expr 6*$ZOOM/16]} {
      # VARIANT 2 - more complex and compact but not much better
      #set other [$win gettags "[lindex $tags 1]&&right"]
      #set onode [$win bbox [string replace [lindex $other 2] 0 0 "n"]]
      #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
      #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
      #set odx [expr $oxarrow2 - $xarrow1]
      #set ody [expr $yarrow1 - $oyarrow2]
      #if {($odx == 0)&&($ody == 0)} {set odx 1}
      #if {([expr $dy*$odx] > [expr $dx*$ody]) && ($ody < 0)} {}

        set point [expr abs([lindex $koord1 1]-[lindex $koord1 3])]

        #DEBUGGING
        #addPoint $win [expr $yarrow1+$point] [expr $xarrow1-$point] "p$item"

        $win create line $xarrow1 $yarrow1 [expr $xarrow1-$point] [expr $yarrow1+$point] \
           -arrow none -width 1 -fill $COLORLEFT -disabledfill red -state normal \
           -tags [list line "p$item" node leftline]

        $win lower "p$item"

        if {[lsearch -exact $tags "inverted"] != -1} {
          $win create oval \
            [expr (8*$xarrow1-5*$point)/8-$INVSIZE] [expr (8*$yarrow1+5*$point)/8-$INVSIZE] \
            [expr (8*$xarrow1-5*$point)/8+$INVSIZE] [expr (8*$yarrow1+5*$point)/8+$INVSIZE] \
            -outline $COLORLEFT -disabledoutline red \
            -fill $COLORLEFT -disabledfill red -state normal \
            -tags [list line "i$item" node leftmark]
        }

        if {$edgetag != ""} {
          $win create text \
            [expr (8*$xarrow1-5*$point)/8-$INVSIZE/4] [expr (8*$yarrow1+5*$point)/8+2*$INVSIZE] \
             -text $edgetag -anchor se  \
             -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]] \
             -fill $COLORLEFT -disabledfill red -state normal \
             -tags [list line "i$item" node lefttag]
        }

        set xarrow1 [expr $xarrow1-$point]
        set yarrow1 [expr $yarrow1+$point]

        set dx [expr $xarrow2 - $xarrow1]
        set dy [expr $yarrow1 - $yarrow2]
        if {($dx == 0)&&($dy == 0)} {set dx 1}
      }
    }

    #do not use bends if they are not required or if the edge is oriented towards up
    #every edge has tag left, right, or both of them (top edge)
    if {($BENDSON == 1) &&
        ([lsearch -exact $tags "double"] == -1) &&
        ([lsearch -exact $tags "right"] != -1) &&
        ([lsearch -exact $tags "left"] == -1) && ($dy <= 0)} \
    {
      if {($dx == 0)&&($dy == 0)} {set dx -8}
      #we have check if right node is on the right side

      # VARIANT 1 - simple
      if {$dx < [expr -6*$ZOOM/16]} {
      # VARIANT 2 - more complex and compact but not much better
      #set other [$win gettags "[lindex $tags 1]&&left"]
      #set onode [$win bbox [string replace [lindex $other 2] 0 0 "n"]]
      #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
      #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
      #set odx [expr $oxarrow2 - $xarrow1]
      #set ody [expr $yarrow1 - $oyarrow2]
      #if {($odx == 0)&&($ody == 0)} {set odx 1}
      #if {([expr $dy*$odx] < [expr $dx*$ody]) && ($ody < 0)} {}

        set point [expr abs([lindex $koord1 1]-[lindex $koord1 3])]

        #DEBUGGING
        #addPoint $win [expr $yarrow1+$point] [expr $xarrow1+$point] "p$item"

        $win create line $xarrow1 $yarrow1 [expr $xarrow1+$point] [expr $yarrow1+$point] \
           -arrow none -width 1 -fill $COLORRIGHT -disabledfill red -state normal \
           -tags [list line "p$item" node rightline]

        $win lower "p$item"

        if {[lsearch -exact $tags "inverted"] != -1} {
          $win create oval \
            [expr (8*$xarrow1+5*$point)/8-$INVSIZE] [expr (8*$yarrow1+5*$point)/8-$INVSIZE] \
            [expr (8*$xarrow1+5*$point)/8+$INVSIZE] [expr (8*$yarrow1+5*$point)/8+$INVSIZE] \
            -outline $COLORRIGHT -disabledoutline red \
            -fill $COLORRIGHT -disabledfill red -state normal \
            -tags [list line "i$item" node rightmark]
        }

        if {$edgetag != ""} {
          $win create text \
            [expr (8*$xarrow1+5*$point)/8+$INVSIZE] [expr (8*$yarrow1+5*$point)/8+2*$INVSIZE] \
             -text $edgetag -anchor sw \
             -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]] \
             -fill $COLORRIGHT -disabledfill red -state normal \
             -tags [list line "i$item" node righttag]
        }

        set xarrow1 [expr $xarrow1+$point]
        set yarrow1 [expr $yarrow1+$point]

        set dx [expr $xarrow2 - $xarrow1]
        set dy [expr $yarrow1 - $yarrow2]
        if {($dx == 0)&&($dy == 0)} {set dx -1}
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

      if {([lsearch -exact $tags "left"] != -1) &&
          ([lsearch -exact $tags "right"] == -1)} {

        if {[lsearch -exact $tags "inverted"] != -1} {
          $win create oval \
            [expr (5*$xarrow1+3*$xarrow2)/8-$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8-$INVSIZE] \
             [expr (5*$xarrow1+3*$xarrow2)/8+$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8+$INVSIZE] \
             -outline $COLORLEFT -disabledoutline red \
             -fill $COLORLEFT -disabledfill red -state normal \
             -tags [list line "i$item" node leftmark]
        }

        if {$edgetag != ""} {
          $win create text \
            [expr ($xarrow1+$xarrow2)/2-$INVSIZE/4] [expr ($yarrow1+$yarrow2)/2+$INVSIZE] \
             -text $edgetag -anchor se \
             -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]] \
             -fill $COLORLEFT -disabledfill red -state normal \
             -tags [list line "i$item" node lefttag]
        }
      }


      if {[lsearch -exact $tags "right"] != -1} {

        if {[lsearch -exact $tags "inverted"] != -1} {
          $win create oval \
             [expr (5*$xarrow1+3*$xarrow2)/8-$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8-$INVSIZE] \
             [expr (5*$xarrow1+3*$xarrow2)/8+$INVSIZE] [expr (5*$yarrow1+3*$yarrow2)/8+$INVSIZE] \
             -outline $COLORRIGHT -disabledoutline red \
             -fill $COLORRIGHT -disabledfill red -state normal \
             -tags [list line "i$item" node rightmark]
        }

        if {$edgetag != ""} {
          $win create text \
            [expr ($xarrow1+$xarrow2)/2+$INVSIZE/4] [expr ($yarrow1+$yarrow2)/2+$INVSIZE] \
             -text $edgetag -anchor sw \
             -font [list -family $FONTFAMILYTAG -size [expr $FONTSIZETAG]] \
             -fill $COLORRIGHT -disabledfill red -state normal \
             -tags [list line "i$item" node righttag]
        }
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
  $win itemconfigure "selection&&double&&right&&inverted" -width 2
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

proc parsefile {win filename} {
  global BDDVARIABLES
  global LABELOFFSETX
  global LABELOFFSETY

  set type ""
  set BDDVARIABLES ""
  set LABELOFFSETX 0
  set LABELOFFSETY 0
  set f [open $filename r]

  while {[gets $f line] >= 0} {
    if {($line != "") && ([string index $line 0] != "#")} {

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

      if {$arg0 == "type"} {
        set arg1 [string map {\" ""} $arg1]
        set type $arg1
      }

      if {$arg0 == "var"} {
        for {set j 1} {$j < $i} {incr j} {
          set arg$j [string map {\" ""} [subst $[subst arg$j]]]
          addVariable [subst $[subst arg$j]]
        }
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
        if {$i == 6} {
          #double edge with tags
          #no edge to terminal zero should be a double edge
          connect $win $arg1 $arg2 $arg3 $arg4 $arg5
        } else {
          if {$i == 5} {
            #single edge with tag
            #edge to terminal zero is always tagged with "1"
            #we change it to "0" to make it invisible, see drawing proc
            if {[$win find withtag "n$arg2 && zero"] != ""} {
              set arg4 "0"
            }
            connect $win $arg1 $arg2 $arg3 $arg4 ""
          } else {
            #single or double edge without tags
            connect $win $arg1 $arg2 $arg3 "" ""
          }
        }
      }

    }
  }

  close $f
  return $type
}

# ############################################
# USER PROCEDURES
# ############################################

proc savefile {} {
  global mainwin
  global BDDNAME

  set filename [tk_getSaveFile -title "Save As" -initialfile "$BDDNAME.bddview" -parent $mainwin]
  if {[string length $filename] != 0} {
    #invoke cd to remember last path
    cd [file dirname $filename]

    bddview_save $filename
  }
}

proc exporttex {} {
  global mainwin
  global BDDNAME

  set filename [tk_getSaveFile -title "Export as Tex/TikZ" -initialfile "$BDDNAME.tex" -parent $mainwin]
  if {[string length $filename] != 0} {
    #invoke cd to remember last path
    cd [file dirname $filename]

    set caption $BDDNAME
    set caption [string map {"_" "\\_"} $caption]
    bddview_export_tex $filename $caption 1
  }
}

proc printfile {} {
  global mainwin
  global BDDNAME

  set filename [tk_getSaveFile -title "Print to File (postscript)" -initialfile "$BDDNAME.ps" -parent $mainwin]
  if {[string length $filename] != 0} {
    bddview_print $filename
  }
}

proc state {s win bb} {
  global OS
  global STATE
  global STATUSBAR
  global COLORMENU
  global COLORGRID
  global COLORNODE

  if {$s != $STATE} {
    switch $STATE {
      pan {$bb itemconfigure 0 -relief flat -bd 2 -highlightthickness 0}
      zoom {$bb itemconfigure 1 -relief flat -bd 2 -highlightthickness 0}
      edit {$bb itemconfigure 2 -relief flat -bd 2 -highlightthickness 0}
      info {$bb itemconfigure 3 -relief flat -bd 2 -highlightthickness 0}
    }
    switch $s {
      pan {
             $win itemconfigure "tekst" -activefill black
             $win itemconfigure "outline" -activeoutline black
             if {$OS == "windows"} {
               $bb itemconfigure 0 -relief solid
             } else {
               $bb itemconfigure 0 -bd 0 -highlightthickness 2
             }
             $win configure -cursor hand2
             set STATUSBAR "\[Pan\]  Left button + drag = scroll  /  Right button = center"
           }
      zoom {
             $win itemconfigure "tekst" -activefill black
             $win itemconfigure "outline" -activeoutline black
             if {$OS == "windows"} {
               $bb itemconfigure 1 -relief solid
             } else {
               $bb itemconfigure 1 -bd 0 -highlightthickness 2
             }
             $win configure -cursor target
             set STATUSBAR "\[Zoom\]  Left button = zoom in  /  Right button = zoom out"
           }
      edit {
             $win itemconfigure "tekst" -activefill red
             $win itemconfigure "outline" -activeoutline black
             if {$OS == "windows"} {
               $bb itemconfigure 2 -relief solid
             } else {
               $bb itemconfigure 2 -bd 0 -highlightthickness 2
             }
             $win configure -cursor fleur
             set STATUSBAR "\[Edit\]  Left button + drag = move subgraph  /  Right button + drag = move node"
           }
      info {
             $win itemconfigure "tekst" -activefill red
             $win itemconfigure "outline" -activeoutline black
             if {$OS == "windows"} {
               $bb itemconfigure 3 -relief solid
             } else {
               $bb itemconfigure 3 -bd 0 -highlightthickness 2
             }
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
        if {($item != "i") && ([string index $item 0] != "#")} {
          addSelected $item
        }
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
      if {($item != "i") && ([string index $item 0] != "#")} {
        addSelectedDepth $item [expr $num + 1]
      }
    }
  } else {
    set i [expr [lsearch -exact $DEPTH $root] + 1]
    set inum [lindex $DEPTH $i]
    if {$num > $inum} {
      set DEPTH [lreplace $DEPTH $i $i $num]
      foreach item [lindex [array get BDD $root] 1] {
        if {($item != "i") && ([string index $item 0] != "#")} {
          addSelectedDepth $item [expr $num + 1]
        }
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

proc drawinfo {win item x y mode} {
  global COLORFG
  global FONTFAMILYLABEL
  global FONTFAMILYINFO
  global SELECTED
  global TERMSELECTED
  global DEPTH
  global TERMDEPTH
  global BDD
  global LABELOFFSETX
  global LABELOFFSETY
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
      if {($item != "i") && ([string index $item 0] != "#")} {
        set variable [$win itemcget "$node && tekst" -text]
      }
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

  set nodes [expr $nodes + [llength $SELECTED] - [llength $TERMSELECTED]]

  set info1 "Root variable: "
  set info2 "Internal BDD nodes: "
  set info3 "BDD terminal depth: "
  set info "$info1$variable\n$info2$nodes\n$info3$TERMDEPTH"

  $win create text $x $y \
     -text $info \
     -font [list -family $FONTFAMILYINFO -size 12 -weight bold] \
     -anchor w \
     -fill $COLORFG -state normal \
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
          -text $infonum \
          -font [list -family $FONTFAMILYLABEL -size 10] \
          -anchor e \
          -fill black -state normal \
          -tags [list infonum]

        $win create text [expr round($x1+0.95*($x2-$x1))] [expr round($y1+1.15*($y2-$y1))] \
          -text $infodepth \
          -font [list -family $FONTFAMILYINFO -size 10 -weight bold] \
          -anchor e \
          -fill black -state normal \
          -tags [list infonum]

      }
    }

    if { $mode == 2 } {

      set infonum [string replace $item 0 0 "#"]
      $win create text $x2 [expr $y1-4] \
        -text $infonum \
        -font [list -family $FONTFAMILYLABEL -size 10] \
        -anchor e \
        -fill black -state normal \
        -tags [list infonum]

      set infocoords "([expr round($x1+$x2)/2-$LABELOFFSETX],[expr round($y1+$y2)/2-$LABELOFFSETY])"
      $win create text $x1 [expr round($y1+1.15*($y2-$y1))] \
         -text $infocoords \
         -font [list -family $FONTFAMILYINFO -size 10] \
         -anchor w \
         -fill black -state normal \
         -tags [list infonum]

      #DEBUGGING
      #puts "drawinfo: infonum=$infonum, x1=$x1, y1=$y1, x2=$x2, y2=$y2, LABELOFFSETX=$LABELOFFSETX, LABELOFFSETY=$LABELOFFSETY, $infocoords"

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
# API
# ############################################

proc bddview_message {text1 text2} {
  tk_messageBox -icon info -message "$text1" -detail "$text2" -type ok
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

proc bddview_bendsOnOff {} {
  global bb3

  $bb3 invoke 0
}

proc bddview_arrowsOnOff {} {
  global bb3

  $bb3 invoke 1
}

proc bddview_gridsOnOff {} {
  global bb3

  $bb3 invoke 2
}

proc bddview_open {filename} {
  global mainwin
  global bb1
  global USEBDDTYPE
  global ZOOM
  global STATE
  global ARROWSON
  global LISTF
  global LISTT

  bddview_clear

  set activezoom $ZOOM

  #change the zoom to default
  while {$ZOOM > 16} {
    zoomOut $mainwin 0 0
  }
  while {$ZOOM < 16} {
    zoomIn $mainwin 0 0
  }

  set type [parsefile $mainwin $filename]

  set LISTF [$mainwin find withtag left]
  set LISTT [$mainwin find withtag right]

  fixedges $mainwin
  boxit $mainwin

  set LISTF ""
  set LISTT ""

  #restore the zoom
  while {$ZOOM > $activezoom} {
    zoomOut $mainwin 0 0
  }
  while {$ZOOM < $activezoom} {
    zoomIn $mainwin 0 0
  }

  #change position to default
  compass "c" $mainwin
  compass "n" $mainwin

  if {$ARROWSON == 1} {
    showarrows $mainwin
  }

  set s $STATE
  set STATE ""
  state $s $mainwin $bb1

  set USEBDDTYPE $type
  return $type
}

proc bddview_save {filename} {
  global mainwin
  global ACTIVEBDDTYPE
  global USEBDDTYPE
  global ZOOM
  global LABELOFFSETX
  global LABELOFFSETY
  global BDD
  global BDDVARIABLES

  if {[string length $filename] != 0} {

    #for better result, change the zoom and the position to default
    while {$ZOOM > 16} {
      zoomOut $mainwin 0 0
    }
    while {$ZOOM < 16} {
      zoomIn $mainwin 0 0
    }
    compass "c" $mainwin
    compass "n" $mainwin

    set f [open $filename w]

    if {$ACTIVEBDDTYPE != ""} {
      #if used from BDD Scout then ACTIVEBDDTYPE has a value
      if {$ACTIVEBDDTYPE == "BIDDYTYPEOBDD"} {
        puts $f "type ROBDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEOBDDC"} {
        puts $f "type ROBDDCE"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEZBDD"} {
        puts $f "type ZBDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEZBDDC"} {
        puts $f "type ZBDDCE"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPETZBDD"} {
        puts $f "type TZBDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPETZBDDC"} {
        puts $f "type TZBDDCE"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEOFDD"} {
        puts $f "type ROFDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEOFDDC"} {
        puts $f "type ROFDDCE"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEZFDD"} {
        puts $f "type ZFDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPEZFDDC"} {
        puts $f "type ZFDDCE"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPETZFDD"} {
        puts $f "type TZFDD"
      } elseif {$ACTIVEBDDTYPE == "BIDDYTYPETZFDDC"} {
        puts $f "type TZFDDCE"
      }
    } else {
      puts $f "type $USEBDDTYPE"
    }

    puts -nonewline $f "var"
    foreach item $BDDVARIABLES {
      puts -nonewline $f " \"$item\""
    }
    puts $f ""

    set llist [$mainwin find withtag "label && tekst"]
    set nlist [$mainwin find withtag "node && !label && tekst"]

    foreach item $llist {
      set name [string range [lindex [$mainwin gettags $item] 0] 1 end]
      set koord [$mainwin coords $item]
      set x [expr floor([lindex $koord 0])-$LABELOFFSETX]
      set y [expr floor([lindex $koord 1])-$LABELOFFSETY]
      set t [$mainwin itemcget $item -text]
      puts $f "label $name \"$t\" $x $y"
    }

    foreach item $nlist {
      set name [string range [lindex [$mainwin gettags $item] 0] 1 end]
      set koord [$mainwin coords $item]
      set x [expr floor([lindex $koord 0])-$LABELOFFSETX]
      set y [expr floor([lindex $koord 1])-$LABELOFFSETY]
      set t [$mainwin itemcget $item -text]
      if {$t == "0"} {
        puts $f "terminal $name \"0\" $x $y"
      } elseif {$t == "1"} {
        puts $f "terminal $name \"1\" $x $y"
      } else {
        puts $f "node $name \"$t\" $x $y"
      }
    }

    foreach item $llist {
      set name [lindex [$mainwin gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      if {[lindex $succ 0] == "i"} {
        set inv "i"
        set succ [lrange $succ 1 end]
      } else {
        set inv ""
      }
      if {[llength $succ] == 1} {
        #label
        set dst [lindex $succ 0]
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] s$inv"
      } elseif {[llength $succ] == 2} {
        #label with tag
        set dst [lindex $succ 0]
        set tag [string range [lindex $succ 1] 1 end]
        if {$tag == "0"} {
          set tag "1"
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] s$inv \"$tag\""
      } else {
        puts "ERROR (bddview_save): corrupted internal structure"
      }
    }

    foreach item $nlist {
      set name [lindex [$mainwin gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      if {[lindex $succ 0] == "i"} {
        set inv "i"
        set succ [lrange $succ 1 end]
      } else {
        set inv ""
      }
      if {[llength $succ] == 0} {
        #terminal node
      } elseif {[llength $succ] == 1} {
        #double line
        set dst [lindex $succ 0]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && left && inverted"] == ""} {
          if {$inv == ""} {
            set type "e"
          } else {
            set type "di"
          }
        } else {
          if {$inv == ""} {
            set type "ei"
          } else {
            set type "d"
          }
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] $type"
      } elseif {[llength $succ] == 2} {
        #two single lines
        set dst [lindex $succ 0]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] l$inv"
        set dst [lindex $succ 1]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] r$inv"
      } elseif {[llength $succ] == 3} {
        #double line with tags
        set dst [lindex $succ 0]
        set tag1 [string range [lindex $succ 1] 1 end]
        set tag2 [string range [lindex $succ 2] 1 end]
        if {$tag1 == "0"} {
          set tag1 "1"
        }
        if {$tag2 == "0"} {
          set tag2 "1"
        }
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && left && inverted"] == ""} {
          if {$inv == ""} {
            set type "e"
          } else {
            set type "di"
          }
        } else {
          if {$inv == ""} {
            set type "ei"
          } else {
            set type "d"
          }
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] $type \"$tag1\" \"$tag2\""
      } elseif {[llength $succ] == 4} {
        #two single lines with tags
        set dst [lindex $succ 0]
        set tag [string range [lindex $succ 1] 1 end]
        if {$tag == "0"} {
          set tag "1"
        }
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] l$inv \"$tag\""
        set dst [lindex $succ 2]
        set tag [string range [lindex $succ 3] 1 end]
        if {$tag == "0"} {
          set tag "1"
        }
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        puts $f "connect [string range $name 1 end] [string range $dst 1 end] r$inv \"$tag\""
      } else {
        puts "ERROR (bddview_save): corrupted internal structure"
      }
    }

    close $f
  }

}

proc bddview_export_tex {filename caption document} {
  global mainwin
  global BDDVIEWVERSION
  global ACTIVEBDDTYPE
  global USEBDDTYPE
  global ZOOM
  global LABELOFFSETX
  global LABELOFFSETY
  global BENDSON
  global ARROWSON
  global BDD
  global BDDNAME

  set scale 1.0

  if {$ACTIVEBDDTYPE != ""} {
    set USEBDDTYPE ""
  }

  if {[string length $filename] != 0} {

    #for better result, change the zoom and the position to default
    while {$ZOOM > 16} {
      zoomOut $mainwin 0 0
    }
    while {$ZOOM < 16} {
      zoomIn $mainwin 0 0
    }
    compass "c" $mainwin
    compass "n" $mainwin

    set llist [$mainwin find withtag "label && tekst"]
    set nlist [$mainwin find withtag "node && !label && tekst"]

    set f [open $filename w]

    puts $f "%prepared using bddview v$BDDVIEWVERSION (biddy.meolic.com)"
    if {$document == 1} {
      puts $f "%date: [clock format [clock seconds] -format {%d.%m.%Y %H:%M:%S}]"
      puts $f "%use 'pdflatex $filename' to produce pdf"
      puts $f "%adapt papersize, \\resizebox and \\begin\{tikzpicture\} to fit the large graph"
      puts $f ""
      puts $f "\\documentclass\[a4paper\]\{report\}"
      puts $f "\\usepackage\[margin=24mm,landscape\]\{geometry\}"
      puts $f "\\usepackage\{tikz\}"
      puts $f "\\usetikzlibrary\{arrows,shapes,decorations.markings\}"
      puts $f ""
      puts $f "\\begin\{document\}"
      puts $f "\\pagenumbering\{gobble\}"
      puts $f ""
      puts $f "\\begin\{figure\}\[p\]"
      puts $f "\\begin\{center\}"
      puts $f "\\resizebox\{!\}\{!\}\{"
      puts $f "%\\resizebox\{\\textwidth\}\{!\}\{"
      puts $f "%\\resizebox\{!\}\{\\textheight\}\{"
    } else {
      puts $f ""
    }
    puts $f "\\begin\{tikzpicture\}\[x=0.15mm,y=-0.15mm\]"
    puts $f "\\definecolor\{colorleft\}\{RGB\}\{0,104,40\}%"
    puts $f "\\definecolor\{colorright\}\{RGB\}\{0,40,104\}%"
    puts $f "\\tikzstyle\{top\}=\[ellipse, inner sep=1mm, fill=white, draw=white\]%"
    puts $f "\\tikzstyle\{var\}=\[ellipse, solid, thick, minimum size=6mm, inner sep=0.5mm, fill=white, draw=black!70\]%"
    puts $f "\\tikzstyle\{leaf\}=\[rectangle, solid, thick, minimum size=6mm, fill=white, draw=black!70\]%"

    if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDD") || ($USEBDDTYPE == "ROBDD")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick, draw=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright\]%"
    }

    if {($ACTIVEBDDTYPE == "BIDDYTYPEOBDDC") || ($USEBDDTYPE == "ROBDDCE")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick, draw=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{dn\}=\[-, densely dashed, thick, bend right=10, draw=colorleft\]%"
      puts $f "\\tikzstyle\{dp\}=\[-, solid, thick, bend left=10, draw=colorright\]%"
      puts $f "\\tikzstyle\{tcomp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.6 with \{\\fill\[radius=1mm,fill=colorright\] (0,0) circle;\}\}\}\]%"
      puts $f "\\tikzstyle\{comp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.5 with \{\\fill\[radius=1mm,fill=colorleft\] (0,0) circle;\}\}\}\]%"
    }

    if {($ACTIVEBDDTYPE == "BIDDYTYPEZBDD") || ($USEBDDTYPE == "ZBDD")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick, draw=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{dn\}=\[-, densely dashed, thick, bend right=10, color=colorleft]\%"
      puts $f "\\tikzstyle\{dp\}=\[-, solid, thick, bend left=10,color=colorright\]%"
    }

    if {($ACTIVEBDDTYPE == "BIDDYTYPEZBDDC") || ($USEBDDTYPE == "ZBDDCE")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick, draw=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright\]%"
      puts $f "\\tikzstyle\{dn\}=\[-, densely dashed, thick, bend right=10, draw=colorleft]\%"
      puts $f "\\tikzstyle\{dp\}=\[-, solid, thick, bend left=10, draw=colorright\]%"
      puts $f "\\tikzstyle\{tcomp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.6 with \{\\fill\[radius=1mm,fill=colorright\] (0,0) circle;\}\}\}\]%"
      puts $f "\\tikzstyle\{comp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.5 with \{\\fill\[radius=1mm,fill=colorright\] (0,0) circle;\}\}\}\]%"
    }

    if {($ACTIVEBDDTYPE == "BIDDYTYPETZBDD") || ($USEBDDTYPE == "TZBDD")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright, anchor=225, pos=0.8, color=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick,  draw=colorleft, anchor=10, pos=0.15, color=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright, anchor=170, pos=0.15, color=colorright\]%"
      puts $f "\\tikzstyle\{dn\}=\[-, densely dashed, thick, bend right=10, draw=colorleft, anchor=305, pos=0.9, color=colorleft]\%"
      puts $f "\\tikzstyle\{dp\}=\[-, solid, thick, bend left=10, draw=colorright, anchor=145, pos=0, color=colorright\]%"
    }

    if {($ACTIVEBDDTYPE == "BIDDYTYPETZBDDC") || ($USEBDDTYPE == "TZBDDCE")} {
      puts $f "\\tikzstyle\{t\}=\[-, solid, thick, draw=colorright, anchor=225, pos=0.8, color=colorright\]%"
      puts $f "\\tikzstyle\{n\}=\[-, densely dashed, thick,  draw=colorleft, anchor=10, pos=0.15, color=colorleft\]%"
      puts $f "\\tikzstyle\{p\}=\[-, solid, thick, draw=colorright, anchor=170, pos=0.15, color=colorright\]%"
      puts $f "\\tikzstyle\{dn\}=\[-, densely dashed, thick, bend right=10, draw=colorleft, anchor=305, pos=0.9, color=colorleft]\%"
      puts $f "\\tikzstyle\{dp\}=\[-, solid, thick, bend left=10, draw=colorright, anchor=145, pos=0, color=colorright\]%"
      puts $f "\\tikzstyle\{tcomp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.6 with \{\\fill\[radius=1mm,fill=colorright\] (0,0) circle;\}\}\}\]%"
      puts $f "\\tikzstyle\{comp\}=\[postaction=\{decorate, decoration=\{markings, mark=at position 0.5 with \{\\fill\[radius=1mm,fill=colorleft\] (0,0) circle;\}\}\}\]%"
    }

    #WRITE LABEL
    foreach item $llist {
      set name [string range [lindex [$mainwin gettags $item] 0] 1 end]
      set t [$mainwin itemcget $item -text]
      #set t [string map {"_" "\\_"} $t]
      set koord [$mainwin coords $item]
      set xcenter [expr floor([lindex $koord 0])-$LABELOFFSETX]
      set ycenter [expr floor([lindex $koord 1])-$LABELOFFSETY]
    }
    puts $f "\\draw ($xcenter,$ycenter) node\[top\] (n$name) \{\$$t\$\};"

    #WRITE NODES
    foreach item $nlist {
      set name [string range [lindex [$mainwin gettags $item] 0] 1 end]
      set koord [$mainwin coords $item]
      set x [expr floor([lindex $koord 0])-$LABELOFFSETX]
      set y [expr floor([lindex $koord 1])-$LABELOFFSETY]
      set t [$mainwin itemcget $item -text]
      #set t [string map {"_" "\\_"} $t]
      if {$t == "0"} {
        puts $f "\\draw ($x,$y) node\[leaf\] (n$name) \{\$0\$\};"
      } elseif {$t == "1"} {
        puts $f "\\draw ($x,$y) node\[leaf\] (n$name) \{\$1\$\};"
      } else {
        puts $f "\\draw ($x,$y) node\[var\] (n$name) \{\$$t\$\};"
      }
    }

    #WRITE TOP EDGE
    foreach item $llist {
      set name [lindex [$mainwin gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      #puts "$name"
      #puts "$succ"
      if {[lindex $succ 0] == "i"} {
        set inv "i"
        set succ [lrange $succ 1 end]
      } else {
        set inv ""
      }
      if {[llength $succ] == 1} {
        #label
        set dst [lindex $succ 0]
        if {$inv == "i"} {
          puts $f "\\draw \[t\] (n[string range $name 1 end]) edge \[tcomp\] (n[string range $dst 1 end]);"
        } else {
          puts $f "\\draw \[t\] (n[string range $name 1 end]) edge (n[string range $dst 1 end]);"
        }
      } elseif {[llength $succ] == 2} {
        #label with tag
        set dst [lindex $succ 0]
        set tag [string range [lindex $succ 1] 1 end]
        if {$tag == "0"} {
          set tag "1"
        }
        if {$inv == "i"} {
          puts "inverted edges + tags are not supported, yet"
        } else {
          puts $f "\\draw \[t\] (n[string range $name 1 end]) edge node \{$tag\} (n[string range $dst 1 end]);"
        }
      } else {
        puts "ERROR (bddview_export_tex): corrupted internal structure"
      }
    }

    #WRITE EDGES
    foreach item $nlist {
      set name [lindex [$mainwin gettags $item] 0]
      set succ [lindex [array get BDD $name] 1]
      #puts "$name"
      #puts "$succ"
      if {[lindex $succ 0] == "i"} {
        set inv "i"
        set succ [lrange $succ 1 end]
      } else {
        set inv ""
      }

      if {[llength $succ] == 0} {
        #terminal node does not have successors
      } elseif {[llength $succ] == 1} {
        #double line
        set dst [lindex $succ 0]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && left && inverted"] == ""} {
          if {$inv == ""} {
            set type "e"
          } else {
            set type "di"
          }
        } else {
          if {$inv == ""} {
            set type "ei"
          } else {
            set type "d"
          }
        }
        if {($type == "d") || ($type == "ei")} {
          puts $f "\\draw \[dn\] (n[string range $name 1 end]) edge \[comp\] (n[string range $dst 1 end]);"
        } else {
          puts $f "\\draw \[dn\] (n[string range $name 1 end]) edge (n[string range $dst 1 end]);"
        }
        if {($type == "di") || ($type == "ei")} {
          puts $f "\\draw \[dp\] (n[string range $name 1 end]) edge \[comp\] (n[string range $dst 1 end]);"
        } else {
          puts $f "\\draw \[dp\] (n[string range $name 1 end]) edge (n[string range $dst 1 end]);"
        }
      } elseif {[llength $succ] == 2} {
        #two single lines
        set dstleft [lindex $succ 0]
        set dstright [lindex $succ 1]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dstleft 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        set bend ""
        if {$BENDSON == 1} {
          set koord1 [$mainwin bbox $name]
          set koord2 [$mainwin bbox $dstleft]
          set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
          set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
          set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
          set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]
          set dx [expr $xarrow2 - $xarrow1]
          set dy [expr $yarrow1 - $yarrow2]
          if {($dx == 0)&&($dy == 0)} {set dx 8}
          # VARIANT 1 - simple
          if {$dx > [expr 6]} {
          # VARIANT 2 - more complex and compact but not much better
          #set onode [$mainwin bbox $dstright]
          #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
          #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
          #set odx [expr $oxarrow2 - $xarrow1]
          #set ody [expr $yarrow1 - $oyarrow2]
          #if {($odx == 0)&&($ody == 0)} {set odx 1}
          #if {([expr $dy*$odx] > [expr $dx*$ody]) && ($ody < 0)} {}
            set bend " -- ++(-20,30)"
          }
        }
        if {$inv == "i"} {
          puts $f "\\draw \[n\] (n[string range $name 1 end])$bend edge \[comp\] (n[string range $dstleft 1 end]);"
        } else {
          puts $f "\\draw \[n\] (n[string range $name 1 end])$bend edge (n[string range $dstleft 1 end]);"
        }
        set lineto [string replace $dstright 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        set bend ""
        if {$BENDSON == 1} {
          set koord1 [$mainwin bbox $name]
          set koord2 [$mainwin bbox $dstright]
          set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
          set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
          set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
          set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]
          set dx [expr $xarrow2 - $xarrow1]
          set dy [expr $yarrow1 - $yarrow2]
          if {($dx == 0)&&($dy == 0)} {set dx -8}
          # VARIANT 1 - simple
          if {$dx < [expr -6]} {
          # VARIANT 2 - more complex and compact but not much better
          #set onode [$mainwin bbox $dstleft]
          #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
          #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
          #set odx [expr $oxarrow2 - $xarrow1]
          #set ody [expr $yarrow1 - $oyarrow2]
          #if {($odx == 0)&&($ody == 0)} {set odx 1}
          #if {([expr $dy*$odx] < [expr $dx*$ody]) && ($ody < 0)} {}
            set bend " -- ++(20,30)"
          }
        }
        if {$inv == "i"} {
          puts $f "\\draw \[p\] (n[string range $name 1 end])$bend edge \[comp\] (n[string range $dstright 1 end]);"
        } else {
          puts $f "\\draw \[p\] (n[string range $name 1 end])$bend edge (n[string range $dstright 1 end]);"
        }
      } elseif {[llength $succ] == 3} {
        #double line with tags
        set dst [lindex $succ 0]
        set tag1 [string range [lindex $succ 1] 1 end]
        set tag2 [string range [lindex $succ 2] 1 end]
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dst 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && left && inverted"] == ""} {
          if {$inv == ""} {
            set type "e"
          } else {
            set type "di"
          }
        } else {
          if {$inv == ""} {
            set type "ei"
          } else {
            set type "d"
          }
        }
        if {($type == "d") || ($type == "ei")} {
          puts "inverted edges + tags are not supported, yet"
        } else {
          puts $f "\\draw \[dn\] (n[string range $name 1 end]) edge node \{$tag1\} (n[string range $dst 1 end]);"
        }
        if {($type == "di") || ($type == "ei")} {
          puts "inverted edges + tags are not supported, yet"
        } else {
          puts $f "\\draw \[dp\] (n[string range $name 1 end]) edge node \{$tag2\} (n[string range $dst 1 end]);"
        }
      } elseif {[llength $succ] == 4} {
        #two single lines with tags
        set dstleft [lindex $succ 0]
        set tagleft [string range [lindex $succ 1] 1 end]
        if {$tagleft == "0"} {
          set tagleft "1"
        }
        set dstright [lindex $succ 2]
        set tagright [string range [lindex $succ 3] 1 end]
        if {$tagright == "0"} {
          set tagright "1"
        }
        set linefrom [string replace $name 0 0 "f"]
        set lineto [string replace $dstleft 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        set bend ""
        if {$BENDSON == 1} {
          set koord1 [$mainwin bbox $name]
          set koord2 [$mainwin bbox $dstleft]
          set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
          set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
          set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
          set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]
          set dx [expr $xarrow2 - $xarrow1]
          set dy [expr $yarrow1 - $yarrow2]
          if {($dx == 0)&&($dy == 0)} {set dx 8}
          # VARIANT 1 - simple
          if {$dx > [expr 6]} {
          # VARIANT 2 - more complex and compact but not much better
          #set onode [$mainwin bbox $dstright]
          #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
          #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
          #set odx [expr $oxarrow2 - $xarrow1]
          #set ody [expr $yarrow1 - $oyarrow2]
          #if {($odx == 0)&&($ody == 0)} {set odx 1}
          #if {([expr $dy*$odx] > [expr $dx*$ody]) && ($ody < 0)} {}
            set bend " -- ++(-20,30)"
          }
        }
        if {$inv == "i"} {
          puts "inverted edges + tags are not supported, yet"
        } else {
          puts $f "\\draw \[n\] (n[string range $name 1 end])$bend edge node \{$tagleft\} (n[string range $dstleft 1 end]);"
        }
        set lineto [string replace $dstright 0 0 "t"]
        if {[$mainwin find withtag "$linefrom && $lineto && inverted"] == ""} {
          set inv ""
        } else {
          set inv "i"
        }
        set bend ""
        if {$BENDSON == 1} {
          set koord1 [$mainwin bbox $name]
          set koord2 [$mainwin bbox $dstright]
          set xarrow1 [expr ([lindex $koord1 0] + [lindex $koord1 2])/2.0]
          set yarrow1 [expr ([lindex $koord1 1] + [lindex $koord1 3])/2.0]
          set xarrow2 [expr ([lindex $koord2 0] + [lindex $koord2 2])/2.0]
          set yarrow2 [expr ([lindex $koord2 1] + [lindex $koord2 3])/2.0]
          set dx [expr $xarrow2 - $xarrow1]
          set dy [expr $yarrow1 - $yarrow2]
          if {($dx == 0)&&($dy == 0)} {set dx -8}
          # VARIANT 1 - simple
          if {$dx < [expr -6]} {
          # VARIANT 2 - more complex and compact but not much better
          #set onode [$mainwin bbox $dstleft]
          #set oxarrow2 [expr ([lindex $onode 0] + [lindex $onode 2])/2.0]
          #set oyarrow2 [expr ([lindex $onode 1] + [lindex $onode 3])/2.0]
          #set odx [expr $oxarrow2 - $xarrow1]
          #set ody [expr $yarrow1 - $oyarrow2]
          #if {($odx == 0)&&($ody == 0)} {set odx 1}
          #if {([expr $dy*$odx] < [expr $dx*$ody]) && ($ody < 0)} {}
            set bend " -- ++(20,30)"
          }
        }
        if {$inv == "i"} {
          puts "inverted edges + tags are not supported, yet"
        } else {
          puts $f "\\draw \[p\] (n[string range $name 1 end])$bend edge node \{$tagright\} (n[string range $dstright 1 end]);"
        }
      } else {
        puts "ERROR (bddview_export_tex): corrupted internal structure"
      }
    }

    puts $f "\\end\{tikzpicture\}"

    if {$document == 1} {
      puts $f "\}\\end\{center\}"
      puts $f "\\caption\{$caption\}"
      puts $f "\\label\{fig:$BDDNAME\}"
      puts $f "\\end\{figure\}"
      puts $f ""
      puts $f "\\end\{document\}"
    }

    close $f
  }
}

proc bddview_print {filename} {
  global mainwin
  global ZOOM
  global XMAX
  global YMAX

  set koord [$mainwin bbox all]
  if {$koord == ""} return

  if {[string length $filename] != 0} {

    #remember last path
    cd [file dirname $filename]

    set x1 [lindex $koord 0]
    set y1 [lindex $koord 1]
    set x2 [lindex $koord 2]
    set y2 [lindex $koord 3]

    if {[expr (1.0*($y2-$y1))/($x2-$x1)] > 1.415125082} {

      $mainwin postscript -file $filename -pageanchor nw -pagex 0m -pagey 297.039m -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1] -pageheight 297.039m

    } else {

      $mainwin postscript -file $filename -pageanchor nw -pagex 0m -pagey 297.039m -x $x1 -width [expr $x2-$x1] -y $y1 -height [expr $y2-$y1] -pagewidth 209.903m

    }
  }
}

# ############################################
# FINAL INITIALIZATION
# ############################################

# if ACTIVEBDDTYPE != "" THEN USEBDDTYPE is ignored
# if ACTIVEBDDTYPE == "" THEN USEBDDTYPE is used
set ACTIVEBDDTYPE ""
set USEBDDTYPE ""

state pan $mainwin $bb1
puts "OK"

if { $argc != 0 } {

  set filename [lindex $argv 0]

  if {[file exists $filename]} {
    bddview_open $filename
  } else {
    puts "Warning: $filename does not exists"
  }
}

#turn bends on
$bb3 invoke 0

