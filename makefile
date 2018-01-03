#################################################################################################
#
# This is the https://github.com/vthierry/mnemonas middleware makefile
# - It is shared under an open source CeCILL-C licence: http://www.cecill.info/licences/Licence_CeCILL-C_V1-en.html
# - Runs <tt>make usage</tt> to get all available commands 
#
#  Notice: as being a very simple middleware only makefile is used (no need of ./configure mechanism)
# 
#################################################################################################

default : test # @todo : cancel for normal use

usage :
	@echo "Usage:"
	@echo "  make cmp		: compiles the code"
	@echo "  make pywrap  		: compiles the python wrapper of the code"
	@echo "  make run  ARGS=$ARGS	: compiles and run the code, with the given arguments"
	@echo "  make grun ARGS=$ARGS  	: compiles and run the code, via a debugger (gdb)"
	@echo "  make vrun ARGS=$ARGS 	: compiles and run the code, via a memory checker (valgrind)"
	@echo "  make api  		: compiles the source api documentation"
	@echo "  make show  		: compiles the source api documentation and show it"
	@echo "  make pdf  MAIN=$MAIN	: compiles the main tex publication"
	@echo "  make clean 	 	: cleans source and latex intermediate files"
	@echo "  make git  		: pulls, commits and pushes in the mnemosyne git repository"
	@echo "  make pub  		: publish on the public github website"
	@echo "  make rrun MAIL=$MAIL	: compile and run the code on the nef cluster, reporting result by mail"
	@echo "This makefile uses the following software set :"
	@echo " - g++ gsl gslcblas swig  		for software development"
	@echo " - makefile git doxygen uncrustify	for source file management and documentation"
	@echo " - pdflatex bibtex 			for publication compilaton"
	@echo " - gdb valgrind                          for code debugging"
	@echo "with other software standard unix distribution"

#
# Souce code compilation
#

export SRC = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*.c)
export INC = $(wildcard src/*.hpp) $(wildcard src/*/*.hpp) $(wildcard src/*/*.h) $(wildcard tex/*.h)

CCFLAGS =  -Isrc -std=c++0x -Wall -Wextra -Wno-unused-parameter
LDFLAGS =  -lgsl -lgslcblas -lm 

ifeq ($(shell hostname),nef-devel)
CCFLAGS += -O4 -D ON_NEF
else
CCFLAGS += -O4 -ggdb
endif

cmp : .build/main.exe

OBJ = $(patsubst src/%.cpp,.build/obj/%.o,$(patsubst src/%.c,.build/obj/%.o, $(SRC)))

.build/main.exe : $(SRC) $(INC)
	@echo 'make cmp'
	mkdir -p $(@D)
	$(MAKE) $(OBJ) 	
	echo "g++ -o $@ .build/obj/**.o"
	g++ $(OBJ) -o $@ $(LDFLAGS)

.build/obj/main.o: src/main.cpp $(INC)
	echo "g++ -c src/main.cpp"
	mkdir -p $(@D)
	g++ $(CCFLAGS) -o $@ -c src/main.cpp

.build/obj/%.o : src/%.cpp src/%.hpp
	echo "g++ -c src/$*.cpp"
	mkdir -p $(@D)
	g++ $(CCFLAGS) -o $@ -c src/$*.cpp

.build/obj/%.o : src/%.c src/%.h
	echo "g++ -c src/$*.c"
	mkdir -p $(@D)
	g++ $(CCFLAGS) -o $@ -c src/$*.c

#
# Python wrapper compilation
#

pywrap : .build/mnemonas.py .build/mnemonas.so

.build/mnemonas.py .build/mnemonas.so : $(SRC) $(INC)
	@echo 'make mnemonas.py mnemonas.so'
	swig -module mnemonas -c++ -python -o .build/mnemonas.C src/main.hpp
	g++ $(CCFLAGS) -fPIC -c -I/usr/include/python3.6m .build/mnemonas.C $(SRC)	
	g++ -shared -o .build/mnemonas.so *.o $(LDFLAGS)
	/bin/rm -f .build/mnemonas.C *.o

#
# Code local execution
#

ifndef ARGS
ARGS = -test
endif

run : .build/main.exe
	@echo 'make run'
	.build/main.exe $(ARGS)

grun : .build/main.exe
	@echo 'make grun'
	(echo "run $(ARGS)" ; echo "backtrace" ; echo "echo\n" ; echo "backtrace full" ; echo "quit") > .build/a.cmd ; gdb -q .build/main.exe -x .build/a.cmd ; ok=1

vrun : .build/main.exe
	@echo 'make vrun'
	ulimit -s 100000 2>/dev/null ; export GLIBCXX_FORCE_NEW=1; valgrind --max-stackframe=100000000 --leak-check=full --show-reachable=yes --show-leak-kinds=all --track-origins=yes .build/main.exe $(ARGS)

#
# Code and latex documentation
#

show : api 
	firefox .build/doc/index.html

api : .build/doc/index.html

.build/doc/index.html : ./src/index.h $(INC) $(SRC)
	$(MAKE) uncrustify
	@echo 'make api .'
	/bin/rm -rf $(@D) ; mkdir -p $(@D)
	echo "<hr/><div align='right'><tt>mnemosyne brainybot (version of `date +%F` at `date +%T`) </tt> </div><hr/>" > .build/doc/footer.html
	zip -9qr .build/doc/sources.zip makefile tex src
	doxygen src/etc/doxygen.cfg
	cp src/etc/*.png $(@D)
	echo '<script>location.replace(".build/doc/index.html");</script>' > index.html 

uncrustify : $(INC) $(SRC)
	if command uncrustify > /dev/null ; then for f in $^ ; do mv $$f $$f~ ; uncrustify -q -c src/etc/uncrustify.cfg -f $$f~ -o $$f ; touch $$f -r $$f~ ; done ; fi

ifndef MAIN
MAIN = tex/BackwardTuning/main
endif

pdf : $(MAIN).pdf clean

$(MAIN).pdf : $(shell find $(dir $(MAIN)) -name '*.tex')
	@echo "latex2pdf $(MAIN)"
	$(MAKE) clean
	cd $(dir $(MAIN)) ; pdflatex $(notdir $(MAIN)) ; bibtex $(notdir $(MAIN)) ; pdflatex $(notdir $(MAIN)) ; pdflatex $(notdir $(MAIN))

%.pdf : %.odp
	ooimpress --invisible --convert-to pdf $^

#
# Publication of the package
#

clean :	
	/bin/rm -f `find . -name '*~' -o -name '.#*#' -o -name '*.o'`
	cd tex ; /bin/rm -f *.aux *.toc *.ind *.bbl *.blg *.dvi *.idx *.lof *.log *.ilg *.nav *.spl *.snm *.sol *.out
	/bin/rm -rf .build stdout doc/$(MAIN).pdf

git :
	@echo "git sync"
	git checkout master ; git pull ; git commit -a -m 'from makefile' ; git push

pub : api
	@echo 'make pub'
	git checkout master ; git pull
	git checkout gh-pages ; git pull origin gh-pages ; git merge master -m '.' ; git push origin gh-pages:gh-pages ; git checkout master

#
# Remote execution on nef-*.inria.fr
#

ifndef MAIL
MAIL = thierry.vieville@inria.fr
endif

here = $(shell basename `pwd`)

rrun-cmp : clean
	rsync --rsh='ssh -C' --archive --delete-excluded ../$(here) nef-frontal.inria.fr:
	ssh nef-frontal.inria.fr '/bin/rm -rf $(here)/.build ; ssh nef-devel.inria.fr make -s -C $(here) cmp'

rrun-run : rrun-cmp
	ssh nef-frontal.inria.fr '/bin/rm -rf $(here)/rrun ; mkdir $(here)/rrun'
	for task in 0 1 ; do ssh nef-frontal.inria.fr "cd $(here)/rrun ; oarsub  --notify 'mail:$(MAIL)' -l /nodes=1,walltime=100 '../.build/main.exe -experiment1 $$task'" ; done

rrun-out :
	ssh nef-frontal.inria.fr "oarstat --format 2 -u `whoami`"
	rsync --rsh='ssh -C' --archive nef-frontal.inria.fr:$(here)/rrun .
	for f in `ls ./rrun/OAR.*.stderr` ; do make `echo $$f | sed s'/.stderr/.stdlog/'` ; done

./rrun/OAR.%.stdlog : ./rrun/OAR.%.stderr
	(ssh nef-frontal.inria.fr oarstat -f -j $* ; echo "STDOUT:" ; cat ./rrun/OAR.$*.stdout ; echo "STDER:" ; cat ./rrun/OAR.$*.stderr ; echo "-------------------------------------------------") > rrun/OAR.$*.stdlog

# Ref: https://wiki.inria.fr/ClustersSophia/User_Guide_new_config

#################################################################################################

#
# On-going work, do not consider
#

# On going

# - Ajouter updateReadout(unsigned int N0) dans KernelEstimation


ARGS = -experiment2

test :
	$(MAKE) pdf
#	firefox doc/$(MAIN).pdf .build/doc/index.html https://vthierry.github.io/mnemonas

#################################################################################################
