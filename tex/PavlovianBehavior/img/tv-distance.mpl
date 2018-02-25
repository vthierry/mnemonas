with(LinearAlgebra):
u_rand := rand(0.0..1.0):
v_rand := proc(n) local v: v := Vector(n, u_rand): return v / Norm(v, 1) end:
d_norm := (v1, v2) -> Norm(v1 - v2, 1) / 2:
ok_1 := map(i -> evalb(abs(convert(v_rand(3), `+`) - 1) < 1e-6), {$1..1000});
ok_l := map(i -> evalb(abs(subs(l = u_rand(), convert(l * v_rand(3) + (1 - l) * v_rand(3), `+`) - 1)) < 1e-6), {$1..1000});
ok_d := map(i -> evalb(d_norm(v_rand(10), v_rand(10)) < 1), {$1..1000});




