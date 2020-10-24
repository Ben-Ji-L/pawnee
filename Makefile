# Fichier Makefile racine
# Cette variable contient la liste des sous répertoires
# pour lesquels il faudra construire un programme.
# Pour l’instant, seul le serveur web est dans la liste.
FOLDERS = webserver/config webserver
# Indique à make les règles qui ne correspondent pas à la création
# d’un fichier
# make lancera toujours la construction de cette règle si elle est
# demandée, même si un fichier/répertoire du nom de la cible existe
# On indique ici la règle all ainsi que les répertoires
.PHONY: all $(FOLDERS)
# La règle qui sera exécutée si on lance make sans argument
all: $(FOLDERS)
# Cette règle va lancer make dans le répertoire webserver
# option -C de make
webserver:
	make -C webserver

check:
	make -C tests/
	./tests/test

clean:
	make -C webserver clean
	make -C tests clean

mrproper:
	make -C webserver mrproper
	make -C tests mrproper

run: all
	./webserver/pawnee

doc:
	doxygen myAwesomeServ.dox

doc-latex: doc
	make -C doc/latex/