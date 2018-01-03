# The recurrent equation
assume(0 < g, g < 1):
subs(z(t0) = (1-g) * x(t0), rsolve({y(t) = g * y(t-1) + z(t), y(0) = 0}, {y(t)}));
# The normalization
simplify((1 - g) * sum(g^t, t=0..infinity));
simplify(solve({(1 - g) * sum(g^t, t=0..W-1) = 1 - e}, {g}));
# Looking at values
Array(1..8, 1..9, proc(i, j) local e: e := array([0, 0.1,0.05,0.02,0.01,0.005,0.002,0.001]): return `if`(i = 1, `if`(j <= 2, "", 2^j), `if`(j = 1, e[i], `if`(j = 2, ":", evalf[2](e[i]^(1/(2^j)))))): end);
