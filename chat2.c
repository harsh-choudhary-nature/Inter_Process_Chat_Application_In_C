//Harsh Choudhary, 2103117

#include <stdio.h>		//for printf etc
#include <stdlib.h>     //dynamic memory allocation
#include <string.h>     //strlen method is used
#include <pthread.h>    //threads are created
#include <fcntl.h>      //file operations
#include <sys/stat.h>   //for mkfifo()
#include <unistd.h>	    //for file number of input/output streams 
#include <termios.h>    //for controlling terminal attributes so that as long we type it is read as a character and no need to press enter key
#include <sys/ioctl.h>  //to get terminal size
#include<errno.h>		//for errno codes

#define FIFO_USER1_TO_USER2 "/tmp/fifo12"
#define FIFO_USER2_TO_USER1 "/tmp/fifo21"
#define BUFFER_SIZE 256

struct winsize w;		//struct to hold the window size
int fd_user1_to_user2;	//fifo for user1 to user2->read end here
int fd_user2_to_user1;  //fifo for user2 to user1->write end here
char message[BUFFER_SIZE];

void getInput(){
	int i=0;
	char c;
	while(i<BUFFER_SIZE && (c=getchar())!='\n'){
		//printf("%c",c);
		message[i++]=c;
		message[i]='\0';
	}
	//buffer[i]='\0';
}

void* sendThread(void* args) {
    while (1) {
        
        printf("You: ");
        //fgets(message, sizeof(message), stdin);
        getInput();
        write(fd_user2_to_user1, message, strlen(message) + 1);
        message[0]='\0';
    }
}

void* receiveThread(void* args) {
    char received[BUFFER_SIZE];
    char* padding=(char*)malloc((w.ws_col)*sizeof(char));
    int messageLen;
    int extraSpaces;
    int i;
    while (1) {
        
        ssize_t bytesRead = read(fd_user1_to_user2, received, sizeof(received));
        if (bytesRead > 0) {
 			messageLen=strlen(received);
 			extraSpaces=w.ws_col-messageLen-10;		
 			for(i=0;i<extraSpaces;++i){
 				padding[i]=' ';
 			}
 			padding[i]='\0';
            printf("\rFriend: %s%s\nYou: %s", received,padding,message);    
            fflush(stdout);    
        }
    }
}

int main() {
//////////////////////////Set Terminal attributes////////////////////////////////
	struct termios old, new;
    char ch;

    // Get the current terminal attributes
    tcgetattr(STDIN_FILENO, &old);
    new = old;

    // Disable canonical mode and echo
    new.c_lflag &= ~(ICANON);
    new.c_cc[VTIME] = 0; // Set read timeout to 0
    new.c_cc[VMIN] = 1;  // Minimum characters to read

    // Apply the new terminal attributes
    tcsetattr(STDIN_FILENO, TCSANOW, &new);
/////////////////////////////////////////////////////////////////////////////////
//////////Terminal width struct info////////////////////////////////////////////
 	
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

    //printf("Terminal width: %d\n", w.ws_col);
///////////////////////////////////////////////////////////////////////////////

    if(mkfifo(FIFO_USER1_TO_USER2, 0666)<0 && errno!=EEXIST){
    	printf("Error creating the fifo\n");
    	return 1;
    }		//creates the fifo if not exists
    if(mkfifo(FIFO_USER2_TO_USER1, 0666)<0 && errno!=EEXIST){
    	printf("Error creating the fifo\n");
    	return 1;
    }
    printf("Welcome to chat world Press Ctrl/Cmd + C to exit\n");
    fd_user1_to_user2 = open(FIFO_USER1_TO_USER2, O_RDONLY);
    fd_user2_to_user1 = open(FIFO_USER2_TO_USER1, O_WRONLY);

    pthread_t send_thread, receive_thread;

    pthread_create(&send_thread, NULL, sendThread, NULL);
    pthread_create(&receive_thread, NULL, receiveThread, NULL);

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(fd_user1_to_user2);
    close(fd_user2_to_user1);

    return 0;
}

