# Exponential decay profile
c := t -> beta + nu * exp(-d * t):

# The instantenous decay formula
exp_d = simplify(subs(d = log(exp_d), (c(t)-c(t-1))/(c(t+1)-c(t))));

# The least-square equation
ok := evalb(expand(c(t) - c(t-1) = (1 - exp(d)) * (c(t) - beta)));
solve({a = 1 - exp(1/tau)}, {tau});

# The >_t t^d t_t recursive computations
assume(0 < g, g < 1):

#eq := {
# L0(T) =        l(T) + g * L0(T-1),
# L1(T) = g * L0(T-1) + g * L1(T-1),
# L2(T) = g * L0(T-1) + 2 * g * L1(T-1)  + g * L2(T-1)}:
#sl := factor(rsolve(eq union {L0(0) = 0, L1(0) = 0, L2(0) = 0}, {L0(T), L1(T), L2(T)}));

# The window gamma 1st order relation
simplify(Solve({1 - r = sum(g^t,t=0..W-1)/sum(g^t,t=0..infinity)}, {g}));

# The mean and variance
#sq := ln(exp(d * T)) =  d * T, {exp(d * T) = Edt, exp(2 * d * T) = Edt^2}:
#m := int(c(t), t = -T .. 0):
#v := simplify(int((c(t) - m)^2, t = -T .. 0)) assuming T > 0, d :: real:
#m1 := subs(sq, m);
#v1 := subs(sq, v);

# Solve at the 2nd order
#sl := factor(solve({c(t) = c0, c(t-1) = c1, c(t-2) = c2}, {beta,nu,d}));
#ok := evalb(simplify(subs(sl, d = ln(1 - (c0 - 2 * c1 + c2) / (c0 - c1)))));
