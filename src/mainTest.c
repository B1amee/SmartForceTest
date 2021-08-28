#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <time.h>
#include <ctype.h>

#define STRING_SIZE 1024
#define ARGUMENTS_SIZE 256
#define ARRAY_ARG_SIZE 4
#define SHIFT_SIZE 5

int fileSearch(char **arguments);
int stringReplace(char *fileName, char **arguments);
int stringSearch(char *inString, int line, char **arguments, FILE *out, char *fileName);
int logger(char *inString, char *replase, char **arguments, int line, char *fileName);
void loggerSubString(char *result, char *inString, char *subString);

int main(int argc, char *argv[]) {
    int flag = 0;  // error flag
    char *arguments[ARRAY_ARG_SIZE];
    if (argc == 4) {
        for (int i = 0; i < ARRAY_ARG_SIZE; i ++) {
            arguments[i] = argv[i];
            printf("%s", arguments[i]);
        }
        flag = fileSearch(arguments);
    } else {
        flag = 5;  // command line arguments error
    }
    if (flag == 1) {
        printf("dir not exist");
    } else if (flag == 2) {
        printf("file not exist");
    } else if (flag == 3) {
        printf("file read error");
    } else if (flag == 4) {
        printf("log file not exist");
    } else if (flag == 5) {
        printf("command line arguments error");
    }
    return 0;
}

int fileSearch(char **arguments) {
    // In this method we iterane all files in the directory
    int flag = 0;  // error flag
    char *dirName = arguments[1];
    struct dirent *dirent;
    DIR *dir = opendir(dirName);
    if (dir == NULL) {
        flag = 1;  // dir not exist
    } else {
        dirent = readdir(dir);
        while (dirent != NULL) {
            if (strcmp(dirent->d_name, ".") == 0 || strcmp(dirent->d_name, "..") == 0) {
                printf("%s\n", dirent->d_name);
                dirent = readdir(dir);
                continue;
            }
            char nextDirName[STRING_SIZE];
            snprintf(nextDirName, STRING_SIZE - 1, "%s/%s", dirName, dirent->d_name);
            DIR *nextDir = opendir(nextDirName);
            if (nextDir != NULL) {
                char *next[4];
                for (int i = 0; i < 4; i ++) {
                    next[i] = arguments[i];
                }
                next[1] = &nextDirName[0];
                fileSearch(next);
                dirent = readdir(dir);
                continue;
            }
            printf("%s\n", nextDirName);
            flag = stringReplace(nextDirName, arguments);
            if (flag != 0) {
                break;
            }
            dirent = readdir(dir);
        }
    }
    return flag;
}

int stringReplace(char *fileName, char **arguments) {
    // In this method we iterane all lines in the file
    int flag = 0;
    if (strstr(fileName, ".txt") != NULL) {
        // open TXT file
        char tempFileName[STRING_SIZE] = "temp";
        FILE *tempFile = fopen(tempFileName, "w");
        FILE *file = fopen(fileName, "r");
        if (file == NULL && tempFile == NULL) {
            flag = 2;  // file not exist
            fclose(file);
            fclose(tempFile);
        } else {
            char inString[STRING_SIZE] = "";
            char *marker = inString;  // EOF marker
            int line = 0;
            while (marker != NULL) {
                flag = stringSearch(inString, line, arguments, tempFile, fileName);
                if (flag != 0) {
                    break;
                }
                marker = fgets(inString, STRING_SIZE - 2, file);
                line++;
            }
            if (feof(file) == 0 && flag == 0) {
                flag = 3;  // file read error
            }
            fclose(file);
            fclose(tempFile);
            if (flag == 0) {
                remove(fileName);
                rename(tempFileName, fileName);
            }
        }
    }
    return flag;
}

int stringSearch(char *inString, int line, char **arguments, FILE *out, char *fileName) {
    // In this method we find target string in inString
    int flag = 0;  // error flag
    char *p;
    char temp[STRING_SIZE]= "";
    char *tempPoint = &temp[0];
    char *inPoint = inString;
    size_t targetLen = strlen(arguments[2]);
    size_t replaceLen = strlen(arguments[3]);
    p = strstr(inPoint, arguments[2]);
    while (p != NULL) {
        memcpy(tempPoint, inPoint, p - inPoint);
        tempPoint += p - inPoint;
        memcpy(tempPoint, arguments[3], replaceLen);
        tempPoint += replaceLen;
        inPoint = p + targetLen;
        p = strstr(inPoint, arguments[2]);
        if (p == NULL) {
            snprintf(tempPoint, STRING_SIZE - (tempPoint - temp) - 1, "%s", inPoint);
        }
        flag = logger(inString, temp, arguments, line, fileName);
        if (flag != 0) {
            break;
        }
    }
    if (strcmp(temp, "") == 0) {
        printf("puts\n");
        fputs(inString, out);
    } else {
        printf("putstemp\n");
        fputs(temp, out);
    }
    return flag;
}

int logger(char *inString, char *replase, char **arguments, int line, char *fileName) {
    // writing changes to .log file
    int flag = 0;  // error flag
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char str[STRING_SIZE];
    strftime(str, STRING_SIZE - 1, "logs/%Y_%m_%d_%H_%M.log", tm);
    FILE *logFile = fopen(str, "a");
    if (logFile == NULL) {
        flag = 4;  // log file not exist
    } else {
        char before[STRING_SIZE] = "", after[STRING_SIZE] = "";
        loggerSubString(before, inString, arguments[2]);
        loggerSubString(after, replase, arguments[3]);
        char logText[STRING_SIZE];
        snprintf(logText, STRING_SIZE, "%s line %d %s -> %s\n", fileName, line, before, after);
        printf("putslog\n");
        fputs(logText, logFile);
        fclose(logFile);
    }
    return flag;
}

void loggerSubString(char *result, char *inString, char *subString) {
    // get the modified string for the log text
    int simb = SHIFT_SIZE;  // indentation symbols
    char *cmp;
    cmp = strstr(inString, subString);
    if (cmp > inString + simb) {
        cmp = cmp - simb;
    }
    int d =(strlen(subString) + simb *2);
    memcpy(result, cmp, d);
    int resultLen = strlen(result);
    for (int i = 0; i < resultLen; i++) {
        if (result[i] == '\n' || result[i] == '\r') {
            result[i] = ' ';
        }
    }
}
