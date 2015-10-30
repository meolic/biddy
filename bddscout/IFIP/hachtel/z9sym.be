@BE1
@invar
(IN0 IN1 IN2 IN3 IN4 IN5 IN6 IN7 IN8)

@sub
GE45 = 
 (OR (AND (NOT IN7) IN1) (AND IN2 (NOT IN1)))
GE46 = 
 (OR (AND (NOT IN7) IN5) (AND (NOT IN5) IN2))
GE15 = 
 (OR (AND IN5 (NOT IN4)) (AND IN6 (NOT IN3)))
GE0 = 
 (OR (AND (NOT IN7) IN6) (AND IN7 (NOT IN6)))
GE1 = 
 (OR (AND (NOT IN4) IN3) (AND IN4 (NOT IN3)))
GE5 = 
 (OR (AND (NOT IN8) IN5) (AND IN7 (NOT IN3)))
GE27 = 
 (OR (AND IN8 (NOT IN7)) (AND IN7 (NOT IN6)))
GE3 = 
 (OR (AND IN3 (NOT IN0)) (AND IN8 (NOT IN7)))
GE21 = 
 (OR (AND IN8 (NOT IN3)) (AND (NOT IN8) IN0))
GE6 = 
 (OR (AND (NOT IN5) IN2) (AND IN5 (NOT IN0)))
GE8 = 
 (OR (AND IN6 (NOT IN0)) GE45)
GE9 = 
 (OR (AND IN7 (NOT IN1)) GE46)
GE16 = 
 (OR (AND IN4 IN3 (NOT IN2)) (AND IN6 IN5 (NOT IN1)) (AND GE15 IN2))
GE24 = 
 (OR (AND (NOT IN6) IN4) (AND IN6 (NOT IN4)))
GE12 = 
 (OR (AND (NOT IN5) IN4 IN3) (AND IN7 IN6 (NOT IN2)) (AND GE0 IN5))
GE19 = 
 (OR (AND (NOT IN6) IN3) (AND IN6 (NOT IN0)))
GE48 = 
 (OR (AND (NOT IN5) IN4) (AND (NOT IN4) IN3))
GE14 = 
 (OR (AND GE1 IN8) (AND GE5 IN6) (AND (NOT IN5) IN4 IN3))
GE29 = 
 (OR (AND GE1 (NOT IN5)) (AND GE27 (NOT IN3)))
GE7 = 
 (OR (AND (NOT IN8) IN7 IN3) (AND IN8 (NOT IN6) IN4) (AND GE3 IN6) (AND GE1 
IN0))
GE4 = 
 (OR (AND (NOT IN4) IN0) (AND IN7 (NOT IN0)))
GE18 = 
 (OR (AND (NOT IN7) IN4) (AND (NOT IN8) IN5))
GE13 = 
 (OR (AND IN7 IN5 (NOT IN2)) (AND IN3 IN2 (NOT IN0)) (AND GE5 IN0))
GE33 = 
 (OR (AND GE21 IN2) (AND IN8 IN3 (NOT IN2)))
GE17 = 
 (OR (AND (NOT IN7) IN5 IN2 (NOT IN0)) (AND GE6 IN7 (NOT IN6)) (AND GE9 IN6 
(NOT IN4)) (AND GE8 (NOT IN5) IN4))
GE22 = 
 (OR GE0 (AND (NOT IN3) IN0))
GE25 = 
 (OR (AND IN6 (NOT IN5) IN4 (NOT IN3)) (AND IN5 (NOT IN4) (NOT IN1) IN0) (AND 
GE24 (NOT IN2) IN1) (AND GE16 (NOT IN0)))
GE20 = 
 (OR (AND IN7 (NOT IN5) (NOT IN4) IN2) (AND (NOT IN6) IN5 IN3 (NOT IN2)) (AND 
GE12 (NOT IN1)))
GE35 = 
 (OR (AND GE6 (NOT IN6) IN4) (AND GE19 (NOT IN4) IN2))
GE43 = 
 (OR (AND (NOT IN1) IN0) (AND IN1 (NOT IN0)))
GE44 = 
 (OR (AND (NOT IN8) IN6) (AND IN8 (NOT IN7)))
GE47 = 
 (OR (AND (NOT IN6) IN5) (AND (NOT IN7) IN6))
GE11 = 
 (OR (AND IN8 (NOT IN2)) GE48)
GE49 = 
 (OR (AND GE14 (NOT IN2)) (AND GE29 IN2))
GE28 = 
 (OR (AND IN6 (NOT IN4) (NOT IN3) IN0) (AND (NOT IN7) IN3 IN2 (NOT IN0)) (AND 
GE7 (NOT IN2)))
GE32 = 
 (OR (AND GE4 IN8) (AND GE18 IN6))
GE36 = 
 (OR (AND GE13 (NOT IN4)) (AND GE33 (NOT IN7)))
GE38 = 
 (OR (AND GE22 IN5 IN4 (NOT IN1)) (AND GE17 IN3) (AND GE25 IN7))
GE39 = 
 (OR (AND GE1 GE0 (NOT IN5)) (AND GE20 (NOT IN0)) (AND GE35 (NOT IN7)))
GE2 = 
 (OR GE44 GE43)
GE10 = 
 (OR (AND (NOT IN5) IN1) GE47)
GE50 = 
 (OR (AND GE11 GE0) GE49)
GE37 = 
 (OR (AND GE32 (NOT IN3) (NOT IN2)) (AND GE28 (NOT IN5)) (AND GE36 (NOT IN6)))
GE40 = 
 (OR (AND GE38 (NOT IN8)) (AND GE39 IN8))
GE23 = 
 (OR (AND (NOT IN8) IN1) (AND IN3 (NOT IN0)))
GE30 = 
 (OR (AND GE2 IN5 (NOT IN4)) (AND GE0 (NOT IN8) IN1) (AND GE4 IN6 (NOT IN1)) 
(AND GE10 IN4 (NOT IN0)))
GE31 = 
 (OR (AND (NOT IN7) (NOT IN6) IN5 IN3) GE50)
GE41 = 
 (OR GE40 (AND GE37 IN1))
GE26 = 
 (OR (AND IN8 (NOT IN6) (NOT IN3) IN0) (AND GE23 (NOT IN7) IN4) (AND GE3 (NOT 
IN4) IN1))
GE34 = 
 (OR (AND IN7 (NOT IN6) (NOT IN5) IN4) (AND IN5 (NOT IN4) IN3 (NOT IN1)))
GE42 = 
 (OR (AND GE31 (NOT IN1) IN0) (AND GE30 (NOT IN3) IN2) GE41)

@out
OUT0 = 
 (OR GE42 (AND GE26 IN5 (NOT IN2)) (AND GE34 IN2 (NOT IN0)))
@end

@BE2
@invar
(IN0 IN1 IN2 IN3 IN4 IN5 IN6 IN7 IN8)

@sub
WRES1 = 
 (OR (AND (NOT IN4) IN3) (AND IN4 (NOT IN3)))
WRES3 = 
 (OR (AND IN3 (NOT IN0)) (AND IN8 (NOT IN7)))
WRES5 = 
 (OR (AND (NOT IN8) IN5) (AND IN7 (NOT IN3)))
WRES21 = 
 (OR (AND IN8 (NOT IN3)) (AND (NOT IN8) IN0))
WRES15 = 
 (OR (AND IN5 (NOT IN4)) (AND IN6 (NOT IN3)))
WRES0 = 
 (OR (AND (NOT IN7) IN6) (AND IN7 (NOT IN6)))
WRES27 = 
 (OR (AND IN8 (NOT IN7)) (AND IN7 (NOT IN6)))
WRES7 = 
 (OR (AND WRES3 IN6) (AND IN8 (NOT IN6) IN4) (AND (NOT IN8) IN7 IN3) (AND WRES1 
IN0))
WRES4 = 
 (OR (AND (NOT IN4) IN0) (AND IN7 (NOT IN0)))
WRES18 = 
 (OR (AND (NOT IN7) IN4) (AND (NOT IN8) IN5))
WRES13 = 
 (OR (AND IN7 IN5 (NOT IN2)) (AND WRES5 IN0) (AND IN3 IN2 (NOT IN0)))
WRES33 = 
 (OR (AND WRES21 IN2) (AND IN8 IN3 (NOT IN2)))
WRES6 = 
 (OR (AND (NOT IN5) IN2) (AND IN5 (NOT IN0)))
WRES8 = 
 (OR (AND (NOT IN7) IN1) (AND IN2 (NOT IN1)) (AND IN6 (NOT IN0)))
WRES9 = 
 (OR (AND (NOT IN7) IN5) (AND IN7 (NOT IN1)) (AND (NOT IN5) IN2))
WRES16 = 
 (OR (AND WRES15 IN2) (AND IN4 IN3 (NOT IN2)) (AND IN6 IN5 (NOT IN1)))
WRES24 = 
 (OR (AND (NOT IN6) IN4) (AND IN6 (NOT IN4)))
WRES12 = 
 (OR (AND WRES0 IN5) (AND (NOT IN5) IN4 IN3) (AND IN7 IN6 (NOT IN2)))
WRES19 = 
 (OR (AND (NOT IN6) IN3) (AND IN6 (NOT IN0)))
WRES23 = 
 (OR (AND (NOT IN8) IN1) (AND IN3 (NOT IN0)))
WRES2 = 
 (OR (AND IN8 (NOT IN7)) (AND (NOT IN8) IN6) (AND (NOT IN1) IN0) (AND IN1 (NOT 
IN0)))
WRES10 = 
 (OR (AND (NOT IN6) IN5) (AND (NOT IN5) IN1) (AND (NOT IN7) IN6))
WRES11 = 
 (OR (AND (NOT IN5) IN4) (AND IN8 (NOT IN2)) (AND (NOT IN4) IN3))
WRES14 = 
 (OR (AND WRES1 IN8) (AND WRES5 IN6) (AND (NOT IN5) IN4 IN3))
WRES29 = 
 (OR (AND WRES1 (NOT IN5)) (AND WRES27 (NOT IN3)))
WRES28 = 
 (OR (AND WRES7 (NOT IN2)) (AND IN6 (NOT IN4) (NOT IN3) IN0) (AND (NOT IN7) IN3 
IN2 (NOT IN0)))
WRES32 = 
 (OR (AND WRES4 IN8) (AND WRES18 IN6))
WRES36 = 
 (OR (AND WRES13 (NOT IN4)) (AND WRES33 (NOT IN7)))
WRES17 = 
 (OR (AND WRES6 IN7 (NOT IN6)) (AND WRES8 (NOT IN5) IN4) (AND WRES9 IN6 (NOT 
IN4)) (AND (NOT IN7) IN5 IN2 (NOT IN0)))
WRES22 = 
 (OR WRES0 (AND (NOT IN3) IN0))
WRES25 = 
 (OR (AND IN6 (NOT IN5) IN4 (NOT IN3)) (AND WRES24 (NOT IN2) IN1) (AND IN5 (NOT 
IN4) (NOT IN1) IN0) (AND WRES16 (NOT IN0)))
WRES20 = 
 (OR (AND IN7 (NOT IN5) (NOT IN4) IN2) (AND (NOT IN6) IN5 IN3 (NOT IN2)) (AND 
WRES12 (NOT IN1)))
WRES35 = 
 (OR (AND WRES6 (NOT IN6) IN4) (AND WRES19 (NOT IN4) IN2))
WRES26 = 
 (OR (AND WRES23 (NOT IN7) IN4) (AND WRES3 (NOT IN4) IN1) (AND IN8 (NOT IN6) 
(NOT IN3) IN0))
WRES30 = 
 (OR (AND WRES2 IN5 (NOT IN4)) (AND WRES0 (NOT IN8) IN1) (AND WRES4 IN6 (NOT 
IN1)) (AND WRES10 IN4 (NOT IN0)))
WRES31 = 
 (OR (AND WRES11 WRES0) (AND (NOT IN7) (NOT IN6) IN5 IN3) (AND WRES29 IN2) (AND 
WRES14 (NOT IN2)))
WRES34 = 
 (OR (AND IN7 (NOT IN6) (NOT IN5) IN4) (AND IN5 (NOT IN4) IN3 (NOT IN1)))
WRES37 = 
 (OR (AND WRES32 (NOT IN3) (NOT IN2)) (AND WRES28 (NOT IN5)) (AND WRES36 (NOT 
IN6)))
WRES38 = 
 (OR (AND WRES22 IN5 IN4 (NOT IN1)) (AND WRES17 IN3) (AND WRES25 IN7))
WRES39 = 
 (OR (AND WRES20 (NOT IN0)) (AND WRES1 WRES0 (NOT IN5)) (AND WRES35 (NOT IN7)))

@out
OUT0 = 
 (OR (AND WRES26 IN5 (NOT IN2)) (AND WRES34 IN2 (NOT IN0)) (AND WRES31 (NOT 
IN1) IN0) (AND WRES30 (NOT IN3) IN2) (AND WRES37 IN1) (AND WRES38 (NOT IN8)) 
(AND WRES39 IN8))
@end