BY PAVITAR SIDHU

There are 4 C program files in here: diskget.c, diskinfo.c, disklist.c, and diskput.c

All these programs require a disk.IMA to passed in through the command arguments.

-diskget will allow you to retrieve a certain file from the Disk Image
-diskput will allow you to insert a file into the disk image
-disklist will list all of the files in the Disk Image
-diskinfo will display general information regarding the Disk Image

This program will only work with FAT12 disk images. As such, a “TEST.IMA” file has 
been included for you to use when running the program.

To run the program, please run the makefile. And then you can use any of the following commands to run whichever program you desire: 

“./diskinfo TEST.IMA”
“./disklist TEST.IMA”
“./diskget TEST.IMA ANS1.PDF” replace ANS1.PDF with name of file you want.
“./diskput TEST.IMA myfile.txt”