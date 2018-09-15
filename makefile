SRC_DIR_UPTOWN := ./uptown_ts3_multitool2
OBJ_DIR_UPTOWN := ./uptown_ts3_multitool2/out
SRC_FILES_UPTOWN := $(wildcard $(SRC_DIR_UPTOWN)/*.cpp)
OBJ_FILES_UPTOWN := $(patsubst $(SRC_DIR_UPTOWN)/%.cpp,$(OBJ_DIR_UPTOWN)/%.o,$(SRC_FILES_UPTOWN))
LDFLAGS :=
CFLAGS := -fPIC -Wall
CXXFLAGS := -std=c++11 -fPIC
INCLUDE := -I./include -I./uptown_ts3_multitool2 
MKDIR_P = mkdir -p
SQLITE_DIR := ./include/sqlite

.PHONY: directories

all: directories uptown_ts3_multitool.so

uptown_ts3_multitool.so: $(OBJ_FILES_UPTOWN) $(SQLITE_DIR)/sqlite3.o
	g++ $(OBJ_FILES_UPTOWN) $(SQLITE_DIR)/sqlite3.o -shared -o $@ #-Wl,--no-undefined

$(OBJ_DIR_UPTOWN)/%.o: $(SRC_DIR_UPTOWN)/%.cpp
	g++ $(INCLUDE) $(CXXFLAGS) -c -o $@ $<

$(SQLITE_DIR)/sqlite3.o: $(SQLITE_DIR)/sqlite3.c
	gcc -c -o $(CFLAGS) $@ $^

directories: ${OBJ_DIR_UPTOWN}

${OBJ_DIR_UPTOWN}:
	${MKDIR_P} ${OBJ_DIR_UPTOWN}