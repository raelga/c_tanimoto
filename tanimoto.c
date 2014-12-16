#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <omp.h>/* OpenMP */
#include <unistd.h> /* close */
#include <sys/mman.h>
#include <fcntl.h>
//#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#define NUM_MAPS 22
#define MAX_FILEPATH 512
#define MODE_OR 1
#define MODE_AND 2
#define MODE_TARGET 3
#define MODE_PROBE 4
/*
unsigned long long crono(int startstop)
{
  static unsigned long pre_time;
  static unsigned long pre_secs;
  struct timespec ts;

  if (startstop)
    {
          clock_gettime(CLOCK_MONOTONIC, &ts);
          pre_time=ts.tv_nsec;
          pre_secs=ts.tv_sec;
          return (ts.tv_sec-pre_secs)*1000000000+ts.tv_nsec;
        }
  else
    {      
          clock_gettime(CLOCK_MONOTONIC, &ts);
          return (ts.tv_sec-pre_secs)*1000000000+ts.tv_nsec - pre_time;
        }

    return 0;
}
*/
int usage() {

    printf("./tanimoto PATH_TARGET PATH_PROBE  MODE\n");
    printf("\tmodes \n\t\t1 target OR probe (default)\n\t\t2 target AND probe \n\t\t3 target \n\t\t4 probe");
    printf("\n"); 
    return 1;
}

int main (int argc, char* argv[]) {

// ##################################################################################
//crono(1);
// ##################################################################################

    float *vTanimoto = calloc(NUM_MAPS, sizeof(float));
	unsigned int *vUnion = calloc(NUM_MAPS, sizeof(unsigned int));
	unsigned int *vIntersec = calloc(NUM_MAPS, sizeof(unsigned int));
	//unsigned int vUnion[NUM_MAPS] =  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	//unsigned int vIntersec[NUM_MAPS] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

	const char* vNames[NUM_MAPS] = { "A", "Br", "C", "Ca",
                        	        "Cl", "F", "Fe", "HD",
                					"I", "Mg", "Mn", "N",  
                					"NA", "NS", "OA", "OS", 
                					"P", "S", "SA", "Zn",
                                    "d", "e"};

    unsigned char mode;
    
    switch (argc) {
        case 3 : mode = 1; break; 
        case 4 : mode = atoi(argv[3]); break;
        default: return usage();
    }

	char* mTarget = argv[1]; 
	char* mProbe = argv[2];
/*
    struct stat st;
    if(stat(mTarget,&st)) {printf("%s not found,\n",mTarget); return usage();}
    if(stat(mProbe,&st)) {printf("%s not found,\n",mProbe); return usage();}
 */

    if((mode>4) || (mode<1)) return usage();


	const float rIntersec[3] = { 0.25, 0.01, 0.005 };
	const float rUnionMin[5] = { 0.25, 0.2,  0.01, 0.01,  0.5 };
	const float rUnionMax[5] = {  100, 100,  100,   0.1,    4 };

	int mapIndex=0;

// ##################################################################################    
//printf("map \t %i \t Main Init \t %lld \n",mapIndex,crono(0));crono(1);
// ##################################################################################

    #pragma omp parallel shared(vTanimoto, vIntersec, vUnion, mTarget, mProbe) private(mapIndex)
    {
    #pragma omp for schedule(guided)
    for (mapIndex=0; mapIndex < NUM_MAPS; mapIndex++) {

// ##################################################################################
//crono(1);
// ##################################################################################

        unsigned int i, j;
        unsigned char iIntersec, iUnion;

// ##################################################################################
//printf("map \t %i \t Map Init \t %lld \n",mapIndex,crono(0));crono(1);
// ##################################################################################

	iIntersec = ((mapIndex == 20)<<1) + (mapIndex == 6);

// ##################################################################################
//printf("map \t %i \t i Intersec \t %lld \n",mapIndex,crono(0));
//crono(1);
// ##################################################################################

    iUnion = ((mapIndex == 21)<<2) + ((mapIndex == 20)*3) + ((mapIndex == 6)<<1) + (mapIndex == 5);
//   printf("dbg %s = %u\n", vNames[mapIndex], iUnion);
// ##################################################################################
//printf("map \t %i \t i Union \t %lld \n",mapIndex,crono(0));
//crono(1);
// ##################################################################################

	unsigned int st = strlen(mTarget);
	unsigned int sp = strlen(mProbe);

    char *fnTarget = calloc(MAX_FILEPATH, sizeof(char));
    char *fnProbe = calloc(MAX_FILEPATH, sizeof(char));

//printf("dbg \t %i \t Get File \t %s \t %s \t %s \n",mapIndex,vNames[mapIndex],mTarget, mProbe);

	for (i = 0; i < st; i++) fnTarget[i] = mTarget[i];
    fnTarget[st++] = '.';
    for (i = 0; i < sp; i++) fnProbe[i] = mProbe[i];
    fnProbe[sp++] = '.';
    
	for (i = 0; i < strlen(vNames[mapIndex]); i++) {
		fnTarget[st+i] = vNames[mapIndex][i];
		fnProbe[sp+i] = vNames[mapIndex][i];
	}

    fnTarget[st+i] = '.'; fnTarget[st+i+1] = 'm'; fnTarget[st+i+2] = 'a'; fnTarget[st+i+3] = 'p';
    fnProbe[sp+i] = '.'; fnProbe[sp+i+1] = 'm'; fnProbe[sp+i+2] = 'a'; fnProbe[sp+i+3] = 'p';
//  st += i; sp+=i;
//printf("dbg \t %i \t Parse Loop \t %s \t %s \n",mapIndex,fnTarget,fnProbe);

// ##################################################################################
//printf("map \t %i \t Parse Loop \t %lld \n",mapIndex,crono(0));crono(1);
// ##################################################################################
/*
//map 	 0 	 Parse Loop 	 520 
//map 	 0 	 Parse String 	 1682 
        char fnTarget[MAX_FILEPATH] = "";
        strcpy(fnTarget, mTarget);
        strcat(fnTarget, vNames[mapIndex]);
//
        char fnProbe[MAX_FILEPATH] = "";
        strcpy(fnProbe, mProbe);
        strcat(fnProbe, vNames[mapIndex]);
*/
// ##################################################################################
//printf("map \t %i \t Parse String \t %lld \n",mapIndex,crono(0)); 
//crono(1);
// ##################################################################################

struct stat buffer;
int status;
int fdTarget;
int fdProbe;
unsigned long fsTarget;
unsigned long fsProbe;

fdTarget = open(fnTarget, O_RDWR);
status = fstat(fdTarget, &buffer);
if(status) printf("%s not found,\n",fnTarget);
char* mmTarget = mmap(0, buffer.st_size, PROT_READ, MAP_SHARED, fdTarget, 0);
fsTarget = buffer.st_size/sizeof(char);

fdProbe = open(fnProbe, O_RDWR);
status = fstat(fdProbe, &buffer);
if(status) printf("%s not found,\n",fnProbe);
char* mmProbe = mmap(0, buffer.st_size, PROT_READ, MAP_SHARED, fdProbe, 0);
fsProbe = buffer.st_size/sizeof(char);

unsigned long jt = 0;
unsigned long jp = 0;

//printf("Move To First START");
// Move to first value
i = 0;
for (jt = 0; ((jt <= fsTarget) && (i<6)); jt++){
	i += (mmTarget[jt] == '\n');
}

i = 0;
for (jp = 0; ((jp <= fsProbe) && (i<6)); jp++){
	i += (mmProbe[jp] == '\n');
}

//printf("dbg\tmap \t %i \t Read File \n",mapIndex);
// ##################################################################################
//printf("map \t %i \t Read File \t %lld \n",mapIndex,crono(0));
//crono(1);
// ##################################################################################

unsigned char eval;
float p1 = 0, p2 = 0;

while((jt<(fsTarget-8)) && (jp<(fsProbe-8))) {

    /*
     * Target
     */

    jt++;
    char f1[8] = " ";
//  for(j=0; (mmTarget[jt+j] != '\n'); j++) f1[j] = mmTarget[jt+j];

    j=1;
    f1[0] = mmTarget[jt];
    f1[1] = mmTarget[jt+1];
    if((f1[1] == '\n')) goto savep1;
    f1[2] = mmTarget[jt+2]; j = 2;
    if((f1[2] == '\n')) goto savep1;
    f1[3] = mmTarget[jt+3];
    f1[4] = mmTarget[jt+4];
    f1[5] = mmTarget[jt+5]; j = 5;
    if((f1[5] == '\n')) goto savep1;
    f1[6] = mmTarget[jt+6]; j = 6;
    if((f1[6] == '\n')) goto savep1;
    f1[7] = mmTarget[jt+7]; j = 7;

savep1:

    p1 = atof(f1);
	jt += j;

    /*
     * Probe
     */

    jp++;
    char f2[8] = "";
//    for(j=0; (mmProbe[jp+j] != '\n'); j++) f2[j] = mmProbe[jp+j];

    j=1;
    f2[0] = mmProbe[jp];
    f2[1] = mmProbe[jp+1];
    if((f2[1] == '\n')) goto savep2;
    f2[2] = mmProbe[jp+2]; j = 2;
    if((f2[2] == '\n')) goto savep2;
    f2[3] = mmProbe[jp+3];
    f2[4] = mmProbe[jp+4];
    f2[5] = mmProbe[jp+5]; j = 5;
    if((f2[5] == '\n')) goto savep2;
    f2[6] = mmProbe[jp+6]; j = 6;
    if((f2[6] == '\n')) goto savep2;
    f2[7] = mmProbe[jp+7]; j = 7;

savep2:

    p2 = atof(f2);
    jp += j;

    if((mapIndex == 21))
    {
        switch (mode) {
            case 2 : eval = (((fabs(p1) <= rUnionMax[iUnion]) & (fabs(p1) >= rUnionMin[iUnion])) & ((fabs(p2) <= rUnionMax[iUnion]) & (fabs(p2) >= rUnionMin[iUnion]))); 
                    goto eval;
            case 3 : eval = ((fabs(p1) <= rUnionMax[iUnion]) & (fabs(p1) >= rUnionMin[iUnion])); 
                    goto eval;
            case 4 : eval = ((fabs(p2) <= rUnionMax[iUnion]) & (fabs(p2) >= rUnionMin[iUnion])); 
                    goto eval;
           default : eval = (((fabs(p1) <= rUnionMax[iUnion]) & (fabs(p1) >= rUnionMin[iUnion])) | ((fabs(p2) <= rUnionMax[iUnion]) & (fabs(p2) >= rUnionMin[iUnion])));
        }
        goto eval;
    }

    if((mapIndex == 20))
    {
        switch (mode) {
            case 2 : eval = (((p1 <= rUnionMax[iUnion]) & (p1 >= rUnionMin[iUnion])) & ((p2 <= rUnionMax[iUnion]) & (p2 >= rUnionMin[iUnion]))); 
                     goto eval;
            case 3 : eval = ((p1 <= rUnionMax[iUnion]) & (p1 >= rUnionMin[iUnion])); 
                     goto eval;
            case 4 : eval = ((p2 <= rUnionMax[iUnion]) & (p2 >= rUnionMin[iUnion])); 
                     goto eval;
           default : eval = (((p1 <= rUnionMax[iUnion]) & (p1 >= rUnionMin[iUnion])) | ((p2 <= rUnionMax[iUnion]) & (p2 >= rUnionMin[iUnion])));
        }
        goto eval;
    }

    switch (mode) {
            case 2 : eval = ((p1 >= rUnionMin[iUnion]) & (p2 >= rUnionMin[iUnion])); break;
            case 3 : eval = (p1 >= rUnionMin[iUnion]); break;
            case 4 : eval = (p2 >= rUnionMin[iUnion]); break;
            default : eval = ((p1 >= rUnionMin[iUnion]) | (p2 >= rUnionMin[iUnion]));
        }

eval:

    if(eval) {
	    vUnion[mapIndex]++;
	    vIntersec[mapIndex] += ((fabs(p1 - p2) <= rIntersec[iIntersec]));
    }
}

//printf("dbg\tmap \t %i \t Map \n",mapIndex);
vTanimoto[mapIndex] = (float) vIntersec[mapIndex]/vUnion[mapIndex];
if(vUnion[mapIndex] == 0) vTanimoto[mapIndex] = 0;

munmap(mmTarget, fsTarget*sizeof(char));
munmap(mmProbe, fsProbe*sizeof(char));

// ##################################################################################
//printf("map \t %i \t Similarity \t %lld \n",mapIndex,crono(0));//crono(1);
// ##################################################################################

/*
	Other stuff : 

    if(fUnion(p1, mapIndex) || fUnion(p2, mapIndex) ){

	Other reading file:

        fpTarget = fopen(fnTarget, "r");
        fpProbe  = fopen(fnProbe, "r");

	for (i=0; i<6; i++) { 
            fgets(line, sizeof(line),fpTarget);
            fgets(line, sizeof(line),fpProbe);
        }
	
      float *v;
      v=(float*) malloc (10000000*sizeof(float*));

	i = 0;
	while (fgets(line, sizeof(line), fpTarget)){
            	v[i] = atof(line);
		fgets(line, sizeof(line), fpProbe);
		v[i+1] = atof(line);
		i+=2;
	}

	fclose(fpTarget);
        fclose(fpProbe);



// ##################################################################################
//printf("map \t %i \t Read File \t %lld \n",mapIndex,crono(0));crono(1);
// ##################################################################################


// ##################################################################################
// ##################################################################################
// ##################################################################################

	Other implementations: Low Memory (~2Mb)

		float p1, p2;
		while (fgets(line, sizeof(line), fpTarget)){
	            	p1 = atof(line);
			fgets(line, sizeof(line), fpProbe);
			p2 = atof(line);
	
        		eval =  (((p1 <= rUnionMax[iUnion]) && (p1 >= rUnionMin[iUnion])) \
					|| ((p2 <=rUnionMax[iUnion]) && (p2 >= rUnionMin[iUnion])));
			vUnion[mapIndex] += eval;
			vIntersec[mapIndex] += (eval && ((fabs(p1 - p2) <= rIntersec[iIntersec])));
	
		}

*/

	}
    }

// ##################################################################################
//crono(1);
// ##################################################################################

for (mapIndex=0; mapIndex < NUM_MAPS; mapIndex++) {
	printf("%s=%f [%u/%u] ", vNames[mapIndex], (float) vTanimoto[mapIndex], vUnion[mapIndex], vIntersec[mapIndex]);
	//    printf("%s=%f ", vNames[mapIndex], vTanimoto[mapIndex]);
}                
printf("\n");

// ##################################################################################
//printf("All \t %i \t Print Output \t %lld \n",mapIndex,crono(0));
// ##################################################################################

    return 0;
}


