//==============================================      CHIP 8     =========================================================
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "Chip-8.h"
#include <ctype.h>

int scaleFactor;
int main(int argc, char *argv[]){
    
    configs userConfig;

    //defaults
    userConfig.SF = 20;
    userConfig.red = 0xFF;
    userConfig.green = 0xFF;
    userConfig.blue = 0xFF;
    userConfig.clockSpeed = INSTRUCTIONS_PER_CYCLE;

    //check command line arguments
    if (argc < 2){      
        SDL_Log("Program Usage: ./Chip-8.exe path/to/rom --SF [value](scalingFactor 1 - 4) --C [value](color R,G,B,Y,W) --Clck [value](500 <=clockspeed <= 1000).");
        exit(EXIT_FAILURE);
    }

    if (argc > 2){
        SDL_Log("%d", argc);
        for (int i = 2; i < argc; i+= 2){
            if (strcmp(argv[i], "-SF") == 0){
                scaleFactor = (int)strtol(argv[i + 1], NULL, 10);
                    userConfig.SF = scaleFactor;
                    continue;
            }

            if (strcmp(argv[i], "-C") == 0){
                if (strcmp(argv[i + 1], "R") == 0){
                    userConfig.red = 0xFF;
                    userConfig.green = 0x00;
                    userConfig.blue = 0x00;
                    continue;
                } 
                else if (strcmp(argv[i + 1], "G") == 0){
                    userConfig.red = 71;
                    userConfig.green = 234;
                    userConfig.blue = 92;
                    continue;
                }
                else if (strcmp(argv[i + 1], "B") == 0){
                    userConfig.red = 0x00;
                    userConfig.green = 0x00;
                    userConfig.blue = 0xFF;
                    continue;            
                }
                else if (strcmp(argv[i + 1], "Y") == 0){
                    userConfig.red = 0xFF;
                    userConfig.green = 0xFF;
                    userConfig.blue = 0x00;
                    continue;
                }
                else if (strcmp(argv[i + 1], "W") == 0){
                    userConfig.red = 0xFF;
                    userConfig.green = 0xFF;
                    userConfig.blue = 0xFF;
                    continue;
                }
                else {
                    SDL_Log("Invalid Color");
                    continue;
                }
            }

            if (strcmp(argv[i], "-Clck")){
                int clock = strtol(argv[i + 1], NULL, 10);
                if (clock >= 1 && clock <= 10){
                    userConfig.clockSpeed = 100 * clock;
                    continue;
                }
                else{
                    SDL_Log("Invalid Clock Speed");
                }
            }
            SDL_Log("%s", argv[i]);
        }       
    }
    
    //initiate SDL 
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Error in Initialising SDL");
        exit(EXIT_FAILURE);
    }
    
    Chip8 chip8;                    //struct chip8
    SDL_Window *chip8_Window;       //ptr to window
    SDL_Renderer *chip8_Renderer;   //ptr to renderer

    //initiate display
    initiateDisplay(&chip8_Window, &chip8_Renderer, &userConfig);

    //initiate emulator
    initiateEmulator(&chip8);



    //load rom
    chip8.romName = argv[1];
    loadRom(chip8.romName, &chip8);

    while (chip8.CHIP8_RUNNING)
    {
        
        //handle user input
        do{
        	userInputs(&chip8);
	} while (chip8.CHIP8_PAUSED);
	    
        //emulate instructions for 1 cycle (~700 inst / frame)
        for (int i = 0; i < userConfig.clockSpeed / 60; i++){ 
            //exe instructions
            executeOpcode(&chip8);
        }

        //delay to match clock speed of Chip-8 (~60hz) or ~16ms
        SDL_Delay(16);

        //draw 
        if (chip8.CHIP8_DRAWING){
            drawScreen(chip8_Renderer, &chip8, &userConfig);
            chip8.CHIP8_DRAWING = FALSE;
        }
        //update timers at 60hz
        updateTimers(&chip8);
    }
    
    //destroy window, render and free all memory
    destroyDisplay(chip8_Window, chip8_Renderer,&chip8);
    
    exit(EXIT_SUCCESS);
}

