#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

void find_sectors_per_FAT(uint8_t *buffer) {
	
	uint16_t sectorsPerFat = (buffer[23] << 8) | buffer[22];
	
	printf("Sectors per FAT: %d\n",sectorsPerFat);
}

void find_FAT_tables(uint8_t *buffer) {
	
	int copiesOfFAT = buffer[16];
	
	printf("Number of FAT copies: %d\n",copiesOfFAT);
}

void find_files_in_root_directory(uint8_t *buffer) {

	//The starting and ending byte index number of the root directory
	int start = 9728;
	int end = 16895;
	
	int index;
	int count;
	int files = 0;
	int flag = 1;
	
	while(start<=end) {
	
		index = start;
		for(count = 0; count<16; count++) {
			//Terminate while loop, no need for further counting
			if(buffer[index] == 0) {
				flag = 0;
				break;
			//Free space, do not count
			}else if(buffer[index] == 0xE5) {
				continue;
			} else {
				files++;
			}
			if(flag == 0) {
				break;
			}
			index += 32;
		}
		
		start += 512;
	}
	
	printf("The number of files in the root directory (not including subdirectories): %d\n",files);
}

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

void find_OS_name(uint8_t *buffer) {
	//Find OS Name
	char name[9];
	name[0] = buffer[3];
	name[1] = buffer[4];
	name[2] = buffer[5];
	name[3] = buffer[6];
	name[4] = buffer[7];
	name[5] = buffer[8];
	name[6] = buffer[9];
	name[7] = buffer[10];
	name[8] = '\0';
	
	printf("OS Name: %s\n",name);
}

void find_label_of_disk(uint8_t *buffer) {
	
	char label[12];
	int i;
	int index = 0;
	for(i = 43; i<54; i++) {
		label[index] = buffer[i];
		index++;
	}
	label[index] = '\0';
	printf("Label of the disk: %s\n",label);
}

int find_total_size(uint8_t *buffer) {

	uint16_t bytesPerSector = (buffer[12] << 8) | buffer[11];
	uint16_t totalSectors = (buffer[20] << 8) | buffer[19];
	printf("Total Sectors = %d\n",totalSectors);
	int totalSize = bytesPerSector * totalSectors;
	
	printf("Total size of the disk: %d Bytes\n",totalSize);
	return totalSize;
}

void find_free_space(uint8_t *buffer,int totalSpace) {
	
	int index = 2; 
	int usedMemory = 0;
	int value;
	
	uint16_t bytesPerSector = (buffer[12] << 8) | buffer[11];
	int numberOfEntriesInFat = (9 * bytesPerSector * 8)/12;
	
	while(index != numberOfEntriesInFat) {
		value = fetch_FAT_entry_number_value(index, buffer);
		if(value != 0) {
			usedMemory += 512;
		}
		index++;
	}
	
	int freeMemory = totalSpace - usedMemory;
	printf("Free space: %d Bytes\n",freeMemory); 
}

int main(int argc, char*arg[]) {

	FILE *diskImageFile = fopen(arg[1],"rb"); 
	uint8_t *buffer; 
	
	//Exit program if invalid file given
	if(diskImageFile == NULL) {
		printf("Please provide a valid file.\n"); 
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
  
  	find_OS_name(buffer); 
  	find_label_of_disk(buffer);
  	int totalSize = find_total_size(buffer);
  	find_free_space(buffer,totalSize); 
  	find_FAT_tables(buffer);
  	find_sectors_per_FAT(buffer);
  	find_files_in_root_directory(buffer); 
  	
	return 0; 
}