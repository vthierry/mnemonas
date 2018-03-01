#assume(0 < g, g < 1, 0 < c):

S := (t, x) -> sum((1-g) * g^s * x(t-s), s = 0..T-t):
p := t -> S(t, x):

# Solution for the different models
z:= e -> collect(subs(g^T = gT / g^2, c^T = cT / c^2, expand(e)), {beta, nu, c}, e -> collect(e, gT, factor)):

x := t -> beta_0:
eq_a := z({p(-1) = p1});
sl_a := solve(z({p(-1) = p1}), beta_0);
sl_a_0 := z(solve(map(limit, eq_a, gT = 0), {beta_0}));
x := t -> beta + nu * t:
eq_ab := z({p(-1) = p1, p(-2) = p2});
m01 = collect(coeff(op(1, op(1, eq_ab)), nu, 1), [T, gT], factor);
m10 = collect(coeff(op(1, op(2, eq_ab)), nu, 1), [T, gT], factor);
sl_ab_0 := z(solve(map(limit, eq_ab, gT = 0), {beta, nu}));
x := t -> beta + nu * c^(-t):
eq_abc_ab := z({p(-1) = p1, p(-2) = p2});
sl_abc_ab_0 := z(solve(map(limit, eq_abc_ab, gT = 0), {beta, nu}));
m01 = factor(coeff(op(1, op(1, eq_abc_ab)), nu, 1));
m10 = factor(coeff(op(1, op(2, eq_abc_ab)), nu, 1));

eq_abc_2 := z({p(-1) = p1, p(-2) = p2, p(-3) = p3}):
eq_abc_zc := collect(op(2, eliminate(eq_abc_2, {beta, nu})), {cT, c}, e -> z(e), distributed);
eq_abc_zc_0 := limit(op(1,eq_abc_zc)/(1-c), gT = 0):
sl_abc_zc_0 := solve(eq_abc_zc_0, {c});
eq_abc_zc_l := z(expand(subs({p1 = c_, p2 = c_^2, p3 = c_^3}, eq_abc_zc))):
eq_abc_zc_l_0 := factor(expand(subs({p1 = c_, p2 = c_^2, p3 = c_^3}, eq_abc_zc_0)));
