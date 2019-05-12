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

#define START_TIME 0
#define END_TIME 1
#define DEBOUNCE_TIME 5000
const uint32_t STACK_SIZE = 512;

LabGPIO LED0(1, 18);
LabGPIO LED1(1, 24);
LabGPIO LED2(1, 26);
LabGPIO LED3(2, 3);
OledTerminal oled;
uint8_t start;
uint8_t end;
uint8_t file_num;
ID3v1_t mp3_files[100];
int lineNum;
int lines;
char position[50] = "main";
int volumeLevel = 0;
int trebbleLevel = 0;
int bassLevel = 0;
ID3v1_t currentSong;
MenuState currentState = MenuState::kSongList;
VS1003 Decoder;

LabGPIO IR(0, 6);      //Port: 0_1
LabGPIO SW2(0, 30);
volatile uint32_t low = 0;
QueueHandle_t buttonQueueHandle;

void toggleLEDState(LabGPIO *gpio_pin);
void button0ISR();
void button1ISR();

void printMetaData(ID3v1_t mp3);
void printFilesToScreen(ID3v1_t mp3);
void printMultipleToScreen(ID3v1_t mp3_files[], int start_index, int end_index);7

void moveMenuDown(ID3v1_t mp3_files[]);
void moveMenuUp(ID3v1_t mp3_files[]);
FRESULT scan_files (char* path, char fileName[10][256], ID3v1_t mp3_files[], uint8_t* file_count);

void printMainMenu();
void backTracker();
void selectTracker();
void moveCursorUp();
void moveCursorDown();
void readIR_ISR();

void vMenuTask(void *pvParameter);

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

enum class IrOpcode : uint16_t
{
    kPower        = 0x0778,
    kSource       = 0x5728,
    kVolumeUp     = 0x7708,
    kVolumeDown   = 0x0F70,
    kMute         = 0x4738,
    kReplay       = 0x48B7,
    kRewind       = 0x6897,
    kPlayPause    = 0x28D7,
    kFastFoward   = 0x18E7,
    kSoundEffect  = 0x6F10,
    kSound        = 0x40BF,
    kBluetooth    = 0x5CA3,
    kLeft         = 0x06F9,
    kRight        = 0x46B9,
    kSoundControl = 0x32CD
}

enum class MenuState : int
{
  kSongList   = 0,
  kNowPlaying = 1,
  kSettings   = 3,
  kTrebble    = 4,
  kBass       = 5
}

//make lookup table for asktriskkkk

int main(void)
{
  line_num = 0;
  lines = 0;
  uint16_t result;
  float voltage;
  
  IR.Init();
  IR.EnableInterrupts();
  IR.SetAsInput();
  IR.AttachInterruptHandler(readIR_ISR, LabGPIO::Edge::kBoth);

  SW2.AttachInterruptHandler(readIR_ISR, LabGPIO::Edge::kBoth);
  RegisterIsr(GPIO_IRQn, readIR_ISR);

  Adc ADC(Adc::Channel::kChannel4);
  ADC.Initialize();
  ADC.BurstMode(true);

  LOG_INFO("Starting Oled...");
  oled_terminal.Initialize();

  Decoder.setVolume(5);  // init volume
  xTaskCreate(
          vMenuTask,          /* Function that implements the task. */
          "vMenuTask",        /* Text name for the task. */
          STACK_SIZE,                     /* Stack size in words, not bytes. */
          NULL,     /* Parameter passed into the task. */
          3,                              /* Priority at which the task is created. */
          NULL );                         /* Used to pass out the created task's handle. */
  buttonQueueHandle = xQueueCreate(10, sizeof(uint16_t));

  //Menu Start
  //**print song list - this is our starting point
  //***********

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


  start = 0;
  end = start + 4;
  // while(1)
  // {
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
      //continue;
  //}
}
void vMenuTask(void * pvParameter)
{
  uint16_t recievedButton = 0;
  if(xQueueReceive(buttonQueueHandle, &recievedButton, portMAX_DELAY))
  {
     switch(opcode)
    {
      case kPower:
          printf("Power\n");
          break;
      case kSource:
          toggleMenu();
          break;
      case kVolumeUp:
          // Volume Min = 0, Volume Max = 10
          volumeLevel++;
          break;
      case kVolumeDown:
          printf("Down\n");
          volumeLevel--;
          break;
      case kMute:
          printf("Mute\n");
          volumeLevel = 0;
          break;
      case kReplay:
          printf("Replay\n");
          //call function to start song
          break;
      case kRewind:
          printf("Rewind\n");
          break;
      case kPlayPause:
          printf("Play/Pause\n");
          //stop sending data
          break;
      case kFastFoward:
          printf("Fast Foward\n");
          break;
      case kSoundEffect:
          printf("Sound Effect\n");
          break;
      case kSound:
          printf("Sound\n");
          break;
      case kBluetooth:
          printf("Bluetooth\n");
          break;
      case kLeft:
          printf("Left\n");
          break;
      case kRight:
          printf("Right\n");
          break;
      case kSoundControl:
          printf("Sound Control\n");
          ToggleMenu();
          break;
      default:
          printf("%04X\n", opcode);
          break;
    }
  }
}
void ToggleMenu()
{
  if (currentMenu == MenuState::kSongList) //This is the state before button 
  {
    currentMenu = MenuState::kNowPlaying;
    oled.Clear();
    //print song info
    //No cursor
  }
  if (currentMenu == MenuState::kNowPlaying)
  {
    currentMenu = MenuState::kSettings;
    oled.Clear();
    //print Trebble and trebble level
    //No cursor
  }
  if (currentMenu == MenuState::kSettings)
  {
    currentMenu = MenuState::kSongList
    oled.Clear();
    //print Song List
    oled.SetCursor(0, 0);
  }
  if (currentMenu == MenuState::kTrebble)
  {
    currentMenu = MenuState::kBass;
    oled.Clear();
    oled.printf("Bass:\nLevel: ");
    if(bassLevel > 5) // Positive Level
    {
      for (int i = 0; i < 5; i++)
      {
        oled.printf(" ");
      }
      for(int i = 5; i < bassLevel; i++)
      {
        oled.printf("*");
      }
    }
    if(bassLevel < 5)
    {
      int spaces = bassLevel;
      for (int i = 0; i < bassLevel; i++)
      {
        oled.printf(" ");
      }
    }
  }
  if (currentMenu == MenuState::kBass)
  {
    currentMenu = MenuState::kTrebble;
    oled.Clear();
    oled.printf("Trebble:\nLevel: ");
  }
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

void readIR_ISR() 
{
    static uint8_t sample = 0;
    static uint16_t opcode = 0;
    static uint32_t start_time = 0;
    static bool read_opcode = false;
    static uint16_t prev_opcode = 0;
    static uint64_t prev_sent_time = 0;
    if(LPC_GPIOINT->IO0IntStatR & (1 << 6))
    {
        start_time = Uptime();
    }
    else
    {
        opcode = (opcode << 1) | (((Uptime() - start_time) >> 7) == 5);
        if(opcode == 0x9E00)
        {
            sample = 0;
        }
        if(++sample == 16)
        {
            if(opcode == prev_opcode)
            {
                if (prev_sent_time + DEBOUNCE_TIME > Uptime())
                {
                    prev_sent_time = Uptime();
                    xQueueSendFromIsr(&buttonQueueHandle, opcode);
                }
            }
            else
            {
                prev_sent_time = Uptime();
                xQueueSendFromIsr(&buttonQueueHandle, opcode);
            }
        }
    }
    LPC_GPIOINT->IO0IntClr |= (1 << 6); //Clear Int Flag
}

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