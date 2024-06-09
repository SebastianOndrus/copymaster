#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <dirent.h>
#include <unistd.h>
#include <time.h>
#include "options.h"



void FatalError(char c, const char* msg, int exit_status);
void PrintCopymasterOptions(struct CopymasterOptions* cpm_options);


int main(int argc, char* argv[])
{
    struct CopymasterOptions cpm_options = ParseCopymasterOptions(argc, argv);

    //-------------------------------------------------------------------
    // Kontrola hodnot prepinacov
    //-------------------------------------------------------------------

    // Vypis hodnot prepinacov odstrante z finalnej verzie
    
    //PrintCopymasterOptions(&cpm_options);
    

    if ( argc > 4 ) {
        if ( cpm_options.link || cpm_options.directory || cpm_options.sparse == 1) {
            FatalError(' ', "KONFLIKT PREPINACOV",42); 
        }
    }

    //-------------------------------------------------------------------
    // K link, nie je kompatibilny s inymi prepinacmi(imp)
    if ( cpm_options.link) {
        if (link(cpm_options.infile,cpm_options.outfile) == -1) {
            int InFile = open( cpm_options.infile,O_RDWR);
            if (InFile == -1) {
                FatalError('K', "VSTUPNY SUBOR NEEXISTUJE",30); 
            }
            int OutFile = open( cpm_options.outfile,O_CREAT | O_EXCL);
            if (OutFile == -1) {
                close(InFile);
                FatalError('K', "VYSTUPNY SUBOR UZ EXISTUJE",30); 
            }
            close(InFile);
            close(OutFile);
            FatalError('K', "INA CHYBA",30); 
        } 
        return 0;
    }
    //-------------------------------------------------------------------

    //-------------------------------------------------------------------
    // Zistenie informacii o suboru
    //-------------------------------------------------------------------
    struct stat stats; 
    stat(cpm_options.infile,&stats);

    //-------------------------------------------------------------------
    // Sparse
    if ( cpm_options.sparse == 1 ) {
        int inputFile = open( cpm_options.infile, O_RDONLY );
        if ( inputFile == -1) {
            FatalError(' ', "SUBOR NEEXISTUJE", 21);
        }
        int outputFile = open( cpm_options.outfile, O_WRONLY | O_CREAT | O_TRUNC, stats.st_mode );
        if ( outputFile == -1) {
            close(inputFile);
            FatalError(' ', "OUTFILE EXISTUJE", 21);
        }

        ftruncate(outputFile,stats.st_size);
        char znak[1];
        int i;
        while ((i=read(inputFile,&znak[0],1)) != 0 ) {
            if ( znak[0] > '!' ) {
                write(outputFile,&znak,1);
            }
            else {
                lseek(outputFile,1,SEEK_CUR);
            }
        }
        
        close(inputFile);
        close(outputFile);
        return 0;
    }
    //-------------------------------------------------------------------


    //-------------------------------------------------------------------
    // Directory
    if ( cpm_options.directory ) {
        if (!S_ISDIR(stats.st_mode)) {
            FatalError('D', "VSTUPNY SUBOR NIE JE ADRESAR", 28);
        }
        DIR *inputDir; 
        struct dirent *DirentFile; 
        inputDir=opendir(cpm_options.infile);
        
        FILE * outputFile;
        outputFile = fopen(cpm_options.outfile,"w");
        if ( outputFile == NULL) {
            FatalError('D', "CHYBA PRI OTVARANI OUTFILE", 28);
        }
        struct stat statsF;
        while ( (DirentFile = readdir(inputDir)) != NULL ) {
            if (!strcmp((char *)DirentFile->d_name, "..") || !strcmp((char *)DirentFile->d_name, ".")) continue;
            // vypis do outfile
            char *buffer = (char *) calloc(70,sizeof(char));
            char filename[50];
            strcpy(filename, cpm_options.infile);
            strcat(filename,"/");
            strcat(filename,DirentFile->d_name);
            stat(filename,&statsF);
            // Rights
            fprintf( outputFile,(S_ISDIR(statsF.st_mode)) ? "d" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IRUSR) && (statsF.st_mode & S_IREAD)) ? "r" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IWUSR) && (statsF.st_mode & S_IWRITE)) ? "w" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IXUSR) && (statsF.st_mode & S_IEXEC)) ? "x" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IRGRP) && (statsF.st_mode & S_IREAD)) ? "r" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IWGRP) && (statsF.st_mode & S_IWRITE)) ? "w" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IXGRP) && (statsF.st_mode & S_IEXEC)) ? "x" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IROTH) && (statsF.st_mode & S_IREAD)) ? "r" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IWOTH) && (statsF.st_mode & S_IWRITE)) ? "w" : "-");
            fprintf( outputFile,((statsF.st_mode & S_IXOTH) && (statsF.st_mode & S_IEXEC)) ? "x " : "- ");
            // Links
            fprintf(outputFile,"%ld ",statsF.st_nlink);
            // IDU
            fprintf(outputFile,"%u ",statsF.st_uid);
            // IDG
            fprintf(outputFile,"%d ",statsF.st_gid);
            // Size
            fprintf(outputFile,"%ld ",statsF.st_size);
            // DD-MM-YYYY
            strftime ( buffer,70,"%d-%m-%Y",gmtime(&statsF.st_mtime));
            fprintf(outputFile,"%s ",buffer);
            // Name
            fprintf(outputFile,"%s\n",DirentFile->d_name);
            free(buffer);
        }

        fclose(outputFile); 
        closedir(inputDir);
        return 0;
    }
    //-------------------------------------------------------------------
    

    //-------------------------------------------------------------------
    // Osetrenie prepinacov pred kopirovanim
    //-------------------------------------------------------------------
    
    if ( cpm_options.lseek && ( cpm_options.create || cpm_options.overwrite || cpm_options.append ) ) {
        FatalError(' ', "KONFLIKT PREPINACOV",42);  
    }

    if (cpm_options.fast && cpm_options.slow) {     // -f & -s
        FatalError(' ', "KONFLIKT PREPINACOV",42);        
    }
    if (cpm_options.overwrite && cpm_options.create) {  // -c & -o
        FatalError(' ', "KONFLIKT PREPINACOV",42);      
    }
    if ( cpm_options.create == 1) {
        if ( cpm_options.create_mode < 1 || cpm_options.create_mode > 777) {
            FatalError('c', "ZLE PRAVA \n",23);
        }
    }
    if (cpm_options.overwrite && cpm_options.append) {  // -a & -o
        FatalError(' ', "KONFLIKT PREPINACOV",42);      
    }
    if (cpm_options.append && cpm_options.create) {     // -a & -c
        FatalError(' ', "KONFLIKT PREPINACOV",42);    
    }
    
    // TODO Nezabudnut dalsie kontroly kombinacii prepinacov ...

    //-------------------------------------------------------------------
    // Nastavenie Flags
    //-------------------------------------------------------------------

    int iFlags = O_RDONLY;
    int oFlags = O_WRONLY | O_CREAT;

    if ( cpm_options.create == 1 ) {
        oFlags = O_EXCL | O_WRONLY | O_CREAT ;
    }
    if ( cpm_options.overwrite == 1) {
        oFlags = O_WRONLY | O_TRUNC;
    }
    if ( cpm_options.append == 1) {
        oFlags = O_WRONLY | O_APPEND;
    }
    if ( cpm_options.lseek == 1) {
        oFlags = O_WRONLY;
    }


    //-------------------------------------------------------------------
    // Kontrola Inode
    //-------------------------------------------------------------------
    if ( cpm_options.inode == 1 ) {
        if (!S_ISREG(stats.st_mode)) {
            FatalError('i', "ZLY TYP VSTUPNEHO SUBORU", 27);
        }
        if ( cpm_options.inode_number != stats.st_ino ) {
            FatalError('i', "ZLY INODE", 27);
        }
    }
    
    if ( cpm_options.create == 0 && cpm_options.overwrite == 0 ) {
        cpm_options.create_mode = stats.st_mode;
    }

    //-------------------------------------------------------------------
    //  Umask
    mode_t umaskNumber = S_IROTH;
    if ( cpm_options.umask == 1) {
        int i = 0;
        while ( cpm_options.umask_options[i][0] ) {
            if (cpm_options.umask_options[i][0] == 'u') {
                if (cpm_options.umask_options[i][1] == '-' ) {
                        if ( cpm_options.umask_options[i][2] == 'r' ) {
                            umaskNumber+= S_IRUSR;
                        }
                        else if ( cpm_options.umask_options[i][2] == 'w' ) {
                            umaskNumber+= S_IWUSR;
                        }
                        else if ( cpm_options.umask_options[i][2] == 'x' ) {
                            umaskNumber+= S_IXUSR;
                        }
                        else {
                            FatalError('u',"ZLA MASKA",32);
                        } 
                }
            }
            else if (cpm_options.umask_options[i][0] == 'g') {
                if (cpm_options.umask_options[i][1] == '-' ) {
                        if ( cpm_options.umask_options[i][2] == 'r' ) {
                            umaskNumber+= S_IRGRP;
                        }
                        else if ( cpm_options.umask_options[i][2] == 'w' ) {
                            umaskNumber+= S_IWGRP;
                        }
                        else if ( cpm_options.umask_options[i][2] == 'x' ) {
                            umaskNumber+= S_IXGRP;
                        }
                        else {
                            FatalError('u',"ZLA MASKA",32);
                        } 
                }
            }
            else if (cpm_options.umask_options[i][0] == 'o') {
                if (cpm_options.umask_options[i][1] == '-' ) {
                    if ( cpm_options.umask_options[i][2] == 'r' ) {
                        umaskNumber+= S_IROTH;
                    }
                    else if ( cpm_options.umask_options[i][2] == 'w' ) {
                        umaskNumber+= S_IWOTH;
                    }
                    else if ( cpm_options.umask_options[i][2] == 'x' ) {
                        umaskNumber+= S_IXOTH;
                    }
                    else {
                        FatalError('u',"ZLA MASKA",32);
                    } 
                }
            }
            else {
                FatalError('u',"ZLA MASKA",32);
            }
            i++;
        }
    }    //-------------------------------------------------------------------


    //-------------------------------------------------------------------
    // Otvorenie suborov
    int inputFile = open( cpm_options.infile, iFlags );
    if ( inputFile == -1) {
        FatalError(' ', "SUBOR NEEXISTUJE", 21);
    }
    int outputFile;
    if ( cpm_options.umask == 1) {
        umask(umaskNumber);
        outputFile = creat(cpm_options.outfile,cpm_options.create_mode);
    }
    else {
        outputFile = open( cpm_options.outfile, oFlags,cpm_options.create_mode );
    }
    //-------------------------------------------------------------------

    //-------------------------------------------------------------------
    // Zistenie velkosti infile suboru a nastavenie velkosti buffra
    //-------------------------------------------------------------------

    struct stat statInfile;          
    fstat(inputFile, &statInfile);
    int sizeOfInput = statInfile.st_size;
    int bufferSize = sizeOfInput;
    //-------------------------------------------------------------------
    // Nastavenie rychlosti citania ak by bol spinac -s
    if ( cpm_options.slow == 1 ) {
        bufferSize = 1;
    }
    if ( cpm_options.lseek == 1 ) {
        bufferSize = cpm_options.lseek_options.num;
    }
    //-------------------------------------------------------------------


    if (outputFile== -1) {       
        if ( cpm_options.create == 1) { 
            FatalError('c', "SUBOR EXISTUJE", 23);
        }
        else if ( cpm_options.overwrite == 1) {
            FatalError('o', "SUBOR NEEXISTUJE", 24);
        }
        else if ( cpm_options.append== 1) {
            FatalError('a', "SUBOR NEEXISTUJE", 22);
        }
        else if ( cpm_options.lseek== 1) {
            FatalError('l', "INA CHYBA", 33);
        }
        FatalError(' ', "INA CHYBA", 21);
    }

    //-------------------------------------------------------------------
    // Nastavenie kurzora

    if ( cpm_options.lseek == 1) {
        off_t SeekError;
        // InFile

        SeekError=lseek(inputFile,cpm_options.lseek_options.pos1,SEEK_SET);
        if ( SeekError < 0 ) {
                FatalError('l', "CHYBA POZICIE infile", 33);
        }
        // OutFile
        if (cpm_options.lseek_options.x == SEEK_CUR || cpm_options.lseek_options.x == SEEK_SET || cpm_options.lseek_options.x == SEEK_END ) {
            SeekError=lseek(outputFile,cpm_options.lseek_options.pos2,cpm_options.lseek_options.x);
            if ( SeekError < 0 ) {
                    FatalError('l', "CHYBA POZICIE outfile", 33);
            }
        }
        else {
            FatalError('l', "Invalid X", 33);
        }
        
    }
    //-------------------------------------------------------------------

    char buf[bufferSize];
    int brzda; // ulozi sa do nej pocet precitanych bitov  

    if ( cpm_options.append == 0 && cpm_options.lseek == 0) {
        truncate(cpm_options.outfile, 0);
    } 

    while ( (brzda = read(inputFile,&buf,bufferSize)) > 0 ) {
        if ( brzda == -1 ) {
            FatalError(' ', "CHYBA CITANIA", 21);
        }
        if (write(outputFile,&buf,brzda) == -1) {
            FatalError(' ', "CHYBA Zapisu", 21);
        }
        if ( cpm_options.lseek == 1) {      // aby precitalo len "num" B
            break;
        }
    }
    if ( cpm_options.chmod == 1 ) { 
        if ( chmod( cpm_options.outfile,cpm_options.chmod_mode ) != 0) {
            FatalError('m', "ZLE PRAVA", 34);
        }
    }

    if ( cpm_options.delete_opt == 1 ) {
        if (remove(cpm_options.infile) != 0) {
            FatalError('d', "SUBOR NEBOL ZMAZANY", 26);
        }
    }

    //-------------------------------------------------------------------
    // Trucate
    if (cpm_options.truncate) {
        if ( cpm_options.truncate_size < 0 ) {
            FatalError('t', "ZAPORNA VELKOST", 31);
        }
        if (S_ISREG(stats.st_mode)) {
            truncate(cpm_options.infile, cpm_options.truncate_size);
        }
        else {
            FatalError('t', "INFILE NIE JE OBYCAJNY SUBOR", 31);
        }
    }
    //-------------------------------------------------------------------
    
    //PrintCopymasterOptions(&cpm_options);
    close(inputFile);
    close(outputFile);
       
    return 0;
}


void FatalError(char c, const char* msg, int exit_status)
{
    fprintf(stderr, "%c:%d\n", c, errno); 
    fprintf(stderr, "%c:%s\n", c, strerror(errno));
    fprintf(stderr, "%c:%s\n", c, msg);
    exit(exit_status);
}

void PrintCopymasterOptions(struct CopymasterOptions* cpm_options)
{
    if (cpm_options == 0)
        return;
    
    printf("infile:        %s\n", cpm_options->infile);
    printf("outfile:       %s\n", cpm_options->outfile);
    
    printf("fast:          %d\n", cpm_options->fast);
    printf("slow:          %d\n", cpm_options->slow);
    printf("create:        %d\n", cpm_options->create);
    printf("create_mode:   %o\n", (unsigned int)cpm_options->create_mode);
    printf("overwrite:     %d\n", cpm_options->overwrite);
    printf("append:        %d\n", cpm_options->append);
    printf("lseek:         %d\n", cpm_options->lseek);
    
    printf("lseek_options.x:    %d\n", cpm_options->lseek_options.x);
    printf("lseek_options.pos1: %ld\n", cpm_options->lseek_options.pos1);
    printf("lseek_options.pos2: %ld\n", cpm_options->lseek_options.pos2);
    printf("lseek_options.num:  %lu\n", cpm_options->lseek_options.num);
    
    printf("directory:     %d\n", cpm_options->directory);
    printf("delete_opt:    %d\n", cpm_options->delete_opt);
    printf("chmod:         %d\n", cpm_options->chmod);
    printf("chmod_mode:    %o\n", (unsigned int)cpm_options->chmod_mode);
    printf("inode:         %d\n", cpm_options->inode);
    printf("inode_number:  %lu\n", cpm_options->inode_number);
    
    printf("umask:\t%d\n", cpm_options->umask);
    for(unsigned int i=0; i<kUMASK_OPTIONS_MAX_SZ; ++i) {
        if (cpm_options->umask_options[i][0] == 0) {
            // dosli sme na koniec zoznamu nastaveni umask
            break;
        }
        printf("umask_options[%u]: %s\n", i, cpm_options->umask_options[i]);
    }
    
    printf("link:          %d\n", cpm_options->link);
    printf("truncate:      %d\n", cpm_options->truncate);
    printf("truncate_size: %ld\n", cpm_options->truncate_size);
    printf("sparse:        %d\n", cpm_options->sparse);
}
