# https://renenyffenegger.ch/notes/development/languages/C-C-plus-plus/GCC/create-libraries/index
# https://lopez-ibanez.eu/epanetlinux

CC			:= gcc
CFLAGS		:= -g -Wall

SRC_DIR 		:= src
BUILD_DIR 	:= build
BIN_DIR			:= bin
LIB_DIR			:= lib
SRC_EXT 		:= c
LIBTOOLKIT  := libepanet2.a

SOURCES 	:= $(shell find $(SRC_DIR) -type f -name *.$(SRC_EXT))
OBJECTS 	:= $(patsubst $(SRC_DIR)/%,$(BUILD_DIR)/%,$(SOURCES:.$(SRC_EXT)=.o))
TARGET 		:= $(LIB_DIR)/$(LIBTOOLKIT)

$(TARGET): $(OBJECTS)
	@echo " Creating static library..."
	ar rcs $(TARGET) $(OBJECTS)
	cp src/epanet2.h $(LIB_DIR)/
	cp src/epanet2_enums.h $(LIB_DIR)/

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.$(SRC_EXT)
	@echo "Creating the object files..."
	@mkdir -p $(BUILD_DIR)
	@echo " $(CC) $(CFLAGS) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $<

.PHONY: clean
clean:
	@echo " Cleaning..."; 
	@echo " $(RM) -r $(BUILD_DIR) $(TARGET)"; $(RM) -r $(BUILD_DIR) $(TARGET)

