

# Define target platform
PLATFORM		?= DESKTOP

# Define paths
SOURCE_PATH		?= src
INCLUDE_PATH	?= include
LIB_PATH		?= lib
EXAMPLES_PATH	?= examples
OUTPUT_PATH		?= out

ifeq ($(PLATFORM), WEB)
	# Compile with emscripten
	CC = emcc
	AR = emar
endif

#Define includes
BINGUS_INCLUDES =	-I$(INCLUDE_PATH) \
					-I$(SOURCE_PATH)

#Define engine object files
BINGUS_SRCS =	$(SOURCE_PATH)/bingus.cpp \
				$(SOURCE_PATH)/debug.cpp \
				$(SOURCE_PATH)/entity.cpp \
				$(SOURCE_PATH)/glad.c \
				$(SOURCE_PATH)/input.cpp \
				$(SOURCE_PATH)/math.cpp \
				$(SOURCE_PATH)/renderer.cpp \
				$(SOURCE_PATH)/shader.cpp \
				$(SOURCE_PATH)/texture.cpp \
				$(SOURCE_PATH)/gui.cpp \
				$(SOURCE_PATH)/window.cpp

#BINGUS_SRCS = $(shell find $(SOURCE_PATH) -name '*.cpp' -or -name '*.c')

BINGUS_OBJS =	bingus.o \
				debug.o \
				entity.o \
				glad.o \
				input.o \
				math.o \
				renderer.o \
				shader.o \
				texture.o \
				gui.o \
				window.o

#BINGUS_OBJS = $(BINGUS_SRCS:%=$(OUTPUT_PATH)%.o)

# Define compiler vars
CC = g++
AR = ar
CFLAGS = -Wall -std=c++17

#all: bingus.a

bingus.a: $(BINGUS_OBJS)
	$(AR) cr $@ $^

bingus.o: $(SOURCE_PATH)/bingus.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

debug.o: $(SOURCE_PATH)/debug.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

entity.o: $(SOURCE_PATH)/entity.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

glad.o: $(SOURCE_PATH)/glad.c
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

input.o: $(SOURCE_PATH)/input.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

math.o: $(SOURCE_PATH)/math.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

renderer.o: $(SOURCE_PATH)/renderer.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

shader.o: $(SOURCE_PATH)/shader.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

texture.o: $(SOURCE_PATH)/texture.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

ui.o: $(SOURCE_PATH)/gui.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)

window.o: $(SOURCE_PATH)/window.cpp
	$(CC) $(CFLAGS) $< -c $(BINGUS_INCLUDES)



examples: hello_window sprites text


hello_window: $(EXAMPLES_PATH)/hello_window.cpp bingus.a
	$(CC) -o $@ $(CFLAGS) $(BINGUS_INCLUDES) $^ -L$(LIB_PATH) -lglfw3 -lgdi32 -lopengl32

sprites: $(EXAMPLES_PATH)/sprites.cpp bingus.a
	$(CC) -o $@ $(CFLAGS) $(BINGUS_INCLUDES) $^ -L$(LIB_PATH) -lglfw3 -lgdi32 -lopengl32

text: $(EXAMPLES_PATH)/text.cpp bingus.a
	$(CC) -o $@ $(CFLAGS) $(BINGUS_INCLUDES) $^ -L$(LIB_PATH) -lglfw3 -lgdi32 -lopengl32


debug:
	echo $(BINGUS_OBJS)

clean:
	rm -f *.exe *.o *.lib


#Build Steps:

# 1. Compile bingus source files into object files
# 2. Compile bingus object files into bingus static lib
# 3. Compile example executables using bingus static lib



#gcc notes:

# -o = compile as exe (use linker)
# -c = compile as object (don't use linker)

#ar notes:

# ar cr libhello.a hello_fn.o bye_fn.o
# cr = create and replace

