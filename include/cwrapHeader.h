#include<stdio.h>
#include<stdlib.h>
#include<string.h>






typedef struct{
	int dirfd;
	char*dirname;
	int flags;

}opened_dir_struct;// Holds opened directory fd and filename

typedef struct{
	char * relative_path;
	int dirfd;
}matched_path;

typedef struct{
	opened_dir_struct * opened_files;
	size_t capacity;
	size_t length;
}Map;

Map* preopen(char* file,int mode);
Map* initializeMap(int );
matched_path map_path(Map* map, const char * path, int mode);
Map* getMap();
char* split_path_file(char *relative_path);
int pathCheck(char *path);
opened_dir_struct * open_directory(char* relative_path,opened_dir_struct *);
int checkCapacity();
Map* increaseMapCapacity();
int findMatchingChars(char *A,char *B);
int  getMostMatchedPath(int matches[]);
Map* add_Opened_dirpath_map(opened_dir_struct ods);
matched_path compareMatched(Map* map,int num,char* character,int mode);
