# Ces variables servent à préciser le compilateur que l’on veut utiliser
# ainsi que les paramètres de compilation
CC = gcc
LD = gcc
CFLAGS = -Wall -W -Werror
LDFLAGS = -lcunit -lpthread
# Le nom de l’exécutable à fabriquer
EXE = test
# Les variables HEADERS, CFILES et OBJS vont contenir respectivement
# la listes des fichiers .h, .c et le nom des fichiers .o à fabriquer
# On utilise la directive particulière $(wildcard ...) qui permet
# de construire automatiquement une liste de fichiers
HEADERS = $(wildcard *.h)
CFILES = $(wildcard *.c)

HEADERS += $(wildcard ../webserver/*.h)
CFILES += $(filter-out ../webserver/main.c, $(wildcard ../webserver/*.c))

HEADERS += $(wildcard ../webserver/*/*.h)
CFILES += $(wildcard ../webserver/*/*.c)

# Cette construction de variable veut dire : remplacer la chaine ".c" par
# ".o" dans la variable CFILES
# Ceci nous permet de construire la liste des fichiers .o à fabriquer
OBJS = $(CFILES:.c=.o)
# Même utilisation que précédemment. On précise les règles qui
# ne fabrique pas de fichier du nom de leur cible
.PHONY: all
# La règle par défaut déclenche la fabrication de l’exécutable
# par dépendance
all: $(EXE)
# Cette règle permet de construire l’exécutable. Elle dépend des fichiers
# .o et effectue l’édition de lien. Rien de nouveau ici
$(EXE): $(OBJS)
		$(LD) $^ $(LDFLAGS) -o $@
