l := [[2,0.06],[4,0.24],[5,0.32],[8,0.6],[12,1.5],[16,3],[20,4.7],[32,10],[40,16.6]]:

with(Statistics):

fe := unapply(NonlinearFit(a+b*v+d*exp(v/c), l ,v), v);
f0 := unapply(NonlinearFit(a+b*v+c*v^(3/2), l ,v), v);
fd := d -> unapply(NonlinearFit(sum(a[i]*v^i, i = 0 .. d), l ,v), v);

e2 := (l, f, d) -> evalf(sqrt(d+convert(map((xy, f) -> (xy[2] - f(xy[1]))^2, l, f), `+`)/(nops(l) - d))):

e2(l, fe, 4);
e2(l, f0, 3);
map(d -> e2(l, fd(d), d+1), [1, 2, 3, 4, 5, 6, 7]);



