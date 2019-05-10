#include <project_config.hpp>
// #include <cstdint>
// #include <fstream>
// #include <iostream>

#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#include "utility/rtos.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "L0_LowLevel/interrupt.hpp"
#include "third_party/FreeRTOS/Source/include/task.h"
#include "third_party/FreeRTOS/Source/include/semphr.h"
#include "L3_Application/oled_terminal.hpp"
#include <cinttypes>
#include <iterator>
#include "L3_Application/commandline.hpp"
#include "L3_Application/commands/rtos_command.hpp"
#include "L3_Application/commands/common.hpp"
#include "L3_Application/commands/lpc_system_command.hpp"
#include "../../library/third_party/fatfs/source/ff.h"

#include "VS1003.hpp"
#include "VS1053.h"

LabSpi SpKaiKai;

typedef union
{
    uint8_t buffer[128];
    struct 
    {
        uint8_t header[3];
        uint8_t title[30];
        uint8_t artist[30];
        uint8_t album[30];
        uint8_t year[4];
        uint8_t comment[28];
        uint8_t zero;
        uint8_t track;
        uint8_t genre;
    } __attribute__((packed));
} ID3v1_t;

OledTerminal oled_terminal;
uint8_t start;
uint8_t end;
uint8_t file_num;
ID3v1_t mp3_files[100];

void printMetaData(ID3v1_t mp3);
void printFilesToScreen(ID3v1_t mp3);
void printMultipleToScreen(ID3v1_t mp3_files[], int start_index, int end_index);
void moveMenuDown(ID3v1_t mp3_files[]);
void moveMenuUp(ID3v1_t mp3_files[]);
FRESULT scan_files (char* path, char fileName[100][256], ID3v1_t mp3_files[], uint8_t* file_count);


//SCLK = 1_0 ch0
//MOSI = 1_1 ch1
//MISO = 1_4 ch2
//xDREQ = 4_28 ch3
//xCS = 0_6 ch4
//xDCS = 0_8 ch5

int main(void)
{
    // Play Music
    LabGPIO XDCS(1, 30);
    LabGPIO XCS(0, 6);
    LabGPIO XRST(0, 25);
    LabGPIO DREQ(1, 23);
  
    // SSP1 PINS

    // SCLK P0.7
    // MOSI 0.9
    // MISO 0.8

    VS1003 Decoder(&XDCS, &XCS, &XRST, &DREQ);

    Decoder.init();

    // Decoder.Initialize(4,28,0,6,0,8);
    // Decoder.Initialize(1,23,0,6,1,30);
    Decoder.sineTest(200);

    FATFS fs;
    FRESULT res;
    char buff[256];
    char fileName[100][256];
    file_num = 0;
    

    res = f_mount(&fs, "", 1);
    

    if (res == FR_OK) {
        LOG_INFO("File System Mounted Succesfully!");
        strcpy(buff, "/");
        res = scan_files(buff, fileName, mp3_files, &file_num);
    }
    else {
        LOG_ERROR("ERROR");
    }

    oled_terminal.printf("%s\n", fileName[0]);
    // for(int i = 0; i < file_num; i++)
    // {
    //     printf("Song: %s\n", fileName[i]);
    // }

    for(int i = 0; i < file_num; i++)
    {
        printf("Song: %s\n", fileName[i]);
        Decoder.playSong(fileName[i]);
    }
    
    // printf("Song: %s\n", mp3_files[0].title);
    // Decoder.playSong((char *) mp3_files[0].title);
 // SpKaiKai.Initialize(8,LabSpi::FrameModes::SPI, 8);
  //SpKaiKai.Transfer(0x12);
  //Decoder.sineTest(200,200);

  return 0;
}

FRESULT scan_files (char* path, char fileName[100][256], ID3v1_t mp3_files[], uint8_t* file_count) /* Start node to be scanned (***also used as work area***) */            
{
    *file_count = 0; 

    FRESULT res;
    DIR dir;
    static FILINFO fno;
    FIL fsrc;            /* File object */
    BYTE buffer[4096];   /* File copy buffer */
    FRESULT fr;          /* FatFs function common result code */
    UINT br, bw;         /* File read/write count */
    FSIZE_t f_rd;        /* File read/write pointer */
    ID3v1_t mp3;

    res = f_opendir(&dir, path);          /* Open the directory */
    if (res == FR_OK) 
    {
        LOG_INFO("We made it this far boiz"); 
        while (1) 
        {
            res = f_readdir(&dir, &fno);  /* Read a directory item */

            // Break on error or end of dir
            if (res != FR_OK || fno.fname[0] == 0) 
            {
                // LOG_ERROR("Error/End of directory");
                break;  /* Break on error or end of dir */
            } 
            if(strstr(fno.fname, ".mp3"))
            {
                printf("Fno.fname: %s\n",fno.fname);
                printf("File Size: %u\n", fno.fsize);

                // open and read files
                fr = f_open(&fsrc, fno.fname, FA_READ);
                fr = f_read(&fsrc, buffer, sizeof buffer, &br);   // Read a chunk of source file 
                if (fr || br == 0) break; /* error or eof */

                int i = 0;
                // save filenames
                while (fno.fname[i] && i < 256) {
                    fileName[*file_count][i] = fno.fname[i];
                    i++;
                }
                fileName[*file_count][i] = '\0';
                printf("\n");

                br = 128;
                f_rd = fno.fsize - 128;
                fr = f_lseek(&fsrc, f_rd);
                if(fr) break;
                fr = f_read(&fsrc, mp3.buffer, sizeof buffer, &br);   // Read a chunk of source file 
                if (fr || br == 0) break; /* error or eof */

                // for(int i = 128; i >= 0; i--)
                // {
                //     printf("%i", mp3.buffer[i]);
                // }
                printMetaData(mp3);
                mp3_files[*file_count] = mp3;
                //printFilesToScreen(mp3);
                printf("\n");

                // get number of files on sd card
                (*file_count)++;     
            }
        }
        f_closedir(&dir);
        printf("File Count: %d \n",*file_count);

        int j = 0;
        for (int i = 0; i < *file_count; i++) 
        {
            printf("File Name: %s\n", fileName[i]);
        }
    }
    return res;
}

void printMetaData(ID3v1_t mp3)
{
    printf("Header: %s\nFilename: %s\nArtist: %s\nAlbum: %s\nYear: %s\nComment: %s\nZero: %i\nTrack: %i\nGenre: %i\n", 
        mp3.header, mp3.title, mp3.artist, mp3.album, mp3.year, mp3.comment,
        mp3.zero, mp3.track, mp3.genre);
}


void printFilesToScreen(ID3v1_t mp3)
{
    oled_terminal.printf("-%s\n", 
        mp3.title);
}

void printMultipleToScreen(ID3v1_t mp3_files[], int start_index, int end_index)
{
    if((start_index + 3) > end_index){
        start_index = end_index - 3;
    }
    oled_terminal.Clear();
    oled_terminal.printf("%i. %s\n"
                        "%i. %s\n"
                        "%i. %s\n"
                        "%i. %s\n", 
                        start_index + 1, mp3_files[start_index].title,
                        start_index + 2, mp3_files[start_index + 1].title,
                        start_index + 3, mp3_files[start_index + 2].title,
                        start_index + 4, mp3_files[start_index + 3].title
                        );
}

void moveMenuDown(ID3v1_t mp3_files[])
{
    start--;
    end = start + 4;
    if(start < 0)
    {
        start = 0;
        end = start + 4;
    }
    else if(end > file_num)
    {
        end = file_num - 1;
        start = file_num - 5;
    }
    printMultipleToScreen(mp3_files, start, end);
}

void moveMenuUp(ID3v1_t mp3_files[])
{
    start++;
    end = start + 4;
    if(start < 0)
    {
        start = 0;
        end = start + 4;
    }
    else if(end > file_num)
    {
        end = file_num - 1;
        start = file_num - 5;
    }
    printMultipleToScreen(mp3_files, start, end);
}