eq := kappa(w) * phi * (phi * w - psi) + D(kappa)(w) * (phi * w - psi)^2 / 2 - phi * epsilon = 0:
dsolve({eq}, {kappa(w)});

# kappa := w -> 2 * epsilon * (phi * w - psi + k) / (phi * w - psi)^2: simplify(eq);
