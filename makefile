#################################################################################################
#
# This is the https://github.com/vthierry/mnemonas middleware makefile
# - It is shared under an open source CeCILL-C licence: http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
# - Runs <tt>make usage</tt> to get all available commands
#
#  Notice: as being a very simple middleware only makefile is used (no need of ./configure mechanism)
#
#################################################################################################

default : todo # @todo : cancel for normal use

usage :
	@echo "Usage:"
	@echo "  make cmp		: compiles the code"
	@echo "  make test		: performs the functional non-regression tests"
	@echo "  make pywrap  		: compiles the python wrapper of the code"
	@echo "  make ccwrap  		: compiles the C/C++  wrapper of the code"
	@echo "  make run ARGS=$ARGS  	: compiles and run the code for the given arguments"
	@echo "  make grun ARGS=$ARGS  	: compiles and run the code, via a debugger (gdb)"
	@echo "  make vrun ARGS=$ARGS 	: compiles and run the code, via a memory checker (valgrind)"
	@echo "  make api  		: compiles the source api documentation"
	@echo "  make show  		: compiles the source api documentation and show it"
	@echo "  make pdf  MAIN=$MAIN	: compiles the main tex publication"
	@echo "  make clean 	 	: cleans source and latex intermediate files"
	@echo "  make git  		: publishes, i.e., pulls, commits and pushes in the mnemosyne git repository"
	@echo "  make rrun MAIL=$MAIL	: compile and run the code on the nef cluster, reporting result by mail"
	@echo "This makefile uses the following software set :"
	@echo " - clang++ gsl gslcblas swig  		for software development"
	@echo " - makefile git doxygen uncrustify	for source file management and documentation"
	@echo " - pdflatex bibtex 			for publication compilaton"
	@echo " - gdb valgrind                          for code debugging"
	@echo "with other software standard unix distribution"

#
# Souce code compilation
#

export SRC = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*.c)
export INC = $(wildcard src/*.hpp) $(wildcard src/*/*.hpp) $(wildcard src/*/*.h) $(wildcard tex/*/*.h)

CCFLAGS =  -Isrc -std=c++0x -x c++ -Wall -Wextra -Wno-unused-parameter -Wno-varargs
LDFLAGS =  -lgsl -lgslcblas -lm

ifeq ($(shell hostname),nef-devel)
CCFLAGS += -O3 -D ON_NEF
else
CCFLAGS += -O3 -ggdb
endif

cmp : .build/main.exe

OBJ = $(patsubst src/%.cpp,.build/obj/%.o,$(patsubst src/%.c,.build/obj/%.o, $(SRC)))

GPP=clang++

.build/main.exe : $(SRC) $(INC)
	@echo 'make cmp'
	@mkdir -p $(@D)
	@$(MAKE) $(OBJ)
	@echo "g++ -o $@ .build/obj/**.o"
	@$(GPP) $(OBJ) -o $@ $(LDFLAGS)

.build/obj/main.o: src/main.cpp $(INC)
	@echo "g++ -c src/main.cpp"
	@mkdir -p $(@D)
	@$(GPP) $(CCFLAGS) -o $@ -c src/main.cpp

.build/obj/%.o : src/%.cpp src/%.hpp
	@echo "g++ -c src/$*.cpp"
	@mkdir -p $(@D)
	@$(GPP) $(CCFLAGS) -o $@ -c src/$*.cpp

.build/obj/%.o : src/%.c src/%.h
	@echo "g++ -c src/$*.c"
	@mkdir -p $(@D)
	@$(GPP) $(CCFLAGS) -o $@ -c src/$*.c

#
# Python and C++ wrapper compilation
#

PINC=$(shell egrep "(util|network)/" < src/mnemomas.hpp | sed 's/^#include "\([^"]"\)/src\/\1/')


pywrap : .build/python/site-packages/mnemonas/mnemonas.py .build/python/site-packages/mnemonas/mnemonas.so

.build/python/site-packages/mnemonas/mnemonas.py .build/python/site-packages/mnemonas/mnemonas.so : $(SRC) $(INC)
	@echo 'make mnemonas.py mnemonas.so'
	@/bin/rm -rf .build/python ; mkdir -p .build/python/site-packages/mnemonas
	@inc=`egrep "(util|network)/" < src/mnemonas.hpp | sed 's/^.include[^"]*"\([^"]*\)".*/src\/\1/'`; echo " 1/3 wrapping: [`echo $$inc | sed 's/src\/[^\/]*\/\([^\.]*\)\.[a-z]*/\1/g'`]" ; (echo "%module mnemonas"; echo "%{"; cat $$inc ; echo "%}" ; cat $$inc) >  .build/mnemonas.i
	@echo " 2/3 swiging   and building mnemonas.py"
	@swig -module mnemonas -c++ -python -o .build/mnemonas.C .build/mnemonas.i
	@mv .build/mnemonas.py .build/python/site-packages/mnemonas
	@echo " 3/3 compiling and linking  mnemonas.so"
	@$(GPP) $(CCFLAGS) -fPIC -c -I/usr/include/python3.6m .build/mnemonas.C $(SRC)
	@$(GPP) -shared -o .build/python/site-packages/mnemonas/mnemonas.so *.o $(LDFLAGS)
	@/bin/rm -f .build/mnemonas.i .build/mnemonas.C *.o

ccwrap : .build/lib/libmnemonas.a .build/lib/libmnemonas.so .build/inc/mnemonas

.build/lib/libmnemonas.a .build/lib/libmnemonas.so : $(SRC) $(INC)
	@echo 'make libmnemonas.(a|so)'
	@/bin/rm -rf .build/lib ; mkdir -p .build/lib
	@$(GPP) $(CCFLAGS) -fPIC -c $(SRC)
	@$(GPP) -o .build/lib/libmnemonas.a *.o $(LDFLAGS)
	@$(GPP) -shared -o .build/lib/libmnemonas.so *.o $(LDFLAGS)
	@/bin/rm -f *.o

.build/inc/mnemonas : $(INC)
	@echo 'make inc/mnemonas'
	@/bin/rm -rf $@ ; mkdir -p $@
	@cd src ; rsync -r --exclude='index.h' --exclude='etc/' --exclude='*_test.h' --exclude='*.c' --exclude='*.cpp' . ../$@

#
# Code local execution
#

ifndef ARGS
ARGS = -test
endif

run : .build/main.exe
	@echo 'make run'
	@.build/main.exe $(ARGS)

grun : .build/main.exe
	@echo 'make grun'
	@(echo "run $(ARGS)" ; echo "echo --- backtrace ------------------------------------------------------------------------------\n"; echo "backtrace" ; echo "echo --- backtrace full -------------------------------------------------------------------------\n" ; echo "backtrace full" ; echo "echo --------------------------------------------------------------------------------------------\n"; echo "quit 0") > .build/a.cmd ; gdb -q .build/main.exe -x .build/a.cmd ; ok=1

vrun : .build/main.exe
	@echo 'make vrun'
	@ulimit -s 100000 2>/dev/null ; export GLIBCXX_FORCE_NEW=1; valgrind --max-stackframe=100000000 --leak-check=full --show-reachable=yes --show-leak-kinds=all --track-origins=yes .build/main.exe $(ARGS)

test : clean .build/main.exe
	@$(MAKE) run ARGS=-test
#
# Code and latex documentation
#

show : api
	@firefox doc/index.html

api : doc/index.html

doc/index.html : ./src/index.h $(INC) $(SRC)
	@$(MAKE) uncrustify
	@echo 'make api .'
	@/bin/rm -rf $(@D) ; mkdir -p $(@D)
	@echo "<hr/><div align='right'><tt>mnemosyne brainybot (version of `date +%F` at `date +%T`) </tt> </div><hr/>" > doc/footer.html
	@zip -9qr doc/sources.zip makefile tex src
	@doxygen src/etc/doxygen.cfg
	@cp src/etc/*.png $(@D)
	@for f in doc/*.html ; do mv $$f $$f~ ; sed 's/\(<td id="projectlogo">\)\(<img alt="Logo" src="logo.png"\/>\)\(<\/td>\)/\1<a href="https:\/\/vthierry.github.io\/mnemonas">\2<\/a>\3/' < $$f~ > $$f ; rm $$f~ ; done
	@echo '<script>location.replace("doc/index.html");</script>' > index.html

uncrustify : $(INC) $(SRC)
	@if command uncrustify > /dev/null ; then for f in $^ ; do mv $$f $$f~ ; uncrustify -q -c src/etc/uncrustify.cfg -f $$f~ -o $$f ; touch $$f -r $$f~ ; done ; fi

ifndef MAIN
MAIN = tex/BackwardTuning/main
endif

pdf : $(MAIN).pdf clean

$(MAIN).pdf : $(shell find $(dir $(MAIN)) -name '*.tex')
	@echo "latex2pdf $(MAIN)"
	@$(MAKE) clean
	@cd $(dir $(MAIN)) ; pdflatex --interaction=batchmode $(notdir $(MAIN)) ; bibtex $(notdir $(MAIN)) ; pdflatex --interaction=batchmode $(notdir $(MAIN)) ; pdflatex --interaction=batchmode $(notdir $(MAIN))

%.pdf : %.odp
	@ooimpress --invisible --convert-to pdf $^

#
# Publication of the package
#

clean :
	@/bin/rm -f `find . -name '*~' -o -name '.#*#' -o -name '*.o'`
	@/bin/rm -f `find tex -name '*.aux' -o -name '*.toc' -o -name '*.ind' -o -name '*.bbl' -o -name '*.blg' -o -name '*.dvi' -o -name '*.idx' -o -name '*.lof' -o -name '*.log' -o -name '*.ilg' -o -name '*.nav' -o -name '*.spl' -o -name '*.snm' -o -name '*.sol' -o -name '*.out'`
	@/bin/rm -rf .build stdout

git : clean api
	@echo "git sync"
	@git checkout master ; git pull ; git add doc/* ; git commit -a -q -m 'from makefile' ; git push -q

#
# Remote execution on nef-*.inria.fr
#

ifndef MAIL
MAIL = thierry.vieville@inria.fr
endif

RRUN = -test

here = $(shell basename `pwd`)

rrun-cmp : clean
	@rsync --rsh='ssh -C' --archive --delete-excluded ../$(here) nef-frontal.inria.fr:
	@ssh nef-frontal.inria.fr '/bin/rm -rf $(here)/.build ; ssh nef-devel.inria.fr make -s -C $(here) cmp'

rrun-run : rrun-cmp
	@ssh nef-frontal.inria.fr '/bin/rm -rf $(here)/rrun ; mkdir $(here)/rrun'
	@ssh nef-frontal.inria.fr "cd $(here)/rrun ; oarsub  --notify 'mail:$(MAIL)' -l /nodes=1,walltime=100 '../.build/main.exe $(RRUN)'"

rrun-out :
	@ssh nef-frontal.inria.fr "oarstat --format 2 -u `whoami`"
	@rsync --rsh='ssh -C' --archive nef-frontal.inria.fr:$(here)/rrun .
	@for f in `ls ./rrun/OAR.*.stderr` ; do make `echo $$f | sed s'/.stderr/.stdlog/'` ; done

./rrun/OAR.%.stdlog : ./rrun/OAR.%.stderr
	@(ssh nef-frontal.inria.fr oarstat -f -j $* ; echo "STDOUT:" ; cat ./rrun/OAR.$*.stdout ; echo "STDER:" ; cat ./rrun/OAR.$*.stderr ; echo "-------------------------------------------------") > rrun/OAR.$*.stdlog

# Ref: https://wiki.inria.fr/ClustersSophia/User_Guide_new_config

#################################################################################################

#
# On-going work, do not consider
#


#
# On going :
#
# - RecurrentTransform : rayon spectral en calant values[] = 1e6 >0 et vir si rayon plus important sur Sparse et NLN 
# - KernelDistributedEstimator : explorer les hyper-paramètres, ajouter mécanisme de reinit des W, insérer updateReadout
# - ObservableCriterion: implémenter la notion d'observable normalisé et voir amélioration avec update
# - Histogram : voir avec value vector pour gerer automatiquement les échelles et le cas m0 == 0
#

ARGS = -experiment2

todo :
	@$(MAKE) run 
#	@firefox doc/$(MAIN).pdf doc/index.html https://vthierry.github.io/mnemonas

#################################################################################################
