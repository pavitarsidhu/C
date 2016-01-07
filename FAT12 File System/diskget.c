#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

uint8_t file_FAT_index;
uint32_t fileSize;

int fetch_FAT_entry_number_value(int index, uint8_t *buffer) {

	/*
	FAT FETCHING INSTRUCTIONS:
	If n is even, then the physical location of the entry is the low four bits in location 1+(3*n)/2
	and the 8 bits in location (3*n)/2
	• If n is odd, then the physical location of the entry is the high four bits in location (3*n)/2 and
	the 8 bits in location 1+(3*n)/2 
	*/
	
	int value;

	if(index % 2) {
		//odd case
		value = ((buffer[((3*index)/2) + 512] & 0xF0) >> 4) | (buffer[(1+(3*index)/2) + 512] << 4);

	} else {
		//even case
		value = buffer[((3*index)/2) + 512] | ((buffer[(1+(3*index)/2) + 512] & 0x0F) << 8);
		
	}
	
	return value;
}

int fetch_physical_sector(int FATEntryNumber) {
	 return (33 + FATEntryNumber - 2); 
}

int find_files_in_root_directory(uint8_t *buffer, char *fileName) {

	//The starting and ending byte index number of the root directory
	int start = 9728;
	int end = 16895;
	
	int index;
	int count;
	int files = 0;
	
	while(start<=end) {
	
		index = start;
		for(count = 0; count<16; count++) {
			//Terminate while loop, no need for further counting
			if(buffer[index] == 0) {
				return 1;
				break;
			//Free space, do not count
			} else if(buffer[index] == 0xE5) {
				continue;
			} else {
			//Find details for file/subdirectory here
				if((buffer[index + 11] & 0x10) == 0x10) {
					//This is a directory, so no need to check it at all
					continue;	
				} else {
					//this is a file
					
					//Check if it can be ignored according to rule given
					if(buffer[index + 11] != 0x0F) {					
					
						int temp;
						int temp2 = 0;
						char name[30];
						for(temp = 0; temp<=7; temp++) {
							if((isprint(buffer[index + temp])) && (buffer[index+temp] != 32)) {
								name[temp2] = buffer[index + temp];
								temp2++;		
							}
						}
						
						//Check if file has an extension
						if((isprint(buffer[index + temp])) && (buffer[index+temp] != 32)) {
							name[temp2] = '.';
							temp2++;
						} 
						
						//Add file extension (if applicable)
						int i;
						for(i = 8;i<11;i++) {
							if((isprint(buffer[index + temp])) && (buffer[index+temp] != 32)) {
								name[temp2] = buffer[index + i];
								temp2++;
							}
						}
						
						name[temp2] = '\0';
						//Check for a file name match
						if((strcmp(name,fileName)) == 0) {
							file_FAT_index = (buffer[index + 27] << 8) | buffer[index + 26];
														
							return 0;
						} 
					}
				}
			}

			index += 32;
			
		}
		start += 512;
	}
	
	return 1;
}

void copy_file(char *fileName,uint8_t *buffer) {

	FILE *filePointer = fopen(fileName,"wb");
	
	//fprintf(filePointer,"%#04x ",0x4D);
	//Find appropriate bytes and write to filePointer
	
	int FATIndexValue = 0;
	int physicalSector = 0;
	int index = 0;
	int i;
	
	while(FATIndexValue != 0xFFF) {
		physicalSector = fetch_physical_sector(file_FAT_index);
		index = physicalSector * 512;
		
		//Add 512 bytes from current sector
		for(i = 0; i<512; i++) {
			//To insert hex use swap %c with %#04x
			//fprintf(filePointer,"%#04x ",buffer[index]);
			putc(buffer[index], filePointer);
			index++;
		}
		//Get next logical sector
		FATIndexValue = fetch_FAT_entry_number_value(file_FAT_index,buffer);
		file_FAT_index = FATIndexValue;
	}
	
	fclose(filePointer);
}

int main(int argc, char*arg[]) {

	if(argc < 3) {
		printf("Please provide a disk image file and the file you would like to search for.\n");
		exit(-1); 
	} 

	FILE *diskImageFile = fopen(arg[1],"rb"); 
	uint8_t *buffer; 
	
	//Exit program if invalid file given
	if(diskImageFile == NULL) {
		printf("Please provide a valid disk image file.\n"); 
		exit(-1); 
	} 
	
	//Jump to end of file in order to get the length of the file
	fseek(diskImageFile, 0, SEEK_END);
	int fileLength = ftell(diskImageFile);
	rewind(diskImageFile);
	
	//Read the file into the buffer and close file pointer to disk Image
	buffer = (uint8_t *)malloc((fileLength+1)*sizeof(uint8_t));
	int itemsInserted = fread(buffer, fileLength, 1, diskImageFile); 
	fclose(diskImageFile);
	
	int findFile = find_files_in_root_directory(buffer, arg[2]);

	if(findFile == 0) {
		//file was found
		
		copy_file(arg[2],buffer);
		
	} else {
		//file was not found
		printf("File not found.\n");
		
		exit(1);
	}
  	
	return 0; 
}