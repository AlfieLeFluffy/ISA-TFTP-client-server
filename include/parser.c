#include <unistd.h>
#include <dirent.h>

void checkFolderExists(char* _folderPath){
    if (_folderPath == NULL) return;
    if(opendir(_folderPath)) return;
    fprintf (stdout, "Selected folder path is not valid (folder doesn't exit / bad folder path)\n");
    exit(1);
}

char* parseFolderPath(char* _folderPath){
    checkFolderExists(_folderPath);
    if (_folderPath == NULL) fprintf(stdout, "Selected default path is base directory of this application\n");
    else fprintf(stdout, "Selected default path is: %s\n", _folderPath);
    return _folderPath;
}

char* parseUploadFilePath(char* _filePath){
    if (access(_filePath, F_OK) == 0) return _filePath;
    fprintf(stdout, "Selected file path is not accesable or doesn't exist: %s\n", _filePath);
    exit(1);
}


char* parseFDownloadilePath(char* _filePath){
    if (access(_filePath, F_OK) != 0) return _filePath;
    fprintf(stdout, "Selected file already exists, can't rewrite: %s\n", _filePath);
    exit(1);
}