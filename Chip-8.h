#ifndef CHIP8_H
#define CHIP8_H

#define TOTAL_MEMORY 4096
#define ROM_ENTERY_POINT 0x200 //Rom load location in memory 
#define TRUE 1
#define FALSE 0
#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define FONT_SET_SIZE 80  
#define INSTRUCTIONS_PER_CYCLE 700
#define STACK_SIZE 16
#define KEYPAD_SIZE 16
#define REGISTER_COUNT 16	//main registers

//globals

typedef struct chip8Config Chip8;
typedef struct cla_changes configs;

 struct chip8Config {        //info about Chip8
	char* romName;

	// memory 
	uint8_t ram[TOTAL_MEMORY];      // total ram for chip8
	uint16_t stack[STACK_SIZE];     // stack, 16 level of nesting (subroutine) , 12 bit for each address

	//registers 
    uint8_t V[REGISTER_COUNT];            //16  registers, [0 - 14] general purpose , [15] flag
	uint16_t PC_reg ;        // Program Counter
	uint16_t Index_reg;      // Index Counter
	uint8_t SP_reg;         // Stack Pointer

	//timers
	uint8_t delayTimer;      //delay timer count at 60hz until <0
	uint8_t soundTimer;      //sound timer count at 60hz until <0

	//display
	uint8_t display[SCREEN_HEIGHT][SCREEN_WIDTH];    //display is 64x32 pixels

	//input
	uint8_t keypad[KEYPAD_SIZE];     //Hex keypad for input

	//state
	 uint8_t CHIP8_DRAWING;      //flag for drawing at screen
 	uint8_t CHIP8_RUNNING;      //flag for quiting chip8
 	uint8_t CHIP8_PAUSED ;      //flag for pausing chip8

	//opcodes
	uint16_t opCode;    //current opcode
	uint16_t nnn;	    //address (12-bit)
	uint8_t nn;		    // 8-bit constant
	uint8_t n;		    // 4-bit nibble
	uint8_t x;		    //4-bit value, the lower 4 bits of the high byte of the instruction
	uint8_t y;		    //4-bit value, the upper 4 bits of the low byte of the instruction

} ;


//Font set for chip8
const static uint8_t fonts[FONT_SET_SIZE] = {
		
        0xF0, 0x90, 0x90, 0x90, 0xF0,   // 0
		0x20, 0x60, 0x20, 0x20, 0x70,   // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0,   // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0,   // 3
		0x90, 0x90, 0xF0, 0x10, 0x10,   // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0,   // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0,   // 6
		0xF0, 0x10, 0x20, 0x40, 0x40,   // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0,   // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0,   // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90,   // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0,   // B
		0xF0, 0x80, 0x80, 0x80, 0xF0,   // C
		0xE0, 0x90, 0x90, 0x90, 0xE0,   // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0,   // E
		0xF0, 0x80, 0xF0, 0x80, 0x80    // F

};

struct cla_changes{
	uint8_t SF;
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint16_t clockSpeed;
};

//function definitions

//chip-8
void executeOpcode(Chip8 *Chip8);
void userInputs(Chip8 *Chip8);
long loadRom(char *romName, Chip8 *Chip8);
void initiateEmulator(Chip8 *Chip8);
void updateTimers(Chip8 *Chip8, Mix_Chunk *sound);

//display
void drawScreen(SDL_Renderer *chip8_Render, Chip8 *Chip8, configs *userConfigs);
void initiateDisplay(SDL_Window** chip8_Window, SDL_Renderer** chip8_Renderer, configs *userConfigs);
void clearRenders(SDL_Renderer *chip8_Renderer,Chip8 *Chip8);
void destroyDisplay(SDL_Window* chip8_Window, SDL_Renderer* chip8_Renderer,Chip8 *Chip8);
//void initAudio(Mix_Chunk *sound);
#endif