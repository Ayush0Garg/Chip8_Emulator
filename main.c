//==============================================      CHIP 8     =========================================================
#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <SDL_mixer.h>
#include "Chip-8.h"
#include <ctype.h>


int main(int argc, char *argv[]){
    
    configs userConfig;
    Chip8 chip8;                    //struct chip8
    SDL_Window *chip8_Window;       //ptr to window
    SDL_Renderer *chip8_Renderer;   //ptr to renderer
    Mix_Chunk *chip8_sound;          //ptr to sound file chip8 play
    int scaleFactor;
    long romSize;

    //defaults
    userConfig.SF = 20;
    userConfig.red = 0xFF;
    userConfig.green = 0xFF;
    userConfig.blue = 0xFF;
    userConfig.clockSpeed = INSTRUCTIONS_PER_CYCLE;

    //check command line arguments
    if (argc < 2){      
        SDL_Log("Program Usage: ./Chip-8.exe path/to/rom -SF [value](scalingFactor 10 - 20) -C [value](color R,G,B,Y,W) -Clck [value](500 <=clockspeed <= 1000).");
        exit(EXIT_FAILURE);
    }

    if (argc > 2){
        SDL_Log("%d", argc);
        for (int i = 2; i < argc; i += 2){
            if (strcmp(argv[i], "-SF") == 0){
                scaleFactor = (int)strtol(argv[i + 1], NULL, 10);
                if (scaleFactor < 10 || scaleFactor > 20) scaleFactor = 15;
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
                if (clock <= 500 || clock >= 1000){
                    clock = 700;
                    userConfig.clockSpeed =  clock;
                    continue;
                }
            }
        }       
    }
    
    //initiate SDL 
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Error in Initialising SDL");
        exit(EXIT_FAILURE);
    }
    

    //initiate display
    initiateDisplay(&chip8_Window, &chip8_Renderer, &userConfig);

    //initiate audio
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) != 0){
        SDL_Log("Error in Audio");
        exit(EXIT_FAILURE);
    };

    chip8_sound = Mix_LoadWAV("beep-02.wav");

    if (chip8_sound == NULL){
        SDL_Log("Error in audio: buffer empty");
        exit(EXIT_FAILURE);
    };

    //initiate emulator
    initiateEmulator(&chip8);

    //load rom
    chip8.romName = argv[1];
    romSize = loadRom(chip8.romName, &chip8);
    SDL_Log("ROM loaded: %s",argv[1]);
    SDL_Log("ROM Size: %0.2f Kbs",(double) romSize / 1024);

    while (chip8.CHIP8_RUNNING)
    {
        
        //handle user input
        do{
        userInputs(&chip8);
        } while (chip8.CHIP8_RUNNING && chip8.CHIP8_PAUSED); //loop to prevent execution when paused
        
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
        updateTimers(&chip8, chip8_sound);
    }
    
    //destroy window, render and free all memory
    destroyDisplay(chip8_Window, chip8_Renderer,&chip8);
    
    exit(EXIT_SUCCESS);
}


/*int main(int argc, char* argv[]) {
	//init
	
	long romSize;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		SDL_Log("Error in Initialising SDL");
		exit(1);
	}

    Chip8 chip8;
	SDL_Window* chip8_Window;
	SDL_Renderer* chip8_Renderer;

	initiateDisplay(&chip8_Window, &chip8_Renderer);
	initiateEmulator(&chip8);

	//load Rom in memory [512] or 0x200 onwards

	chip8.romName = argv[1];
	romSize = loadRom(chip8.romName, &chip8);
	
	printf("Rom loaded\n");
	SDL_Log("0x%X ", chip8.PC_reg);
	//SDL_Log("ROM SIZE : %d", SIZE);
	//debug();
	while (chip8.CHIP8_RUNNING) {
		userInputs(&chip8);
		if (chip8.CHIP8_PAUSED) continue;

		for (int i = 0; i < 700 / 60; i++)
		executeOpcode(&chip8);
        SDL_Delay(16);
		if (chip8.CHIP8_DRAWING){
        	drawScreen(chip8_Renderer,&chip8);
			chip8.CHIP8_DRAWING = FALSE;
		}
		updateTimers(&chip8);
	}
}*/
