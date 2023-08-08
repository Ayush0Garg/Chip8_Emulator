#include <SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include "Chip-8.h"
#include <SDL_audio.h>

//fetch , decode and execute opcodes

/*
	* Opcodes are 16-byte long instructions 
	* tells emulator about next instruction to execute 
	* opcode  = two 8-byte long codes from  memory starting at 0x200 
*/


void executeOpcode(Chip8 *Chip8) {
	
	//fetch opcode
	uint16_t opCode;
	uint8_t firstByte = Chip8->ram[Chip8->PC_reg];
	uint8_t SecondByte = Chip8->ram[Chip8->PC_reg + 1];
	opCode = firstByte << 8 | SecondByte;
	Chip8->opCode = opCode;
	//components of opcode

	//decode opcode
	switch (opCode & 0xF000) {

		// opcode 0x000
		case 0x0000:					
			
			switch (opCode & 0x00FF) {
				
				//opcode 00E0 : clear screen
				case 0x00E0:		

					for (int i = 0; i < SCREEN_HEIGHT; i++) {
						for (int j = 0; j < SCREEN_WIDTH; j++) {
							Chip8->display[i][j] = FALSE;
						}
					}
					Chip8->PC_reg += 2; 
					break;

				//opcode 00EE : return from subroutine
				case 0x00EE:		

					Chip8->SP_reg--;
					Chip8->PC_reg = Chip8->stack[Chip8->SP_reg];
					Chip8->PC_reg += 2;
					break;

				default:

					SDL_Log("Unknown OPCODE 0x%X\n", opCode);
					break;
			}

			break;

		//opcode 1NNN : jump to address NNN
		case 0x1000:		

			Chip8->nnn = opCode & 0x0FFF;
			Chip8->PC_reg = Chip8->nnn;
			break;
		
		//opcode 2NNN : call subroutine at address NNN
		case 0x2000:		

			Chip8->nnn = opCode & 0x0FFF;
			Chip8->stack[Chip8->SP_reg] = Chip8->PC_reg;
			Chip8->SP_reg++;
			Chip8->PC_reg = Chip8->nnn;
			break;

		//opcode 3XNN : if V[x] == NN skip next instruction
		case 0x3000:		

			Chip8->nn = opCode & 0x00FF;
			Chip8->x = (opCode & 0x0F00) >> 8;
			
			if (Chip8->V[Chip8->x] == Chip8->nn) {
				Chip8->PC_reg += 4;
			}

			else Chip8->PC_reg += 2;
			break;

		//opcode 4XNN : if V[x] != NN skip next instruction
		case 0x4000:		

			Chip8->nn = opCode & 0x00FF;
			Chip8->x = (opCode & 0x0F00) >> 8;
			if (Chip8->V[Chip8->x] != Chip8->nn) {
				Chip8->PC_reg += 4;
			}
			else Chip8->PC_reg += 2;
			break;

		//opcode 5XY0 : if V[x] == V[y] skip next instruction
		case 0x5000:	

			Chip8->x = (opCode & 0x0F00) >> 8;
			Chip8->y = (opCode & 0x00F0) >> 4;
			if (Chip8->V[Chip8->x] == Chip8->V[Chip8->y]) {
				Chip8->PC_reg += 4;
				break;
			}
			Chip8->PC_reg += 2;
			break;

		//opcode 6XNN : store NN into register V[x]
		case 0x6000:	

			Chip8->nn = opCode & 0x00FF;
			Chip8->x = (opCode & 0x0F00) >> 8;
			Chip8->V[Chip8->x] = Chip8->nn;
			Chip8->PC_reg += 2;
			break;

		//opcode 7XNN : store V[x] = V[x] + NN
		case 0x7000:	
	
			Chip8->x = (opCode & 0x0F00) >> 8;
			Chip8->nn = opCode & 0x00FF;
			Chip8->V[Chip8->x] += Chip8->nn;
			Chip8->PC_reg += 2;
			break;
		
		//opcodes 8XYN
		case 0x8000:
			
			//opcode 8XY0 : set V[x] = V[y]
			switch (opCode & 0x000F) {		
				case 0x0000:	
	
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[Chip8->x] =  Chip8->V[Chip8->y];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY1 : set V[x] = V[x] | V[y]
				case 0x0001:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] | Chip8->V[Chip8->y];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY2 : set V[x] = V[x] & V[y]
				case 0x0002:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] & Chip8->V[Chip8->y];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY3 : V[x] = V[x] ^ V[y]
				case 0x0003:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] ^ Chip8->V[Chip8->y];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY4 : set V[x] = V[x] + V[y], set VF = 1 if (V[x] = V[y]) > 0xFF
				case 0x0004:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					uint16_t sum = (Chip8->V[Chip8->x] + Chip8->V[Chip8->y]);
					Chip8->V[0xF] = (sum > 255) ? 1 : 0;
					Chip8->V[Chip8->x] = sum & 0xFF;
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY5 : set V[x] = V[x] - V[y], set VF = 1 if V[x] > V[y] 	
				case 0x0005:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[0xF] = (Chip8->V[Chip8->x] > Chip8->V[Chip8->y]) ? 1 : 0;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] - Chip8->V[Chip8->y];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY6 : set V[x] = V[x] >> 1, set VF = 1 if V[x] have 1 in lsb
				case 0x0006:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[0xF] = Chip8->V[Chip8->x] & 0x1;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] >> 1;
					Chip8->PC_reg += 2;
					break;

				//opcode 8XY7 : set Vx = V[y] - V[x], set VF = 1 if V[x] < V[y]
				case 0x0007:	

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[0xF] = (Chip8->V[Chip8->y] > Chip8->V[Chip8->x]) ? 1 : 0;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->y] - Chip8->V[Chip8->x];
					Chip8->PC_reg += 2;
					break;

				//opcode 8XYE : set V[x] = V[x] << 1, set VF = 1 if V[x] have 1 as msb
				case 0x000E:	 

					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->y = (opCode & 0x00F0) >> 4;
					Chip8->V[0xF] = (Chip8->V[Chip8->x] >> 7) & 0x1;
					Chip8->V[Chip8->x] = Chip8->V[Chip8->x] << 1;
					Chip8->PC_reg += 2;
					break;

				default:
					SDL_Log("Unknown OPCODE 0x%X\n", opCode);
					break;
			}
			break;

		//opcode 9XY0 : skip next info if V[x] != V[y]
		case 0x9000:	

			Chip8->x = (opCode & 0x0F00) >> 8;
			Chip8->y = (opCode & 0x00F0) >> 4;
			if (Chip8->V[Chip8->x] != Chip8->V[Chip8->y]){ 
				Chip8->PC_reg += 4;
				break;
			}
			Chip8->PC_reg += 2;
			break;

		//opcode ANNN : set Index = NNN
		case 0xA000:	

			Chip8->nnn = (opCode & 0x0FFF);
			Chip8->Index_reg = Chip8->nnn;
			Chip8->PC_reg += 2;
			break;

		//opcode BNNN : jump to address NNN + V[0]
		case 0xB000:	

			Chip8->nnn = (opCode & 0x0FFF);
			Chip8->PC_reg = Chip8->nnn + Chip8->V[0];
			break;

		//opcode CXNN : set V[x] = random & NN, (0 <= random <= 255)
		case 0xC000:	
			/* 
				{random = rand() % (upper - lower + 1) + lower }
				rand() = return random no [0 , RAND_MAX] , RAND_MAX >= 32767	
			*/
			uint8_t random;
			Chip8->nn = opCode & 0x00FF;
			Chip8->x = (opCode & 0x0F00) >> 8;
			random = rand() % 256;
			Chip8->V[Chip8->x] = random & Chip8->nn;
			Chip8->PC_reg += 2;
			break;
		
		//opcode DXYN : draw N-height sprite at coords X,Y; Read from memory location I (index),
		//screen pixels are XOR'd with sprite bits, 
		//VF  = 1 if any display pixels are erased (collision)
		case 0xD000:
			Chip8->x = (opCode & 0x0F00) >> 8;
			Chip8->y = (opCode & 0x00F0) >> 4;
			Chip8->n = opCode & 0x000F;
			uint8_t Xcoordinate = Chip8->V[Chip8->x];
			uint8_t Ycoordinate = Chip8->V[Chip8->y];
			uint8_t spriteData;

			// Reset collision register to FALSE
			Chip8->V[0xF] = FALSE;
			for (int y = 0; y < Chip8->n; y++) {		//loop  for heigth N
				
				spriteData = Chip8->ram[Chip8->Index_reg + y];
				
				for (int x = 0; x < 8; x++) {				//loop for iterating over each bit
					
					if (((spriteData << x) & (0x80)) >> 7 != 0) {
						
						if (Chip8->display[y + Ycoordinate][x + Xcoordinate] == 1) {
							Chip8->V[0xF] = TRUE;
						}
						
						Chip8->display[y + Ycoordinate][x + Xcoordinate] ^= 1;
					}
				}
			}

			Chip8->CHIP8_DRAWING = TRUE;
			Chip8->PC_reg += 2;
			break;
		
		//opcodes EX
		case 0xE000:
			switch (opCode & 0x00FF) {

			//opcode EX9E : skip next instruction if key with value = V[x] is pressed	
			case 0x009E:
				Chip8->x = (opCode & 0x0F00) >> 8;
				if (Chip8->keypad[Chip8->V[Chip8->x]] != FALSE) {
					Chip8->PC_reg += 4;
				}
				else{
					Chip8->PC_reg += 2;
				}
				break;
				
			//opcode EXA1 : skip next instruction if key with value V[x] is not pressed
			case 0x00A1:
				Chip8->x = (opCode & 0x0F00) >> 8;
				if (Chip8->keypad[Chip8->V[Chip8->x]] == FALSE) {
					Chip8->PC_reg += 4;
					
				}
				else{
					Chip8->PC_reg += 2;
				}
				break;

			default:
				SDL_Log("Unknown OPCODE 0x%X\n", opCode);
				break;

			}
			break;

		//opcodes FX
		case 0xF000:

			switch (opCode & 0x00FF) {

				//opcode FX07 : set V[x] = delay timer	
				case 0x0007:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->V[Chip8->x] = Chip8->delayTimer;
					Chip8->PC_reg += 2;
					break;
				
				//opcode FX0A : await a key press and set V[x] = key pressed
				case 0x000A:
					Chip8->x = (opCode & 0x0F00) >> 8;
					uint8_t isKeyPressed = FALSE;
					for (int i = 0; i < KEYPAD_SIZE; i++){
						if(Chip8->keypad[i]){
							isKeyPressed = TRUE;
							Chip8->V[Chip8->x] = i;
							break;
						}
					}
					if (!isKeyPressed) break;
					Chip8->PC_reg += 2;
					break;
				
				//opcode FX15 : set delay timer = V[x]	
				case 0x0015:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->delayTimer = Chip8->V[Chip8->x];
					Chip8->PC_reg += 2;
					break;
				
				//opcode FX18 : set sound timer = V[x]
				case 0x0018:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->soundTimer = Chip8->V[Chip8->x];
					Chip8->PC_reg += 2;
					break;
				
				//opcode FX1E : set Index = Index + V[x]
				case 0x001E:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->Index_reg += Chip8->V[Chip8->x];
					Chip8->PC_reg += 2;
					break;

				//opcode FX29 : set Index = sprite data in V[x]
				case 0x0029:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->Index_reg = Chip8->V[Chip8->x] * 5; //5 byte long sprite
					Chip8->PC_reg += 2;
					break;

				//opcode FX33 : store 100's , 10's, 1's place digit of V[x] in memory locations 
				//[index], [index + 1], [index + 2] respectively 
				case 0x0033:
					Chip8->x = (opCode & 0x0F00) >> 8;
					Chip8->ram[Chip8->Index_reg] = Chip8->V[Chip8->x] / 100;
					Chip8->ram[Chip8->Index_reg + 1] = (Chip8->V[Chip8->x] / 10) % 10;
					Chip8->ram[Chip8->Index_reg + 2] = (Chip8->V[Chip8->x] % 100) % 10;
					Chip8->PC_reg += 2;
					break;
				
				//opcode FX55 : store V[ 0 - X ] in memory locations [index - (index + X)]
				case 0x0055:
					Chip8->x = (opCode & 0x0F00) >> 8;
                   for (uint8_t i = 0; i <= Chip8->x; i++)  {
                            Chip8->ram[Chip8->Index_reg+i] = Chip8->V[i];
                    }
					
					Chip8->PC_reg += 2;
                    break;

				//opcode FX65 : load from memory locations [index - (index + X)] into V[0 - X]
				case 0x0065:
					Chip8->x = (opCode & 0x0F00) >> 8;
                   for (uint8_t i = 0; i <= Chip8->x; i++)  {
                            Chip8->V[i] = Chip8->ram[Chip8->Index_reg+i]; 
                    }

					Chip8->PC_reg += 2;
                    break;

				default:
					SDL_Log("Unknown OPCODE 0x%X\n", opCode);
					break;
			}
			break;

		default:
			SDL_Log("Unknown OPCODE 0x%X\n", opCode);
			break;
	}

}

//handle user inputs like key press, exit , pause etc
void userInputs(Chip8 *Chip8) {
	SDL_Event chip8_events;
	while (SDL_PollEvent(&chip8_events)) {
		switch (chip8_events.type) {
		
		case SDL_QUIT:			// close emulator using close button
			Chip8->CHIP8_RUNNING = FALSE;
			break;

		case SDL_KEYDOWN:
			switch (chip8_events.key.keysym.sym) {
				
				case SDLK_SPACE:		// pause The emulation
					if (Chip8->CHIP8_PAUSED == TRUE) {
						Chip8->CHIP8_PAUSED = FALSE;
						SDL_Log("EMULATER UNPAUSED");
						break;
					}
					Chip8->CHIP8_PAUSED = TRUE;
					SDL_Log("EMULATER PAUSED");
					break;

				case SDLK_TAB:		// close the emulator
					SDL_Log("EMULATER CLOSED");
					Chip8->CHIP8_RUNNING = FALSE;
					break;

				case SDLK_BACKSPACE:	//reset emulator for current rom
					//TODO
					//resetEmulator(Chip8);
					break;

			/*
				Original Keyboard           Emulator Keyboard
				+ - + - + - + - +           + - + - + - + - +
				| 1 | 2 | 3 | C |           | 1 | 2 | 3 | 4 |
				+ - + - + - + - +           + - + - + - + - +
				| 4 | 5 | 6 | D |           | Q | W | E | R |
				+ - + - + - + - +           + - + - + - + - +
				| 7 | 8 | 9 | E |           | A | S | D | F |
				+ - + - + - + - +           + - + - + - + - +
				| A | 0 | B | F |           | Z | X | C | V |
				+ - + - + - + - +           + - + - + - + - +

				update keypad for each key press and release
			*/

				case SDLK_1: Chip8->keypad[0x1] = TRUE; break;
				case SDLK_2: Chip8->keypad[0x2] = TRUE; break;
				case SDLK_3: Chip8->keypad[0x3] = TRUE; break;
				case SDLK_4: Chip8->keypad[0xC] = TRUE; break;

				case SDLK_q: Chip8->keypad[0x4] = TRUE; break;
				case SDLK_w: Chip8->keypad[0x5] = TRUE; break;
				case SDLK_e: Chip8->keypad[0x6] = TRUE; break;
				case SDLK_r: Chip8->keypad[0xD] = TRUE; break;

				case SDLK_a: Chip8->keypad[0x7] = TRUE; break;
				case SDLK_s: Chip8->keypad[0x8] = TRUE; break;
				case SDLK_d: Chip8->keypad[0x9] = TRUE; break;
				case SDLK_f: Chip8->keypad[0xE] = TRUE; break;
				
				case SDLK_z: Chip8->keypad[0xA] = TRUE; break;
				case SDLK_x: Chip8->keypad[0x0] = TRUE; break;
				case SDLK_c: Chip8->keypad[0xB] = TRUE; break;
				case SDLK_v: Chip8->keypad[0xF] = TRUE; break;

				default: break;
			}

			break;

		case SDL_KEYUP:
			switch (chip8_events.key.keysym.sym){
				case SDLK_1: Chip8->keypad[0x1] = FALSE;  break;
				case SDLK_2: Chip8->keypad[0x2] = FALSE;  break;
				case SDLK_3: Chip8->keypad[0x3] = FALSE;  break;
				case SDLK_4: Chip8->keypad[0xC] = FALSE;  break;

				case SDLK_q: Chip8->keypad[0x4] = FALSE;  break;
				case SDLK_w: Chip8->keypad[0x5] = FALSE;  break;
				case SDLK_e: Chip8->keypad[0x6] = FALSE;  break;
				case SDLK_r: Chip8->keypad[0xD] = FALSE;  break;

				case SDLK_a: Chip8->keypad[0x7] = FALSE;  break;
				case SDLK_s: Chip8->keypad[0x8] = FALSE;  break;
				case SDLK_d: Chip8->keypad[0x9] = FALSE;  break;
				case SDLK_f: Chip8->keypad[0xE] = FALSE;  break;
				
				case SDLK_z: Chip8->keypad[0xA] = FALSE;  break;
				case SDLK_x: Chip8->keypad[0x0] = FALSE;  break;
				case SDLK_c: Chip8->keypad[0xB] = FALSE;  break;
				case SDLK_v: Chip8->keypad[0xF] = FALSE;  break;
			}
								
			break;
		}
	}
}


//load rom into memory from starting from location 512 or 0x200 onwards
long loadRom(char *romName, Chip8 *Chip8) {
	
	long romSize;
	uint8_t *romBuffer;
	FILE* rom = fopen(romName, "rb");
	
	//get rom size
	fseek(rom, 0, SEEK_END);
	romSize = ftell(rom);
	rewind(rom);
	
	//check if rom larger than memory 

	long maxRomSize = sizeof(Chip8->ram) - ROM_ENTERY_POINT;
	if (romSize > maxRomSize) {
		SDL_Log("%s rom is too large \n max rom size allowed: %l", romName, maxRomSize);
		exit(1);
	}

	romBuffer = (uint8_t *) malloc(sizeof(uint8_t) * romSize); //memory allocated to buffer
	if (romBuffer == NULL) {
		SDL_Log("Error in Rom loading\n%s", SDL_GetError());
		exit(1);
	}

	//load rom into buffer

	if (!fread(romBuffer, sizeof(uint8_t), romSize, rom)) {
		SDL_Log("Error in Rom loading\n%s", SDL_GetError());
		exit(1);
	 }

	//load rom in memory [512 or 0x200] onwards

	for (int i = 0; i < romSize; i++) {
		Chip8->ram[i + ROM_ENTERY_POINT] = romBuffer[i];
	}

	//free buffer and close rom file
	free(romBuffer);
	fclose(rom);
	
	return romSize; 
}

//initiate emulator
void initiateEmulator(Chip8 *Chip8) {	
	
	//Flags
	Chip8->CHIP8_PAUSED = FALSE;
	Chip8->CHIP8_RUNNING = TRUE;
	Chip8->CHIP8_DRAWING = FALSE;

	//clear display pixels
	for (int i = 0; i < SCREEN_HEIGHT; i++){        
		for (int j = 0; j < SCREEN_WIDTH; j++){
			Chip8->display[i][j] = 0;
		}
	}

	//clear ram
	for (int i = 0; i < 4096; i++) {
		Chip8->ram[i] = 0;
	}

	//initiate special registers
	Chip8->PC_reg = 0x200;
	Chip8->SP_reg = 0;
	Chip8->Index_reg = 0;
	
	//clear stack
	for (int i = 0; i < STACK_SIZE; i++) {
		Chip8->stack[i] = 0;
	}

	//clear registers V[0 - F]
	for (int i = 0; i < 16; i++) {
		Chip8->V[i] = 0;
	}

	//load font in memory [0 - 80]
	for (int i = 0; i < FONT_SET_SIZE; i++) {
		Chip8->ram[i] = fonts[i];
	}

	//initiate timers
	Chip8->soundTimer = 0;
	Chip8->delayTimer = 0;
	
	//keystate
	for (int i = 0; i < KEYPAD_SIZE; i++) {
		Chip8->keypad[i] = FALSE;
	}

}

//update timers if timers > 0
void updateTimers(Chip8 *Chip8){
	if (Chip8->delayTimer != 0)
	Chip8->delayTimer--;
	if (Chip8->soundTimer != 0) //TODO : add beep sound
	Chip8->soundTimer--;
}

//draw on screen at given coordinates

void drawScreen(SDL_Renderer *chip8_Render, Chip8 *Chip8, configs *userConfigs) {
	
    //rectangle dimensions
    SDL_Rect pixel;
	pixel.x = 0;
	pixel.y = 0;
	pixel.w = userConfigs->SF - 1;
	pixel.h = userConfigs->SF - 1;

	//loop over each display pixel
	for (uint8_t i = 0; i < SCREEN_HEIGHT ; i++) {
		for (uint8_t j = 0; j < SCREEN_WIDTH ; j++) {
			pixel.x = j * (userConfigs->SF);
			pixel.y = i * (userConfigs->SF);

			if (Chip8->display[i][j] == 0) {
				//draw bg color
				SDL_SetRenderDrawColor(chip8_Render, 0x00, 0x00, 0x00, 0xFF);
				SDL_RenderFillRect(chip8_Render, &pixel);
			}

			else{
				//draw fg color
				SDL_SetRenderDrawColor(chip8_Render, userConfigs->red, userConfigs->green, userConfigs->blue, 0xFF);
				SDL_RenderFillRect(chip8_Render, &pixel);
			}
		}
	}

    //show render at screen
	SDL_RenderPresent(chip8_Render);    
}


//create window and render 
void initiateDisplay(SDL_Window** chip8_Window, SDL_Renderer** chip8_Renderer, configs *userConfigs) {
	
	//window
	*(chip8_Window) = SDL_CreateWindow(
		"Chip8",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		SCREEN_WIDTH * userConfigs->SF,
		SCREEN_HEIGHT * userConfigs->SF,
		SDL_WINDOW_RESIZABLE |
		SDL_WINDOW_SHOWN |
		SDL_WINDOW_ALLOW_HIGHDPI
	);
	if (!*(chip8_Window)) SDL_Log("Error in Window Creation");

	//render
	*(chip8_Renderer) = SDL_CreateRenderer(
		*(chip8_Window),
		-1,
		SDL_RENDERER_ACCELERATED
	);
	if (!*(chip8_Renderer)) SDL_Log("Error in Render Creation");

}

//clear render
void clearRenders(SDL_Renderer *chip8_Renderer, Chip8 *Chip8){

	SDL_SetRenderDrawColor(chip8_Renderer, 0x0, 0x0, 0x0, 0xFF);
	SDL_RenderClear(chip8_Renderer);

	for (int i = 0; i < SCREEN_HEIGHT; i++){
		for (int j = 0; j < SCREEN_WIDTH; j++){
			Chip8->display[i][j] = 0;
		}
	}
}

//destroy window and render & stop all sdl processes
void destroyDisplay(SDL_Window* chip8_Window, SDL_Renderer* chip8_Renderer, Chip8 *Chip8){
    //TODO : close audio
    SDL_DestroyRenderer(chip8_Renderer);
    SDL_DestroyWindow(chip8_Window);
    SDL_Quit();
}

//audio
