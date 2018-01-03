# tex/results/robust_criterion_with_noise_criterion=b_LinearNonLinearTransform_N=4/costsgnuplot display script (automatically generated, do NOT edit)
title="title \"`basename $0 | sed 's/.gnuplot.sh$//'`\""
while [ \! -z "$1" ] ; do case "$1" in
 --png)  line1='set term png'; line2='set output "tex/results/robust_criterion_with_noise_criterion=b_LinearNonLinearTransform_N=4/costs.png"';;
 *) echo '$0 [--png]'; exit;;
esac; shift; done

cat << EOD | gnuplot -persist
$line1
$line2
set logscale y
set format y '%.1e'
c(x) = 2.9405e+12*exp(-x/0.264125)+0.0434753
plot "tex/results/robust_criterion_with_noise_criterion=b_LinearNonLinearTransform_N=4/costs.dat" using 1:2 with lines linecolor "black" notitle, c(x) with lines linecolor "red" notitle
EOD
