
/*
	Tims_I2C_Nokia_Eyes v1.0.

	This library is ported from:
	Part of TIM-01 Robot Project.

	It is for an eye module I have designed for my robot.
	Details are here: https://robot-tim01.blogspot.com/2021/02/eyes.html

	By Tim Jackson
	Use Freely, Just give me credit where due :)

	Tim-01_Eyes.
	The Head section of my Robot.

	To make its Eyes I am using:
	Two Nokia LPH7366 screens, with an internal PCD8544 driver (9 pin).
	Connecting to the Nokia screens via an I2C, PCF8574 (Ground, Vdd, SDA, SCL to 8 Pin).
	I will be using 5 to 3.3 volt logic converters from the PCF8574 to the PCD8544 driver of the LPH7366.
	To give backlighting to the eyes I am using two SK9822 Intelegent RGB LEDs.

	My work is always, work in progress.
	Details of my project here: https://robot-tim01.blogspot.com/2020/04/robot-tim-01.html

	More projects here: https://timsblogplace.blogspot.com/2018/07/blog-post.html


	Nokia Display LPH7366: 84x48
			+--------------------------+
			|     1 2 3 4 5 6 7 8 9    |	         PCD8544 Driver Pins:
			|     # # # # # # # # #    |							1 .. VDD	3.3v (up to 7.4 mA)
			|  ===#=#=#=#=#=#=#=#=#==  |							2 .. SCLK	Clock.
			+--======================--+							3 .. SDIN	Serial In.
			|                          |							4 .. D/C	Data / Command.
			|                          |							5 .. SCE	(CS) Cable select (active low).
			|        rear view         |							6 .. Osc	Oscillator, attach to 3.3v rail.
			|   connector is visible   |							7 .. GND	Ground.
			|                          |							8 .. Vout	LCD supply voltage (from driver internal dc/dc converter).
			|  Display Name LPH7366    |							9 .. RES	Reset (active low).
			|                          |
			+--------------------------+


	PCF8574 Module:
			+----------------------------------------+		PCF8574 Pins:
			|		1 2 3 4 5 6 7 8 9				 |			 I2C: VCC, Ground, SDA, SCL.
			|                                   	 |			Pins:
			| VCC								VCC  |				1	P0	= SDIN
			| GND								GND	 |				2	P1	= SCLK (PCD8544) / CKI (SK9822)
			| SDA		component side			SDA	 |				3	P2	= D/C
			| SCL								SCL	 |				4	P3	= RES
			|										 |				5	P4	= SCE	(CS) Cable select Left  Nokia Display.
			+----------------------------------------+				6	P5	= SCE	(CS) Cable select Right Nokia Display.
																	7	P6	= SDI (SK9822)
																	8	P7	= ?
																	9	INT = PCF8574 interupt.

	SK9822 Intelegent RGB LED:
			+-----------------------+		SK9822 Pins:
			|                       |			Vcc	=	5v Power.
			| VCC				GND |			CKO	=	Clock Out.
			| CKO	Top View	SCI	|			SDO	=	Data  Out.
			| SDO				SDI	|			SDI	=	Data  In.
			|						|			SCI	=	Clock In.
			+-----------------------+			GND	=	Ground, Negative Power.

	I2C Addresses used in my project:
		0x20	=	32	MCP23017 in Chasis.
		0x21	=	33	MCP23017 in Torso.
		0x22	=	34	PCF8574  in Head.
		0x04	=	4	Arduino controlling Dive (ATMega 328).
		0x05	=	5	AtTiny controling Matrix (ATiny85).
		0x06	=	6	Arduino controlling Arms (ATMega 168).
		0x50	=	80	Address of the EEPROM ATMEL 24C256/128/64/32. (All 24C EEPROM address are 80 to 87)
		0x68	=	104	Address of the CLOCK DS1307.

No Serial
	Sketch uses 7484 bytes (23%) of program storage space. Maximum is 32256 bytes.
	Global variables use 1422 bytes (69%) of dynamic memory, leaving 626 bytes for local variables. Maximum is 2048 bytes.



*/

#ifndef TIMS_I2C_NOKIA_EYES_H
#define TIMS_I2C_NOKIA_EYES_H
#include <Arduino.h>
#include <Wire.h>


/*
	Font table:
		Characers are 8x5 (5 bytes).
		8x1 Space is done throgh code (1 byte).

		Note!
		Charecters are vertical Bytes. This is to make it compatable with other 8x5 fonts.

		This font has charecters from ASCII 32 to 127.
*/
static const byte BasicChar5x8[][5] PROGMEM = {
	0x00, 0x00, 0x00, 0x00, 0x00,	// 032 space 0x20
	0x00, 0x00, 0x5f, 0x00, 0x00,	// 033 !
	0x00, 0x07, 0x00, 0x07, 0x00,	// 034 "
	0x14, 0x7f, 0x14, 0x7f, 0x14,	// 035 #
	0x24, 0x2a, 0x7f, 0x2a, 0x12,	// 036 $
	0x23, 0x13, 0x08, 0x64, 0x62,	// 037 %
	0x36, 0x49, 0x55, 0x22, 0x50,	// 038 &
	0x00, 0x05, 0x03, 0x00, 0x00,	// 039 '
	0x00, 0x1c, 0x22, 0x41, 0x00,	// 040 (
	0x00, 0x41, 0x22, 0x1c, 0x00,	// 041 )
	0x14, 0x08, 0x3e, 0x08, 0x14,	// 042 *
	0x08, 0x08, 0x3e, 0x08, 0x08,	// 043 +
	0x00, 0x50, 0x30, 0x00, 0x00,	// 044 ,
	0x08, 0x08, 0x08, 0x08, 0x08,	// 045 -
	0x00, 0x60, 0x60, 0x00, 0x00,	// 046 .
	0x20, 0x10, 0x08, 0x04, 0x02,	// 047 /
	0x3e, 0x51, 0x49, 0x45, 0x3e,	// 048 0
	0x00, 0x42, 0x7f, 0x40, 0x00,	// 049 1
	0x42, 0x61, 0x51, 0x49, 0x46,	// 050 2
	0x21, 0x41, 0x45, 0x4b, 0x31,	// 051 3
	0x18, 0x14, 0x12, 0x7f, 0x10,	// 052 4
	0x27, 0x45, 0x45, 0x45, 0x39,	// 053 5
	0x3c, 0x4a, 0x49, 0x49, 0x30,	// 054 6
	0x01, 0x71, 0x09, 0x05, 0x03,	// 055 7
	0x36, 0x49, 0x49, 0x49, 0x36,	// 056 8
	0x06, 0x49, 0x49, 0x29, 0x1e,	// 057 9
	0x00, 0x36, 0x36, 0x00, 0x00,	// 058 :
	0x00, 0x56, 0x36, 0x00, 0x00,	// 059 ;
	0x08, 0x14, 0x22, 0x41, 0x00,	// 060 <
	0x14, 0x14, 0x14, 0x14, 0x14,	// 061 =
	0x00, 0x41, 0x22, 0x14, 0x08,	// 062 >
	0x02, 0x01, 0x51, 0x09, 0x06,	// 063 ?
	0x32, 0x49, 0x79, 0x41, 0x3e,	// 064 @
	0x7e, 0x11, 0x11, 0x11, 0x7e,	// 065 A
	0x7f, 0x49, 0x49, 0x49, 0x36,	// 066 B
	0x3e, 0x41, 0x41, 0x41, 0x22,	// 067 C
	0x7f, 0x41, 0x41, 0x22, 0x1c,	// 068 D
	0x7f, 0x49, 0x49, 0x49, 0x41,	// 069 E
	0x7f, 0x09, 0x09, 0x09, 0x01,	// 070 F
	0x3e, 0x41, 0x49, 0x49, 0x7a,	// 071 G
	0x7f, 0x08, 0x08, 0x08, 0x7f,	// 072 H
	0x00, 0x41, 0x7f, 0x41, 0x00,	// 073 I
	0x20, 0x40, 0x41, 0x3f, 0x01,	// 074 J
	0x7f, 0x08, 0x14, 0x22, 0x41,	// 075 K
	0x7f, 0x40, 0x40, 0x40, 0x40,	// 076 L
	0x7f, 0x02, 0x0c, 0x02, 0x7f,	// 077 M
	0x7f, 0x04, 0x08, 0x10, 0x7f,	// 078 N
	0x3e, 0x41, 0x41, 0x41, 0x3e,	// 079 O
	0x7f, 0x09, 0x09, 0x09, 0x06,	// 080 P
	0x3e, 0x41, 0x51, 0x21, 0x5e,	// 081 Q
	0x7f, 0x09, 0x19, 0x29, 0x46,	// 082 R
	0x46, 0x49, 0x49, 0x49, 0x31,	// 083 S
	0x01, 0x01, 0x7f, 0x01, 0x01,	// 084 T
	0x3f, 0x40, 0x40, 0x40, 0x3f,	// 085 U
	0x1f, 0x20, 0x40, 0x20, 0x1f,	// 086 V
	0x3f, 0x40, 0x38, 0x40, 0x3f,	// 087 W
	0x63, 0x14, 0x08, 0x14, 0x63,	// 088 X
	0x07, 0x08, 0x70, 0x08, 0x07,	// 089 Y
	0x61, 0x51, 0x49, 0x45, 0x43,	// 090 Z
	0x00, 0x7f, 0x41, 0x41, 0x00,	// 091 [
	0x02, 0x04, 0x08, 0x10, 0x20,	// 092 \ (keep this to escape the backslash)
	0x00, 0x41, 0x41, 0x7f, 0x00,	// 093 ]
	0x04, 0x02, 0x01, 0x02, 0x04,	// 094 ^
	0x40, 0x40, 0x40, 0x40, 0x40,	// 095 _
	0x00, 0x01, 0x02, 0x04, 0x00,	// 096 `
	0x20, 0x54, 0x54, 0x54, 0x78,	// 097 a
	0x7f, 0x48, 0x44, 0x44, 0x38,	// 098 b
	0x38, 0x44, 0x44, 0x44, 0x20,	// 099 c
	0x38, 0x44, 0x44, 0x48, 0x7f,	// 100 d
	0x38, 0x54, 0x54, 0x54, 0x18,	// 101 e
	0x08, 0x7e, 0x09, 0x01, 0x02,	// 102 f
	0x0c, 0x52, 0x52, 0x52, 0x3e,	// 103 g
	0x7f, 0x08, 0x04, 0x04, 0x78,	// 104 h
	0x00, 0x44, 0x7d, 0x40, 0x00,	// 105 i
	0x20, 0x40, 0x44, 0x3d, 0x00,	// 106 j
	0x7f, 0x10, 0x28, 0x44, 0x00,	// 107 k
	0x00, 0x41, 0x7f, 0x40, 0x00,	// 108 l
	0x7c, 0x04, 0x18, 0x04, 0x78,	// 109 m
	0x7c, 0x08, 0x04, 0x04, 0x78,	// 110 n
	0x38, 0x44, 0x44, 0x44, 0x38,	// 111 o
	0x7c, 0x14, 0x14, 0x14, 0x08,	// 112 p
	0x08, 0x14, 0x14, 0x18, 0x7c,	// 113 q
	0x7c, 0x08, 0x04, 0x04, 0x08,	// 114 r
	0x48, 0x54, 0x54, 0x54, 0x20,	// 115 s
	0x04, 0x3f, 0x44, 0x40, 0x20,	// 116 t
	0x3c, 0x40, 0x40, 0x20, 0x7c,	// 117 u
	0x1c, 0x20, 0x40, 0x20, 0x1c,	// 118 v
	0x3c, 0x40, 0x30, 0x40, 0x3c,	// 119 w
	0x44, 0x28, 0x10, 0x28, 0x44,	// 120 x
	0x0c, 0x50, 0x50, 0x50, 0x3c,	// 121 y
	0x44, 0x64, 0x54, 0x4c, 0x44,	// 122 z
	0x00, 0x08, 0x36, 0x41, 0x00,	// 123 {
	0x00, 0x00, 0x7f, 0x00, 0x00,	// 124 |
	0x00, 0x41, 0x36, 0x08, 0x00,	// 125 }
	0x10, 0x08, 0x08, 0x10, 0x08,	// 126 ~
	0x78, 0x46, 0x41, 0x46, 0x78	// 127 DEL 0x7F
};
/*
	An example of Sprites.
	Numbers_18x24 array has 10 Sprites, 0 to 9.
	Each 18 x 24 pixels.
	Iris_32x30 array has one sprite, the Iris I use for my Eyes.
	They can be used with: Buffer_DrawSprite_F for those in Program Memory, or with Buffer_DrawSprite_S for arrays in SRAM.

	Sprite(images) have horizontal Bytes, HI Bit first. Sprite are to be Single Bit Ordered (Indexed) arrays.
*/
static const byte Numbers_18x24[][54] PROGMEM = {
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xC0, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xC0, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xC0, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0xFF, 0xFC,
0x03, 0xC0, 0x01, 0xF8, 0x00, 0x7E, 0x00, 0x1F, 0x80, 0x03, 0xE0, 0x00, 0x78, 0x00, 0x1E, 0x00, 0x07, 0x80, 0x01, 0xE0, 0x00, 0x78, 0x00, 0x1E, 0x00, 0x07, 0x80, 0x01, 0xE0, 0x00, 0x78, 0x00, 0x1E, 0x00, 0x07, 0x80, 0x01, 0xE0, 0x00, 0x78, 0x00, 0x1E, 0x00, 0x0F, 0xC0, 0x0F, 0xFC, 0x07, 0xFF, 0x81, 0xFF, 0xE0, 0x3F, 0xF0,
0x07, 0xF8, 0x07, 0xFF, 0x83, 0xFF, 0xF1, 0xFF, 0xFE, 0x7C, 0x1F, 0xBE, 0x03, 0xFF, 0x80, 0x7D, 0xC0, 0x0F, 0x00, 0x03, 0xC0, 0x01, 0xF0, 0x00, 0xFC, 0x00, 0x7E, 0x0F, 0xFF, 0x8F, 0xFF, 0xC7, 0xFF, 0xE1, 0xFF, 0xE0, 0xFE, 0x00, 0x3E, 0x00, 0x0F, 0x80, 0x03, 0xE0, 0x00, 0xFF, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF, 0xFD, 0xFF, 0xFE,
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFC, 0x00, 0xF7, 0x00, 0x3C, 0x00, 0x0F, 0x00, 0x03, 0xC0, 0x01, 0xF0, 0x1F, 0xFC, 0x0F, 0xFE, 0x03, 0xFF, 0x80, 0x7F, 0xF0, 0x00, 0x7C, 0x00, 0x0F, 0x00, 0x03, 0xDC, 0x00, 0xFF, 0x00, 0x3F, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0xFF, 0xFC,
0x60, 0x00, 0x3C, 0x00, 0x0F, 0x00, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x3C, 0x00, 0x0F, 0x00, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x3C, 0x18, 0x0F, 0x0F, 0x03, 0xC3, 0xC0, 0xF0, 0xF0, 0x3C, 0x3C, 0x0F, 0x0F, 0x03, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x3C, 0x00, 0x0F, 0x00, 0x01, 0x80,
0x7F, 0xFF, 0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xF8, 0x00, 0x3C, 0x00, 0x0F, 0x00, 0x03, 0xE0, 0x00, 0xFF, 0xF8, 0x3F, 0xFF, 0x8F, 0xFF, 0xF1, 0xFF, 0xFE, 0x00, 0x0F, 0xC0, 0x01, 0xF0, 0x00, 0x3D, 0x80, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x80, 0x7F, 0xF0, 0x3E, 0x7F, 0xFF, 0x8F, 0xFF, 0xC1, 0xFF, 0xE0, 0x1F, 0xE0,
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFC, 0x00, 0xEF, 0x00, 0x03, 0xC0, 0x00, 0xF0, 0x00, 0x3E, 0x00, 0x0F, 0xFF, 0xFB, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x7F, 0xC0, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0xFF, 0xFC,
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xDC, 0x00, 0xF0, 0x00, 0x3C, 0x00, 0x1F, 0x00, 0x0F, 0xC0, 0x07, 0xE0, 0x03, 0xF0, 0x00, 0xF8, 0x00, 0x7C, 0x00, 0x1F, 0x00, 0x0F, 0x80, 0x03, 0xE0, 0x01, 0xF0, 0x00, 0x7C, 0x00, 0x3E, 0x00, 0x0F, 0x80, 0x07, 0xC0, 0x01, 0xF0, 0x00, 0xF8, 0x00, 0x1C, 0x00,
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xC0, 0x0F, 0xF0, 0x03, 0xFE, 0x01, 0xFF, 0xFF, 0xFD, 0xFF, 0xFE, 0x7F, 0xFF, 0xBF, 0xFF, 0xFF, 0x80, 0x7F, 0xC0, 0x0F, 0xF0, 0x03, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0xFF, 0xFC,
0x3F, 0xFF, 0x1F, 0xFF, 0xEF, 0xFF, 0xFF, 0xFF, 0xFF, 0xF8, 0x07, 0xFC, 0x00, 0xFF, 0x00, 0x3F, 0xC0, 0x0F, 0xF0, 0x03, 0xFE, 0x01, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 0xCF, 0xFF, 0xF0, 0x00, 0x7C, 0x00, 0x0F, 0x00, 0x03, 0xC0, 0x00, 0xF7, 0x00, 0x3F, 0xE0, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 0xF7, 0xFF, 0xF8, 0xFF, 0xFC
};
static const byte Iris_32x30[] PROGMEM = {
0x00, 0x0F, 0xF0, 0x00, 0x00, 0x74, 0x4F, 0x00, 0x01, 0xE4, 0x41, 0x80, 0x02, 0x34, 0xEF, 0x40, 0x0E, 0x14, 0x28, 0x30, 0x0F, 0x3C, 0xF8, 0x30, 0x12, 0x04, 0x23, 0xF8, 0x22, 0x84, 0x39, 0x24,
0x73, 0xB6, 0xE3, 0x82, 0x52, 0xDE, 0x26, 0x06, 0x50, 0x62, 0xFD, 0x2E, 0x9D, 0x36, 0xAB, 0x39, 0x85, 0x23, 0xA2, 0x21, 0xF4, 0xA3, 0xDF, 0xE1, 0x9F, 0xFE, 0x50, 0x01, 0x80, 0x0A, 0x7F, 0xE9,
0x87, 0xFB, 0xC5, 0x2F, 0x84, 0x45, 0xC4, 0xA1, 0x9C, 0xD5, 0x6C, 0xB9, 0x74, 0xBF, 0x46, 0x0A, 0x60, 0x64, 0x7B, 0x4A, 0x41, 0xC7, 0x6D, 0xCE, 0x24, 0x9C, 0x21, 0x44, 0x1F, 0xC4, 0x20, 0x48,
0x0C, 0x1F, 0x3C, 0xF0, 0x0C, 0x14, 0x28, 0x70, 0x02, 0xF7, 0x2C, 0x40, 0x01, 0x82, 0x27, 0x80, 0x00, 0xF2, 0x2E, 0x00, 0x00, 0x0F, 0xF0, 0x00
};


class NokiaEyes : public Print {

private:

	byte	_PCF8574_I2C_Address;		//	PCF8574 I2C Address.
	uint8_t	_PCD8544_PinState = 0;		//	Keep trak of the curret state of the pins on the PCD8544.
	boolean _PCD8544_inverted = false;	//	Used with Buffer, keep trak if screen is inverted. (There are two methods of inverting screen)
	boolean _TextInverted = false;		//	Used with write, to Print Text Direct to PCD8544 Nokia screen.
	boolean _Buffer_IsDefault = true;	//	True to use Buffer 1, false to use buffer 2.
	uint8_t _CurrentEye = 0;			//	Used to set which Eye(s) Text is printed using Arduino type Print.
	byte	_CurrentX = 0;
	byte	_CurrentY = 0;

	/*
		Eye LCD Driver = PCD8544
	*/
#define PCD8544_COMMAND	LOW
#define PCD8544_DATA	HIGH

#define WHITE					0
#define BLACK					1
#define PCD8544_NORMAL_MODE		0x0C
#define PCD8544_INVERTED		0x0D
#define PCD8544_X_ADDRESS		0x80
#define PCD8544_Y_ADDRESS		0x40
#define PCD8544_WIDTH			84
#define PCD8544_HEIGHT			48
#define BUFFER_SIZE				(PCD8544_WIDTH* PCD8544_HEIGHT / 8)


#define PCD8544_TEMPERATUR_COEFFICIENT_0	0x04
#define PCD8544_SETBIAS_SYSTEM				0x14
#define PCD8544_FUNCTION_SET				0x20
#define PCD8544_EXTENDED_INSTRUCTION		0x21
#define PCD8544_CONTRAST_ADDRESS			0x80
#define PCD8544_CONTRAST_DEFAULT			0xA6

	/*
		PCD8544
		1 .. VDD	3.3v (up to 7.4 mA)
		2 .. SCLK	Clock.
		3 .. SDIN	Serial In.
		4 .. D/C	Data / Command.
		5 .. SCE	(CS) Cable select (active low).
		6 .. Osc	Oscillator, attach to 3.3v rail.
		7 .. GND	Ground.
		8 .. Vout	LCD supply voltage (from driver internal dc/dc converter).
		9 .. RES	Reset (active low).

		I2C PCF8574
		HEAD_EYES_SCE_LEFT_HI	=	Left Eye	Off,	Right Eye	On.
		HEAD_EYES_SCE_RIGHT_HI	=	Right Eye	Off,	Left Eye	On.
		HEAD_EYES_SCE_BOTH_LOW	=	Both Eyes On.

	*/
#define HEAD_EYES_SDIN_HI		B00000001	//	PCF8574	pin		P0			HI = 1.
#define HEAD_EYES_SCLK_HI		B00000010	//	PCF8574 pin		P1			Read on HI.
#define HEAD_EYES_DC_HI			B00000100	//	PCF8574 pin		P2			HI command mode.
#define HEAD_EYES_RESET_HI		B00001000	//	PCF8574 pin		P3			HI normal mode.
#define HEAD_EYES_SCE_LEFT_HI	B00010000	//	PCF8574 pin		P4			LOW Active.
#define HEAD_EYES_SCE_RIGHT_HI	B00100000	//	PCF8574 pin		P5			LOW Active.
#define HEAD_EYES_SCE_BOTH_LOW	B00000000	//	PCF8574 pins	P4 and P5	LOW Active.
#define SK9822_DATA_HI			B01000000	//	PCF8574 pin		P6			Used for SK9822 Data in. Inteligent LED.
#define SK9822_DATA_LOW			B00000000	//	PCF8574 pin		P6			Used for SK9822 Data in. Inteligent LED.
#define AUX_HI					B10000000	//	PCF8574 pin		P7			Auxillary switch. HI = on.
#define AUX_LOW					B00000000	//	PCF8574 pin		P7			Auxillary switch. LOW = on.

#define HEAD_EYES_RESET_LOW		B00000000	//	LOW to reset.

#define Swap(a, b) { int8_t tp = a; a = b; b = tp; }	//	Swop two values for each other.

	//	Make simpler command for eyes.
#define RIGHT	B00010000
#define LEFT	B00100000
#define BOTH	B00000000


	/*
		To PCD8544.

		HEAD_EYES_SDIN_HI		B00000001
		HEAD_EYES_SCLK_HI		B00000010
		HEAD_EYES_DC_HI			B00000100
		HEAD_EYES_RESET_HI		B00001000
		HEAD_EYES_SCE_LEFT_HI	B11100000
		HEAD_EYES_SCE_RIGHT_HI	B11010000
		HEAD_EYES_SCE_BOTH_LOW	B00110000

		Send data to Nokia Display (PCD8544) via I2C (PCF8574).
		shiftOut(SDIN_Pin, DCLK_Pin, MSBFIRST, data); This how I want to send through PCF8574.
		(Which Eye, Is it Data or Command, Data sent).
	*/
	void PCF8574_I2C_Write(uint8_t eye, byte data_or_command, uint8_t data);
	/*
		Check Printed charector will be within bounds of Screen.
	*/
	void CheckCharPosition(byte x, byte y);

	/*
	To SK9822.

	Send data to RGB (SK9822) via I2C (PCF8574).
	shiftOut(SDIN_Pin, DCLK_Pin, MSBFIRST, data); This how I want to send through PCF8574.
	(Data sent).
*/
	void SK9822_I2C_Write(uint8_t data);
	/*
		To SK9822

		Sends Byte (8 bit Value).
		(Byte)
	*/
	void SK9822_Value(uint8_t v);

public:
	//	Buffers.
	static byte Buffer01[BUFFER_SIZE];
	static byte Buffer02[BUFFER_SIZE];

	/*
		Set the Addres of the I2C PCF8574 Module.
		(I2C Address)
	*/
	NokiaEyes(byte PCF8574_Address);

	/*
		Auxillary switch.
	*/
	void AUX(bool on);

	/*
		Change the colour of the eyes.
		2 x SK9822

		Brightness			= 0 to 31.
		Blue Duty Cycle		= 0 to 255.
		Green Duty Cycle	= 0 to 255.
		Red Duty Cycle		= 0 to 255.
		(Left Brightness, Left Blue, Left Green, Left Red, Right Brightness, Right Blue, Right Green, Right Red)
	*/
	void SK9822_EyeColour(uint8_t bl, uint8_t lb, uint8_t lg, uint8_t lr, uint8_t br, uint8_t rb, uint8_t rg, uint8_t rr);

	/*
		Uses Arduino Print.
		Used to Print Text Direct to PCD8544 Nokia screen.
	*/
	virtual size_t write(uint8_t);
	/*
		Invert Print Text.
		Used with write, to Print Text Direct to PCD8544 Nokia screen.
		(True or False)
	*/
	void TextInverted(boolean invert);

	/*
		Clears the Screen(s) but does not change the buffer.
		(Which Eye)
	*/
	void PCD8544_Clear(uint8_t eye);
	/*
		To PCD8544.

		Set Address Pointer.
		Move to a point on screen. (Byte)
		Eye = LEFT, RIGHT or BOTH.
		X = 0 to 83. Pixels horozontaly.
		Y = 0 to 5. Lines of text verticaly. Each line is 8 pixels.

		(Which Eye, X, Y)
	*/
	void PCD8544_GoToXY(uint8_t eye, int x, int y);
	/*
		To PCD8544.

		Begin using the PCD8544
		(Which Eye)
	*/
	void PCD8544_Begin(uint8_t eye);
	/*
		To PCD8544.

		Update Screen with Screen Buffer.
		(Which Eye, Which Buffer)
	*/
	void PCD8544_UpdateDisplay(uint8_t eye, byte buffer[]);
	/*
		To PCD8544.

		Change the Contrast of the Screen.
		(Which Eye, 0 to 127)	50 is good starting point.
	*/
	void PCD8544_ChangeContrast(uint8_t eye, byte contrast);
	/*
		To PCD8544.

		Inverts the Display.
		(Which Eye)
	*/
	void PCD8544_InvertDisplay(uint8_t eye);

	/*
		Set which buffer to use.
		True	= Buffer01 (Default).
		False	= Buffer02.
		(true or false)
	*/
	void Buffer_UseDefault(boolean defaultBuffer);
	/*
		Change a Buffer.

		Inverts the Display Buffer.
		(Which Buffer)
	*/
	void Buffer_InvertDisplay(byte buffer[]);
	/*
		Change a Buffer.

		Change a Pixel at X,Y, Coordinates.
		0 = White.
		1 = Black.
		(Which Buffer, X, Y, Black or White)
	*/
	void Buffer_ChangePixel(byte buffer[], byte x, byte y, boolean bw = BLACK);
	/*
		Change a Buffer.

		Draw Line
		0 = White.
		1 = Black.
		(Which Buffer, From point XY, To point XY, Black or White)
	*/
	void Buffer_DrawLine(byte buffer[], int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Vertical Line.
		0 = White.
		1 = Black.
		(Which Buffer, Top X, Top Y, Length, Black or White)
	*/
	void Buffer_DrawVerticalLine(byte buffer[], int8_t x, int8_t y, int8_t length, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Horizontal Line.
		0 = White.
		1 = Black.
		(Which Buffer, Left X, Left Y, Length, Black or White)
	*/
	void Buffer_DrawHorizontalLine(byte buffer[], int8_t x, int8_t y, int8_t length, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Rectangle.
		0 = White.
		1 = Black.
		(Which Buffer, Top Left X, Top Left Y, Width, Hight, Black or White)
	*/
	void Buffer_DrawRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t hight, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Filled Rectangle.
		0 = White.
		1 = Black.
		(Which Buffer, Top Left X, Top Left Y, Width, Height, Black or White)
	*/
	void Buffer_DrawFilledRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Rectangle with Rounded Corners.
		0 = White.
		1 = Black.
		(Which Buffer, Top Left X, Top Left Y, Width, Height, Radius, Black or White)
	*/
	void Buffer_DrawRoundRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, int8_t r, uint16_t bw);
	/*
		Change a Buffer.

		Draw a filled Rectangle with Rounded Corners.
		0 = White.
		1 = Black.
		(Which Buffer, Top Left X, Top Left Y, Width, Height, Radius, Black or White)
	*/
	void Buffer_DrawFilledRectRC(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, int8_t r, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Circle.
		0 = White.
		1 = Black.
		Thickness = 1 to Radius of circle (Use as Fill) max 255
		(Which Buffer, Centre X, Centre Y, Radius max 255, Black or White, Line Thickness)
	*/
	void Buffer_DrawCircle(byte buffer[], byte x0, byte y0, int8_t radius, boolean bw, int8_t lineThickness);
	/*
		Change a Buffer.

		Draw a Rounded Corners. Used by Rectangle with Rounded Corners.

		0 = White.
		1 = Black.
		(Which Buffer, Position X, Position Y, Radius, Corner, Black or White)
	*/
	void Buffer_DrawRC(byte buffer[], int8_t x0, int8_t y0, int8_t r, uint8_t corner, uint16_t bw);
	/*
		Change a Buffer.

		Draw a filled Rounded Corners. Used by Filled Rectangle with Rounded Corners.

		0 = White.
		1 = Black.
		(Which Buffer, Position X, Position Y, Radius, Corner, Delata, Black or White)
	*/
	void Buffer_DrawFilledRC(byte buffer[], int8_t x0, int8_t y0, int8_t r, uint8_t corner, int8_t delta, uint16_t bw);
	/*
		Change a Buffer.

		Draw a Charecter.
		0 = White.
		1 = Black.
		Charecters limited to those in Charecter table, ASCII[][5]
		(Which Buffer, Charecter, Top Left X, Top Left Y, Black or White)
	*/
	void Buffer_DrawCharecter(byte buffer[], char character, byte x, byte y, boolean bw);
	/*
		Change a Buffer.

		Print String, uses Buffer_DrawCharecter, max 14 charecters.
		0 = White.
		1 = Black.
		(Which Buffer, String, Top Left X, Top Left Y, Black or White)
	*/
	void Buffer_Print(byte buffer[], char thisString[], byte x, byte y, boolean bw);
	/*
			Change a Buffer.

		Insert small array from Program Memory into a ScreenBuffer.
		0 = White.
		1 = Black.
		Change the Background = True or False.
		(Which Buffer, A const horizontal 8bit array, Top Left X, Top Left Y, Width, Height, Black or White, Change Background?)
	*/
	void Buffer_DrawSprite_F(byte buffer[], const uint8_t* bitArray, int8_t x, int8_t y, int8_t sWidth, int8_t sHeight, uint16_t bw, uint16_t bg);
	/*
			Change a Buffer.

		Insert small array from SRAM into a ScreenBuffer.
		0 = White.
		1 = Black.
		Change the Background = True or False.
		(Which Buffer, A const horizontal 8bit array, Top Left X, Top Left Y, Width, Height, Black or White, Change Background?)
	*/
	void Buffer_DrawSprite_S(byte buffer[], uint8_t bitArray[], int8_t x, int8_t y, int8_t sWidth, int8_t sHeight, uint16_t bw, uint16_t bg);
	/*
		Change a Buffer.

		Copy One Buffer to the Other Buffer.
		(Buffer to Copy From, Buffer to Copy To,)
	*/
	void Buffer_ToBuffer(byte buffer01[], byte buffer02[]);
	/*
		Change a Buffer.

		Fill Secreen Buffer with Black or White Pixels.
		0 = White.
		1 = Black.
		(Which Buffer, Black or White)
	*/
	void Buffer_FillScreen(byte buffer[], boolean bw);

};
#endif
