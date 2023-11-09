

void checkFolderExists(char* _folderPath){
        if (_folderPath == NULL){
                return;
        }
        if(opendir(_folderPath)){
                return;
        }
        fprintf (stdout, "Selected folder path is not valid (folder doesn't exit / bad folder path)\n");
        exit(1);
}

char* parseFolderPath(char* _folderPath){
        checkFolderExists(_folderPath);
        if (_folderPath == NULL){
                fprintf(stdout, "Selected Default path is base directory of this application\n");
        }
        else{
                fprintf(stdout, "Selected Default path is %s\n", _folderPath);
        }
        return _folderPath;
}