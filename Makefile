cc=g++
exe=main

DIR_INC = ./includes
DIR_SRC = ./src
DIR_OBJ = ./obj

SRC = $(wildcard $(DIR_SRC)/*.cpp)
DIR = $(notdir $(SRC))
OBJ = $(patsubst %.cpp, ${DIR_OBJ}/%.o, $(notdir ${SRC}))

TARGET = main

CFLAGS = -g -Wall -I${DIR_INC}

${TARGET}:${OBJ}
	$(cc) ${OBJ} -o $@ -lpthread

${DIR_OBJ}/%.o:${DIR_SRC}/%.cpp
	$(cc)  -c $< -o $@

#	@echo $(SRC)
#	@echo $(DIR)
#	@echo $(OBJ)

.PHONY:clean	
clean:
	find ${DIR_OBJ} -name *.o -exec rm -rf {} \;