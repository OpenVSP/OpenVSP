set term pdfcairo noenhanced 
set out 'compare.complex.pdf' 
set grid 
set mxtics 
set mytics 

###################################### COMPLEX STEP ###################################### 

set xlabel 'Node' 
set ylabel 'pF_pX' 
set title 'pF_pX' 
plot 'prop.gradient'         using 1:5 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:5 index 0  title 'Adjoint, API' with lines lw 2,\
     'prop.complex.gradient' using 1:5 index 0  title 'Complex Step' with points pt 6 lw 1
#pause -1 'Hit return to continue' 


set xlabel 'Node' 
set ylabel 'pF_pY' 
set title 'pF_pY' 
plot 'prop.gradient'         using 1:6 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:6 index 0  title 'Adjoint, API' with lines lw 2,\
     'prop.complex.gradient' using 1:6 index 0  title 'Complex Step' with points pt 6 lw 1
#pause -1 'Hit return to continue' 


set xlabel 'Node' 
set ylabel 'pF_pZ' 
set title 'pF_pZ' 
plot 'prop.gradient'         using 1:7 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:7 index 0  title 'Adjoint, API' with lines lw 2,\
     'prop.complex.gradient' using 1:7 index 0  title 'Complex Step' with points pt 6 lw 1
#pause -1 'Hit return to continue' 



set xlabel 'Node' 
set ylabel 'pF_pX' 
set title 'pF_pX' 
plot 'prop.gradient'         using 1:5 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:5 index 0  title 'Adjoint, API' with points pt 6 lw 1
#pause -1 'Hit return to continue' 


set xlabel 'Node' 
set ylabel 'pF_pY' 
set title 'pF_pY' 
plot 'prop.gradient'         using 1:6 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:6 index 0  title 'Adjoint, API' with points pt 6 lw 1


set xlabel 'Node' 
set ylabel 'pF_pZ' 
set title 'pF_pZ' 
plot 'prop.gradient'         using 1:7 index 0  title 'Adjoint, 2 step' with lines lw 2,\
     'prop.opt.gradient'     using 1:7 index 0  title 'Adjoint, API' with points pt 6 lw 1
#pause -1 'Hit return to continue' 

