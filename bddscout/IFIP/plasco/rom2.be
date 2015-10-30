@BE1
@invar
(P4 P5 P1 P3 P2 ERR A6 A2 A5 A4 A1 A3 A0)

@out
R1 = 
(
OR
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A2) (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A4 (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND A2 (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) P5))
R2 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) ERR)
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND P1 P2 P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A3) P1 P2 (NOT P3) (NOT P4) (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A5 P1 (NOT P2) P3 (NOT P4) (NOT P5))
(AND A2 (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND A1 P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R3 = 
(
OR
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND P1 P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A5) P1 (NOT P2) P3 (NOT P4) (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A5 P1 (NOT P2) P3 (NOT P4) (NOT P5))
(AND A4 (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND A3 P1 P2 (NOT P3) (NOT P4) (NOT P5)))
R4 = 
(
OR
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND P1 P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A5) P1 (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A3) P1 P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A2) (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A1) P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R5 = 
(
OR
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A0) (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND A0 (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R6 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) ERR)
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) ERR))
MUX = 
(
OR
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) P5)
(AND (NOT P1) (NOT P2) (NOT P3) P4)
(AND (NOT P1) (NOT P2) P3)
(AND (NOT P1) P2)
(P1))

@end

@BE2
@invar
(A0 A1 A2 A3 A4 A5 A6 P1 P2 P3 P4 P5 ERR)

@out
R1 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4))
(AND (NOT A0) (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A4 (NOT P1) P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND A2 (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A2) (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5)))
R2 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) ERR)
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) (NOT P1) (NOT P2) (NOT P3) P4 (NOT P5))
(AND P1 P2 P3 (NOT P4) (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A5 P1 P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A3) P1 P2 (NOT P4) (NOT P5))
(AND A2 (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND A1 P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R3 = 
(
OR
(AND (NOT A3) (NOT P1) (NOT P2) P3 P4 (NOT P5) ERR)
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND A1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND A3 (NOT P1) (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A5 P1 P3 (NOT P4) (NOT P5))
(AND (NOT A5) P1 P3 (NOT P4) (NOT P5))
(AND A4 (NOT P1) P3 (NOT P4) (NOT P5))
(AND A3 P1 P2 (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R4 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A2) P1 P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A1) (NOT P1) P2 (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A0) P1 (NOT P2) (NOT P3) P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND A2 P1 P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A1 (NOT P1) P2 (NOT P3) P4 (NOT P5) (NOT ERR))
(AND A0 P1 (NOT P2) (NOT P3) P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) P3 P4 (NOT P5))
(AND P1 P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A5) P1 P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND (NOT A3) P1 P2 (NOT P4) (NOT P5))
(AND (NOT A2) (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A1) P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5)))
R5 = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) P3 P4 (NOT P5))
(AND (NOT P1) (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P3) P4 (NOT P5)))
R6 = 
(
OR
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR)))
MUX = 
(
OR
(AND A6 P1 P2 P3 P4 (NOT P5) ERR)
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) ERR)
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) ERR)
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) ERR)
(AND A6 P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A6) P1 P2 P3 P4 (NOT P5) (NOT ERR))
(AND A5 (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A5) (NOT P1) P2 P3 P4 (NOT P5) (NOT ERR))
(AND A4 P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT A4) P1 (NOT P2) P3 P4 (NOT P5) (NOT ERR))
(AND (NOT P1) (NOT P2) P3 P4 (NOT P5))
(AND A6 (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND (NOT A6) (NOT P1) P2 P3 (NOT P4) (NOT P5))
(AND A5 P1 P3 (NOT P4) (NOT P5))
(AND (NOT A5) P1 P3 (NOT P4) (NOT P5))
(AND A4 (NOT P1) P3 (NOT P4) (NOT P5))
(AND (NOT A4) (NOT P1) (NOT P2) P3 (NOT P4) (NOT P5))
(AND A3 P1 P2 (NOT P4) (NOT P5))
(AND (NOT A3) P1 P2 (NOT P4) (NOT P5))
(AND A2 (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A2) (NOT P1) P2 (NOT P3) (NOT P4) (NOT P5))
(AND A1 P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT A1) P1 (NOT P2) (NOT P3) (NOT P4) (NOT P5))
(AND (NOT P3) P4 (NOT P5))
(P5))

@end