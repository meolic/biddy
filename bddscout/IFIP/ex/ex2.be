@BE1
@invar
(a b c d)

@sub
s1 = (or (not a) d)

@out
o = (and b s1)

@end


@BE2
@invar
(a b c d)

@sub
s2 = (or (not c) d)

@out
o = (and b s2)

@end

@DCS
(or (and a b (not c))   (and (not a) b c (not d))  )
