#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <linux/limits.h>

char *parsedCommand[1000];
char command[1000];
int flag=0;int file;


void parseCommand(){
    if( command[strlen(command)-1] == '&' ){//Run in the background
        command[strlen(command)-1]='\0';
        flag=1;
    }

        parsedCommand[0] = strtok(command, " ");
        if(strcmp(parsedCommand[0],"export")==0){
            //Case export:
            int i = 0;
            while (parsedCommand[i] != NULL) {
                i++;
                parsedCommand[i] = strtok(NULL, "=");

            }
            //printf(parsedCommand[2]);
        }else {
            int i = 0;
            while (parsedCommand[i] != NULL) {
                i++;
                parsedCommand[i] = strtok(NULL, " ");

            }
        }


}



void printDir(){
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL){
        fprintf(stdout, "%s\n", cwd);
    }
}


void execute_command() {
    int pid = fork();

    if( pid == -1 ){

        printf("Forking child failed!!\n");
        exit(0);

    } else if(pid==0) {

        int x;
        char *eval;
        if(parsedCommand[1]!=NULL) {

            eval = parsedCommand[1];
        }else {
            eval = parsedCommand[0];
        }
            char *ch[1000];

            //printf("d5lt");
            if (eval[0] == '$') {//evaluation!!
                eval = eval + 1;
                char *v = getenv(eval);
              //  printf(v);
                if(parsedCommand[1]!=NULL) {
                    ch[0] = parsedCommand[0];
                    ch[1] = strtok(v, " ");

                    int i = 1;
                    while (ch[i] != NULL) {
                        i++;
                        ch[i] = strtok(NULL, " ");

                    }
                    x = execvp(parsedCommand[0], ch);
                }else{
                    ch[0]=v;
                    x = execvp(ch[0], ch);
                }
                //parsedCommand[1] = v;
                //printf(parsedCommand[1]);
                //printf("yarab");
                //printf(ch[0]);
                //x = execvp(parsedCommand[0], ch);

            }else {
                //no evaluation!!!
                x = execvp(parsedCommand[0], parsedCommand);
            }

        if(x!=0){//Error Happened!!
            printf("Unknown Command!!\n");
            exit(x);
        }

        exit(0);
    }else{

        if(flag==0){
            //wait there was no "&" entered i.e. not working in the background.
            waitpid(pid, NULL, 0);
        }else{
            //Don't wait ya parentttt, it's running the background.
            flag=0;
        }
    }
}



void execute_shell_builtin(){

        if (strcmp(parsedCommand[0],"cd")==0) {

            if(parsedCommand[1]!=NULL && strcmp(parsedCommand[1],"~")!=0 ) {//just do the given command and don't print teh current location.
                //printf(parsedCommand[1]);
                int x = chdir(parsedCommand[1]);

                if (x != 0) {//Error Happened!!
                    printf("The Directory is not found!!\n");
                }else{
                    printDir();
                }

            }else if(parsedCommand[1]==NULL || strcmp(parsedCommand[1],"~")==0 ){
                //print the current directory if "cd" with no args is entered.
                chdir("HOME");
                printDir();
            }


        } else if (strcmp(parsedCommand[0],"export")==0) {

            char *t;
            t=parsedCommand[2];//the value after "=".
            if(t[0]=='\"'){

                t[strlen(parsedCommand[2])-1]='\0';//remove the quotation in end.
                // printf(t);
                t=t+1;//remove the quotation in the beginning.
                //  printf(t);
                setenv(parsedCommand[1],t,1);//set the value to the given variable in the environment.
                // printf(getenv("x"));
            }else{
                setenv(parsedCommand[1],parsedCommand[2],1);//if it was just a value without double quotations.
            }

        } else if (strcmp(parsedCommand[0], "echo")==0) {

            char *ech;
            char *ec;
            ech=parsedCommand[1];
            //remove the double quotations.
            if(parsedCommand[2]!=NULL) {//If there was a space separating 2 entered strings.
                ec = parsedCommand[2];
                ec[strlen(parsedCommand[2])-1]='\0';
                ech=ech+1;
                if(ech[0]=='$'||ec[0]=='$'){
                   if(ech[0]=='$'){
                       ech=ech+1;
                       printf("%s ", getenv(ech));
                   } else{
                       printf("%s ",ech);
                   }
                   if(ec[0]=='$'){
                       ec=ec+1;
                       printf("%s\n",getenv(ec));
                   }else{
                       printf("%s\n",ec);
                   }
                }else{
                    printf("%s ",ech);
                    printf("%s\n",ec);
                }
            }else{
                ech[strlen(parsedCommand[1])-1]='\0';
                ech=ech+1;

                if(ech[0]=='$') {//Case substituting an already entered variable.
                    //remove it, then get it's corresponding value.
                    ech=ech+1;
                    printf("%s\n", getenv(ech));

                }else {
                    char *r[1000];
                    r[0]=strtok(ech,"$");
                    int i = 0;
                    while (r[i] != NULL) {
                        i++;
                        r[i] = strtok(NULL, "=");

                    }
                    printf("%s",ech);
                    if(r[1]!=NULL) {
                        printf("%s\n", getenv(r[1]));
                    }else{
                        printf("\n");
                    }
                }
            }

        }
}


void shell(){

    while(1) {
        //Taking the input

        memset(parsedCommand, 0, sizeof parsedCommand);
        printf("Your Command >> ");
        fgets(command, 1000, stdin);

        //If the user entered "enter":
        if(command[0]=='\n'){
            continue;

        }

        //Removing the "\n" entered at teh end of the input.
        command[strlen(command)-1]='\0';
        printf("\n");

        //Parsing the input.
        parseCommand();
        // printf(parsedCommand[1]);

        //Execution of the entered command:
        if(strcmp(parsedCommand[0],"cd")==0|| strcmp(parsedCommand[0],"export")==0|| strcmp(parsedCommand[0],"echo")==0){//Built in command.
            execute_shell_builtin();

        }else if(strcmp(parsedCommand[0],"exit")==0){//Exit command to terminate the shell.
            exit(0);

        }else{
           // printf("3ashhh mayoyyy");
            execute_command();
        }
    }
}


void Handler(){

    int state;
    int pid;
    pid = wait3 (&state, WNOHANG, (struct rusage *)NULL );

    //Write in the logfile.
    if(file != 1){
        write(file,"Child terminated!!\n",strlen("Child terminated!!\n"));
    }

    //Avoid zombie process.
    if (pid == 0 || pid == -1) {
        return;
    }
}


void set_environment(){//setting the environment to the current directory.
    char cwd[PATH_MAX];
    chdir(getcwd(cwd, sizeof(cwd)));
}


int main() {

    signal (SIGCHLD, Handler);
    set_environment();

    //creating a logfile if it wasn't created before.
    file = open("logfile.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

    shell();

    return 0;
}
