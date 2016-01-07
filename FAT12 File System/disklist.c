#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>

int fileCount = 0;

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
			} else if(buffer[index] == 0xE5) {
				continue;
			} else {
			//Find details for file/subdirectory here
			
				if((buffer[index + 11] & 0x10) == 0x10) {
					//this is a directory

					//Check if it can be ignored according to rule given
					if(buffer[index + 11] != 0x0F) {					
						//Get file name
						int temp;
						int temp2 = 0;
						char name[30];
						for(temp = 0; temp<8; temp++) {
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
					
						uint16_t time = (buffer[index + 15] << 8) | buffer[index + 14]; 
						
						int hour = (time & 63488)  >> 11;
						int min = (time & 2016) >> 5; 
						int sec = (time & 0x001F) * 2; 
						
						uint16_t date = (buffer[index + 17] << 8) | buffer[index + 16]; 
						
						int year = ((date & 0xFE00) >> 9) + 1980;
						int month = (date & 0x01E0) >> 5;
						int day = (date & 0x001F); 
					
						long size = (buffer[index + 31] << 24) | (buffer[index + 30] << 16) 
						| (buffer[index + 29] << 8) | (buffer[index + 28]);
						
						printf("D %ld %s %d-%d-%d %02d:%02d:%02d\n",size,name,year,month,day,hour,min,sec);
					}
					
				} else {
					//this is a file
					
					//Check if it can be ignored according to rule given
					if(buffer[index + 11] != 0x0F) {					
					
						//Get file name
						int temp;
						int temp2 = 0;
						char name[30];
						for(temp = 0; temp<8; temp++) {
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
					
						uint16_t time = (buffer[index + 15] << 8) | buffer[index + 14]; 
						
						int hour = (time & 63488)  >> 11;
						int min = (time & 2016) >> 5; 
						int sec = (time & 0x001F) * 2; 
						
						uint16_t date = (buffer[index + 17] << 8) | buffer[index + 16]; 
						
						int year = ((date & 0xFE00) >> 9) + 1980;
						int month = (date & 0x01E0) >> 5;
						int day = (date & 0x001F); 
						
						long size = (buffer[index + 31] << 24) | (buffer[index + 30] << 16) 
						| (buffer[index + 29] << 8) | (buffer[index + 28]);
						
						printf("F %ld %s %d-%d-%d %02d:%02d:%02d\n",size,name,year,month,day,hour,min,sec);
				
					}
				}	

			
				fileCount++;
			}
			
			if(flag == 0) {
				break;
			}
			index += 32;
		}
		
		start += 512;
	}
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
  
  	find_files_in_root_directory(buffer); 
  	
	return 0; 
}