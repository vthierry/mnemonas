assume(0 < g, g < 1):
S := (t, x) -> sum((1-g) * g^s * x(s), s = T..t):
p := t -> S(t, x):
v := t -> S(t, t -> x(t) - x(t-1)):

x := t -> a:
solve({p(1) - p(0) = g * (1-g) * d0}, {a});
x := t -> a + b * t:
solve({p(1) - p(0) = g * (1-g) * d0, p(2) - p(1) = g^2 * (1-g) * d1}, {a,b});
x := t -> a + b * c^t:
solve({p(1) - p(0) = g * (1-g) * d0, p(2) - p(1) = g^2 * (1-g) * d1,  v(0) - v(1) = g * (1-g) * d2}, {a,b,c});

simp := e -> factor(simplify(e));

factor(eliminate(subs(c^T = cT, g^T = gT, expand({p(0) = g * (1-g) * p0, p(1) = g * (1-g) * p1, p(2) = g * (1-g) * p2, v(0) = g * (1-g) * v0, v(1) = g * (1-g) * v1})), {cT, gT}));
