#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* 
 * Define the name schemes for EXT files, LOG files and COP files.
 */
#define EXTFILEAPPEND "_ext";
#define LOGFILEAPPEND "_log";
#define COPFILEAPPEND "cop";

/* 
 * Structure to represent a single line of the EXT file.
 */
typedef struct
{
    char content[1000];
    size_t next;
} TextLine;

/* 
 * Structure representing a single log.
 */
typedef struct
{
    char operation[100];
    size_t line_no;
    size_t totalNoLines;
    unsigned short current;
} Log;

/* 
 * Get the number of lines of a file.
 */
size_t getNoOfLines(char *fileName)
{
    FILE *file;
    file = fopen(fileName, "r");
    size_t lineNo = 1;
    char c;
    char prev;

    // Until EOF is reached
    for (c = fgetc(file); c != EOF; c = fgetc(file))
    {
        // Count the number of new line characters, corresponding to the number of lines
        if (c == '\n')
            lineNo++;

        prev = c;
    }

    if (prev == '\n')
    {
        lineNo--;
    }

    fclose(file);
    return lineNo;
}

/* 
 * Check if a file exists.
 * 
 * Returns: 
 * 1 = File exists
 * 0 = File doesn't exist
 */
short unsigned fileExists(char *fileName)
{
    FILE *file;
    // If file can be opened in read mode, it means it exists.
    if ((file = fopen(fileName, "r")))
    {
        fclose(file);
        return 1;
    }

    return 0;
}

/*
 * Get the name of the EXR file associated with the input file.
 */
char *getExtName(char *fileName)
{
    char extFileAppend[] = EXTFILEAPPEND;
    size_t fileNameCount = 0;

    // Count the number of characters in the file name
    while (fileName[fileNameCount])
        fileNameCount++;

    // Allocate the memory space for the EXT file name
    char *fileExt = calloc(fileNameCount + sizeof(extFileAppend) + 2, sizeof(char));

    // Append _ext (indicated by EXTFILEAPPEND) to the end of the file name, and prepend it with . to make the file hidden
    fileExt[0] = '.';
    fileExt = strcat(fileExt, fileName);
    fileExt = strcat(fileExt, extFileAppend);

    return fileExt;
}

/* 
 * Get the name of the LOG file associated with the input file.
 */
char *getLogName(char *fileName)
{
    char logFileAppend[] = LOGFILEAPPEND;
    size_t fileNameCount = 0;

    // Count the number of characters in the file name
    while (fileName[fileNameCount])
        fileNameCount++;

    // Allocate the memory space for the LOG file name
    char *fileLog = calloc(fileNameCount + sizeof(logFileAppend) + 2, sizeof(char));

    // Append _log (indicated by LOGFILEAPPEND) to the end of the file name, and prepend it with . to make the file hidden
    fileLog[0] = '.';
    fileLog = strcat(fileLog, fileName);
    fileLog = strcat(fileLog, logFileAppend);

    return fileLog;
}

/* 
 * Get an array of logs associated with the input file.
 */
Log *getLogs(char *fileName)
{
    char *logFile = getLogName(fileName);

    // Allocate the memory space for the array of logs. Number of lines of the log file corresponds to the number of logs.
    Log *logs = calloc(getNoOfLines(logFile), sizeof(Log));

    // Calculate the number of character required to represent the greatest long unsigned number
    size_t numLogDataMaxLength = snprintf(NULL, 0, "%lu", (size_t)pow(2, 8 * sizeof(size_t)));

    // Allocate the memory space required to represent the data regarding a log as strings
    char *operation = calloc(sizeof(logs[0].operation) + 1, sizeof(char));
    char *line_no = calloc(numLogDataMaxLength + 1, sizeof(char));
    char *totalNoLines = calloc(numLogDataMaxLength + 1, sizeof(char));
    char *current = calloc(sizeof(logs[0].current) + 1, sizeof(char));

    unsigned short columnFlag = 0;
    size_t logCounter = 0;
    size_t charCounter = 0;
    size_t noOfColumns = 4;

    FILE *file;
    file = fopen(logFile, "r");
    char c;

    for (c = fgetc(file); c != EOF; c = fgetc(file))
    {
        if (c == '|')
        {
            // switch the column when | is seen in the file
            columnFlag = (columnFlag + 1) % noOfColumns;
            charCounter = 0;
        }
        else if (c == '\n')
        {
            // put the log in the array of logs when the new line character is seen in the file
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
    }

    fclose(file);

    free(operation);
    free(line_no);
    free(totalNoLines);
    free(current);
    free(logFile);

    return logs;
}

/* 
 * Write a log to the end of the LOG file associated with the input file.
 */
void saveToLog(char *fileName, Log log)
{
    char *fileLog = getLogName(fileName);

    // Calculate the number of characters required to represent the data regarding a log as strings
    size_t logLineNoLength = snprintf(NULL, 0, "%lu", log.line_no);
    size_t logTotalNoLinesLength = snprintf(NULL, 0, "%lu", log.totalNoLines);
    size_t logCurrentLength = snprintf(NULL, 0, "%u", log.current);
    size_t noOfColumns = 4;

    // Allocate space in memory for the log data in string format
    char *logEntry = calloc(strlen(log.operation) + logLineNoLength + logTotalNoLinesLength + logCurrentLength + noOfColumns + 2, sizeof(char));
    char *logLineNoString = calloc(logLineNoLength + 1, sizeof(char));
    char *logTotalNoLinesString = calloc(logTotalNoLinesLength + 1, sizeof(char));
    char *logCurrentString = calloc(logCurrentLength + 1, sizeof(char));

    // "Cast" the log data to strings if not string already
    snprintf(logLineNoString, logLineNoLength + 1, "%lu", log.line_no);
    snprintf(logTotalNoLinesString, logTotalNoLinesLength + 1, "%lu", log.totalNoLines);
    snprintf(logCurrentString, logCurrentLength + 1, "%u", log.current);

    // Concatenate the log data in string format with eachother, putting | after every piece of data
    logEntry = strcat(logEntry, log.operation);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logLineNoString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logTotalNoLinesString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, logCurrentString);
    logEntry = strcat(logEntry, "|");
    logEntry = strcat(logEntry, "\n");

    // Append the concatinated string to the LOG file associated with the input file
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

/* 
 * Create the LOG file associated with the input file from an array of logs.
 */
void *generateLogFile(char *fileName, Log *logs)
{
    char *logFile = getLogName(fileName);
    size_t logCounter;

    // Remove the LOG file associated with the input file
    remove(logFile);

    // Put every log in the array of logs to the LOG file
    for (logCounter = 0; logs[logCounter].operation[0]; logCounter++)
        saveToLog(fileName, logs[logCounter]);

    free(logFile);
}

/* 
 * Removes the logs after the CURRENT.
 */
void flushLogs(char *fileName)
{
    char *logFile = getLogName(fileName);

    // if the log file exists
    if (fileExists(logFile) == 1)
    {

        Log *logs = getLogs(fileName);
        size_t newLogsNo = 0;

        // Count the number of logs before the current
        while (logs[newLogsNo].current == 0)
            newLogsNo++;

        Log *newLogs = calloc(newLogsNo + 1, sizeof(Log));
        size_t logCounter;

        // Put all the logs up to the current in the array containing the 'new' logs
        for (logCounter = 0; logs[logCounter].current == 0; logCounter++)
            newLogs[logCounter] = logs[logCounter];

        // Put the current log in the array containing the 'new' logs
        newLogs[logCounter] = logs[logCounter];

        generateLogFile(fileName, newLogs);

        free(logs);
        free(newLogs);
    }

    free(logFile);
}

/*
 * Append a line representation to the EXT file associated with the input file.
 */
void sendToExt(char *fileName, TextLine line)
{

    printf("%lu", line.next);
    char *fileExt = getExtName(fileName);

    // Calculate the number of characters required to represent the data regarding a line as strings
    size_t lineNextLength = snprintf(NULL, 0, "%lu", line.next);
    size_t noOfColumns = 2;

    // Allocate space in memory for the line data in string format
    char *message = calloc(strlen(line.content) + lineNextLength + noOfColumns + 2, sizeof(char));
    char *lineNextString = calloc(lineNextLength + 1, sizeof(char));

    // "Cast" the line data to strings if not string already
    snprintf(lineNextString, lineNextLength + 1, "%lu", line.next);

    // Concatenate the line data in string format with eachother, putting | after every piece of data
    message = strcat(message, line.content);
    message = strcat(message, "|");
    message = strcat(message, lineNextString);
    message = strcat(message, "|");
    message = strcat(message, "\n");

    // Append the concatinated string to the EXT file associated with the input file
    FILE *file;
    file = fopen(fileExt, "a");
    fputs(message, file);
    fclose(file);

    free(message);
    free(fileExt);
    free(lineNextString);
}

/* 
 * Get an array of line representation associated with the input file.
 */
TextLine *getExtTextLines(char *fileName)
{
    char *extFile = getExtName(fileName);

    // Allocate the memory space for the array of line representations.
    // Number of lines of the EXT file corresponds to the number of line representations.
    TextLine *lines = calloc(getNoOfLines(extFile), sizeof(TextLine));

    // Calculate the number of character required to represent the greatest long unsigned number
    size_t numLineDataMaxLength = snprintf(NULL, 0, "%lu", (size_t)pow(2, 8 * sizeof(size_t)));

    char *content = calloc(strlen(lines[0].content) + 1, sizeof(char));
    char *next = calloc(numLineDataMaxLength + 1, sizeof(char));

    unsigned short columnFlag = 0;
    size_t lineCounter = 0;
    size_t charCounter = 0;

    FILE *file;
    file = fopen(extFile, "r");
    char c;

    for (c = fgetc(file); c != EOF; c = fgetc(file))
    {
        if (c == '|')
        {
            // switch the column when | is seen in the file
            columnFlag = 1 - columnFlag;
            charCounter = 0;
        }
        else if (c == '\n')
        {
            // put the line representation in the array of line representations
            // when the new line character is seen in the file
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
    }

    fclose(file);

    free(content);
    free(next);
    free(extFile);

    return lines;
}

/* 
 * Given the line number of a line, get the index of it's representation in the array of line representation.
 */
long getTextLineIndex(TextLine *lines, size_t line_no)
{
    TextLine current = lines[0];
    size_t traverseCounter;
    size_t nextIndex = 0;

    // Traverse through the lines, until the number of "hops" made is not lower than the line number
    // provided, or until there are no more "hops" to be made (meaning we reached the end of lines).
    for (traverseCounter = 0; traverseCounter < line_no && current.next != 0; traverseCounter++)
    {
        nextIndex = current.next;
        current = lines[current.next];
    }

    // If the number of "hops" made is equal to the line number provided, the line number corresponding to the "current"
    // line is returned (ids, or line numbers are not stores, therefore if we wish to return the line number of the "current" line
    // we have to actually return what the previous line pointed to as next).
    if (traverseCounter == line_no)
        return nextIndex;

    return -1;
}

/* 
 * Get the length the document would have if the EXT file would be compiled without compiling it.
 */
size_t getCurrentDocumentLength(char *fileName)
{
    TextLine *lines = getExtTextLines(fileName);
    TextLine current = lines[0];
    size_t traverseCounter;
    size_t nextIndex = 0;

    // Traverse through the line representations, until the current line points to 0 (interpreted as NULL),
    // keeping track of how many lines have visited.
    for (traverseCounter = 0; current.next != 0; traverseCounter++)
    {
        nextIndex = current.next;
        current = lines[current.next];
    }

    free(lines);

    return traverseCounter;
}

/*
 * Create the EXT file associated with the input file from an array of line representations.
 */
void *generateExtFile(char *fileName, TextLine *lines)
{
    char *extFile = getExtName(fileName);
    size_t lineCounter;

    // // Remove the EXT file associated with the input file
    remove(extFile);

    // // Put every line representation in the array of line representations to the EXT file
    for (lineCounter = 0; lines[lineCounter].content[0] != 0; lineCounter++)
        sendToExt(fileName, lines[lineCounter]);

    free(extFile);
}

/*
 * Append a log to the end of logs associated with the input file.
 */
void *appendToLog(char *fileName, Log log)
{
    char *fileLog = getLogName(fileName);

    // Remove all the logs after the CURRENT
    flushLogs(fileName);

    // If the LOG file doesn't exist, create it.
    if (fileExists(fileLog) == 0)
    {
        FILE *file;
        file = fopen(fileLog, "w");
        fclose(file);
    }

    free(fileLog);

    Log *logs = getLogs(fileName);
    size_t logsCounter = 0;

    // Count how many logs associated with the input file there are.
    while (logs[logsCounter].operation[0])
        logsCounter++;

    // Allocate the memory space required to store the logs already present + one
    // more log that we will be added in one moment.
    Log *logsCopy = calloc(logsCounter + 1, sizeof(Log));

    // Copy the logs already present, setting all the currents to 0 (the new log
    // that will be added next is the new current).
    for (size_t i = 0; i < logsCounter; i++)
    {
        strcpy(logsCopy[i].operation, logs[i].operation);
        logsCopy[i].line_no = logs[i].line_no;
        logsCopy[i].totalNoLines = logs[i].totalNoLines;
        logsCopy[i].current = 0;
    }

    // Add the new log to the array of logs.
    strcpy(logsCopy[logsCounter].operation, log.operation);
    logsCopy[logsCounter].line_no = log.line_no;
    logsCopy[logsCounter].totalNoLines = log.totalNoLines;
    logsCopy[logsCounter].current = log.current;

    // Generate the lOG file using the new array of logs.
    generateLogFile(fileName, logsCopy);

    free(logs);
    free(logsCopy);
}

/* 
 * Get a line indicated by line_no from the input file.
 */
char *getLine(char *fileName, size_t line_no)
{
    // Exit if the line number provided exceeds the number of lines
    // of the input file.
    if (line_no > getNoOfLines(fileName) || line_no < 1)
        exit(1);

    // Allocate memory space for the output string
    char *output = calloc(1000, sizeof(char));
    int lineCounter = 1;

    FILE *file;
    file = fopen(fileName, "r");
    char c;

    // Traverse through the characters in the input file, taking count of the number of new line
    // characters seen (which corresponds to the line number if started with 1),
    // until it reaches the line number provided.
    for (c = fgetc(file); lineCounter < line_no; c = fgetc(file))
    {
        if (c == '\n')
            lineCounter++;
    }

    // Traverse though the characters in the line, concatinating them to the output string
    // until the new line character is reached.
    for (c; c != '\n' && c != EOF; c = fgetc(file))
        output = strncat(output, &c, 1);

    return output;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *                                                                           *
 *                          Line-level Controlers                            *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* 
 * Controller for displaying a line.
 */
void lineShowController(char *fileName, size_t line_no, char *flag)
{
    size_t noOfLines = getNoOfLines(fileName);
    char *extFile = getExtName(fileName);

    if (*flag == 'r' || fileExists(extFile) == 0)
    {
        /*
         * If the r flag was provided, or the EXT file for the input file doesn't exist
         * get the line using getLine and display it
         */
        free(extFile);

        if (line_no > noOfLines || line_no < 1)
        {
            printf("ERROR | Line number outside of the allowed range!\n");
            exit(1);
        }

        char *line = getLine(fileName, line_no);
        printf("%s\n", line);
        free(line);
    }
    else
    {
        /*
         * If the r flag wasn't provided and the EXT file for the input file does exist - 
         * display the line from the array of line representations corresponding to
         * the line number given (it displays the line that running the show line controller
         * with the r flag would display after compiling the EXT file first).
         */

        free(extFile);

        if (line_no > getCurrentDocumentLength(fileName) || line_no < 1)
        {
            printf("ERROR | Line number outside of the allowed range!\n");
            exit(1);
        }

        TextLine *lines = getExtTextLines(fileName);
        int lineIndex = getTextLineIndex(lines, line_no);

        char *line = lines[lineIndex].content;
        printf("%s\n", line);

        free(lines);
    }
}

/* 
 * Controller for inserting a line.
 */
void lineInsertController(char *fileName, size_t line_no, char *content)
{
    char *extFile = getExtName(fileName);

    // If the EXT file associated with the input file doesn't exist, a line cannot be inserted
    // and so an adequate message is displayed.
    if (fileExists(extFile) == 0)
    {
        printf("The ext file for file %s does not exist. Run $ ./q2 b %s to generete it.\n", fileName, fileName);
        exit(1);
    }

    // If the input file doesn't exist, a line cannot be inserted and so an adequate message is displayed.
    if (fileExists(fileName) == 0)
    {
        printf("The file %s does not exist. Run $ ./q2 c %s to create it.\n", fileName, fileName);
        exit(1);
    }

    free(extFile);

    // If the line number provided exceeds the length of the document + 1 or is less than 1,
    // display a suitable error message.
    if (line_no > getCurrentDocumentLength(fileName) + 1 || line_no < 1)
    {
        printf("ERROR | Line number outside of the allowed range!\n");
        exit(2);
    }

    // If the length of the string to be written to file exceeds 999 characters, the line cannot be inserted
    // and so an adequate error message is displayed
    if (strlen(content) > 999)
    {
        printf("ERROR | Too long line input!\n");
        exit(3);
    }

    TextLine *lines = getExtTextLines(fileName);
    size_t linesCounter = 0;
    TextLine preLine;

    // Count how many line representations associated with the input file there are.
    while (lines[linesCounter].content[0])
        linesCounter++;

    // The index of the line repressentation that is associated with the line number one less than
    // the line number provided is retrived
    int preLineIndex = getTextLineIndex(lines, line_no - 1);

    // Allocate the memory space required to store the lines already present + one
    // more line that we will be added in one moment.
    TextLine *linesCopy = calloc(linesCounter + 1, sizeof(TextLine));

    // Copy the lines already present.
    for (size_t i = 0; i < linesCounter; i++)
    {
        strcpy(linesCopy[i].content, lines[i].content);
        linesCopy[i].next = lines[i].next;
    }

    // Create a new line representation at index linesCounter (which is empty) by copying
    // the string provided, end setting the value of next, to the value that the line before
    // it pointed. Make the line before the added line point to the newly added line.
    strcpy(linesCopy[linesCounter].content, content);
    preLine = lines[preLineIndex];
    linesCopy[linesCounter].next = preLine.next;
    linesCopy[preLineIndex].next = linesCounter;

    // Generate the EXT file using the new array of line representations.
    generateExtFile(fileName, linesCopy);

    // Send a suitable message to the LOG file associated with the input file.
    Log log = {"LINE_INSERTED", linesCounter, getCurrentDocumentLength(fileName), 1};
    appendToLog(fileName, log);

    free(lines);
    free(linesCopy);
}

/* 
 * Contoller for deleting a line.
 */
void lineDeleteController(char *fileName, int line_no)
{
    char *extFile = getExtName(fileName);

    // If the EXT file associated with the input file doesn't exist, a line cannot be deleted
    // and so an adequate message is displayed.
    if (fileExists(extFile) == 0)
    {
        printf("The EXT file for file %s does not exist. Run $ ./q2 b %s to generete it.\n", fileName, fileName);
        exit(1);
    }

    free(extFile);

    // If the line number provided exceeds the length of the document or is less than 1,
    // display a suitable error message.
    if (line_no > getCurrentDocumentLength(fileName) || line_no < 1)
    {
        printf("ERROR | Line number outside of the allowed range!\n");
        exit(1);
    }

    TextLine *lines = getExtTextLines(fileName);

    // Set the line before the line to be deleted to point to the line the line to be deleted points to.
    // This way, when we follow the pointers, we skip the deleted line (without removing it completely).
    int preLineIndex = getTextLineIndex(lines, line_no - 1);
    int lineIndex = getTextLineIndex(lines, line_no);
    lines[preLineIndex].next = lines[lineIndex].next;

    // Generate the EXT file using the updated array of line representations.
    generateExtFile(fileName, lines);

    // Send a suitable message to the LOG file associated with the input file.
    Log log = {"LINE_DELETED", lineIndex, getCurrentDocumentLength(fileName), 1};
    appendToLog(fileName, log);

    free(lines);
}

/* 
 * Controller for appending a line.
 */
void lineAppendController(char *fileName, char *content)
{
    // Insert a line after the last line.
    size_t noOfLines = getCurrentDocumentLength(fileName);
    lineInsertController(fileName, noOfLines + 1, content);
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 
 *                                                                           *
 *                          File-level Controlers                            *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/* 
 * Compile the EXT file associated with the input file.
 */
void compileExt(char *fileName)
{
    TextLine *lines = getExtTextLines(fileName);
    size_t noOfLines = getCurrentDocumentLength(fileName);
    size_t lineCounter;
    int lineIndex;

    // Remove the input file
    remove(fileName);

    FILE *file;
    file = fopen(fileName, "a");

    // Traverse through the lines in the EXT file associated with the input file
    for (lineCounter = 0; lineCounter < noOfLines; lineCounter++)
    {
        lineIndex = getTextLineIndex(lines, lineCounter);

        // If the currently visited line points to a line, put the content of that next line
        // to a file (we start at the BEGIN header, which we do not want to put, but we want
        // to put the first line the BEGIN header point to).
        if (lines[lineIndex].next)
        {
            fputs(lines[lines[lineIndex].next].content, file);
            fputs("\n", file);
        }
    }

    fclose(file);
    free(lines);
}

/*
 * Controller for creating a file.
 */
void fileCreateController(char *fileName, char *flag)
{
    // If the file already exists, unless the o (overwrite) flag has been provided
    // display a suitable error message
    if (fileExists(fileName) == 1 && *flag != 'o')
    {
        printf("ERROR | File %s already exists. Run $ ./q2 c %s o to overwrite it.\n", fileName, fileName);
        exit(1);
    }

    char *fileExt = getExtName(fileName);
    char *fileLog = getLogName(fileName);

    FILE *file;

    // Create the input file
    file = fopen(fileName, "w");
    fclose(file);

    if (*flag != 'q')
    {
        // Unless the q (quiet) flag had been provided (used e.g. when undoing a file deletion)
        // create the EXT and LOG files associated with the file, sending a suitable log.
        file = fopen(fileLog, "w");
        fclose(file);

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
    free(fileLog);
}

/*
 * Controller for copying a file.
 */
void fileCopyController(char *src, char *dest, char *flag)
{
    // The destination file is created
    fileCreateController(dest, flag);
    size_t noLines;
    size_t line_no;
    char *fileExt = getExtName(src);

    if (fileExists(fileExt) == 1)
    {
        //     /*
        //      * If the source file has an EXT file associated with it,
        //      * append lines to the destination file according to the EXT file
        //      * of the source file (this is so that uncompiled files can be copied
        //      * and the destination file can have the uncompiled lines that the source file had,
        //      * except the lines that were deleted - a desired behaviour !).
        //      */

        noLines = getCurrentDocumentLength(src);
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
        /*
         * If the source file doesn't have an EXT file associated with it,
         * append lines to the destination file according to the source file itself
         * (get the lines directly from the source file and append them to the destination file).
         */

        noLines = getNoOfLines(src);
        char *line;

        for (line_no = 1; line_no <= noLines; line_no++)
        {
            line = getLine(src, line_no);
            lineAppendController(dest, line);
            free(line);
        }
    }

    free(fileExt);
}

/*
 * Controller for deleting a file.
 */
void fileDeleteController(char *fileName, char *flag)
{
    // If the input file doesn't exist, unless an a (delete all) flag has been provided
    // output a suitable error message.
    if (fileExists(fileName) == 0 && *flag != 'a')
    {
        printf("ERROR | File %s doesn't exist!\n", fileName);
        exit(1);
    }

    char *extFile = getExtName(fileName);
    char *logFile = getLogName(fileName);

    if (fileExists(fileName) == 1)
    {
        // If the input file exists, it is deleted and a suitable message is displayed.
        printf("Deleting file %s!\n", fileName);
        remove(fileName);
    }

    if (fileExists(logFile) == 1 && *flag != 'q')
    {
        // If the LOG file associated with the input file exists, and the q (quiet) flag
        // has not been provided, a log is sent to the logs associated with the input file.
        Log log = {"FILE_DELETED", 0, 0, 1};
        appendToLog(fileName, log);
    }

    if (*flag == 'a' || *flag == 'x')
    {
        char decision = 'n';

        if (fileExists(extFile) == 0 && fileExists(logFile) == 0)
        {
            // If the EXT and LOG files associated with the input file don't exist,
            // a suitable error message is displayed.
            printf("ERROR | The LOG and EXT files of file %s don't exist!\n", fileName);
        }
        else if (*flag == 'x')
        {
            decision = 'y';
        }
        else
        {
            // Before the deletion of the EXT and LOG files, the user is asked to confirm their choice as
            // the action cannot be undone. This is a measure taken to make sure the user won't delete a file
            // they do not wish to delete.
            printf("Deleting the EXT and LOG files associated with %s. \n", fileName);
            printf("WARNING: If done so, all data in file %s will be lost, with no way to undo the deletion!\n", fileName);
            printf("Do you want to continue [y/n] : ");
            scanf("%c", &decision);
        }

        if (decision == 'y')
        {
            // If the user has positively confirmed their choice, the EXT and LOG files
            // associated with the input file are deleted.
            remove(extFile);
            remove(logFile);
        }
        else
        {
            // If there was an error (the files EXT and LOG files didn't exist) or if the user didn't positively
            // confirm their choice, a suitable message is displayed on the screen.
            printf("The EXT and LOG files associated with %s not deleted. \n", fileName);
        }
    }

    free(logFile);
    free(extFile);
}

/*
 * Controller for displaying a file.
 */
void fileShowController(char *fileName)
{
    // TODO: show the file in EXT
    FILE *file;
    file = fopen(fileName, "r");
    char c;

    for (c = fgetc(file); c != EOF; c = fgetc(file))
        printf("%c", c);

    fclose(file);
}

/*
 * Display the logs associated with the input file.
 */
void showLogs(char *fileName)
{
    char *logFile = getLogName(fileName);

    // If the LOG file associated with the input file doesn't exist,
    // a suitable error message is displayed.
    if (fileExists(logFile) == 0)
    {
        printf("ERROR | The log file for file %s doesn't exist!\n", fileName);
        exit(1);
    }

    free(logFile);

    Log *logs = getLogs(fileName);
    size_t logsCounter;
    TextLine *lines = getExtTextLines(fileName);

    // Format the print the logs
    for (size_t logsCounter = 0; logs[logsCounter].operation[0]; logsCounter++)
    {
        printf("Operation: %s - Number of Lines: %lu", logs[logsCounter].operation, logs[logsCounter].totalNoLines);

        // Print the line the log points to
        if (logs[logsCounter].line_no)
            printf(" - Line: %s", lines[logs[logsCounter].line_no].content);

        if (logs[logsCounter].current)
            printf(" - CURRENT");

        printf("\n");
    }
}

/*
 * Display the length of the compiled or uncompiled versions of the input file
 */
void showLength(char *fileName, char *flag)
{
    // If the input file does not exist a suitable error message is dispayed.
    if (fileExists(fileName) == 0)
    {
        printf("The file %s does not exist", fileName);
        exit(1);
    }

    size_t length;
    char *extFile = getExtName(fileName);

    if (*flag == 'r' || fileExists(extFile) == 0)
    {
        // If the r (real) flag has been provided, or the EXT file associated with
        // the input file does not exist print the actual length of the file
        length = getNoOfLines(fileName);
    }
    else
    {
        // If the r (real) flag has not been provided and the EXT file associated with
        // the input file does exist, print the number of lines the file would have if
        // it would be compiled.
        length = getCurrentDocumentLength(fileName);
    }

    free(extFile);

    printf("The file %s has %lu lines.\n", fileName, length);
}

/*
 * Generate the EXT and LOG files for a file which doesn't have them - required for operations 
 * like inserting a line or deleting a line.
 */
void buildExtLog(char *fileName) // test this
{
    char *fileNameCopy = calloc(strlen(fileName) + 1, sizeof(char));
    strcpy(fileNameCopy, fileName);
    char fileCopyAppendix[] = COPFILEAPPEND;

    char *fileCopy = calloc(strlen(fileNameCopy) + strlen(fileCopyAppendix) + 1, sizeof(char));
    fileCopy = strcat(fileCopy, fileNameCopy);
    fileCopy = strcat(fileCopy, fileCopyAppendix);

    fileCopyController(fileNameCopy, fileCopy, "o");
    fileCopyController(fileCopy, fileNameCopy, "o");
    // fileDeleteController(fileCopy, "x");
    compileExt(fileNameCopy);

    free(fileCopy);
    free(fileNameCopy);
}

/*
 * Controller for undoing previously done operations.
 */
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

/*
 * Controller for redoing previously undone operations.
 */
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

/*
 * Display the manual for the editor.
 */
void showHelp()
{
    printf("\n");
    printf("CS132 Command Line Text Editor - by Jakub Ucinski\n");
    printf("\n");
    printf("\n");
    printf("File-level interactions\n");
    printf("\n");
    printf("Create File: $ ./q2 [c|C] [file] *(o) : o flag - overwrite a file with name given if such already exists \n");
    printf("Copy File: $ ./q2 [p|P] [source] [destination] *(o) : o flag - overwrite a file with name given for source if such already exists \n");
    printf("Delete File: $ ./q2 [d|D] [file] *(a) : a flag - also delete the LOG and EXT files associated with the given file. WARNING: after using the a flag, the deletion of the file cannot be undone!\n");
    printf("Show File: $ ./q2 [s|S] [file] \n");
    printf("\n");
    printf("\n");
    printf("Line-level interactions\n");
    printf("\n");
    printf("Show Line: $ ./q2 [q|Q] [file] [line number] *(r) : r flag - show the real, current line with the line number given\n");
    printf("Insert Line: $ ./q2 [i|I] [file] [line number] [text content - MAX 1000 CHARACTERS]\n");
    printf("Delete Line: $ ./q2 [x|X] [file] [line number]\n");
    printf("Append Line: $ ./q2 [a|A] [file] [text content - MAX 1000 CHARACTERS]\n");
    printf("\n");
    printf("\n");
    printf("Undo: $ ./q2 [u|U] [file]\n");
    printf("Redo: $ ./q2 [r|R] [file]\n");
    printf("Show Logs: $ ./q2 [l|L] [file]\n");
    printf("Show length of file: $ ./q2 [n|N] [file] *(r) : r flag - show the real, current length of the file \n");
    printf("Compile ext file: $ ./q2 [m|M] [file]\n");
    printf("Generate the EXT and LOG files: $ ./q2 [b|B] [file]\n");
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

    case 'q':
    case 'Q':
        if (argc < 4)
        {
            printf("ERROR || Incorrect number of arguments\n");
            break;
        }
        else if (argc > 4)
        {
            lineShowController(argv[2], atoi(argv[3]), argv[4]);
            break;
        }

        lineShowController(argv[2], atoi(argv[3]), "\0");

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

    case 'x':
    case 'X':
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