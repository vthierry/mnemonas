r := (i, u) -> f[i, u] / sum(f[j, u], j = 0 .. n):

e := sum(r(i, u) * log(r(i, u)), i = 0 ..n):

numer(simplify(diff(e, f[k, u])));
