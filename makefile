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
	@echo "  make pdf  		: compiles the main tex publication"
	@echo "  make clean 	 	: cleans source and latex intermediate files"
	@echo "  make cleanall 	 	: cleans and delete all generated files"
	@echo "  make git  		: pulls, commits and pushes in the mnemosyne git repository"
	@echo "  make pub  		: publish on the public github website"
	@echo "  make rrun  		: compile and run the code on the nef cluster"
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

CCFLAGS =  -Isrc -std=c++0x -Wall -Wextra -Wno-unused-parameter -ggdb
LDFLAGS =  -lgsl -lgslcblas -lm 

ifeq ($(shell hostname),nef-devel)
CCFLAGS += -O3 -D ON_NEF
endif

cmp : .build/main.exe

.build/main.exe : $(SRC) $(INC)
	mkdir -p $(@D)
	g++ $(CCFLAGS) $(SRC) -o $@ $(LDFLAGS)

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
ARGS = -experiment2 -test true # -experiment1 '{ what : all }'
endif

run : .build/main.exe
	.build/main.exe $(ARGS)

grun : .build/main.exe
	(echo "run $(ARGS)" ; echo "backtrace" ; echo "echo\n" ; echo "backtrace full" ; echo "quit") > .build/a.cmd ; gdb -q .build/main.exe -x .build/a.cmd ; ok=1

vrun : .build/main.exe
	ulimit -s 100000 2>/dev/null ; export GLIBCXX_FORCE_NEW=1; valgrind --max-stackframe=100000000 --leak-check=full --show-reachable=yes --show-leak-kinds=all --track-origins=yes .build/main.exe $(ARGS)

#
# Code and latex documentation
#

show : api 
	firefox .build/doc/index.html

api : .build/doc/index.html .build/doc.zip

.build/doc/index.html : uncrustify doc/main.pdf clean ./src/index.h $(INC) $(SRC)
	@echo 'make api .'
	/bin/rm -rf $(@D) ; mkdir -p $(@D)
	echo "<hr/><div align='right'><tt>mnemosyne brainybot (version of `date +%F` at `date +%T`) </tt> </div><hr/>" > .build/doc/footer.html
	zip -9qr .build/doc/sources.zip makefile tex src
	cp doc/main.pdf .build/doc
	$(MAKE) doc/presentation.pdf ; mv presentation.pdf .build/doc
	doxygen src/etc/doxygen.cfg
	cp src/etc/*.png $(@D)
	echo '<script>location.replace("doc/index.html");</script>' > .build/doc.html 

uncrustify : $(INC) $(SRC)
	if command uncrustify > /dev/null ; then for f in $^ ; do mv $$f $$f~ ; uncrustify -q -c src/etc/uncrustify.cfg -f $$f~ -o $$f ; touch $$f -r $$f~ ; done ; fi

.build/doc.zip :
	rm -rf $@ ; cd .build ; zip -9qr doc.zip doc.html doc

pdf : doc/main.pdf clean

doc/main.pdf : $(wildcard tex/*.tex) $(wildcard tex/results/*.tex)
	@echo "latex2pdf main"
	$(MAKE) clean
	cd tex ; pdflatex main ; bibtex main ; pdflatex main ; pdflatex main
	mv tex/main.pdf doc

%.pdf : %.odp
	ooimpress --invisible --convert-to pdf $^

#
# Publication of the package
#

clean :	
	/bin/rm -f `find . -name '*~' -o -name '.#*#' -o -name '*.o'`
	cd tex ; /bin/rm -f *.aux *.toc *.ind *.bbl *.blg *.dvi *.idx *.lof *.log *.ilg *.nav *.spl *.snm *.sol *.out
	/bin/rm -rf stdout

cleanall : clean
	/bin/rm -rf .build doc/main.pdf # tex/results/*

git : api
	git pull ; git commit -a -m 'from makefile' ; git push

GIT_DIR = /home/vthierry/Work/mnemosyne/mnemonas/

pub : api
	cd $(GIT_DIR) ; git checkout master ; git pull
	rsync --archive --delete-excluded makefile src tex .build/doc $(GIT_DIR)
	cd $(GIT_DIR) ;\
	 (echo "# mnemonas" ;\
	  echo "- This is one https://team.inria.fr/mnemosyne research code distribution" ;\
	  echo "- See https://vthierry.github.io/mnemonas") > README.md ;\
	 echo "<script>location.replace('doc/index.html');</script>" > index.html
	cd $(GIT_DIR) ; git add --all ; git commit -m '.' -a ; git push
	cd $(GIT_DIR) ; git checkout gh-pages ; git pull origin gh-pages ; git merge master -m '.' ; git push origin gh-pages:gh-pages ; git checkout master

#
# Remote execution on nef-*.inria.fr
#

MAIL = thierry.vieville@inria.fr

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

# Améliorations:
# - Ajouter des tirages aléatoires multiples des sequence generator et reverse engeneering (en restant dans le bassin d attraction de la solution)
# - Creuser le calcul des kappas, voir souci de la singularité en epsilon/px, voir temps de convergence par rapport à 1 seul 2nd order, voir à ajouter d'autres estimations de get(n, t)
# - Voir à utiliser le Fit pour contrôler la convergence
# - Implémenter des \nu automatiques sur les critères robustes

test :
	$(MAKE) run api
#	$(MAKE) pdf
#	firefox doc/main.pdf .build/doc/index.html https://vthierry.github.io/mnemonas
#	cp doc/main.pdf .build/doc.zip ~/Desktop

#################################################################################################
