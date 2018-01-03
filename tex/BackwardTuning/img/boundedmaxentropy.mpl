# Defines a truncated exponential profile, for u = 0..1
assume(0 <= u, u < 1);
a := b / (exp(b) - (b + 1)):
p_b := u -> a * (exp(b * (1 - u)) - 1):
# Verifies constraints and compute the mean m, standard deviation sqrt(s2) and entropy e
sl_b := simplify(solve({
 int(p_b(u), u = 0..1) = 1,
 p_b(1) = 0,
 int(p_b(u) * u, u = 0..1) = m,
 int(p_b(u) * u^2, u = 0..1) = s2 + m^2,
 -int(p_b(u) * log(p_b(u)), u = 0..1) = e
}, {m, e, s2}));
# Verifies that the profile is between uniform, linear and dirac profiles
simplify([
 p_min   = limit(p_b(u), b = -infinity),
 m_min   = limit(subs(sl_b, m), b = -infinity),
 s2_min  = limit(subs(sl_b, s2), b = -infinity),
 e_min   = limit(subs(sl_b, e), b = -infinity),
 e_min_b = simplify(series(subs(sl_b, e), b = -infinity, 3)),
 p_0     = limit(p_b(u), b = 0),
 m_0     = limit(subs(sl_b, m), b = 0),
 s2_0    = limit(subs(sl_b, s2), b = 0),
 e_0     = limit(subs(sl_b, e), b = 0),
 p_max   = limit(p_b(u), b = infinity),
 m_max   = limit(subs(sl_b, m), b = infinity),
 s2_max  = limit(subs(sl_b, s2), b = infinity),
 e_max   = limit(subs(sl_b, e), b = infinity)
]);
# Designs a change of variable for better representation
c_b := b -> 1 / (2 + exp(b / 4)):
simplify({
 limit(c_b(b), b = infinity)   - limit(subs(sl_b, m), b = infinity),
 limit(c_b(b), b = -infinity)  - limit(subs(sl_b, m), b = -infinity),
 limit(c_b(b), b = 0)          - limit(subs(sl_b,m), b = 0),
 limit(diff(c_b(b), b), b = 0) - limit(diff(subs(sl_b, m), b), b = 0)
});
b_c := unapply(solve(c_b(b) = c, b), c);
exp_b_c := simplify(exp(b_c(c)));
simplify([
 m_min = series(subs(sl_b, b = b_c(c), m), c, 1),
 s2_min = series(subs(sl_b, b = b_c(c), s2), c, 1),
 e_min = series(subs(sl_b, b = b_c(c), s2), c, 1)
# m_max = series(subs(sl_b, b = b_c(c), m), c = 1/2, 1),
# s2_max = series(subs(sl_b, b = b_c(c), s2), c = 1/2, 1),
# e_max = series(subs(sl_b, b = b_c(c), s2), c = 1/2, 1)
]);

# Plots the mean, standard deviation, entropy and profiles
#plot(subs(sl_b, m),                     b = -100..100, title="m(b)", labels=["", ""]);
#plot([subs(sl_b, b = b_c(c), m), c],    c = 0..1/2, title="m(c)", labels=["", ""]);
#plot([subs(sl_b, b = b_c(c), sqrt(s2)),  c/sqrt(3), convert(evalf(expand(series(subs(sl_b, b = b_c(c), sqrt(s2)), c = 1/10, 2))),polynom)], c = 0..1/2, title="s(c)", labels=["", ""]);
#plot(subs(sl_b, b = b_c(c), e/log(2)), c = 0..1/2,         title="e(c)", labels=["", ""]);
#plot(subs(sl_b, b = b_c(c), e/log(2)), c = 0..1/1000,      title="e(c)", labels=["", ""]);
#plot(subs(sl_b, b = b_c(c), e/log(2)), c = 1/2-1/10..1/2, title="e(c)", labels=["", ""]);
#c_e_saddle := fsolve(factor(diff(subs(sl_b, b = b_c(c), e), c$2)), c, 0.46..0.47);
#e_c_zero := map(d -> evalf(subs(sl_b, b = b_c(10^(-10^d)), e)), [1,2,3,4]);
#plot(map(c -> if b_c(c) = 0 then limit(p_b(u), b = 0) else subs(b = b_c(c), p_b(u)) fi, [0.4999, fsolve(subs(b = b_c(c), p_b(0)) = 3, c), 0.333, fsolve(subs(b = b_c(c), p_b(0)) = 5, c)]), u = 0..1, title="p(u)", labels=["", ""]);

# Plots s(m)
#with(ArrayTools):
#s_m := m0 -> if m0 = 0 then 0 elif m0 = 1/2 then evalf(1/sqrt(12)) else evalf(subs(sl_b, b = fsolve(subs(sl_b, m) = m0, b, maxsols = 1), sqrt(s2))) fi:
#N := 100: l_m := evalf([seq(u/2/N,u=0..N)]): l_s := Array(map(m0 -> s_m(m0), l_m)):
#for i from 2 to Size(l_s, 2) - 1 do if l_s[i] = 0 then l_s[i] := (l_s[i-1]+l_s[i+1])/2: print("correcting: ", i, l_s[i]) fi od;
#for i to Size(l_s, 2) do if not (evalf(l_m[i] * 2 / sqrt(12)) <= l_s[i] and l_s[i] <= l_m[i]) then print(i, l_m[i], l_s[i], l_m[i] / 0.5 / sqrt(12)) fi od;
#plot(l_m, l_s, title="s(m)", labels=["", ""]);

# Builds look-up tables
N := 100:
#m_c := [0, op(map(k ->  evalf(subs(sl_b, b = b_c(c), c = k / (2 * N), m)), [$1..N-1])), 0.5];
#s_c := [0, op(map(k ->  evalf(subs(sl_b, b = b_c(c), c = k / (2 * N), sqrt(s2))), [$1..N-1])), 12.0^(-0.5)];
#e_c := [8, op(map(k ->  evalf(subs(sl_b, b = b_c(c), c = k / (2 * N), e)), [$1..N-1])), 0];
# Relative entropy
#p_b0 := unapply(limit(p_b(u), b = 0), u):
#D_b_b0 := int(p_b(u) * log(p_b(u)), u = 0..1) + int(p_b(u) * log(p_b0(u)), u = 0..1);
#D_b0_b := int(p_b0(u) * log(p_b0(u)/p_b(u)), u = 0..1);
#D_u_b := -int(1 * log(p_b(u)/1), u = 0..1);
#plot(subs(b = b_c(c), Db0/log(2)), c = 0..1/2,         title="e(c)", labels=["", ""]);
#plot(subs(b = b_c(c), D0b/log(2)), c = 0..1/2,         title="e(c)", labels=["", ""]);


