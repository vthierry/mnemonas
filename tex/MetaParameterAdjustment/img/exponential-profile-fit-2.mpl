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
sl_ab_0 := z(solve(map(limit, eq_ab, gT = 0), {beta, nu}));
x := t -> beta + nu * c^(-t):
eq_abc_ab := z({p(-1) = p1, p(-2) = p2});
sl_abc_ab_0 := z(solve(map(limit, eq_abc_ab, gT = 0), {beta, nu}));

eq_abc_2 := z({p(-1) = p1, p(-2) = p2, p(-3) = p3}):
eq_abc_zc := collect(op(2, eliminate(eq_abc_2, {beta, nu})), {cT, c}, e -> z(e), distributed);
eq_abc_zc_0 := limit(op(1,eq_abc_zc)/(1-c), gT = 0):
sl_abc_zc_0 := solve(eq_abc_zc_0, c);
eq_abc_zc_l := factor(expand(subs({p1 = c_, p2 = c_^2, p3 = c_^3}, eq_abc_zc_0)));

# Algo
# - a from eq_a
# - a,b from eq_ab
# - c0 from sl_abc_zc_0
# - lineseach (|eq_abc_zc/(1-c)|, 0..(1+c0)/2)
# - a,b given c from eq_abc_ab
#
code1 := [
 op(sl_a),
 ab_aff_00 = coeff(op(1, op(1, eq_ab)), beta, 1),
 ab_aff_01 = coeff(op(1, op(1, eq_ab)), nu, 1),
 ab_aff_10 = coeff(op(1, op(2, eq_ab)), beta, 1),
 ab_aff_11 = coeff(op(1, op(2, eq_ab)), nu, 1),
 c_0 = sl_abc_zc_0
]:
CodeGeneration[C](code1, optimize);

code2 := [
 zero = op(1, eq_abc_zc)
]:
CodeGeneration[C](code2, optimize);

code3 := [
 ab_exp_00 = coeff(op(1, op(1, eq_abc_ab)), beta, 1),
 ab_exp_01 = coeff(op(1, op(1, eq_abc_ab)), nu, 1),
 ab_exp_10 = coeff(op(1, op(2, eq_abc_ab)), beta, 1),
 ab_exp_11 = coeff(op(1, op(2, eq_abc_ab)), nu, 1)
]:
CodeGeneration[C](code3, optimize);



