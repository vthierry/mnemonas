# Exponential decay profile
c := t -> a * exp(-d * t) + b:

# The instantenous decay formula
d = simplify(subs(d = log(d), (c(t)-c(t-1))/(c(t+1)-c(t))));

# The log of decay variation
lt := simplify(log(factor(expand(c(t) - c(t-1)))));

# The >_t t^d t_t recursive computations
eq := {
 L0(T) =        l(T) + g * L0(T-1),
 L1(T) = g * L0(T-1) + g * L1(T-1),
 L2(T) = g * L0(T-1) + 2 * g * L1(T-1)  + g * L2(T-1)}:
sl := factor(rsolve(eq union {L0(0) = 0, L1(0) = 0, L2(0) = 0}, {L0(T), L1(T), L2(T)}));

# The window gamma 1st order relation
simplify(solve({1 - r = sum(g^t,t=0..W-1)/sum(g^t,t=0..infinity)}, {g}));

# The interpolation time
solve({c(t) = c1, c(0) = c0, d = 1/tau}, {t, a, d});

# The mean and variance
#sq := ln(exp(d * T)) =  d * T, {exp(d * T) = Edt, exp(2 * d * T) = Edt^2}:
#m := int(c(t), t = -T .. 0):
#v := simplify(int((c(t) - m)^2, t = -T .. 0)) assuming T > 0, d :: real:
#m1 := subs(sq, m);
#v1 := subs(sq, v);

sl := factor(solve({c(t) = c0,c(t-1)= c1,c(t-2)=c2},{a,b,d}));
zero := simplify(subs(sl, d - ln(1 - (c0 - 2 * c1 + c2) / (c0 - c1))));
