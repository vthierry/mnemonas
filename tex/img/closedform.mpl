# Calculates the step signal threshold
eqh := h = 1 / (4 * (1 - 2^(1/2-tau))):
simplify(subs(rsolve({c(t) = c(t-1) / 2 + h, c(0) = 0}, {c(t)}), eqh, t = tau-1/2, {c(t) - 1/2}));
delta_h :=simplify( subs(rau = 2^tau, series(simplify(subs(tau = log(rau)/log(2), subs(eqh, tau = tau - 1, h) - subs(eqh, h))), rau=infinity, 2)));
# Calculates the step signal threshold with simmoid function
upsilon := (u) -> 1/(1+exp(-4*(u - 1/2)/epsilon)):
c_n := c -> (1 - upsilon(c)) * c / 2 + h:
h_oo := solve(c_n(1/2) = 1/2, h);
h_1 := solve(c_n(0) = 1/2, h);

# Solve the step clock sequence problem
z  := (t, T) -> if t < T - 1 then o[t] - o[t+1] else o[t] fi:
eqs := eval(subs(T = 6, {'seq(sum('z(n, T)', n = t .. T-1) - o[t], t = 0 .. T-1)'}));
delta_h := simplify(subs(tau = 2^rau, subs(eqh, tau = tau + 1, h) - subs(eqh, h)));
delta_h := simplify(subs(tau = log(rau)/log(2), subs(eqh, tau = tau + 1, h) - subs(eqh, h)));
