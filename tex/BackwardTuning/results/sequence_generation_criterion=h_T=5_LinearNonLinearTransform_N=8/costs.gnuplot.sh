# tex/results/sequence_generation_criterion=h_T=5_LinearNonLinearTransform_N=8/costsgnuplot display script (automatically generated, do NOT edit)
title="title \"`basename $0 | sed 's/.gnuplot.sh$//'`\""
while [ \! -z "$1" ] ; do case "$1" in
 --png)  line1='set term png'; line2='set output "tex/results/sequence_generation_criterion=h_T=5_LinearNonLinearTransform_N=8/costs.png"';;
 *) echo '$0 [--png]'; exit;;
esac; shift; done

cat << EOD | gnuplot -persist
$line1
$line2
set logscale y
set format y '%.1e'
c(x) = 0.000236542*exp(-x/23.1799)+2.26819e-05
plot "tex/results/sequence_generation_criterion=h_T=5_LinearNonLinearTransform_N=8/costs.dat" using 1:2 with lines linecolor "black" notitle, c(x) with lines linecolor "red" notitle
EOD
