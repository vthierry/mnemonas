#
# On-going work
#
# - 
# - Ajouter une NLN sparse avec D taux de connection est un index avec random::booleans et ajoutera reverse engineering
# - Tester sequence avec N croissant en boucle partir de N = T puis descendre
# - Voir au calcul dune KL pour N0 = 1 avec Histogramme en cas pour evaluer l estimation via un Gibbs
#

dft : run
#	firefox $(BUILD)/html/index.html
#	acroread main.pdf

#
# BrainyBot compilation and execution main makefile
#

usage :
	@echo "Usage:"
	@echo "  make cmp	: compiles the code"
	@echo "  make run  	: compiles and run the code"
	@echo "  make grun  	: compiles and run the code, via a debugger (gdb)"
	@echo "  make vrun  	: compiles and run the code, via a memory checker (valgrind)"
	@echo "  make api  	: compiles the user source api documentation"
	@echo "  make pdf  	: compiles the user tex publication"
	@echo "  make clean  	: cleans source and latex intermediate files"
	@echo "  make git  	: pulls, commits and pushes in the mnemosyne git repository"
	@echo "  make pub  	: publish on the public github repository"
	@echo "  make rrun  	: compile and run the code on the nef cluster"
	@echo "This makefile uses the following software set :"
	@echo " - makefile git                            for source file management"
	@echo " - g++ gsl gslcblas doxygen uncrustify zip for software development and documentation"
	@echo " - pdflatex bibtex graphicx hyperref       for publication compilaton"
	@echo " - gdb valgrind                            for optional code debugging"
	@echo "other software standard unix distribution"

#
# Compilation
#

export SRC = $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*.c)
export INC = $(wildcard src/*.hpp) $(wildcard src/*/*.hpp) $(wildcard src/*/*.h) $(wildcard tex/*/*.h)

CCFLAGS =  -Isrc -std=c++0x -Wall -Wextra -Wno-unused-parameter -ggdb
LDFLAGS =  -lgsl -lgslcblas -lm 

ifeq ($(shell hostname),nef-devel)
CCFLAGS += -D ON_NEF
endif

cmp : .build/a.out

.build/a.out : $(SRC) $(INC)
	mkdir -p $(@D)
	g++ $(CCFLAGS) $(SRC) -o $@ $(LDFLAGS)

#
# Execution
#

run : .build/a.out
	.build/a.out

grun : .build/a.out
	(echo "run" ; echo "backtrace" ; echo "echo\n" ; echo "backtrace full" ; echo "quit") > .build/a.cmd ; gdb -q .build/a.out -x .build/a.cmd ; ok=1

vrun : .build/a.out
	ulimit -s 100000 2>/dev/null ; export GLIBCXX_FORCE_NEW=1; valgrind --max-stackframe=100000000 --leak-check=full  --show-leak-kinds=all --track-origins=yes .build/a.out

#
# Documentation
#

api : .build/doc/index.html 

.build/doc/index.html : uncrustify main.pdf clean ./src/index.h $(INC) $(SRC)
	@echo 'make api .'
	/bin/rm -rf $(@D) ; mkdir -p $(@D)
	@echo "<hr/><div align='right'><tt>mnemosyne brainybot (version of `date +%F` at `date +%T`) </tt> </div><hr/>" > .build/doc/footer.html
	$(MAKE) clean ; zip -9qr .build/doc/sources.zip makefile tex src
	cp main.pdf .build/doc
	doxygen src/etc/doxygen.cfg
	cp src/etc/*.png $(@D)

uncrustify : $(INC) $(SRC)
	for f in $^ ; do mv $$f $$f~ ; uncrustify -q -c src/etc/uncrustify.cfg -f $$f~ -o $$f ; touch $$f -r $$f~ ; done

#
# Publication
#

pdf : main.pdf clean

%.pdf : tex/%.tex $(wildcard tex/*.tex)
	echo "latex2pdf $*"
	$(MAKE) clean
	cd tex ; pdflatex $* ; bibtex $* ; pdflatex $* ; pdflatex $*
	mv tex/main.pdf .

clean :	
	/bin/rm -f `find . -name '*~' -name '.#*#'`
	cd tex ; /bin/rm -f *.aux *.toc *.ind *.bbl *.blg *.dvi *.idx *.lof *.log *.ilg *.nav *.spl *.snm *.sol *.out
	/bin/rm -fr .build/a.out

git : api
	git pull ; git commit -a -m 'from makefile' ; git push

GIT_DIR = /home/vthierry/Work/mnemosyne/mnemonas/

pub : api
	cd $(GIT_DIR) ; git checkout master ; git pull
	rsync --archive makefile src tex .build/doc $(GIT_DIR)
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

rrun : clean
	echo 'rsync' ; rsync --rsh='ssh -C' --archive --delete-excluded ../vthierry-2017 nef-frontal.inria.fr:
	echo 'cmp' ; ssh nef-frontal.inria.fr 'ssh nef-devel.inria.fr make -s -C vthierry-2017 cmp'
#	echo 'run-nef' ; ssh nef-frontal.inria.fr 'ssh nef-devel.inria.fr "cd vthierry-2017 ; .build/a.out"'
	echo 'run-oar' ; ssh nef-frontal.inria.fr 'ssh nef-devel.inria.fr "cd vthierry-2017 ; oarsub .build/a.out"'

rrun-log :
	rsync --rsh='ssh -C' --archive nef-frontal.inria.fr:vthierry-2017 ..

rrun-see :
	ssh nef-frontal.inria.fr 'ssh nef-devel.inria.fr "echo oarstat: ; oarstat | grep `whoami`; echo done."'

# Ref: https://wiki.inria.fr/ClustersSophia/User_Guide_new_config
