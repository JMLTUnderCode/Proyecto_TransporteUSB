# Renombrado de archivos.
COMP 		 = cc

LINK 		 = cc

FILE 		 = proyect_1_op1_AstridLauraJunior.c

OBJ 		 = proyect_1_op1_AstridLauraJunior.o

LIBRERYS = standard_lib.h

PROGRAM  = proyect_1_op1_AstridLauraJunior.exe

# Funcion de make
.PHONY: all clean

all: $(OBJ)

clean: 
	rm $(OBJ)

# Compilacion
OBJECTS: $(OBJ)

$(OBJ) : $(FILE) $(LIBRERYS)
	$(COMP) $(FILE) $(LIBRERYS) -o $(OBJ)

# 
