#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <limits.h> 

int depth = -1;
long long subdir_size = 0;

void strsize(char *buffer, long long size) {
	char units[5][2] = {"TB", "GB", "MB", "KB", "B"};
	int i = 4;
	float d = (float)size;
	while (1) {
		if (d < 1024) {
			sprintf(buffer, "%.2f", d);
			int j = 0;
			while(*(buffer + j)) {
				j++;
			}
			*(buffer + j) = ' ';
			j++;
			*(buffer + j) = units[i][0];
			*(buffer + j + 1) = units[i][1];
			j += 2;
			while(j < 10) {
				*(buffer + j) = 0;
				j++;
			}
			break;
		}
		i--;
		d /= 1024;
	}
}

void fill_partitions() {
	int i = 0;
	for (int i = 0; i < depth; i++) {
		printf("|");
		printf("%*s", 5 - 1, "");
	}
}

void print_directory(char *path) {
	depth++;
	long long total_size = 0;
	DIR *i_dir;
	if (!(i_dir = opendir(path))) {
		fill_partitions();
		printf("Failed to open directory: %s\n", strerror(errno));
		depth--;
		return;
	}
	struct dirent *dir_info;
	int j = 0;
	while (*(path + j)) {
		j++;
	}
	while ((dir_info = readdir(i_dir))) {
		if (strcmp(dir_info->d_name, ".") && strcmp(dir_info->d_name, "..")) {
			char new_path[PATH_MAX + 1];
			struct stat file_info;
			strcpy(new_path, path);
			if (strcmp(path, "./")) {
				new_path[j] = '/';
				j++;
				strcpy(new_path + j, dir_info->d_name);
				j--;
			} else {
				strcpy(new_path + j, dir_info->d_name);
			}
			if (stat(new_path, &file_info)) {
				fill_partitions();
				printf("Stat failed: %s\n", strerror(errno));
			} else {
				char *mtime = ctime(&file_info.st_mtime);
				int i = 0;
				while (*(mtime + i) != 10) {
					i++;
				}
				*(mtime + i) = 0;
				char mode[12];
				strmode(file_info.st_mode, (char *)&mode);
				char size[10];
				strsize(size, file_info.st_size);
				total_size += file_info.st_size;
				fill_partitions();
				printf("%10s %14s %6s %9s %20s %s", mode, getpwuid(file_info.st_uid)->pw_name, getgrgid(file_info.st_gid)->gr_name, size, mtime, dir_info->d_name);
				if (dir_info->d_type == 4) {
					printf(" [DIRECTORY]\n");
					if (strcmp(dir_info->d_name, ".") && strcmp(dir_info->d_name, "..")) {
						print_directory(new_path);
						total_size += subdir_size;
						subdir_size = 0;
					}
				} else {
					printf("\n");
				}
			}
		}
	}
	closedir(i_dir);
	char size[10];
	strsize(size, total_size);
	fill_partitions();
	printf("Total size: %s\n", size);
	subdir_size = total_size;
	depth--;
}

int main() {
	char path[PATH_MAX + 1] = "./";
	print_directory(path);
	return 0;
}