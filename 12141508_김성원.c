//12141508 Kim Seong Won
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#define MAX_CMD_ARG 30
#define MAX_INT_INST 20

const char *prompt = "myshell> ";
const char *usr_signal = "USRSIGNAL\0";
char* cmdvector[MAX_CMD_ARG];
char  cmdline[BUFSIZ];
char*  my_internal_list[MAX_INT_INST] = {
	"cd" , "exit"
};


void fatal(char *str){
	perror(str);
	exit(1);
}

int change_directory(char* arg){
	if(chdir(arg) != 0)
		printf("Fail to change directory:(\n");
	else
		printf("The directory change was successful:)\n");

	return 0;
}

int my_exit(){
	printf("Exit the shell!\n");
	exit(0);
}

int my_shell_internal_instruction(char* instruction){
	int index = 0;		
	for(; my_internal_list[index] != NULL && index < MAX_INT_INST ; index++){
		//printf("%d\n" , strcmp(my_internal_list[index] , instruction));
		if(strcmp(my_internal_list[index] ,  instruction) == 0)
			return index;
	}

	return -1;  	
}

int internal_instruction(int index, char** arg){
	int result = 0;
	switch(index){
		case 0:
			change_directory(arg[1]);
			break;
		case 1:
			my_exit();
			break;

		default:
			break;
	}
	return result;
}

int makelist(char *s, const char *delimiters, char** list, int MAX_LIST){	
	int i = 0;
	int numtokens = 0;
	char *snew = NULL;

	if( (s==NULL) || (delimiters==NULL) ) return -1;

	snew = s + strspn(s, delimiters);	/* Skip delimiters */
	if( (list[numtokens]=strtok(snew, delimiters)) == NULL )
		return numtokens;

	numtokens = 1;

	while(1){
		if( (list[numtokens]=strtok(NULL, delimiters)) == NULL)
			break;
		if(numtokens == (MAX_LIST-1)) return -1;
		numtokens++;
	}
	return numtokens;
}

void catch_signal(int signo){
	strcpy(cmdline , usr_signal);
	printf("\n");
}

void catch_child(int signo){
	while(waitpid(-1 , NULL , WNOHANG) > 0) {}
	strcpy(cmdline, "\0");
}

void my_redirection(){
	int fd = -1;
	for(int j = 0 ; cmdvector[j] != NULL ; j++){
		if( strcmp(cmdvector[j] , ">" ) == 0 ){
			if(cmdvector[j + 1] == NULL){
				fatal("main()");
			}else{

				if( (fd = open(cmdvector[j + 1] , O_RDWR | O_CREAT | O_TRUNC, 0644)) == -1 ){
					fatal("main()");
				}

				dup2(fd , 1);
				close(fd);

				//delete '>' and the file name from the vector

				for(int k = j ; cmdvector[k] != NULL ; k++){
					if(cmdvector[k + 2] = NULL) break;
					cmdvector[k] = cmdvector[k + 2];
				}
			}
			j--;
		}else if( strcmp(cmdvector[j] , "<" ) == 0  ){
			if(cmdvector[j + 1] == NULL){
				fatal("main()");
			}else{
				if( (fd = open(cmdvector[j + 1] , O_RDONLY | O_CREAT, 0644)) == -1){
					fatal("main()");
				}

				dup2(fd , 0);
				close(fd);

				//delete '<' and the file name from the vector
				for(int k = j ; cmdvector[k] != NULL ; k++){
					cmdvector[k] = cmdvector[k + 2];
				}
				j--;
			}
		}
	}
}

int pipe_connect(char** cmdvector){
	char* string[MAX_CMD_ARG][MAX_CMD_ARG];
	int j = 0, k = 0, l = 0, m = 0;

	for(k = 0 ; cmdvector[k] != NULL ; k++){
	//	printf("%s\n" , cmdvector[k]);
		if( strcmp(cmdvector[k] , "|") == 0 ){
			string[j][l] = NULL;
			j++;
			l = 0;
			
		}
		else
			string[j][l++] = cmdvector[k];
	}

	if(j == 0) return 0;

	for(; strcmp( cmdvector[m] , string[0][0] ) != 0 ; m++){
//		printf("%s\n" , cmdvector[m]);
	}

	for(k = m; strcmp( cmdvector[k] , string[j][l - 1] ) != 0 ;k++ ){
//		printf("%s\n" , cmdvector[k]);
	}
	//	printf("%s\n" , cmdvector[k]);
	for(int t = k - m ; t > 0 ; t--){
		cmdvector[m] = cmdvector[k + 1];
		m++;
		k++;
	}
	cmdvector[m] = NULL;

//	printf("%s\n" , string[0][0]);
//	printf("%s\n" , string[1][0]);
//	printf("%s\n" , string[2][0]);
//
	int p[j][2];
	pid_t pid[MAX_CMD_ARG];

	for(k = 0; k < j ; k++){
	pipe(p[k]);
	}


	l = 0;

		switch( pid[l] = fork() ) {
		case -1: 
			perror ("fork call");
			exit(2);
		case 0:
			 dup2(p[0][1] , 1);

			for(k = 0 ; k < j ; k++){

			 close(p[k][0]);
			 close(p[k][1]);
			}
			 execvp(string[l][0] , string[l]);
		default:
		break;
		}	
	l++;

	for(; l < j ; l++){
		
		switch( pid[l] = fork() ) {
		case -1: 
			perror ("fork call");
			exit(2);
		case 0:
			 dup2(p[l - 1][0] , 0);
			 dup2(p[l][1] , 1);
			for(k = 0 ; k < j ; k++){

			 close(p[k][0]);
			 close(p[k][1]);
			}
			 execvp(string[l][0] , string[l]);
		default:
		break;
		}	

	}

	switch( pid[l] = fork() ) {
		case -1: 
			perror ("fork call");
			exit(2);
		case 0:
			 dup2(p[l - 1][0] , 0);
			 for(k = 0 ; k < j ; k++){

			 close(p[k][0]);
			 close(p[k][1]);
			}

			 execvp(string[l][0] , string[l]);
		default :
		break;
	}	
for(k = 0 ; k < j ; k++){

			 close(p[k][0]);
			 close(p[k][1]);
			}


	return pid[1];

}

int main(int argc, char**argv){
	int i=0;
	int temp_index = -1;
	pid_t pid;
	static struct sigaction act, ch_act, temp_act;
	bool flag = false;


	act.sa_handler = catch_signal;
	ch_act.sa_handler = catch_child;
	temp_act.sa_handler = SIG_IGN;

	sigaction(SIGTSTP , &act , NULL);
	sigaction(SIGINT , &act , NULL);
	sigaction(SIGQUIT, &act, NULL);
	sigaction(SIGCHLD , &ch_act , NULL);

	while (1) {
		if(flag == false){
			flag = false;
			fputs(prompt, stdout);
		}

		fgets(cmdline, BUFSIZ, stdin);

		if(strcmp(cmdline , usr_signal) == 0){
			strcpy(cmdline , "");
			continue;	
		}else if(cmdline[0] == '\0'){
			flag = true;
			continue;
		}



		cmdline[strlen(cmdline) -1] = '\0';
		int number_of_token = makelist(cmdline, " \t", cmdvector, MAX_CMD_ARG);


		//	for(int j = 0 ; j < number_of_token ; j++){
		//		printf("%s\n" , cmdvector[j]);
		//	}



		if( (temp_index = my_shell_internal_instruction(cmdline)) >= 0){
			internal_instruction(temp_index, cmdvector);		
		}else{
			if(strcmp(cmdvector[number_of_token - 1] , "&") == 0){
				cmdvector[number_of_token - 1] = NULL;
				switch(pid=fork()){
					case 0:
						sigaction(SIGINT , &temp_act , NULL);
						sigaction(SIGQUIT , &temp_act , NULL);
						sigaction(SIGTSTP , &temp_act , NULL);
						my_redirection();
						pipe_connect(cmdvector);
						execvp(cmdvector[0], cmdvector);
						exit(1);
					case -1:
						exit(1);
					default:
						break;
				}
			}else{
				switch(pid=fork()){
					case 0:
						my_redirection();
						pipe_connect(cmdvector);
						execvp(cmdvector[0], cmdvector);
						fatal("main()");
					case -1:
						fatal("main()");
					default:
						wait(pid);
						break;
				}
			}
		}
		flag = false;
	}

	return 0;
} 
