COMPILER = gcc
SRC_FILES = browniefs.c

build: $(FILES)
	$(COMPILER) $(SRC_FILES) -o browniefs `pkg-config fuse --cflags --libs`

clean:
	rm browniefs

