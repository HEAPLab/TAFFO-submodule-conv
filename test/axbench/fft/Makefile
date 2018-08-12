CC			:= clang++
LD			:= clang++

OX          := -O0

CFLAGS		:= -Wall -Wnarrowing -std=c++11
LFLAGS		:= -lboost_regex 
HEADERS     := src
INCLUDE 	:= -I${ANALOG_INC} -I${HEADERS}
LIB			:= -L${ANALOG_LIB} 

MODULE		:= fft.out

CPP_FILES       := $(wildcard src/*.cpp)
LL_FILES        := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.ll))) 
OBJ_FILES       := $(addprefix obj/,$(notdir $(CPP_FILES:.cpp=.o))) 
OBJ_FIX         := obj/main.fixp.o

.PHONY: all clean
	
all: DIR $(MODULE)

.PRECIOUS: obj/%.ll obj/%.o obj/%.s

DIR:
	@echo ${CPP_FILES}
	@echo ${OBJ_FILES}
	if [ ! -d "./bin" ];then 	\
		mkdir bin;				\
	fi
	if [ ! -d "./obj" ];then 	\
		mkdir obj;				\
	fi

$(MODULE): $(OBJ_FILES) $(OBJ_FIX)
	$(LD) $(OBJ_FILES) $(LIB) $(LFLAGS) -o bin/$@
	$(LD) $(OBJ_FIX) $(LIB) $(LFLAGS) -o bin/$@.fixp


obj/%.ll : src/%.cpp
	$(CC) -S -emit-llvm $< -o $@

obj/%.s:  obj/%.ll
	$(CC) -S  $^ -o $@ $(OX)

obj/%.o: obj/%.s
	$(CC) -c  $< -o $@

obj/main.fixp.ll : $(LL_FILES)
	llvm-link -S $^ -o obj/main.ll
	opt -load=$(PASSLIB) -S -flttofix -dce -debug -stats $(OX) obj/main.ll -o $@

$(OBJ_FIX) : obj/main.fixp.ll
	$(CC) -S $^ -o obj/main.fixp.s $(OX)
	$(CC) -c obj/main.fixp.s -o $@

clean:
	@rm -rf *.o
	@rm -rf *.d
	@rm -rf *.out
	@rm -rf bin
	@rm -rf obj
	@rm -rf *.ll
	@rm -rf *.s
