#assume(0 < g, g < 1, 0 < c):

S := (t, x) -> sum((1-g) * g^s * x(t-s), s = 0..T-t):
p := t -> S(t, x):

# Solution for the different models
z:= e -> collect(subs(g^T = gT / g^2, c^T = cT / c^2, expand(e)), {a, b, c}, e -> collect(e, gT, factor)):

x := t -> a:
eq_a := z({p(-1) = p1});
#sl_a_0 := z(solve(map(limit, eq_a, gT = 0), {a}));
x := t -> a + b * t:
eq_ab := z({p(-1) = p1, p(-2) = p2});
#sl_ab_0 := z(solve(map(limit, eq_ab, gT = 0), {a, b}));
x := t -> a + b * c^(-t):
eq_abc_ab := z({p(-1) = p1, p(-2) = p2});
#sl_abc_ab_0 := z(solve(map(limit, eq_abc_ab, gT = 0), {a, b}));
#eq_abc_1 := z({p(-1) = p1, p(-2) = p2, p(-3) = p3, p(-4) = p4});
#eq_abc_c := collect(factor(map(e -> e / (c^2 * (1 - gT * g) - g * (1 - gT)) / (1 - gT), op(2, eliminate(eq_abc_1, {a, b, cT})))), c, factor);
#eq_abc_c_0 := map(e -> collect(factor(e / (g-c^2)), c, factor), map(limit, eq_abc_c, gT = 0));

eq_abc_2 := z({p(-1) = p1, p(-2) = p2, p(-3) = p3}):
eq_abc_zc := collect(op(2, eliminate(eq_abc_2, {a, b})), {cT, c}, e -> z(e), distributed);
eq_abc_zc_0 := map(limit, eq_abc_zc, gT = 0):
sl_abc_zc_0 := solve(eq_abc_zc_0, c);
eq_abc_zc_l := factor(expand(subs({p1 = c_, p2 = c_^2, p3 = c_^3}, eq_abc_zc_0)));

# Algo
# - a from eq_a
# - a,b from eq_ab
# - c0 from sl_abc_zc_0
# - lineseach (|eq_abc_zc/(1-c)|, 0..(1+c0)/2)
# - a,b given c from eq_abc_ab
#
CodeGeneration[C](op(1,eq_abc_zc));


