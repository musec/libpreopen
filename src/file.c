<<<<<<< HEAD
#include<stdio.h>
#include<stdlib.h>

int dir_fd;
typedef struct{
	int dirfd;
	char*dirname;
	int flags;

}opened_dir_struct;// Holds opened directory fd and filename
typedef struct{
	char * relative_path;
	int dirfd;
}split_path;

void open_directory(char* relative_path,opened_dir_struct *ods);
opened_dir_struct **  add_OpenDir(opened_dir_struct **old_ods,opened_dir_struct *ods,int *len);
int findMatchingChars(char *A,char *B);
split_path lookupDir(opened_dir_struct **old_ods,char* a_filepath,int length);
split_path  getMostMatchedPath(int matches[],opened_dir_struct **openedpaths,int length ,char *newPath);
split_path lookupDir(opened_dir_struct**ods,char* a_filepath,int length);
=======
>>>>>>> bd1ecc9515603afe40a9312dd4a5db3cfba6bf94

