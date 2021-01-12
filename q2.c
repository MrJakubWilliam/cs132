#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define EXTFILEAPPEND "_ext";
#define LOGFILEAPPEND "_log";

typedef struct
{
    char content[1000];
    size_t next;
} TextLine;

typedef struct
{
    char operation[100];
    size_t line_no;
    size_t totalNoLines;
    unsigned short current;
} Log;

size_t getNoOfLines(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    char c = fgetc(file);
    size_t lineNo = 0;

    while (c != EOF)
    {
        if (c == '\n')
        {
            lineNo++;
        }
        c = fgetc(file);
    }
    fclose(file);

    return lineNo;
}

short unsigned fileExists(char *fileName)
{
    FILE *file;

    if ((file = fopen(fileName, "r")))
    {
        fclose(file);
        // printf("ERROR | File %s already exists!\n", fileName);
        // exit(1);
        return 1;
    }

    return 0;
}

char *getExtName(char *fileName)
{
    char extFileAppend[] = EXTFILEAPPEND;
    size_t fileNameCount = 0;

    while (fileName[fileNameCount])
        fileNameCount++;

    char *fileExt = calloc(fileNameCount + sizeof(extFileAppend), sizeof(char));

    fileExt[0] = '.';
    fileExt = strcat(fileExt, fileName);
    fileExt = strcat(fileExt, extFileAppend);

    return fileExt;
}

char *getLogName(char *fileName)
{
    char logFileAppend[] = LOGFILEAPPEND;
    size_t fileNameCount = 0;

    while (fileName[fileNameCount])
        fileNameCount++;

    char *fileLog = calloc(fileNameCount + sizeof(logFileAppend), sizeof(char));

    fileLog[0] = '.';
    fileLog = strcat(fileLog, fileName);
    fileLog = strcat(fileLog, logFileAppend);

    return fileLog;
}

Log *getLogs(char *fileName)
{
    char *logFile = getLogName(fileName);
    Log *logs = calloc(getNoOfLines(logFile), sizeof(Log));

    char *operation = calloc(sizeof(logs[0].operation), sizeof(char));
    char *line_no = calloc(sizeof(logs[0].line_no), sizeof(char));
    char *totalNoLines = calloc(sizeof(logs[0].totalNoLines), sizeof(char));
    char *current = calloc(sizeof(logs[0].current), sizeof(char));

    unsigned short columnFlag = 0;
    size_t logCounter = 0;
    size_t charCounter = 0;
    size_t noOfColumns = 4;

    FILE *file;
    file = fopen(logFile, "r");
    char c = fgetc(file);

    while (c != EOF)
    {
        if (c == '|')
        {
            columnFlag = (columnFlag + 1) % 4;
            charCounter = 0;
        }
        else if (c == '\n')
        {
            strcpy(logs[logCounter].operation, operation);
            logs[logCounter].line_no = atoi(line_no);
            logs[logCounter].totalNoLines = atoi(totalNoLines);
            logs[logCounter].current = atoi(current);
            logCounter++;
        }
        else if (columnFlag == 0)
        {
            operation[charCounter] = c;
            operation[charCounter + 1] = '\0';
            charCounter++;
        }
        else if (columnFlag == 1)
        {
            line_no[charCounter] = c;
            line_no[charCounter + 1] = '\0';
            charCounter++;
        }
        else if (columnFlag == 2)
        {
            totalNoLines[charCounter] = c;
            totalNoLines[charCounter + 1] = '\0';
            charCounter++;
        }
        else if (columnFlag == 3)
        {
            current[charCounter] = c;
            current[charCounter + 1] = '\0';
            charCounter++;
        }

        c = fgetc(file);
    }

    fclose(file);

    free(operation);
    free(line_no);
    free(totalNoLines);
    free(current);
    free(logFile);

    return logs;
}

void saveToLog(char *fileName, Log log)
{
    char *fileLog = getLogName(fileName);
    size_t logLineNoLength = snprintf(NULL, 0, "%lu", log.line_no);
    size_t logTotalNoLinesLength = snprintf(NULL, 0, "%lu", log.totalNoLines);
    size_t logCurrentLength = snprintf(NULL, 0, "%u", log.current);
    size_t noOfColumns = 4;

    char *logEntry = calloc(strlen(log.operation) + logLineNoLength + logTotalNoLinesLength + logCurrentLength + noOfColumns + 1, sizeof(char));
    char *logLineNoString = malloc(logLineNoLength + 1);
    char *logTotalNoLinesString = malloc(logTotalNoLinesLength + 1);
    char *logCurrentString = malloc(logCurrentLength + 1);

    snprintf(logLineNoString, logLineNoLength + 1, "%lu", log.line_no);
    snprintf(logTotalNoLinesString, logTotalNoLinesLength + 1, "%lu", log.totalNoLines);
    snprintf(logCurrentString, logCurrentLength + 1, "%u", log.current);

    logEntry = strcat(logEntry, log.operation);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logLineNoString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logTotalNoLinesString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logCurrentString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, "\n");

    FILE *file;
    file = fopen(fileLog, "a");
    fputs(logEntry, file);

    fclose(file);

    free(fileLog);
    free(logEntry);
    free(logLineNoString);
    free(logTotalNoLinesString);
    free(logCurrentString);
}

void *generateLogFile(char *fileName, Log *logs)
{
    char *logFile = getLogName(fileName);
    size_t logCounter = 0;

    remove(logFile);

    while (logs[logCounter].operation[0] != 0)
    {
        saveToLog(fileName, logs[logCounter]);
        logCounter++;
    }

    free(logFile);
}

//Removes the logs after the CURRENT
void flushLogs(char *fileName)
{
    if (fileExists(getLogName(fileName)) == 1)
    {

        Log *logs = getLogs(fileName);
        size_t newLogsNo = 0;

        while (logs[newLogsNo].current == 0)
            newLogsNo++;

        Log *newLogs = calloc(newLogsNo + 1, sizeof(Log));
        size_t logCounter = 0;

        while (logs[logCounter].current == 0)
        {
            newLogs[logCounter] = logs[logCounter];
            logCounter++;
        }

        newLogs[logCounter] = logs[logCounter];

        generateLogFile(fileName, newLogs);

        free(logs);
        free(newLogs);
    }
}

// void sendToLog(char *fileName, Log log)
// {
//     char *fileLog = getLogName(fileName);
//     size_t logLineNoLength = snprintf(NULL, 0, "%lu", log.line_no);
//     size_t logTotalNoLinesLength = snprintf(NULL, 0, "%lu", log.totalNoLines);
//     size_t logCurrentLength = snprintf(NULL, 0, "%u", log.current);
//     size_t noOfColumns = 4;

//     char *logEntry = calloc(strlen(log.operation) + logLineNoLength + logTotalNoLinesLength + logCurrentLength + noOfColumns + 1, sizeof(char));
//     char *logLineNoString = malloc(logLineNoLength + 1);
//     char *logTotalNoLinesString = malloc(logTotalNoLinesLength + 1);
//     char *logCurrentString = malloc(logCurrentLength + 1);

//     snprintf(logLineNoString, logLineNoLength + 1, "%lu", log.line_no);
//     snprintf(logTotalNoLinesString, logTotalNoLinesLength + 1, "%lu", log.totalNoLines);
//     snprintf(logCurrentString, logCurrentLength + 1, "%u", log.current);

//     logEntry = strcat(logEntry, log.operation);
//     logEntry = strcat(logEntry, "|");
//     logEntry = strcat(logEntry, logLineNoString);
//     logEntry = strcat(logEntry, "|");
//     logEntry = strcat(logEntry, logTotalNoLinesString);
//     logEntry = strcat(logEntry, "|");
//     logEntry = strcat(logEntry, logCurrentString);
//     logEntry = strcat(logEntry, "|");
//     logEntry = strcat(logEntry, "\n");

//     flushLogs(fileName);

//     FILE *file;
//     file = fopen(fileLog, "a");
//     fputs(logEntry, file);

//     fclose(file);

//     free(fileLog);
//     free(logEntry);
//     free(logLineNoString);
//     free(logTotalNoLinesString);
//     free(logCurrentString);
// }

void sendToExt(char *fileName, TextLine line)
{
    char *fileExt = getExtName(fileName);
    size_t lineNextLength = snprintf(NULL, 0, "%lu", line.next);
    size_t noOfColumns = 2;

    char *message = calloc(strlen(line.content) + lineNextLength + noOfColumns + 1, sizeof(char));
    char *lineNextString = malloc(lineNextLength + 1);

    snprintf(lineNextString, lineNextLength + 1, "%lu", line.next);

    message = strcat(message, line.content);
    message = strcat(message, "|");
    message = strcat(message, lineNextString);
    message = strcat(message, "|");
    message = strcat(message, "\n");

    FILE *file;
    file = fopen(fileExt, "a");
    fputs(message, file);

    fclose(file);

    free(fileExt);
    free(message);
    free(lineNextString);
}

TextLine *getExtTextLines(char *fileName)
{
    char *extFile = getExtName(fileName);
    TextLine *lines = calloc(getNoOfLines(extFile), sizeof(TextLine));

    char *content = calloc(sizeof(lines[0].content), sizeof(char));
    char *next = calloc(sizeof(lines[0].next), sizeof(char));

    unsigned short columnFlag = 0;
    size_t lineCounter = 0;
    size_t charCounter = 0;

    FILE *file;
    file = fopen(extFile, "r");
    char c = fgetc(file);

    while (c != EOF)
    {
        if (c == '|')
        {
            columnFlag = 1 - columnFlag;
            charCounter = 0;
        }
        else if (c == '\n')
        {
            strcpy(lines[lineCounter].content, content);
            lines[lineCounter].next = atoi(next);
            lineCounter++;
        }
        else if (columnFlag == 0)
        {
            content[charCounter] = c;
            content[charCounter + 1] = '\0';
            charCounter++;
        }
        else if (columnFlag == 1)
        {
            next[charCounter] = c;
            next[charCounter + 1] = '\0';
            charCounter++;
        }

        c = fgetc(file);
    }

    fclose(file);

    free(content);
    free(next);
    free(extFile);

    return lines;
}

int getTextLineIndex(TextLine *lines, size_t line_no)
{
    TextLine current = lines[0];
    size_t traverseCounter = 0;
    size_t nextIndex = 0;

    while (traverseCounter < line_no && current.next != 0)
    {
        nextIndex = current.next;
        current = lines[current.next];
        traverseCounter++;
    }

    if (traverseCounter == line_no)
    {
        return nextIndex;
        // return
    }

    return -1;
}

size_t getCurrentLengthOfDocument(char *fileName)
{
    TextLine *lines = getExtTextLines(fileName);
    TextLine current = lines[0];
    size_t traverseCounter = 0;
    size_t nextIndex = 0;

    while (current.next != 0)
    {
        nextIndex = current.next;
        current = lines[current.next];
        traverseCounter++;
    }

    free(lines);

    return traverseCounter;
}

void *generateExtFile(char *fileName, TextLine *lines)
{
    char *extFile = getExtName(fileName);
    size_t lineCounter = 0;

    remove(extFile);

    while (lines[lineCounter].content[0] != 0)
    {
        sendToExt(fileName, lines[lineCounter]);
        lineCounter++;
    }

    free(extFile);
}

void *appendToLog(char *fileName, Log log)
{
    char *fileLog = getLogName(fileName);

    flushLogs(fileName);

    if (fileExists(fileLog) == 0)
    {

        FILE *file;
        file = fopen(fileLog, "w");
        fclose(file);
        free(fileLog);
    }

    Log *logs = getLogs(fileName);
    size_t logsCounter = 0;

    while (logs[logsCounter].operation[0] != 0)
        logsCounter++;

    Log *logsCopy = calloc(logsCounter + 1, sizeof(Log));

    for (size_t i = 0; i < logsCounter; i++)
    {
        strcpy(logsCopy[i].operation, logs[i].operation);
        logsCopy[i].line_no = logs[i].line_no;
        logsCopy[i].totalNoLines = logs[i].totalNoLines;
        logsCopy[i].current = 0;
    }

    strcpy(logsCopy[logsCounter].operation, log.operation);
    logsCopy[logsCounter].line_no = log.line_no;
    logsCopy[logsCounter].totalNoLines = log.totalNoLines;
    logsCopy[logsCounter].current = log.current;

    generateLogFile(fileName, logsCopy);

    free(logs);
    free(logsCopy);
}

char *getLine(char *fileName, size_t line_no)
{
    char *output = calloc(1000, sizeof(char));
    size_t noOfLines = getNoOfLines(fileName);
    int lineCounter = 1;

    FILE *file;
    file = fopen(fileName, "r");
    char c = fgetc(file);

    while (lineCounter < line_no)
    {
        if (c == '\n')
            lineCounter++;

        c = fgetc(file);
    }

    while (c != '\n')
    {
        output = strncat(output, &c, 1);
        c = fgetc(file);
    }

    return output;
}

/* Line-level Controlers */

void lineShowController(char *fileName, int line_no)
{
    size_t noOfLines = getNoOfLines(fileName);

    if (line_no > noOfLines)
    {
        printf("ERROR | Line number outside of the allowed range!\n");
        exit(1);
    }

    char *line = getLine(fileName, line_no);
    printf("%s\n", line);

    free(line);
}

void lineInsertController(char *fileName, size_t line_no, char *content)
{
    if (fileExists(getExtName(fileName)) == 0)
    {
        printf("The ext file for file %s does not exist. Run $ ./q2 b %s to generete it.\n", fileName, fileName);
        exit(1);
    }

    if (line_no > getCurrentLengthOfDocument(fileName) + 1 || line_no < 1)
    {
        printf("ERROR | Line number outside of the allowed range!\n");
        exit(2);
    }

    if (strlen(content) > 999)
    {
        printf("ERROR | Too long line input!\n");
        exit(3);
    }

    TextLine *lines = getExtTextLines(fileName);
    size_t linesCounter = 0;
    TextLine preLine;

    while (lines[linesCounter].content[0] != 0)
        linesCounter++;

    int preLineIndex = getTextLineIndex(lines, line_no - 1);

    TextLine *linesCopy = calloc(linesCounter + 1, sizeof(TextLine));

    for (size_t i = 0; i < linesCounter; i++)
    {
        strcpy(linesCopy[i].content, lines[i].content);
        linesCopy[i].next = lines[i].next;
    }

    strcpy(linesCopy[linesCounter].content, content);

    preLine = lines[preLineIndex];
    linesCopy[linesCounter].next = preLine.next;
    linesCopy[preLineIndex].next = linesCounter;

    generateExtFile(fileName, linesCopy);

    Log log = {"LINE_INSERTED", linesCounter, getCurrentLengthOfDocument(fileName), 1};
    appendToLog(fileName, log);

    free(lines);
    free(linesCopy);
}

void lineDeleteController(char *fileName, int line_no)
{
    if (fileExists(getExtName(fileName)) == 0)
    {
        printf("The ext file for file %s does not exist. Run $ ./q2 b %s to generete it.\n", fileName, fileName);
        exit(1);
    }

    if (line_no > getCurrentLengthOfDocument(fileName) || line_no < 1)
    {
        printf("ERROR | Line number outside of the allowed range!\n");
        exit(1);
    }

    TextLine *lines = getExtTextLines(fileName);
    size_t linesCounter = 0;

    while (lines[linesCounter].content[0] != 0)
        linesCounter++;

    int preLineIndex = getTextLineIndex(lines, line_no - 1);
    int lineIndex = getTextLineIndex(lines, line_no);

    lines[preLineIndex].next = lines[lineIndex].next;

    generateExtFile(fileName, lines);

    Log log = {"LINE_DELETED", lineIndex, getCurrentLengthOfDocument(fileName), 1};
    appendToLog(fileName, log);

    free(lines);
}

void lineAppendController(char *fileName, char *content)
{
    size_t noOfLines = getCurrentLengthOfDocument(fileName);
    lineInsertController(fileName, noOfLines + 1, content);
}

/* File-level Controlers */

void compileExt(char *fileName)
{
    TextLine *lines = getExtTextLines(fileName);
    size_t noOfLines = getCurrentLengthOfDocument(fileName);
    size_t lineCounter;
    int lineIndex;

    remove(fileName);

    FILE *file;
    file = fopen(fileName, "a");

    for (lineCounter = 0; lineCounter < noOfLines; lineCounter++)
    {
        lineIndex = getTextLineIndex(lines, lineCounter);

        if (lines[lineIndex].next != 0)
        {
            fputs(lines[lines[lineIndex].next].content, file);
            fputs("\n", file);
        }
    }

    fclose(file);
    free(lines);

    //see if you need logs
}
// flags : o - overrite, q - quiet
void fileCreateController(char *fileName, char *flag)
{
    if (fileExists(fileName) == 1 && *flag != 'o')
    {
        printf("ERROR | File %s already exists!\n", fileName);
        exit(1);
    }

    char *fileExt = getExtName(fileName);

    FILE *file;

    file = fopen(fileName, "w");
    fclose(file);

    if (*flag != 'q')
    {

        file = fopen(fileExt, "w");
        fclose(file);

        TextLine line = {"BEGIN", 0};
        Log logBegin = {"BEGIN", 0, 0, 0};
        Log log = {"FILE_CREATED", 0, 0, 1};

        sendToExt(fileName, line);
        appendToLog(fileName, logBegin);
        appendToLog(fileName, log);
    }
    free(fileExt);
}

void fileCopyController(char *src, char *dest, char *flag)
{
    fileCreateController(dest, flag);

    size_t noLines;
    size_t line_no;

    if (fileExists(getExtName(src)) == 1)
    {
        noLines = getCurrentLengthOfDocument(src);

        TextLine *srcLines = getExtTextLines(src);
        size_t textLineIndex;

        for (line_no = 1; line_no <= noLines; line_no++)
        {
            textLineIndex = getTextLineIndex(srcLines, line_no);
            lineAppendController(dest, srcLines[textLineIndex].content);
        }
    }
    else
    {
        noLines = getNoOfLines(src);
        char *line;

        for (line_no = 1; line_no <= noLines; line_no++)
        {
            line = getLine(src, line_no);
            lineAppendController(dest, line);
        }
    }
}

void fileDeleteController(char *fileName, char *flag)
{
    if (fileExists(fileName) == 0 && *flag != 'a')
    {
        printf("ERROR | File %s doesn't exist!\n", fileName);
        exit(1);
    }

    if (*flag == 'a')
    {
        char *extFile = getExtName(fileName);
        char *logFile = getLogName(fileName);

        if (fileExists(extFile) == 0 && fileExists(logFile) == 0)
        {
            printf("ERROR | The log and ext files of file %s don't exist!\n", fileName);
            exit(1);
        }

        remove(extFile);
        remove(logFile);

        free(extFile);
        free(logFile);
    }
    else
    {
        if (fileExists(getExtName(fileName)) == 1 && *flag != 'q')
        {
            Log log = {"FILE_DELETED", 0, 0, 1};
            appendToLog(fileName, log);
        }
    }

    remove(fileName);
}

void fileShowController(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    char c = fgetc(file);

    while (c != EOF)
    {
        printf("%c", c);
        c = fgetc(file);
    }
    fclose(file);
}

void showLogs(char *fileName)
{
    char *logFile = getLogName(fileName);
    if (fileExists(logFile) == 0)
    {
        printf("ERROR | The log file for file %s doesn't exist!\n", fileName);
        exit(1);
    }

    free(logFile);

    Log *logs = getLogs(fileName);
    size_t logsCounter = 0;

    while (logs[logsCounter].operation[0] != 0)
        logsCounter++;

    for (size_t i = 0; i < logsCounter; i++)
    {
        printf("Operation: %s - Number of Lines: %lu", logs[i].operation, logs[i].totalNoLines);

        if (logs[i].current == 1)
            printf(" - CURRENT");

        printf("\n");
    }
}

void showLength(char *fileName, char *flag)
{
    size_t length;
    if (*flag == 'r')
    {
        length = getNoOfLines(fileName);
    }
    else
    {
        if (fileExists(getExtName(fileName)) == 0)
        {
            printf("The ext file for file %s does not exist. Run $ ./q2 b %s to generete it.\n", fileName, fileName);
            exit(1);
        }

        length = getCurrentLengthOfDocument(fileName);
    }

    printf("The file %s has %lu lines.\n", fileName, length);
}

void buildExtLog(char *fileName)
{
    char *fileNameCopy = calloc(strlen(fileName) + 1, sizeof(char));
    fileNameCopy = strcat(fileNameCopy, fileName);
    fileNameCopy = strcat(fileNameCopy, "e");

    fileCopyController(fileName, fileNameCopy, "o");
    fileCopyController(fileNameCopy, fileName, "o");

    fileDeleteController(fileNameCopy, "a");
    compileExt(fileName);

    free(fileNameCopy);
}

void undoController(char *fileName)
{
    char *logFile = getLogName(fileName);
    if (fileExists(logFile) == 0)
    {
        printf("ERROR | The log file for file %s doesn't exist!\n", fileName);
        exit(1);
    }

    free(logFile);

    Log *logs = getLogs(fileName);
    size_t current = 0;
    TextLine *lines = getExtTextLines(fileName);

    while (logs[current].current == 0)
        current++;

    if (current != 0)
    {
        logs[current - 1].current = 1;

        logs[current].current = 0;
        if (strcmp(logs[current].operation, "LINE_INSERTED") == 0)
        {
            size_t extLineCounter = 0;
            while (lines[extLineCounter].next != logs[current].line_no)
                extLineCounter++;

            lines[extLineCounter].next = lines[logs[current].line_no].next;
        }
        else if (strcmp(logs[current].operation, "LINE_DELETED") == 0)
        {
            size_t extLineCounter = 0;
            while (lines[extLineCounter].next != lines[logs[current].line_no].next)
                extLineCounter++;

            lines[extLineCounter].next = logs[current].line_no;
        }
        else if (strcmp(logs[current].operation, "FILE_CREATED") == 0)
        {
            fileDeleteController(fileName, "q");
        }
        else if (strcmp(logs[current].operation, "FILE_DELETED") == 0)
        {
            compileExt(fileName);
        }

        generateExtFile(fileName, lines);
    }
    generateLogFile(fileName, logs);

    free(logs);
    free(lines);
}

void redoController(char *fileName)
{
    char *logFile = getLogName(fileName);
    if (fileExists(logFile) == 0)
    {
        printf("ERROR | The log file for file %s doesn't exist!\n", fileName);
        exit(1);
    }

    Log *logs = getLogs(fileName);
    size_t current = 0;
    TextLine *lines = getExtTextLines(fileName);

    while (logs[current].current == 0)
        current++;

    if (current < getNoOfLines(logFile) - 1)
    {
        logs[current + 1].current = 1;
        logs[current].current = 0;

        if (strcmp(logs[current + 1].operation, "LINE_INSERTED") == 0)
        {
            size_t extLineCounter = 0;
            while (lines[extLineCounter].next != lines[logs[current + 1].line_no].next)
                extLineCounter++;

            lines[extLineCounter].next = logs[current + 1].line_no;
        }
        else if (strcmp(logs[current + 1].operation, "LINE_DELETED") == 0)
        {
            size_t extLineCounter = 0;
            while (lines[extLineCounter].next != logs[current + 1].line_no)
                extLineCounter++;

            lines[extLineCounter].next = lines[logs[current + 1].line_no].next;
        }
        else if (strcmp(logs[current + 1].operation, "FILE_CREATED") == 0)
        {
            fileCreateController(fileName, "q");
        }
        else if (strcmp(logs[current + 1].operation, "FILE_DELETED") == 0)
        {
            fileDeleteController(fileName, "\0");
        }
        generateExtFile(fileName, lines);
    }

    generateLogFile(fileName, logs);

    free(logFile);
    free(logs);
    free(lines);
}

void showHelp()
{
    printf("\n");
    printf("CS132 Command Line Text Editor - by Jakub Ucinski\n");
    printf("\n");
    printf("\n");
    printf("File-level interactions\n");
    printf("\n");
    printf("Create File: $ ./q2 [c|C] [file]\n");
    printf("Copy File: $ ./q2 [p|P] [source] [destination]\n");
    printf("Delete File: $ ./q2 [d|D] [file] *(a) : The a flag can be used to delete the file and it's log and the ext file corresponding to the file. WARNING: after using the a flag, the deletion of the file cannot be reversed!\n");
    printf("Show File: $ ./q2 [s|S] [source] [destination]\n");
    printf("\n");
    printf("\n");
    printf("Line-level interactions\n");
    printf("\n");
    printf("Show Line: $ ./q2 [x|X] [file] [line number]\n");
    printf("Insert Line: $ ./q2 [x|X] [file] [line number] [text content - MAX 1000 CHARACTERS]\n");
    printf("Delete Line: $ ./q2 [q|Q] [file] [line number]\n");
    printf("Append Line: $ ./q2 [a|A] [file] [text content - MAX 1000 CHARACTERS]\n");
    printf("\n");
    printf("\n");
    printf("Undo: $ ./q2 [u|U] [file]\n");
    printf("Redo: $ ./q2 [r|R] [file]\n");
    printf("Show Logs: $ ./q2 [l|L] [file]\n");
    printf("Show length of file: $ ./q2 [n|N] [file]\n");
    printf("Compile ext file: $ ./q2 [m|M] [file]\n");
    printf("Show help: $ ./q2 [h|H]\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (argc <= 1)
        return 1;

    int messageReturned;

    switch (*argv[1])
    {
    case 'c':
    case 'C':

        if (argc == 3)
            fileCreateController(argv[2], "\0");
        else if (argc > 3)
            fileCreateController(argv[2], argv[3]);
        else
            fileCreateController("./default.txt", "\0");

        break;

    case 'p':
    case 'P':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        if (argc == 4)
            fileCopyController(argv[2], argv[3], "\0");
        else if (argc > 4)
            fileCopyController(argv[2], argv[3], argv[4]);
        else
            fileCopyController(argv[2], "./default.txt", "\0");

        break;

    case 'd':
    case 'D':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }
        else if (argc > 3)
        {
            fileDeleteController(argv[2], argv[3]);
            break;
        }

        fileDeleteController(argv[2], "\0");

        break;

    case 's':
    case 'S':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        fileShowController(argv[2]);

        break;

    case 'x':
    case 'X':
        if (argc < 4)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        lineShowController(argv[2], atoi(argv[3]));

        break;

    case 'i':
    case 'I':
        if (argc < 5)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        lineInsertController(argv[2], atoi(argv[3]), argv[4]);

        break;

    case 'q':
    case 'Q':
        if (argc < 4)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        lineDeleteController(argv[2], atoi(argv[3]));

        break;

    case 'a':
    case 'A':
        if (argc < 4)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        lineAppendController(argv[2], argv[3]);

        break;

    case 'm':
    case 'M':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        compileExt(argv[2]);

        break;

    case 'l':
    case 'L':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        showLogs(argv[2]);

        break;

    case 'n':
    case 'N':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }
        else if (argc > 3)
        {
            showLength(argv[2], argv[3]);
            break;
        }

        showLength(argv[2], "\0");

        break;

    case 'b':
    case 'B':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        buildExtLog(argv[2]);

        break;

    case 'u':
    case 'U':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        undoController(argv[2]);

        break;

    case 'r':
    case 'R':
        if (argc < 3)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }

        redoController(argv[2]);

        break;

    case 'h':
    case 'H':
        showHelp();

        break;

    default:
        printf("%c", *argv[1]);

        break;
    }

    return 0;
}