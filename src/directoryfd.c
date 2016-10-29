<<<<<<< HEAD
/*
 * directoryfd.c
 *
 *  Created on: Oct 15, 2016
 *      Author: ujay
 */
=======
>>>>>>> 57c6d95f33e8e77d68167105660a8f3ceeeafeeb
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <limits.h>
<<<<<<< HEAD
#include <sys/types.h>
#include <dirent.h>
#include<fcntl.h>
#include<string.h>
#include"cwrapHeader.h"

int main(){
	int g;
	g=open("/usr/home/ujay//cd",O_RDONLY);

return 0;
}



=======
#include"prehead.h"
#include <sys/types.h>
#include <dirent.h>




int main(){
	opened_dir_struct * ods,*ods1,*ods2,*ods3;
	int i,bestMatchmatchNum,k=0;
	opened_dir_struct **ods_array;
	int len=3;
	int matchNum[len];
	char *testPath="/usr/home/ujay/";

	split_path splitPath;
	ods_array=(opened_dir_struct**)malloc(3*sizeof(opened_dir_struct*));

	char * newPath="/usr/ports/editors";
	ods=(opened_dir_struct*)malloc(sizeof(opened_dir_struct));
	ods->dirname="/usr/share/man/man8/";
	open_directory(ods->dirname,ods);

	ods1=(opened_dir_struct*)malloc(sizeof(opened_dir_struct));
	ods1->dirname="/usr/home/ujay/Documents/";
	open_directory(ods1->dirname,ods1);
	
	ods2=(opened_dir_struct*)malloc(sizeof(opened_dir_struct));
	ods2->dirname="/usr/ports/editors/vim";
	open_directory(ods2->dirname,ods2);

	ods3=(opened_dir_struct*)malloc(sizeof(opened_dir_struct));
	ods3->dirname="/usr/home/ujay/workspace/HelloWorld/";
	open_directory(ods3->dirname,ods3);

	ods_array[0]=ods;
	
	ods_array[1]=ods1;

	ods_array[2]=ods2;

	ods_array=add_OpenDir(ods_array,ods3,&len);
	for(i=0;i<len;i++){

		matchNum[i]=findMatchingChars(ods_array[i]->dirname,testPath);
		if(matchNum[i]>k){
			k=matchNum[i];
			bestMatchmatchNum=i;
		}
	}
	
	
	
	splitPath=lookupDir(ods_array,testPath,len);
	if(splitPath.relative_path!=NULL){
		printf("\n %s:%d\n",splitPath.relative_path,splitPath.dirfd);
	}
	else{
		printf("SplitPath is null\n");
	}
	

	/*splitPath=lookupDir(ods_array,newPath,3);
	if(splitPath!=NULL){
		//printf("%s\n",splitPath->relative_path);
		printf("%d\n",splitPath->dirfd);
	}
	else{
		printf("splitPath is NULL\n");
	}*/
	for(i=0;i<3;i++){
		free(ods_array[i]);
	}
	free(ods_array);
	return 0;
}
>>>>>>> 57c6d95f33e8e77d68167105660a8f3ceeeafeeb
