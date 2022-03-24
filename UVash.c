/*  Enrique Rojo Alvarez
 *  Estructuras de sistemas operativos
 *  UVaShell Practica 2
 *  Build of a simple Unix shell
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAXARGS 128

void CreaProceso(char *MiArgv[],int Argc);
int ExitSh(int Argc);
int Cd(char *MiArgv[],int Argc);
int IniciarGrabacion(int Argc);
int FinalizaGrabacion(int Argc);
void Muestra(int cont, char *MiGrabacion[]);
void Recupera(char *MiArgv[],int Argc);

int main (int argc, char *argv[]){

    FILE *fd;
    char *Linea;
    char *Palabra;
    size_t n;
    ssize_t ret;
    char *Delimita=" ";
    char *Delimita2 = ">";
    char **MiArgv;
    bool built_in;
    bool from_file;
    bool out_file;
    bool error;
    bool grabacion = false;
    char **MiGrabacion;
    int file_open;
    int save_out;


    if (argc > 2){      // Invalid arguments
        fprintf(stderr, "An error has occurred\n");
        exit(0);
    }

    if (argc == 2){       // Bach mode

        fd = fopen(argv[1], "r");
        if(fd == NULL)
            exit(0);
        from_file=true;

    }                   // Interactive mode
    else{

        fd=stdin;
        from_file=false;

    }


    MiArgv=(char**) malloc(MAXARGS*sizeof(char*));
    MiGrabacion=(char**) malloc(MAXARGS*sizeof(char*));

    Linea=NULL;
    n=0;
    int cont = 0;   // Counter for command record
    int i;


    if (!from_file)     // On bach mode it won't print UVaSh>
        printf("UVaSh> ");

    while ((ret=getline(&Linea, &n, fd)) != EOF){

        Linea[ret-1]='\0';
        built_in = false;       // Built_in commands detector
        out_file = false;       // Output on file detector
        error = false;          // Error-handling
        Palabra=strtok(Linea, Delimita);    // Takes the first word


        if (grabacion){
            MiGrabacion[cont] = Linea;
            cont++;
        }

        if (Palabra == NULL) {
            error = true;
            fprintf(stderr, "An error has occurred\n");

        }

        i=0;
        MiArgv[i]=Palabra;

        while (Palabra != NULL){

            MiArgv[i]=Palabra;

            if (strcmp(Palabra, Delimita2) == 0)    // "Word" == ">"
                out_file = true;

            i++;
            Palabra=strtok(NULL, Delimita);     // Takes nest word
        }
        MiArgv[i]=NULL;     // Last word like NULL

        if (out_file){      //Output on file
            if (strcmp(MiArgv[i-2], Delimita2) == 0){   // Check syntax: ">"+"File-name"+"NULL"
                save_out = dup(1);  // Copy of stdout
                char *out_file_name = MiArgv[i-1];
                file_open = open(out_file_name, O_CREAT|O_RDWR|O_TRUNC, 0600);  // Open file
                dup2(file_open, 1);     // Output on file
                MiArgv[i-2] = NULL;     //Clear the command after ">"
                MiArgv[i-1] = NULL;

            }
            else{   // Siyntax Error

                error = true;
                fprintf(stderr, "An error has occurred\n");

            }

        }

        if (!error){
            if (strcmp(MiArgv[0], "exit") == 0){   // Recieves an "exit" command

                built_in=true;
                ExitSh(i);

            }

            if (strcmp(MiArgv[0], "cd") == 0 ){   //Receives a cd command

                built_in=true;
                Cd(MiArgv, i);

            }

            if (strcmp(MiArgv[0], "IniciarGrabacion") == 0){
                built_in = true;
                if (IniciarGrabacion(i)==0)
                    grabacion = true;
            }

            if (strcmp(MiArgv[0], "FinalizaGrabacion") == 0){
                built_in = true;
                if (FinalizaGrabacion(i)==0){
                    grabacion = false;
                    cont = 0;
                   //TODO vaciar vector MiGrabacion
                }
            }

            if (strcmp(MiArgv[0], "Recupera") == 0){
                built_in = true;
                Recupera(MiArgv, i);
            }

            if (strcmp(MiArgv[0], "Muestra") == 0){
                built_in = true;
                Muestra(cont, MiGrabacion);
            }

            if (!built_in)    // Not built_in and without errors
                CreaProceso(MiArgv, i);

            if (out_file){
                fflush(stdout);     // Clear stdout
                close(file_open);
                dup2(save_out, 1);  // recovery of initial stdout
                close(save_out);

            }

        }
        
        free(Linea);
        Linea=NULL;
        n=0;

        if (!from_file)         // On bach mode it won't print UVaSh>
            printf("UVaSh> ");

    }

        exit(0);
}

// Creates a new process to execute the command

void CreaProceso(char *MiArgv[],int Argc){
    pid_t PidHijo;
    PidHijo=fork();

    if (PidHijo==0){ //Hijo
        execvp(MiArgv[0], MiArgv);
        exit(1); //Redundante

    }

    wait(NULL); //Padre

}

// Built-in for exit command
int ExitSh(int Argc){

    if (Argc == 1)      // Recieved an "exit" call without arguments
        exit(0);
    else{               // exit call with arguments

        fprintf(stderr, "An error has occurred\n");
        return 1;

    }

}

// Built-in for Cd Command
int Cd(char *MiArgv[],int Argc){

        if (Argc == 2){    // Arguments valid for cd (only can be 1)

            if(chdir(MiArgv[1]) != 0) {  // Tests if it's a non-valid PATH

                fprintf(stderr, "An error has occurred\n");
                return 1;   // Non-valid

            }
            else
                return 0;   // Valid
        }
        else {   // Arguments not valid for cd (0 or >1)

            fprintf(stderr, "An error has occurred\n");
            return 1;

        }

}

// Built-in for command record
int IniciarGrabacion(int Argc){

    if (Argc == 1)
        return 0;
    else
        return 1;   // Command with argumets
}

int FinalizaGrabacion(int Argc){

    if (Argc == 1)
        return 0;
    else
        return 1;   // Command with argumets
}

void Muestra(int cont, char *MiGrabacion[]){

    int c;

    for(c = 0; c < cont; c++){

        printf("%i: %s \n",(c+1), MiGrabacion[c]);
    }
}

void Recupera(char *MiArgv[],int Argc){
}

