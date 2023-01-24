#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>

char* get_input_line();
char** get_tokens(char* line);
int exec_cmds(char** cmd);
void interactive_mode();

char error_message[30] = "An error has occurred\n";
char** path;
int path_cnt = 0;
bool ifredirect = false;
char* output_file = NULL;
bool allspace = false;

char* trim_space(char* str) {
    allspace = false;
    while(isspace(*str)) str++;
    if (*str == 0) {
        // all space
        allspace = true;
        return str;
    }
    return str;
}


int main(int argc, char **argv){
        path = malloc(sizeof(char*));
        path[0] = strdup("/bin");
        path_cnt = 1;
        if(argc == 1) {
            // interactive mode
            //printf("enter interactive mode\n");
            interactive_mode();
        } else if(argc == 2) {
            // batch mode
            // printf("before enter batch mode\n");
            batch_mode(argv);
        } else {
            fprintf(stderr, "%s", error_message);
            exit(1);
        }

        return 0;
}

char* strrstr(const char *haystack, const char *needle)
{
	char *r = NULL;

	if (!needle[0])
		return (char*)haystack + strlen(haystack);
	while (1) {
		char *p = strstr(haystack, needle);
		if (!p)
			return r;
		r = p;
		haystack = p + 1;
	}
}

void handle_if(char* if_ptr, char* line){
    //sth on the left side of if
    if(if_ptr != line){
        fprintf(stderr, "%s", error_message);
        exit(0);
    } else {
        if_ptr = if_ptr + 2;
        char* then_ptr = strstr(if_ptr, "then");
        then_ptr--;
        *then_ptr = '\0';
        then_ptr = then_ptr + 5;
        if(!isspace(*then_ptr)){
            // then_ is not then
            fprintf(stderr, "%s", error_message);
            exit(0);
        } else {
            //char* fi_ptr = strstr(then_ptr, "fi");
            // find last occurence of fi
            char* fi_ptr = strrstr(then_ptr, "fi");
            if(fi_ptr == NULL){
                // if...then without fi
                fprintf(stderr, "%s", error_message);
                exit(0);
            } else {
                fi_ptr--;
                *fi_ptr = '\0';
                fi_ptr = fi_ptr + 3;
                // sth after last fi
                fi_ptr = trim_space(fi_ptr);
                if(!allspace){
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
            }

            //if...then
            // finding operator
            char* op_1_ptr = strstr(if_ptr, "==");
            char* op_2_ptr = strstr(if_ptr, "!=");
            char* op_3_ptr = strstr(if_ptr, ">");
            char* op_4_ptr = strstr(if_ptr, "<");
            char* op_5_ptr = strstr(if_ptr, ">=");
            char* op_6_ptr = strstr(if_ptr, "<=");

            int operator = 0;
            int cnst = -1;
            char** cnst_ptr = NULL;
            char** conditional_cmd = NULL;
            char** then_cmd = NULL;
            if(op_1_ptr != NULL){
                operator = 1;
                op_1_ptr--;
                *op_1_ptr = '\0';
                op_1_ptr = op_1_ptr + 3;
                //handle constant
                op_1_ptr = trim_space(op_1_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_1_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else if(op_2_ptr != NULL){
                operator = 2;
                op_2_ptr--;
                *op_2_ptr = '\0';
                op_2_ptr = op_2_ptr + 3;
                //handle constant
                op_2_ptr = trim_space(op_2_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_2_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else if(op_3_ptr != NULL){
                operator = 3;
                op_3_ptr--;
                *op_3_ptr = '\0';
                op_3_ptr = op_3_ptr + 2;
                //handle constant
                op_3_ptr = trim_space(op_3_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_3_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else if(op_4_ptr != NULL){
                operator = 4;
                op_4_ptr--;
                *op_4_ptr = '\0';
                op_4_ptr = op_4_ptr + 2;
                //handle constant
                op_4_ptr = trim_space(op_4_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_4_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else if(op_5_ptr != NULL){
                operator = 5;
                op_5_ptr--;
                *op_5_ptr = '\0';
                op_5_ptr = op_5_ptr + 3;
                //handle constant
                op_5_ptr = trim_space(op_5_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_5_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else if(op_6_ptr != NULL){
                operator = 6;
                op_6_ptr--;
                *op_6_ptr = '\0';
                op_6_ptr = op_6_ptr + 3;
                //handle constant
                op_6_ptr = trim_space(op_6_ptr);
                if(allspace){
                    // no constant
                    fprintf(stderr, "%s", error_message);
                    exit(0);
                }
                cnst_ptr  = get_tokens(op_6_ptr);
                cnst = atoi(cnst_ptr[0]);
            } else {
                // no operator
                fprintf(stderr, "%s", error_message);
                exit(0);
            }

            //if_ptr becomes first condtional cmd
            if_ptr = trim_space(if_ptr);
            if(allspace){
                // no constant
                fprintf(stderr, "%s", error_message);
                exit(0);
            }
            conditional_cmd  = get_tokens(if_ptr);
            //exec first conditional cmd
            int rslt = exec_cmds(conditional_cmd);
            // printf("cnst: %d\n", cnst);
            // printf("reslt: %d\n", rslt);
            bool if_then_cmd = false;
            // printf("operator: %d\n", operator);
            switch(operator) {
                case 1 :
                    if(rslt == cnst){
                        if_then_cmd = true;
                    }
                    break;
                case 2 :
                    if(rslt != cnst){
                        if_then_cmd = true;
                    }
                    break;
                case 3 :
                    if(rslt > cnst){
                        if_then_cmd = true;
                    }
                    break;
                case 4 :
                    if(rslt < cnst){
                        if_then_cmd = true;
                    }
                    break;
                case 5 :
                    if(rslt >= cnst){
                        if_then_cmd = true;
                    }
                    break;
                case 6 :
                    if(rslt <= cnst){
                        if_then_cmd = true;
                    }
                    break;
                default :
                    fprintf(stderr, "%s", error_message);
                    exit(0);
            }

            // handle cmd after then
            if(if_then_cmd){
                //printf("enter if_then_cmd!!\n");
                then_ptr = trim_space(then_ptr);
                if(allspace){
                    //all space
                    exit(0);
                }
                //if there's if after then
                char* if_after_then_ptr = NULL;
                if_after_then_ptr = strstr(then_ptr, "if");
                if(if_after_then_ptr != NULL){
                    // printf("enter recursive then!!\n");
                    // printf("if_after_then_ptr: %s\n", if_after_then_ptr);
                    // printf("then_ptr: %s\n", then_ptr);
                    handle_if(if_after_then_ptr, then_ptr);
                } else {
                    then_cmd  = get_tokens(then_ptr);
                    exec_cmds(then_cmd);
                }
            }
            // char** first_tokens;
            // first_tokens = get_tokens(if_ptr);
        }
    }
}

void batch_mode(char** argv){
    // printf("enter batch mode\n");

    FILE* fp = NULL;
    fp = fopen(argv[1], "r");
    // printf("%s\n", argv[1]);
    if(fp == NULL){
        fprintf(stderr, "%s", error_message);
        exit(1);
    }

    char* line = NULL;
    char** cmds;
    size_t bufferSize = 0;
   /*  if (getline(&line, &bufferSize, fp) == EOF) {
        free(line);
        exit(0);
    }*/
    
    while (1) {
        if (getline(&line, &bufferSize, fp) == EOF) {
            free(line);
            exit(0);
        }

        if( line[strlen(line) - 1] == '\n'){
            line[strlen(line) - 1] = '\0';
        }
        //printf("----%s\n", line);

        ifredirect = false;
        output_file = NULL;
        line = trim_space(line);
        if(allspace){
            continue;
        }

        //handle if...then...fi
        char* if_ptr = strstr(line, "if");
        if(if_ptr != NULL){
            handle_if(if_ptr, line);
        } else {
            cmds = get_tokens(line);
            if(cmds != NULL){
                exec_cmds(cmds); 
            }
        }
        free(cmds);
    }
    free(line);

}

void interactive_mode(){
    char* line;
    char** cmds;
    while(1){
        printf("wish> ");
        line = get_input_line();
        //printf("after get_input_line\n");
        //printf("%d\n", strlen(line));
        //printf("%s\n", line);

        //printf("\n");
        cmds  = get_tokens(line);
        //printf("after get_tokens\n");
        exec_cmds(cmds);
        free(line);
        free(cmds);
    }
}

char* get_input_line(){
    char* line = NULL;
    size_t bufferSize = 0;
    if (getline(&line, &bufferSize, stdin) == EOF) {
        free(line);
        exit(0);
    }
    //printf("get_input_line\n");
    line[strlen(line) - 1] = '\0';
    return line;
}

char** get_tokens(char* line){
    // redirect mode
    char* out_ptr = strstr(line, ">");
    if (out_ptr != NULL) {
        *out_ptr = '\0';
        out_ptr++;
    }

    // printf("%s\n",line);
    // printf("enter get_tokens\n");
    char** tokens = (char **)malloc(sizeof(char*) * 32);
    char* token;
    // printf("enter get_tokens_1\n");
    int pos = 0;
    int bs = 32;
    // printf("enter get_tokens_2\n");
    
    token = strtok(line, " ");
    if(token == NULL){
        if(out_ptr != NULL){
           // redirect without cmd
           fprintf(stderr, "%s", error_message);
           exit(0);
        }
    } else {
        tokens[pos++] = token;
    }

	while ((token = strtok(NULL, " ")) != NULL)
	{
        tokens[pos++] = token;
		if (pos >= bs)
		{
			bs += 32;
			tokens = realloc(tokens, bs * sizeof(char *));
		}
	}
    
	tokens[pos] = NULL;

    if(out_ptr != NULL){
        char* output_token = strtok(out_ptr, " ");
        if(output_token == NULL){
            // no file
            fprintf(stderr, "%s", error_message);
            exit(0);
        } else {
            // if there are more than one file output return error
            if (strtok(NULL, " ") != NULL){
                fprintf(stderr, "%s", error_message);
                exit(0);
            }

            output_file = strdup(output_token);
            ifredirect = true;
        }
    } else {
        ifredirect = false;
    }

    return tokens;
}

int exec_cmds(char** cmd){
    // printf("enter exec_cmds\n");

    if(strcmp(cmd[0], "exit") == 0) {
        if(cmd[1] != NULL){
            fprintf(stderr, "%s", error_message);
        }
        exit(0);
    } else if(strcmp(cmd[0], "cd") == 0) {
        exec_cd(cmd);
    } else if(strcmp(cmd[0], "path") == 0) {
        if(cmd[1] == NULL){
            for(int i = 0; i < path_cnt; i++){
                free(path[i]);
            }
            path_cnt = 0;
        } else {
            path_cnt = 0;
            for(int i  = 1; cmd[i]; i++){
                path = realloc(path, sizeof(char*) * (path_cnt + 1));
                path[path_cnt] = NULL;
                //printf("%s\n", cmd[i]);
                path[path_cnt++] = strdup(cmd[i]);
            }
            /*
            printf("path_cnt: %d\n",path_cnt);
            for(int i  = 1; i < path_cnt; i++){
                printf("path: %s\n", cmd[i]);
            }
            */
        }
    } else {
        pid_t pid;
        int status;
        pid = fork();
        if(pid == 0){
            //if redirect is true then change output from stdout to file
            if(ifredirect == true && output_file != NULL){
               // printf("**%s\n", output_file);
                const int fd = open(output_file, O_CREAT | O_TRUNC | O_WRONLY | FD_CLOEXEC, 0666);
                dup2(fd, STDOUT_FILENO);
                close(fd);
               // free(output_file);
            }
            
            // child process created successfully
            char* cmd_arg = NULL;
            for(int i = 0; i < path_cnt; i++){
                //printf("path[i]: %s\n", path[i]);
                int cmd_size = strlen(path[i]) + strlen(cmd[0]) + 2;
                cmd_arg = realloc(cmd_arg, cmd_size);
                memset(cmd_arg, 0, cmd_size);
                strcat(cmd_arg, path[i]);
                //int path_size = strlen(path[i]);
                //if((path[i][path_size-2]) != '/'){
                    strcat(cmd_arg, "/");
                //}
                strcat(cmd_arg, cmd[0]);
                //printf("---%s\n", cmd[0]);
                //printf("---%s\n", cmd_arg);
                execv(cmd_arg, cmd);
                // if(access(cmd_arg, F_OK|X_OK) == 0){
                //     execv(cmd_arg, cmd);
                // }
            }
            fprintf(stderr, "%s", error_message);
            exit(1);
        } else if(pid < 0){
            // child process created unsuccessfully
            fprintf(stderr, "%s", error_message);
            exit(1);
        } else {
            // parent process wait for child process to complete
            if(waitpid(pid, &status, 0) > 0){
                if (WIFEXITED(status) && WEXITSTATUS(status)) {
                    // return the value of the result of execv from program
                    return WEXITSTATUS(status);
                }
            }
        }
    }
      
    return 0;
}

int exec_cd(char** args){
    // printf("enter exec_cd\n");
    if(args[1] == NULL){
        fprintf(stderr, "%s", error_message);
        return 0;
    }

    if(chdir(args[1]) != 0){
        fprintf(stderr, "%s", error_message);
        return 0;
    }
    return 1;
}