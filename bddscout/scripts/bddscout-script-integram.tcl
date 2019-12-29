# Example script for Bdd Scout
# Robert Meolic, 2019

# LOGIČNA UGANKA (Slovene)
# 1. Andrej ali Borut je iz Maribora.
# 2. Največji je iz Ljubljane, najmanjši pa ni iz Celja.
# 3. Če je Andrej manjši od Ceneta, potem ni iz Maribora.
# 4. Cene je večji od tistega iz Celja.

# LOGIC RIDDLE (English)
# 1. Andrej or Borut is from Maribor.
# 2. The largest is from Ljubljana, the smallest is not from Celje.
# 3. If Andrej is smaller than Cene, then he is not from Maribora.
# 4. Cene is larger than the one from Celje.

# Encoding:
# Person: Andrej(A), Borut(B), Cene(C)
# City: Maribor(mb) ~1 ~0, Ljubljana(lj) ~1 0, Celje(ce): 1 ~0
# Size: small(s) ~3 ~2, middle(m) ~3 2, large(l) 3 ~2
#
# Booolean equations:
# E1.  Amb + Bmb
# E2a. (Al -> Alj) * (Bl -> Blj) * (Cl -> Clj)
# E2b. (As -> ~Ace) * (Bs -> ~Bce) * (Cs -> ~Cce)
# E3.  (As * Cm + As * Cl + Am * Cl) -> ~Amb
# E4a. (Ace * As + Bce * Bs) * (Cm + Cl)
# E4b. (Ace * Am + Bce * Bm) * Cl

# RESULT:
# Ace * Bmb * Clj * Am * Bs * Cl

proc EVAL { s } {bddscout_parse_input_infix $s}

# switch BDD manager (if neccessary)
bddscout_change_bddtype "ROBDD"

EVAL "Amb = ~a1 * ~a0"
EVAL "Alj = ~a1 * a0"
EVAL "Ace = a1 * ~a0"
EVAL "Bmb = ~b1 * ~b0"
EVAL "Blj = ~b1 * b0"
EVAL "Bce = b1 * ~b0"
EVAL "Cmb = ~c1 * ~c0"
EVAL "Clj = ~c1 * c0"
EVAL "Cce = c1 * ~c0"

EVAL "As = ~a2 * ~a3"
EVAL "Am = ~a2 * a3"
EVAL "Al = a2 * ~a3"
EVAL "Bs = ~b2 * ~b3"
EVAL "Bm = ~b2 * b3"
EVAL "Bl = b2 * ~b3"
EVAL "Cs = ~c2 * ~c3"
EVAL "Cm = ~c2 * c3"
EVAL "Cl = c2 * ~c3"

EVAL "SYSTEM1 = Amb * Blj * Cce + Amb * Bce * Clj + Alj * Bmb * Cce + Alj * Bce * Cmb + Ace * Bmb * Clj + Ace * Blj * Cmb"
EVAL "SYSTEM2 = As * Bm * Cl + As * Bl * Cm + Am * Bs * Cl + Am * Bl * Cs + Al * Bs * Cm + Al * Bm * Cs"

EVAL "E1 = Amb + Bmb"
EVAL "E2a = (~Al + Alj) * (~Bl + Blj) * (~Cl + Clj)"
EVAL "E2b = (~As + ~Ace) * (~Bs + ~Bce) * (~Cs + ~Cce)"
EVAL "E2 = T2a * T2b"
EVAL "E3 =  ~(As * Cm + As * Cl + Am * Cl) + ~Amb"
EVAL "E4a = (Ace * As + Bce * Bs) * (Cm + Cl)"
EVAL "E4b = (Ace * Am + Bce * Bm) * Cl"
EVAL "E4 = T4a + T4b"
EVAL "RESULT = SYSTEM1 * SYSTEM2 * T1 * T2 * T3 * T4"

# update GUI - refresh list of known Boolean functions in the current BDD manager
# this will not select/show any of the constructed function
bddscout_update
