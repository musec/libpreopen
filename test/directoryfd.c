
/*
 * directoryfd.c
 *
 *  Created on: Oct 15, 2016
 *      Author: ujay
 *
 * This test doesn't actually run yet:
 * XFAIL: *
 */

#include <sys/types.h>

#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "libpo.h"

int main(){
	int g,j,i;
	/*g=open("/usr/ports/editors",O_RDONLY);
	g=open("/usr/home/ujay/cd",O_RDONLY);
	g=open("/usr/share/man/man8/",O_RDONLY);
	g=open("/usr/home/ujay/Documents/",O_RDONLY);
	g=open("/usr/home/ujay/workspace/HelloWorld/",O_RDONLY);*/

	int matched[5];
	matched_path mp;
	char* file="/usr/home/";
	Map * map= getMap();
	map=preopen(map,"/usr/ports/editors",O_RDONLY);
	map=preopen(map,"/usr/home/ujay/cd",O_RDONLY);
	map=preopen(map,"/usr/share/man/man8/",O_RDONLY);
	map=preopen(map,"/usr/home/ujay/Documents/",O_RDONLY);
	map=preopen(map,"/usr/home/ujay/workspace/HelloWorld/",O_RDONLY);
	//mp=map_path(map,"/usr/home/ujay/Documents/mybin",O_RDONLY);
	j=open("/usr/home/ujay/workspace/HelloWorld/",O_RDONLY);
	g=open("/usr/home/ujay/Documents/cd",O_RDONLY);
	j=open("/usr/home/ujay/cd",O_RDONLY);

	j=open("/usr/ports/editors",O_RDONLY);

	/*Map * map= getMap();

	g=open("/usr/ports/editors",O_RDONLY);
	j=open("/usr/home/ujay/cd",O_RDONLY);
	j=open("/usr/home/ujay/workspace/HelloWorld/",O_RDONLY);*/

	//printf("%s\n",mp.relative_path);
	//printf("%d\n",mp.dirfd);
return 0;
}



