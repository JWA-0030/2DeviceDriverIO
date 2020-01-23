#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>

#define BUFFER_LENGTH 256
static char receive[BUFFER_LENGTH];

int main(){
	
	int ret, fd, fd2;
	char stringToSend[BUFFER_LENGTH];
	
	printf("Starting device test code example...\n");
	
	fd = open("/dev/HW3Read", O_RDWR);
	fd2 = open("/dev/HW3Write", O_RDWR);
	
	if(fd < 0 || fd2 < 0){
		perror("Failed to open the device...");
		return errno;	
	}
	
	printf("Opened Kernel Module Character Driver\n");
	printf("Type in a short string to send to the kernel module:\n");
	
	scanf("%[^\n]%*c", stringToSend);
	
	printf("Writing message to the device [%s].\n", stringToSend);
	
	ret = write(fd2, stringToSend, strlen(stringToSend));
	
	if(ret < 0){
		perror("Failed to write the message to the device.");
		return errno;	
	}

	printf("Press ENTER to read back from the device...\n");
	
	getchar();
	
	printf("Reading from the device...\n");
	
	ret = read(fd, receive, BUFFER_LENGTH);
	
	if(ret < 0){
		perror("Failed to read the message from the device.");
		return errno;	
	}
	
	printf("The received message is: [%s]\n", receive);
	printf("End of the program\n");
	
	close(fd);
	close(fd2);
	
	return 0;
	
}
