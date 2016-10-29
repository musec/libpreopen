#include<stdio.h>
#include<stdlib.h>





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

Map* preopen(char* file,int mode,int flag);
Map* initializeMap(int );
matched_path map_path(Map *map,  char * path);
Map* getMap();
char* split_path_file(char *relative_path);
int pathCheck(char *path);
void open_directory(char* relative_path,opened_dir_struct *ods);
int checkCapacity(Map*map);
Map* increaseMapCapacity(Map *map);
int findMatchingChars(char *A,char *B);
matched_path  getMostMatchedPath(int matches[],char *newPath,int mode, int flag);


