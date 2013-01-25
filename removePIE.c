#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <stdint.h>
#include <mach-o/loader.h>

void hexify(unsigned char *data, uint32_t size){
	while(size--)
		printf("%02x", *data++);}

void fcopy(FILE *f1, FILE *f2){
    char            buffer[BUFSIZ];
    size_t          n;

    while ((n = fread(buffer, sizeof(char), sizeof(buffer), f1)) > 0){
        if (fwrite(buffer, sizeof(char), n, f2) != n)
            printf("Error copying backup");}
}

int main(int argc, char *argv[]){
	struct mach_header currentHeader;
	//int32_t magic = 0;
	FILE *fp; //edited file pointer
	FILE *fw; //backup file pointer
	char fwName[80];
	char fwPrefix[4] = ".bak"; //app.bak
	
	if(argc < 1){	
		printf("Please enter the filename binary: in the format removePIE filename");
		return EXIT_FAILURE;}
	if((fp = fopen(argv[1], "rb+")) == NULL) {
		printf("Error, unable to open file\n");
		return EXIT_FAILURE; }
	//create app.bak filename
	strlcpy(fwName, argv[1], strlen(argv[1])+1);
	strlcat(fwName, fwPrefix, strlen(fwPrefix)+1);
	if((fw = fopen(fwName, "wb")) == NULL){
	return EXIT_FAILURE;}
	
	if((fread(&currentHeader.magic, sizeof(int32_t), 1, fp)) == (int)NULL)
	{printf("Error reading magic constant in file\n");
		return EXIT_FAILURE;}
	if(currentHeader.magic == MH_MAGIC){ //little endian
		printf("loading header\n");
		fseek(fp, 0, SEEK_SET);
		if((fread(&currentHeader, sizeof(currentHeader), 1, fp)) == (int)NULL)
		{
			printf("Error reading MACH-O header");
			return EXIT_FAILURE;
		}	
		fseek(fp, 0, SEEK_SET); //set fp back to 0 to get full copy
		printf("\nbacking up application binary...\n");
		fcopy(fp, fw);
		fclose(fw);
		printf("\nbinary backed up to:\t%s\n", fwName);
		printf("\nmach_header:\t");
		hexify((unsigned char *)&currentHeader,sizeof(currentHeader));	
		printf("\noriginal flags:\t");
		hexify((unsigned char *)&currentHeader.flags, sizeof(currentHeader.flags));
		printf("\nDisabling ASLR/PIE ...\n");
		currentHeader.flags &= ~MH_PIE;
		printf("new flags:\t");
		hexify((unsigned char *)&currentHeader.flags, sizeof(currentHeader.flags));
		
		fseek(fp, 0, SEEK_SET);
		if((fwrite(&currentHeader, sizeof(char), 28, fp)) == (int)NULL)
		{
			printf("Error writing to application file %s\n",fwName);
		}
		printf("\nASLR has been disabled for %s\n", argv[1]);
		//exit and close memory
		//free(mach_header);
		fclose(fp);
		return EXIT_SUCCESS;	
	}
	else if(currentHeader.magic == MH_CIGAM) // big endian
	{
		printf("file is big-endian, not an iOS binary");
		return EXIT_FAILURE;
	}
	else
	{
		printf("File is not a MACH_O binary"); 
		return EXIT_FAILURE;
	}
	//exit
	return EXIT_FAILURE;	 
}
