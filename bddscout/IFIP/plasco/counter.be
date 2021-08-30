@BE1
@invar
(CA CB A0 A1 A2 A3)

@out
TCB = 
(
CA)
TA0 = 
(
OR
(AND CB A0)
(AND (NOT CA) A0)
(AND CA (NOT CB) (NOT A0)))
TA1 = 
(
OR
(AND CB A1)
(AND (NOT CA) A1)
(AND CA (NOT CB) (NOT A0) A1 (NOT A3))
(AND CA (NOT CB) A0 (NOT A1) (NOT A3)))
TA2 = 
(
OR
(AND CB A2)
(AND (NOT CA) A2)
(AND CA (NOT CB) A0 A1 (NOT A2))
(AND CA (NOT CB) (NOT A0) A1 A2)
(AND CA (NOT CB) (NOT A1) A2))
TA3 = 
(
OR
(AND CB A3)
(AND (NOT CA) A3)
(AND CA (NOT CB) A0 A1 A2)
(AND CA (NOT CB) (NOT A0) A3))
B0 = 
(
OR
(AND (NOT A0) A1)
(AND (NOT A0) (NOT A2)))
B1 = 
(
OR
(AND (NOT A0) A2)
(A3)
(AND (NOT A1) A2)
(AND (NOT A0) (NOT A1)))
B2 = 
(
OR
(AND (NOT A0) (NOT A2))
(AND A0 A2)
(A1)
(A3))
B3 = 
(
OR
(AND (NOT A2) (NOT A3))
(AND A0 A1)
(AND (NOT A0) (NOT A1))
(A3))
B4 = 
(
OR
(A0)
(A2)
((NOT A1)))
B5 = 
(
OR
(AND A0 (NOT A1) A2)
(AND (NOT A0) A1)
(AND A1 (NOT A2))
(AND (NOT A0) (NOT A2))
(A3))
B6 = 
(
OR
(AND A1 (NOT A2))
(AND (NOT A1) A2)
(AND (NOT A0) A1)
(A3))

@end


@BE2
@invar
(CA CB A0 A1 A2 A3)

@out
TCB = 
(
CA)
TA0 = 
(
OR
(AND CB A0)
(AND (NOT CA) A0)
(AND CA (NOT CB) (NOT A0)))
TA1 = 
(
OR
(AND (NOT A0) A1 (NOT A3))
(AND CA (NOT CB) A0 (NOT A1) (NOT A3))
(AND CB A1)
(AND (NOT CA) A1))
TA2 = 
(
OR
(AND (NOT A0) A2)
(AND A0 (NOT A1) A2)
(AND CB A0 A1 A2)
(AND (NOT CA) A0 A1 A2)
(AND CA (NOT CB) A0 A1 (NOT A2)))
TA3 = 
(
OR
(AND (NOT A0) A3)
(AND CB A0 A3)
(AND (NOT CA) A3)
(AND CA (NOT CB) A0 A1 A2))
B0 = 
(
OR
(AND (NOT A0) (NOT A1) (NOT A2))
(AND (NOT A0) A1))
B1 = 
(
OR
(AND (NOT A0) A3)
(AND A0 A3)
(AND (NOT A0) A2)
(AND A0 (NOT A1) A2)
(AND (NOT A0) (NOT A1) (NOT A2)))
B2 = 
(
OR
(AND (NOT A0) A3)
(AND A0 A3)
(AND (NOT A0) A1 (NOT A3))
(AND CA (NOT CB) A0 A1 A2)
(AND A0 (NOT A1) A2)
(AND CB A0 A1 A2)
(AND (NOT CA) A0 A1 A2)
(AND A1 (NOT A2))
(AND (NOT A0) (NOT A1) (NOT A2)))
B3 = 
(
OR
(AND (NOT A0) A3)
(AND A0 A3)
((NOT A2))
(AND CA (NOT CB) A0 A1 A2)
(AND (NOT A0) (NOT A1) A2)
(AND CB A0 A1 A2)
(AND (NOT CA) A0 A1 A2))
B4 = 
(
OR
(AND A0 A3)
(AND CA (NOT CB) A0 (NOT A1) (NOT A3))
(AND CA (NOT CB) A0 A1 A2)
(AND (NOT A0) A2)
(AND CA (NOT CB) A0 A1 (NOT A2))
(AND (NOT A0) (NOT A1) (NOT A2))
(AND CB A0)
(AND (NOT CA) A0))
B5 = 
(
OR
(AND (NOT A0) A3)
(AND A0 A3)
(AND A0 (NOT A1) A2)
(AND A1 (NOT A2))
(AND (NOT A0) (NOT A1) (NOT A2))
(AND (NOT A0) A1))
B6 = 
(
OR
(AND (NOT A0) A3)
(AND A0 A3)
(AND A0 (NOT A1) A2)
(AND (NOT A0) (NOT A1) A2)
(AND A1 (NOT A2))
(AND (NOT A0) A1))

@end
