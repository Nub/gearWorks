#include <stdio.h>
#include <time.h>

#define GWB_DEFAULT_FILE "./gwBuildLog.log"
#define GWB_VERSION 0.1

void printUsage(){
	printf("    Gear Works Build Manager Ver. %.1f\n---------------------------------------------\nUsage:gwBuildManager [-amn:sv][filename]\n-a Add to value\n-m Subtract from value\n-n New Log File\n-b Add to build(default file)\n:s Subversion\n:v Version\nExample:\ngwBuildManager -a:v buildLog.log\nOutput:\nV.1.0 build:0123\n(adds 1 to the major version)\n", GWB_VERSION);
}

//checks if the arg is a modifier or a filename
int isFilename(char *arg){
	if(arg[0] == '-')
		return 0;
	else
		return 1;
}

void makeNewLogFile(char *filename){
	FILE *fp = fopen(filename, "w");
	if(!fp)
		return;
	
	time_t rawtime;
	struct tm * timeinfo;
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	
	fprintf(fp, "Log Created On:%s\nV0.0 build:0000",asctime (timeinfo));
	fclose(fp);
}

void appendToLog(char *filename, float v, int b){
	FILE *fp = fopen(filename, "a");
	if(!fp)
		return;
	
	time_t rawtime;
	struct tm * timeinfo;
	
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	
	fprintf(fp, "\nV%.1f build:%.4d date:%s",v,b,asctime (timeinfo));
	printf("V%.1f build:%.4d date:%s",v,b,asctime (timeinfo));
	fclose(fp);
}

int main (int argc, const char * argv[]) {
	
	int buildVersion = 0;
	float version = 0;
	
	if(argc > 3 || argc <= 1){
		printUsage();
	}else if(argc == 2){
		if(isFilename((char*)argv[1])){
			//adjust build version in log
			FILE *fp = fopen((char*)argv[1], "r");
			if(!fp)
				return 0;
			char buffer[200];
			while(fgets(buffer,200,fp));//get last line
			fclose(fp);
			sscanf(buffer,"V%f build:%d",&version, &buildVersion);
			buildVersion ++;
			appendToLog((char*)argv[1],version,buildVersion);
		}else{
			if(argv[1][1] == 'n'){
				makeNewLogFile(GWB_DEFAULT_FILE);
				return 0;//we are done here
			}
			//adjust versions in default log
			FILE *fp = fopen(GWB_DEFAULT_FILE, "r");
			if(!fp)
				return 0;
			char buffer[200];
			while(fgets(buffer,200,fp));//get last line
			fclose(fp);
			sscanf(buffer,"V%f build:%d",&version, &buildVersion);
			buildVersion ++;
			if(argv[1][1] == 'a'){
				if(argv[1][3] == 's'){
					version += 0.1f;
				}else{
					version += 1.0f;
				}
			}else if(argv[1][1] == 'm'){
				if(argv[1][3] == 's'){
					version -= 0.1f;
				}else{
					version -= 1.0f;
				}
			}
			appendToLog(GWB_DEFAULT_FILE,version,buildVersion);
		}
	}else if(argc == 3){
		if(argv[1][1] == 'n'){
			makeNewLogFile((char*)argv[2]);
		}else{
			FILE *fp = fopen((char*)argv[2], "r");
			if(!fp)
				return 0;
			char buffer[200];
			while(fgets(buffer,200,fp));//get last line
			fclose(fp);
			sscanf(buffer,"V%f build:%d",&version, &buildVersion);
			
			buildVersion ++;
			if(argv[1][1] == 'a'){
				if(argv[1][3] == 's'){
					version += 0.1f;
				}else{
					version += 1.0f;
				}
			}else if(argv[1][1] == 'm'){
				if(argv[1][3] == 's'){
					version -= 0.1f;
				}else{
					version -= 1.0f;
				}
			}
			appendToLog((char*)argv[2],version,buildVersion);
			
		}
		//adjust in custom file and versions
	}
	return 0;
}
