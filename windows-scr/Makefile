# Flying Toasters Screensaver - Makefile
# For use with MinGW-w64 (gcc)
#
# Usage:
#   make          - Build the screensaver
#   make clean    - Remove build artifacts
#   make install  - Copy to Windows screensaver directory (requires admin)

CC = gcc
WINDRES = windres
CFLAGS = -O2 -Wall -mwindows -DUNICODE -D_UNICODE
LDFLAGS = -lscrnsavw -lcomctl32 -lgdi32 -luser32 -ladvapi32

TARGET = flying_toasters.scr
SOURCES = flying_toasters.c
RESOURCES = flying_toasters.rc
OBJECTS = flying_toasters.o
RES_OBJ = flying_toasters_res.o

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJECTS) $(RES_OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS) $(RES_OBJ) $(LDFLAGS)
	@echo "Build complete: $(TARGET)"

# Compile C source
$(OBJECTS): $(SOURCES)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile resources
$(RES_OBJ): $(RESOURCES)
	$(WINDRES) $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(RES_OBJ) $(TARGET)
	@echo "Cleaned build artifacts"

# Install to system screensaver directory (requires elevated privileges)
install: $(TARGET)
	@echo "Installing to Windows screensaver directory..."
	@echo "This requires administrator privileges."
	copy $(TARGET) "$(SYSTEMROOT)\System32\$(TARGET)"
	@echo "Installed. Select 'Flying Toasters' in Screen Saver Settings."

# Test run (screensaver mode)
test: $(TARGET)
	./$(TARGET) /s

# Test configuration dialog
testconfig: $(TARGET)
	./$(TARGET) /c

.PHONY: all clean install test testconfig
