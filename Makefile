# Makefile especifico para Proyecto 2 de Sistemas Operativos 1.
# Grupo 20: Astrid, Laura, Jhonaiker y Junior.
# Este Makefile compila el archivo .c principal para el proyecto 2
# descrito como "proyecto_1_op1_AstridLauraJhonaikerJunior.c", este consta
# de un .h "standard_lib.h" para su correcto funcionamiento.

###################################################
# DEFINES:

COMP 		 = cc -c

LINK 		 = cc

FILE 		 = proyect_1_op1_AstridLauraJhonaikerJunior.c

OBJDIR   = 

OBJS 		 = proyect_1_op1_AstridLauraJhonaikerJunior.o

LIBRERYS = standard_lib.h

PROGRAM  = simutransusb

FLAGS    = -pthread

###################################################
# Especificacion de Parametros para Make.

.PHONY: all clean

all: $(PROGRAM)

clean: 
	rm $(PROGRAM)
	rm $(OBJS)
#	rm $(OBJDIR)

##################################################
# Compilacion de archivos.

$(OBJS): $(FILE) $(LIBRERYS)
	$(COMP) $(FILE) $(FLAGS) -o $(OBJS)

$(PROGRAM): $(OBJS)
	$(LINK) $(OBJS) $(FLAGS) -o $(PROGRAM)

#$(OBJDIR):
#	mkdir $(OBJDIR)

#################################################

