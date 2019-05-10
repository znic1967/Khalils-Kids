#include <project_config.hpp>
// #include <cstdint>
// #include <fstream>
// #include <iostream>

#include <FreeRTOS.h>
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include <project_config.hpp>

#include <cstdint>
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "LabGPIO.hpp"
#include "L0_LowLevel/interrupt.hpp"
#include "L3_Application/oled_terminal.hpp"
#include "L1_Drivers/adc.hpp"
#include "utility/map.hpp"
#include <cinttypes>
#include "third_party/fatfs/source/ff.h"

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

// Declared globally to be used in ISR functions
LabGPIO LED0(1, 18);
LabGPIO LED1(1, 24);
LabGPIO LED2(1, 26);
LabGPIO LED3(2, 3);
OledTerminal oled_terminal;
uint8_t start;
uint8_t end;
uint8_t file_num;
ID3v1_t mp3_files[100];
int line_num;
int lines;
char position[50] = "main";
ID3v1_t currentSong;

void toggleLEDState(LabGPIO *gpio_pin);
void button0ISR();
void button1ISR();

void printMetaData(ID3v1_t mp3);
void printFilesToScreen(ID3v1_t mp3);
void printMultipleToScreen(ID3v1_t mp3_files[], int start_index, int end_index);
void moveMenuDown(ID3v1_t mp3_files[]);
void moveMenuUp(ID3v1_t mp3_files[]);
FRESULT scan_files (char* path, char fileName[10][256], ID3v1_t mp3_files[], uint8_t* file_count);

void printMainMenu();
void backTracker();
void selectTracker();
void moveCursorUp();
void moveCursorDown();


void toggleLEDState(LabGPIO *gpio_pin)
{
    if(gpio_pin->ReadBool())
    {
        gpio_pin->SetLow();
    }
    else
    {
        gpio_pin->SetHigh();
    }
}
//Select
void button0ISR()
{
    LOG_INFO("button 0 ISR!");
    toggleLEDState(&LED0);
    selectTracker();
}
//Up
void button1ISR()
{
    LOG_INFO("button 1 ISR!");
    toggleLEDState(&LED1);
    moveMenuUp(mp3_files);
    moveCursorUp();
}
//Down
void button2ISR()
{
    LOG_INFO("button 2 ISR!");
    toggleLEDState(&LED2);
    moveMenuDown(mp3_files);
    moveCursorDown();
}
//Back
void button3ISR()
{
    LOG_INFO("button 3 ISR!");
    toggleLEDState(&LED3);
    backTracker();
}

int main(void)
{
  line_num = 0;
  lines = 0;
  //   //Play Music
  // LabGPIO XDCS(1, 30);
  // LabGPIO XCS(0, 6);
  // LabGPIO XRST(0, 25);
  // LabGPIO DREQ(1, 23);
  
  // // SSP1 PINS

  // // SCLK P0.7
  // // MOSI 0.9
  // // MISO 0.8

  // VS1003 Decoder(&XDCS, &XCS, &XRST, &DREQ);

  // Decoder.init();

  // Decoder.sineTest(200);

    LOG_INFO("Setting up Interrupts...");
    LabGPIO::Init();
    LabGPIO::EnableInterrupts();
    LED0.SetAsOutput();
    LED1.SetAsOutput();

    LabGPIO button0(0, 16);
    button0.SetAsInput();
    button0.AttachInterruptHandler(button0ISR, LabGPIO::Edge::kFalling);

    LabGPIO button1(0, 17);
    button1.SetAsInput();
    button1.AttachInterruptHandler(button1ISR, LabGPIO::Edge::kFalling);

    LabGPIO button2(0, 22);
    button2.SetAsInput();
    button2.AttachInterruptHandler(button2ISR, LabGPIO::Edge::kFalling);

    LabGPIO button3(0, 0);
    button3.SetAsInput();
    button3.AttachInterruptHandler(button3ISR, LabGPIO::Edge::kFalling);

    Adc ADC(Adc::Channel::kChannel4);
    ADC.Initialize();
    ADC.BurstMode(true);

    LOG_INFO("Starting Oled...");
    oled_terminal.Initialize();
    printMainMenu();

    uint16_t result;
    float voltage;

    // LOG_INFO("Mounting filesystem...");
   
    // FATFS fs;
    // FRESULT res;
    // char buff[256];
    // char fileName[100][256];
    // file_num = 0;
    

    // res = f_mount(&fs, "", 1);
    

    // if (res == FR_OK) {
    //     LOG_INFO("File System Mounted Succesfully!");
    //     strcpy(buff, "/");
    //     res = scan_files(buff, fileName, mp3_files, &file_num);
    // }
    // else {
    //     LOG_ERROR("ERROR");
    // }
    ID3v1_t Song1;
    ID3v1_t Song2;
    ID3v1_t Song3;

    char name[30]= "Song1";

    uint8_t p[30];
    p = reinterpret_cast< uint8_t*>(name);
    Song1.title = p;
  
    LOG_INFO("Song title: %s", Song1.title);

    start = 0;
    end = start + 4;
    while(1)
    {
        // for(int i = 0; i < file_num; i++)
        // {
        //     moveMenuDown(mp3_files);
        //     Delay(1000);
        // }
        // for(int i = 0; i < file_num; i++)
        // {
        //     moveMenuUp(mp3_files);
        //     Delay(1000);
        // }
        // // moving down
        // for(int i = 6; i < file_count - 1; i++)
        // {
        //     // move down 1
        //     start = i;
        //     end = i + 2;
        //     if((i + 2) > (file_count - 1))
        //     {
        //         start = file_count - 2;
        //         end = file_count - 1;
        //     }
        //     printMultipleToScreen(mp3_files, start, end);
        //     Delay(1000);
        // }

        // // moving up 
        // for(int i = file_count - 2; i >= 0; i--)
        // {
        //     start = i;
        //     end = i + 2;
        //     if((i + 2) > (file_count - 1)){
        //         start = file_count - 3;
        //         end = file_count - 1;
        //     }
        //     printMultipleToScreen(mp3_files, start, end);
        //     Delay(1000);
        // }
        continue;
    }
    



    // TEST DISPLAYING VOLUME FROM POT (via ADC)
    // while(1)
    // {   
    //     LOG_INFO("Converting");
    //     result = ADC.ReadResult();

    //     // voltage = Map(result, 0, 1023, 0.0f, 3.3f);
    //     oled_terminal.printf("adc = %u\n", result);
    //     Delay(250);
    // }


    // //Play Music
    // LabGPIO XDCS(1, 30);
    // LabGPIO XCS(0, 6);
    // LabGPIO XRST(0, 25);
    // LabGPIO DREQ(1, 23);

    // VS1003 Decoder(&XDCS, &XCS, &XRST, &DREQ);

    // Decoder.init();

    // // Decoder.Initialize(4,28,0,6,0,8);
    // // Decoder.Initialize(1,23,0,6,1,30);
    // Decoder.sineTest(200);
    // // SpKaiKai.Initialize(8,LabSpi::FrameModes::SPI, 8);
    // //SpKaiKai.Transfer(0x12);
    // //Decoder.sineTest(200,200);
}







FRESULT scan_files (char* path, char fileName[10][256], ID3v1_t mp3_files[], uint8_t* file_count) /* Start node to be scanned (***also used as work area***) */            
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
    oled_terminal.Clear();
    for(int i = start_index; i <= end_index; i++)
    {
        oled_terminal.printf("%i. %s\n", i + 1, mp3_files[i].title);
    }
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

void moveCursorDown()
{
  if (line_num < lines)
  {
    oled_terminal.SetCursor(0, ++line_num);
  }
}
void moveCursorUp()
{
  if (line_num != 0)
  {
    oled_terminal.SetCursor(0, --line_num);
  }
}
void printMainMenu()
{
  oled_terminal.printf("1. Song List");
  oled_terminal.printf("1. Settings");
}
void backTracker()
{
  if (position == "main")
  {
    //Do nothing
  }
  if ((position == "settings") || (positon == "songlist"))
  {
    oled_terminal.Clear();
    printMainMenu();
    line_num = 0;
    position = "main"
  }
  if (position == "nowplaying")
  {
    printMultipleToScreen();
    line_num = 0;
    position == "songlist"
  }
}
void selectTracker()
{
  if (position == "main")
  {
    //printMainMenu();
    if (line_num == 1) 
    {
      position = "songlist";
      lines = filecount; // all songs in songlist (for cursor)
      printMultipleToScreen(mp3_files, 0, file_count);
      line_num = 0;
    }
    if (line_num == 2)
    {
      position = "settings";
      lines = 2;
      oled_terminal.Clear();
      oled_terminal.printf("1.Treble Up\n2.Treble Down\n3. Bass Up\n4. Bass Down");
      line_num = 0;
    }
  }
  if (position == "songlist") //on button click you go into nowplaying
  {
    position = "nowplaying";
    currentSong = mp3_files[line_num];
    //playSong(mp3_files[line_num].title);
    oled_terminal.Clear();
    oled_terminal.printf(currentSong.title, "\n", currentSong.artist, "\n", currentSong.album);
    //print if paused or playing?
    
  }
  if (position == "nowplaying")
  {
    //Do nothing
  }
  if (position == "settings")
  {
    lines = 4;
    if(line_num == 1)
    {
      //Call treble up fxn
    }
    if(line_num == 2)
    {
      //Call treble down fxn
    }
    if(line_num == 3)
    {
      //Call bass up fxn
    }
    if(line_num == 4)
    {
      //Call bass down fxn
    }
  }



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