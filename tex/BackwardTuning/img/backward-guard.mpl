sat := t -> omega - exp(-(t - omega) / alpha - 1) * alpha :
eq1 := {D(sat)(omega - alpha) = 1, sat(omega - alpha) = omega - alpha};
sl1 := solve(eq1, {a, c});
curvature_omega1 := simplify(subs(sl1,(D@@2)(sat)(omega - alpha)));
assume(alpha > 0): simplify(int(t - sat(t), t = omega - alpha..omega) + int(omega - sat(t), t = omega - alpha..infinity));

thr := t -> t^2 / (4 * nu) + nu:
eq0 := {D(thr)(0) = 0, D(thr)(t0) = 1, thr(t0) = t0}:
sl0 := solve(eq0, {t0});

nu := 1/16: alpha := 1/8: omega := 1:
plot(u-> if u < 2 * nu then thr(u) elif u < omega - alpha then u else sat(u) fi, 0 .. 2);
