a := u -> u^2/2*Heaviside(1-abs(u))+(abs(u)-1/2)*Heaviside(abs(u)-1):
b := u -> Heaviside(1-abs(u)) * (u^2 * (u^2 - 3) + 3) * u^2 + Heaviside(abs(u)-1):

# plot(a(u), u=-2..2); plot(b(u), u=-2..2);

a_nu := nu * a((u - u0) / nu):
b_nu := b((u - u0) / nu):

ok := [
 (d_a_nu_1p = diff(a_nu, u) assuming 0 < nu, u0 < u, nu < u - u0),
 (d_a_nu_1n = diff(a_nu, u) assuming 0 < nu, u0 > u, nu < u0 - u),
 (d_a_nu_2p = diff(a_nu, u) assuming 0 < nu, u0 < u, u - u0 < nu),
 (d_a_nu_2n = diff(a_nu, u) assuming 0 < nu, u0 > u, u0 - u < nu),
 (d_b_nu_1p = diff(b_nu, u) assuming 0 < nu, u0 < u, nu < u - u0),
 (d_b_nu_1n = diff(b_nu, u) assuming 0 < nu, u0 > u, nu < u0 - u),
 (d_b_nu_2p = factor(simplify((diff(b_nu, u) assuming 0 < nu, u0 < u, u - u0 < nu), {(u - u0) / nu = v}))),
 (d_b_nu_2n = factor(simplify((diff(b_nu, u) assuming 0 < nu, u0 > u, u0 - u < nu), {(u - u0) / nu = v})))
]:

Db := map(d -> limit((D@@d)(b)(u), u = 1, left), [0,1,2,3,4,5,6,7,8,9,10,11]);

p1 := u -> u^2 / (nu + abs(u)):
series(p1(u), u=0, 3) assuming 0 < u, 0 < nu;
series(p1(u), u=infinity, 1) assuming 0 < u, 0 < nu;
factor(D(p1)(u)) assuming 0 < u, 0 < nu;
factor((D@@2)(p1)(u)) assuming 0 < u, 0 < nu;
plot(map(nu0 -> subs(nu = nu0, p1(u)), [1,2,4,8]),u=-5..5);

p0 := u -> u^2 / (nu + u^2):
series(p0(u), u=0, 3) assuming 0 < u, 0 < nu;
series(p0(u), u=infinity, 1) assuming 0 < u, 0 < nu;
factor(D(p0)(u)) assuming 0 < u, 0 < nu;
factor((D@@2)(p0)(u)) assuming 0 < u, 0 < nu;
plot(map(nu0 -> subs(nu = nu0, p0(u)), [1,2,4,8]),u=-5..5);
