#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint8_t *buffer;
uint8_t *copyFileBuffer;

int fetch_physical_sector(int FATEntryNumber) {
	 return (33 + FATEntryNumber - 2); 
}

int add_file(char *diskName, char *newFile, uint16_t firstFreeFATIndex, uint32_t fileSize) {

	//The starting and ending byte index number of the root directory
	int start = 9728;
	int end = 16895;
	
	int index;
	int count;
	int flag = 0;
	int FAT_index = 0;
	
	while(start<=end) {
		index = start;
		for(count = 0; count<16; count++) {
			if((buffer[index] == 0xE5) || (buffer[index] == 0x00)) {
				
				FAT_index = index;
				
				int i = 0;
				//insert the name
				while(*newFile != '\0') {		
					if(*newFile != 46) {
						buffer[index + i] = *newFile;
						newFile++;
						i++;
					} else {
						newFile++;
						break;
					}
				}
				
				//insert the extension
				i = 8;
				while(*newFile != '\0') {
					buffer[index + i] = *newFile;
					newFile++;
					i++;
				}
				
				//file attribute
				buffer[i] = 0x01;
				
				//file first cluster (first FAT index)
				buffer[index + 27] = ((firstFreeFATIndex & 0xFF00) >> 8);
				buffer[index + 26] = (firstFreeFATIndex & 0x00FF);
								
				//insert file size
				buffer[index + 31] = (fileSize & 0xFF000000) >> 24;
				buffer[index + 30] = (fileSize & 0x00FF0000) >> 16;
				buffer[index + 29] = (fileSize & 0x0000FF00) >> 8;
				buffer[index + 28] = (fileSize & 0x000000FF); 
					
				flag = 1;
				return FAT_index;
			} 
			
			index += 32;
			
			//exit for loop if free entry already found
			if(flag == 1) {
				break;
			}
		}
		
		//exit while loop if free entry already found
		if(flag == 1) {
			break;
		}
		
		start += 512;
	}
	
	return FAT_index;
}

int fetch_FAT_entry_number_value(int index) {	
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

int find_total_size() {

	uint16_t bytesPerSector = (buffer[12] << 8) | buffer[11];
	uint16_t totalSectors = (buffer[20] << 8) | buffer[19];
	int totalSize = bytesPerSector * totalSectors;
	
	return totalSize;
}

void check_free_space(int totalSpace, long filelen) {
	
	int index = 2; 
	int usedMemory = 0;
	int value;
	
	uint16_t bytesPerSector = (buffer[12] << 8) | buffer[11];
	int numberOfEntriesInFat = (9 * bytesPerSector * 8)/12;
	
	while(index != numberOfEntriesInFat) {
		value = fetch_FAT_entry_number_value(index);
		if(value != 0) {
			usedMemory += 512;
		}
		index++;
	}
	
	int freeMemory = totalSpace - usedMemory;
	
	if(filelen > freeMemory) {
		printf("Not enough free space in the disk image.");
		exit(0);
	}
}

int fetch_free_FAT_index(long previous) {
	int index = 2; 
	int usedMemory = 0;
	int value;
	
	uint16_t bytesPerSector = (buffer[12] << 8) | buffer[11];
	int numberOfEntriesInFat = (9 * bytesPerSector * 8)/12;
	
	while((index != numberOfEntriesInFat)) {
	
		if(index == previous) {
			index++;
			continue;
		} else {
	
			value = fetch_FAT_entry_number_value(index);
			if(value == 0) {
				return index;
				usedMemory += 512;
			}
			index++;
		}
	}
	
	return 0;
}

void update_FAT_packing(long index, long value) {
	//value is the next FAT entry

	uint8_t fourBitByte;
	uint8_t eightBitByte;
 
	if(index % 2) { 
 		//This is for the odd index case
 		
		fourBitByte = (buffer[512 + (3*index)/2] & 0x0F) + ( value << 4 );
		eightBitByte = value >> 4;
   
     	buffer[512 + (3*index)/2] = fourBitByte;
     	buffer[513 + (3*index)/2] = eightBitByte;
       
    } else { 
    	//This is for the even index case
 
		fourBitByte = (buffer[513 + (3*index)/2] & 0xF0) + ( value >> 8 );
    	eightBitByte = value & 0xFF;
               
		buffer[512 + (3*index)/2] = eightBitByte;
		buffer[513 + (3*index)/2] = fourBitByte;
    }
}

void insert_bytes(uint16_t firstFATIndex, long totalBytesToInsert) {
	long bytesInserted = 0;
	long previousFATIndex = 0;  
	long currentFATIndex = firstFATIndex;
	//long totalBytesToInsert = 0;
	
	long copyFileBufferIterator = 0;
	
	long index;
	
	long finalFlag = 0;
	
	//find total number of bytes needed to be inserted
	int i = 0;

	while(bytesInserted < totalBytesToInsert) {
		int physicalSector = fetch_physical_sector(currentFATIndex);
		index = physicalSector * 512; 
		
		for(i = 0;i<512;i++) {
			
			//Before inserting bytes, you must check if limit has already been hit
			if(bytesInserted == totalBytesToInsert) {
				buffer[index] = 0x00;
				index++;
				copyFileBufferIterator++;
				finalFlag = 1;
			} else {
				buffer[index] = copyFileBuffer[copyFileBufferIterator];
				index++;
				copyFileBufferIterator++;
				bytesInserted++;
			}	
		}
		
		//find next FAT Index to use
		previousFATIndex = currentFATIndex;
		currentFATIndex = fetch_free_FAT_index(previousFATIndex);
		//check if final flag is set. If it is set, set value of previousFatIndex entry to 0xFFF
		if(finalFlag == 1) {
			update_FAT_packing(previousFATIndex,0xFFF);
			break;
		}
		
		//do FAT packing updates
		update_FAT_packing(previousFATIndex,currentFATIndex);
	}
}


int main(int argc, char*arg[]) {

	if(argc < 3) {
		printf("Please provide a disk image file and a file to copy to the root directory of the disk image.\n");
		exit(-1); 
	} 
	
	FILE *diskImageFile = fopen(arg[1],"rb");  
	
	
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
	
	
	FILE *filePointer = fopen(arg[2],"rb");
	long filelen;
	
	
	if(filePointer == NULL) {
		printf("File not found.\n");
		exit(-1);
	} else {
	
		fseek(filePointer, 0, SEEK_END);
		filelen = ftell(filePointer);
		rewind(filePointer);
		
		copyFileBuffer = (uint8_t *)malloc((filelen+1)*sizeof(uint8_t));
		fread(copyFileBuffer, filelen, 1, filePointer);
		fclose(filePointer);
	
		int totalSpace = find_total_size();
		check_free_space(totalSpace,filelen);
		
		uint16_t firstFreeIndex = fetch_free_FAT_index(0);
		int rootDirectoryEntry = add_file(arg[1], arg[2], firstFreeIndex, filelen);
		
		insert_bytes(firstFreeIndex, filelen);	
		
		//Write the newly created disk image array to the disk image file
		FILE *fileptr = fopen(arg[1], "wb+");
		fwrite(buffer, sizeof(uint8_t), fileLength + 1, fileptr);
		fclose(fileptr);
	}
	return 0; 
}