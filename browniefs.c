#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <stddef.h>

clock_t mount_time;

FILE *f;

uint64_t chunks;

struct chunkinfo {
	uint64_t offset;
	uint64_t length;
};

struct chunkinfo *chunk_list;

struct chunkinfo *get_chunk(const char *path) {
	unsigned int chunk = 0;
	if (sscanf(path, "/%u", &chunk) != 1) {
		return NULL;
	}

	if (chunk >= chunks) {
		return NULL;
	}

	return &chunk_list[chunk];
}

static int do_getattr(const char *path, struct stat *st) {
	st->st_uid = getuid();
	st->st_gid = getgid();
	st->st_atime = mount_time;
	st->st_mtime = mount_time;
	
	if (strcmp(path, "/" ) == 0) {
		st->st_mode = S_IFDIR | 0555;
		st->st_nlink = 2;

		return 0;
	}

	struct chunkinfo *c = get_chunk(path);
	if (c == NULL) {
		return -ENOENT;
	}

	st->st_mode = S_IFREG | 0444;
	st->st_nlink = 1;
	st->st_size = c->length;

	return 0;
}

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) {
	char name[64];

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);

	if (strcmp(path, "/") == 0) {
		for (uint16_t i = 0; i < chunks; i++) {
			sprintf(name, "%u", i);
			filler(buffer, name, NULL, 0);
		}
	}
	
	return 0;
}

static int do_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) {
	int read;
	
	struct chunkinfo *c = get_chunk(path);
	if (c == NULL) {
		return -ENOENT;
	}

	read = size > c->length - offset ? c->length - offset : size;

	fseek(f, c->offset + offset, SEEK_SET);
	fread(buffer, read, 1, f);
	
	return read;
}

static struct fuse_operations operations = {
	.getattr = do_getattr,
	.readdir = do_readdir,
	.read = do_read,
};

int main(int argc, char *argv[]) {

	// argv[1]: device/imagae file
	// argv[2]: mount point

	if (argc != 3) {
		return 1;
	}

	char signature[8];

	f = fopen(argv[1], "r");
	fread(signature, sizeof(signature), 1, f);
	if (strncmp(signature, "BROWNIE1", sizeof(signature)) != 0) {
		printf("Fehler.");
		return 1;
	}
	fread(&chunks, sizeof(chunks), 1, f);

	chunk_list = malloc(sizeof(struct chunkinfo) * chunks);
	for (int i = 0; i < chunks; i++) {
		fread(&chunk_list[i].offset, sizeof(uint64_t), 1, f);
		fread(&chunk_list[i].length, sizeof(uint64_t), 1, f);
	}

	mount_time = time(NULL);

	return fuse_main(argc - 1, &argv[1], &operations, NULL);
}

