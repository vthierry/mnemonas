# Backward tuning sigmoid function with epsilon = e, beta_max = k
s := (1 + k * (1-e)/e) * (1 - exp(-e/k*u));
# Series development
series(s, u = 0, 2); series(s, e = 0, 2); series(convert(series(s, u = 0, 3), polynom), e = 1, 1);
# Plots against k and e
plot(map(v->subs(k=1,e=v,s),[1,1/10,1/100]),u=0..100);
plot(map(v->subs(k=v,e=1,s),[10,1,1/10]),u=0..10,color=[brown,black,black,black]);
# Identify convergence
evalf(subs(U=10,k=1,e=1/8000,int(u/(1-e+e*k)-s,u=1..U)/U));
evalf(subs(U=10,k=1,e=1/9000,int(u/(1-e+e*k)-s,u=1..U)/U));

