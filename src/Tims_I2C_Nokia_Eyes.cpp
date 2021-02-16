
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
																	7	P6	= ?
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


*/

#include <Arduino.h>
#include <Tims_I2C_Nokia_Eyes.h>

/*
	Screen Buffers for 84 x 48 display.
	Using two buffers becouse I want options for two Eyes.

	Buffer.						  Sprite

								Sequential Bytes 76543210 76543210 7

		 BBBBBB			 B				  	     bbbbbbbb bbbbbbbb b    b
		 yyyyyy			 y				  	     iiiiiiii iiiiiiii i    i
		 tttttt			 t				  	     tttttttt tttttttt t    t
		 eeeeee			 e				  	     00000000 00111111 1
		 012345			 87				  	     01234567 89012345 6    Width-1
	bit0 *********.......*			   Bit0xWidth******** ******** ******
	bit1 *********		 *			   Bit1xWidth******** ******** ******
	bit2 *********		 *			   Bit2xWidth******** ******** ******
	bit3 *********		 *                          .
	bit4 *********		 *							.
	bit5 *********		 *							.
	bit6 *********		 *							.
	bit7 *********		 *			 ByteHeight-1******** ******** ******
			.
			.
			.
			.
	repeted screen height/8

	Note!
	Buffers have vertical   Bytes.
	Sprite  have horizontal Bytes, HI Bit first. Sprite are to be Single Bit Ordered (Indexed) arrays.

	Change the PCD8544_ScreenBuffer0? array, then update with: PCD8544_UpdateDisplay(which Eye)
*/
static byte NokiaEyes::Buffer01[BUFFER_SIZE] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x28, 0x30, 0x14, 0x22, 0xB0, 0xF9, 0x79, 0x2D, 0x01, 0x25, 0x28, 0x0A, 0x18, 0x74, 0x70, 0xC8, 0xD0, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0xF8, 0xF8, 0x18, 0x18, 0x18, 0x00, 0xE0, 0xE0, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0xC0, 0x80, 0x80, 0xC0, 0xE0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x00,
0xC0, 0x40, 0xC0, 0x00, 0xC0, 0x00, 0x00, 0x00, 0xC0, 0x40, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xF9, 0xF6, 0xFE, 0xF6, 0xFF, 0x71, 0x4F, 0x00, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0x80, 0x00, 0x01, 0x00, 0xF1, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x07, 0x00, 0x00, 0x00, 0x00,
0x07, 0x07, 0x00, 0x00, 0x07, 0x07, 0x00, 0x01, 0x07, 0x07, 0x01, 0x00, 0x07, 0x07, 0x00, 0x04, 0x00, 0x00, 0x07, 0x00, 0x05, 0x05, 0x07, 0x00, 0x07, 0x05, 0x07, 0x00, 0x07, 0x04, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0xCF, 0x3E, 0xF3, 0x9F, 0xF1, 0x0C, 0xE0, 0xE0, 0x00, 0x00, 0x01, 0x01, 0x0F, 0x0D, 0x0F, 0x89,
0x87, 0xDE, 0xF3, 0x13, 0x1F, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x0C, 0x1A, 0xA4, 0xC4,
0x81, 0x83, 0x09, 0x0C, 0x07, 0x1E, 0xF7, 0xDB, 0x0D, 0x3F, 0xBF, 0x9F, 0xFF, 0xA7, 0xFB, 0xD7, 0xFF, 0xDD, 0xDD, 0xFF, 0x0D, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x30, 0x30, 0x30, 0x30, 0xFF, 0xFF, 0x00, 0x00, 0xFF, 0xFF, 0x31, 0x31, 0x31, 0x31, 0x01, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF,
0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x03, 0x01, 0x01, 0x03, 0xFF, 0xFF, 0x00, 0xDE, 0x99, 0x33, 0x84, 0x04, 0x19, 0x70, 0x81, 0x02, 0x46, 0x85, 0x03, 0x07, 0xF6, 0x3A, 0x43, 0xC1, 0x00, 0x16, 0xE6, 0x60, 0x60, 0xC1, 0xFE, 0xC9, 0x7E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x0F, 0x00,
0x00, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x00, 0x00, 0x07, 0x0F, 0x0E, 0x0C, 0x0C, 0x0E, 0x0F, 0x07, 0x82, 0x04, 0x11, 0x03, 0x3B, 0x03, 0x0C, 0x1E, 0x6F, 0x60, 0x00, 0x01, 0x20, 0x40, 0x81, 0x82, 0x60, 0x67, 0x7C, 0x3E, 0x00, 0x67, 0x30, 0x01, 0x01, 0x0F, 0x0C, 0x78,
0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
static byte NokiaEyes::Buffer02[BUFFER_SIZE] = {
0x00, 0x00, 0x04, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0xFE, 0xFE, 0xFE, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x04, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFE, 0x3E, 0xFE, 0xF8, 0x80, 0x00, 0x00, 0x80, 0xF8, 0xFE, 0x3E, 0xFE, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xF8, 0xFC, 0x1E, 0x06, 0x06, 0x06, 0x06, 0x1C, 0xFC,
0xF8, 0xE0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xE0, 0xE0, 0x70, 0x38, 0xFC, 0xFE, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x03, 0x3F, 0xFF, 0xF8, 0xF8, 0xFF, 0x3F, 0x03, 0x00, 0xFF, 0xFF, 0xFF, 0x00,
0x00, 0x00, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x00, 0x3F, 0xFF, 0xFF, 0xC0, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFF, 0xFF, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00,
0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x01, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x03, 0x03, 0x03, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0xE0, 0xE0, 0x00, 0x00, 0x00, 0xE0, 0xE0, 0xE0, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0xE0, 0x60, 0x60, 0x60, 0x60,
0xE0, 0xC0, 0xC0, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x03, 0x07, 0x1F, 0x3F, 0xFC, 0xF0, 0xFC, 0x3E, 0x1F, 0x07, 0x03, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0x30, 0x30,
0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x3F, 0x3C, 0x38, 0x78, 0x78, 0x70, 0x70, 0xF1, 0xE3, 0xE3, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x3F, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x10, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3F, 0x3F, 0x3F, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x38, 0x10, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x1F, 0x1E, 0x38, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x1F, 0x0F, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/*
	Set the Addres of the I2C PCF8574 Module.
	(I2C Address)
*/
NokiaEyes::NokiaEyes(byte PCF8574_Address) {
	_PCF8574_I2C_Address = PCF8574_Address;
}

/*
	Auxillary switch.

	On	=	true.
	Off	=	false.
	(On or Off)
*/
void NokiaEyes::AUX(bool on) {

	if (on) { _PCD8544_PinState += AUX_HI; }
	else { _PCD8544_PinState += AUX_LOW; }

	Wire.beginTransmission(_PCF8574_I2C_Address);
	Wire.write(_PCD8544_PinState);
	Wire.endTransmission();

}

/*
	To SK9822

	Sends Byte (8 bit Value).
	(Byte)
*/
void NokiaEyes::SK9822_Value(uint8_t v) {

	Wire.beginTransmission(_PCF8574_I2C_Address);

	for (uint8_t i = 8; i > 0; i--) {

		if (bitRead(v, i - 1)) {
			Wire.write(SK9822_DATA_HI | HEAD_EYES_RESET_HI);
			Wire.write(SK9822_DATA_HI | HEAD_EYES_RESET_HI | HEAD_EYES_SCLK_HI);
		}
		else {
			Wire.write(SK9822_DATA_LOW | HEAD_EYES_RESET_HI);
			Wire.write(SK9822_DATA_LOW | HEAD_EYES_RESET_HI | HEAD_EYES_SCLK_HI);
		}

	}
	_PCD8544_PinState = _PCD8544_PinState | HEAD_EYES_RESET_HI;
	Wire.write(_PCD8544_PinState);
	Wire.endTransmission();

}
/*
To SK9822.
	Change the colour of the eyes. (2 x SK9822)

	SK9822_DATA_HI
	SK9822_DATA_LOW
	HEAD_EYES_SCLK_HI

		Brightness			= 0 to 31.
		Blue Duty Cycle		= 0 to 255.
		Green Duty Cycle	= 0 to 255.
		Red Duty Cycle		= 0 to 255.
		(Left Brightness, Left Blue, Left Green, Left Red, Right Brightness, Right Blue, Right Green, Right Red)
*/
void NokiaEyes::SK9822_EyeColour(uint8_t bl, uint8_t lb, uint8_t lg, uint8_t lr, uint8_t br, uint8_t rb, uint8_t rg, uint8_t rr) {

	uint8_t brightness_L = bl;
	if (brightness_L > 31) { brightness_L = 31; }
	brightness_L += B11100000;
	uint8_t brightness_R = br;
	if (brightness_R > 31) { brightness_R = 31; }
	brightness_R += B11100000;

	//	Start_Frame.
	SK9822_Value(0);
	SK9822_Value(0);
	SK9822_Value(0);
	SK9822_Value(0);
	//	Left Eye.
	SK9822_Value(brightness_L);
	SK9822_Value(lb);
	SK9822_Value(lg);
	SK9822_Value(lr);
	//	Right Eye.
	SK9822_Value(brightness_R);
	SK9822_Value(rb);
	SK9822_Value(rg);
	SK9822_Value(rr);
	// End_Frame.
	SK9822_Value(255);
	SK9822_Value(255);
	SK9822_Value(255);
	SK9822_Value(255);

}

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
void NokiaEyes::PCF8574_I2C_Write(uint8_t eye, byte data_or_command, uint8_t data) {

	Wire.beginTransmission(_PCF8574_I2C_Address);
	Wire.write((HEAD_EYES_SDIN_HI, (data & 128) != 0) | (data_or_command << 2) | HEAD_EYES_RESET_HI | eye);

	for (uint8_t i = 0; i < 8; i++) {

		Wire.write((HEAD_EYES_SDIN_HI, (data & 128) != 0) | (data_or_command << 2) | HEAD_EYES_RESET_HI | eye);
		Wire.write((HEAD_EYES_SDIN_HI, (data & 128) != 0) | (data_or_command << 2) | HEAD_EYES_RESET_HI | eye | HEAD_EYES_SCLK_HI);
		data <<= 1;
	}
	_PCD8544_PinState = _PCD8544_PinState | HEAD_EYES_RESET_HI;
	Wire.write(_PCD8544_PinState);
	Wire.endTransmission();

}
/*
	To PCD8544.

	Set Address Pointer.
	Move to a point on screen. (Byte)
	Eye = LEFT, RIGHT or BOTH.
	X = 0 to 83. Pixels horozontaly.
	Y = 0 to 5. Lines of text verticaly. Each line is 8 pixels.

	(Which Eye, X, Y)
*/
void NokiaEyes::PCD8544_GoToXY(uint8_t eye, int x, int y) {

	_CurrentEye = eye;
	_CurrentX = x;
	_CurrentY = y;

	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_X_ADDRESS | x);	//	X point.
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_Y_ADDRESS | y);	//	Y Point.
}
/*
	To PCD8544.

	Begin using the PCD8544
	(Which Eye)
*/
void NokiaEyes::PCD8544_Begin(uint8_t eye) {

	//I2C Reset the LCD to a known state
	Wire.beginTransmission(_PCF8574_I2C_Address);
	Wire.write(0xFF);
	Wire.write(HEAD_EYES_RESET_LOW);
	Wire.write(HEAD_EYES_RESET_HI);
	Wire.endTransmission();

	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_EXTENDED_INSTRUCTION);		//	Tell PCD8544 an extended command follows.
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_CONTRAST_DEFAULT);			//	Set LCD Vop (Contrast)
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_TEMPERATUR_COEFFICIENT_0);	//	Set Temp coefficent
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_SETBIAS_SYSTEM);			//	LCD bias mode 1:48 (try 0x13)
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_FUNCTION_SET);				//	We must send 0x20 before modifying the display control mode
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_NORMAL_MODE);				//	Set display control, normal mode.


}
/*
	To PCD8544.

	Update Screen with Screen Buffer.
	(Which Eye, Which Buffer)
*/
void NokiaEyes::PCD8544_UpdateDisplay(uint8_t eye, byte buffer[]) {

	PCD8544_GoToXY(eye, 0, 0);
	for (int i = 0; i < (PCD8544_WIDTH * PCD8544_HEIGHT / 8); i++) {

		PCF8574_I2C_Write(eye, PCD8544_DATA, buffer[i]);
	}
}
/*
	To PCD8544.

	Change the Contrast of the Screen.
	(Which Eye, 0 to 127)	50 is good starting point.
*/
void NokiaEyes::PCD8544_ChangeContrast(uint8_t eye, byte contrast) {

	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_EXTENDED_INSTRUCTION);			//	Tell PCD8544 an extended command follows.
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_CONTRAST_ADDRESS | contrast);	//	Change PCD8544 Vop (Contrast).
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_FUNCTION_SET);					//	Change to display mode.
}
/*
	To PCD8544.

	Inverts the Display.
	(Which Eye)
*/
void NokiaEyes::PCD8544_InvertDisplay(uint8_t eye) {

	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_EXTENDED_INSTRUCTION);					//	Tell PCD8544 an extended command follows.
	if (_PCD8544_inverted) { PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_NORMAL_MODE); }	//	Switch to Normal.
	else { PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_INVERTED); }						//	Switch to Inverted.
	PCF8574_I2C_Write(eye, PCD8544_COMMAND, PCD8544_FUNCTION_SET);							//	Change to display mode.
	_PCD8544_inverted = !_PCD8544_inverted;
}
/*
	Clears the Screen(s) but does not change the buffer.
	(Which Eye)
*/
void NokiaEyes::PCD8544_Clear(uint8_t eye) {

	this->PCD8544_GoToXY(eye, 0, 0);
	for (size_t i = 0; i < (PCD8544_WIDTH * PCD8544_HEIGHT / 8); i++) {
		this->PCF8574_I2C_Write(eye, PCD8544_DATA, 0x00);
	}
}

/*
	Set which buffer to use.
	True	= Buffer01 (Default).
	False	= Buffer02.
	(true or false)
*/
void NokiaEyes::Buffer_UseDefault(boolean defaultBuffer) {
	_Buffer_IsDefault = defaultBuffer;
}
/*
	Change a Buffer.

	Inverts the Display Buffer.
	(Which Buffer)
*/
void NokiaEyes::Buffer_InvertDisplay(byte buffer[]) {

	for (int i = 0; i < (PCD8544_WIDTH * PCD8544_HEIGHT / 8); i++) {

		buffer[i] = ~buffer[i] & 0xFF;
	}
}
/*
	Change a Buffer.

	Change a Pixel at X,Y, Coordinates.
	0 = White.
	1 = Black.
	(Which Buffer, X, Y, Black or White)
*/
void NokiaEyes::Buffer_ChangePixel(byte buffer[], byte x, byte y, boolean bw = BLACK) {

	if ((x >= 0) && (x < PCD8544_WIDTH) && (y >= 0) && (y < PCD8544_HEIGHT)) {	//	Make sure it is not out of bounds.
		byte shift = y % 8;

		if (bw)
			buffer[x + (y / 8) * PCD8544_WIDTH] |= 1 << shift;		//	If black, set the bit.
		else
			buffer[x + (y / 8) * PCD8544_WIDTH] &= ~(1 << shift);	//	If white clear the bit.
	}
}
/*
	Change a Buffer.

	Draw Line
	0 = White.
	1 = Black.
	(Which Buffer, From point XY, To point XY, Black or White)
*/
void NokiaEyes::Buffer_DrawLine(byte buffer[], int8_t x0, int8_t y0, int8_t x1, int8_t y1, uint16_t bw) {

	int8_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		Swap(x0, y0);
		Swap(x1, y1);
	}

	if (x0 > x1) {
		Swap(x0, x1);
		Swap(y0, y1);
	}

	int8_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int8_t err = dx / 2;
	int8_t ystep;

	if (y0 < y1) {
		ystep = 1;
	}
	else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			Buffer_ChangePixel(buffer, y0, x0, bw);
		}
		else {
			Buffer_ChangePixel(buffer, x0, y0, bw);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}
/*
	Change a Buffer.

	Draw a Vertical Line.
	0 = White.
	1 = Black.
	(Which Buffer, Top X, Top Y, Length, Black or White)
*/
void NokiaEyes::Buffer_DrawVerticalLine(byte buffer[], int8_t x, int8_t y, int8_t length, uint16_t bw) {

	Buffer_DrawLine(buffer, x, y, x, y + length - 1, bw);
}
/*
	Change a Buffer.

	Draw a Horizontal Line.
	0 = White.
	1 = Black.
	(Which Buffer, Left X, Left Y, Length, Black or White)
*/
void NokiaEyes::Buffer_DrawHorizontalLine(byte buffer[], int8_t x, int8_t y, int8_t length, uint16_t bw) {

	Buffer_DrawLine(buffer, x, y, x + length - 1, y, bw);
}
/*
	Change a Buffer.

	Draw a Rectangle.
	0 = White.
	1 = Black.
	(Which Buffer, Top Left X, Top Left Y, Width, Hight, Black or White)
*/
void NokiaEyes::Buffer_DrawRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t hight, uint16_t bw) {

	Buffer_DrawHorizontalLine(buffer, x, y, width, bw);
	Buffer_DrawHorizontalLine(buffer, x, y + hight - 1, width, bw);
	Buffer_DrawVerticalLine(buffer, x, y, hight, bw);
	Buffer_DrawVerticalLine(buffer, x + width - 1, y, hight, bw);
}
/*
	Change a Buffer.

	Draw a Filled Rectangle.
	0 = White.
	1 = Black.
	(Which Buffer, Top Left X, Top Left Y, Width, Height, Black or White)
*/
void NokiaEyes::Buffer_DrawFilledRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, uint16_t bw) {

	for (int8_t i = x; i < x + width; i++) {
		Buffer_DrawVerticalLine(buffer, i, y, height, bw);
	}
}
/*
	Change a Buffer.

	Draw a Rectangle with Rounded Corners.
	0 = White.
	1 = Black.
	(Which Buffer, Top Left X, Top Left Y, Width, Height, Radius, Black or White)
*/
void NokiaEyes::Buffer_DrawRoundRect(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, int8_t r, uint16_t bw) {

	Buffer_DrawHorizontalLine(buffer, x + r, y, width - 2 * r, bw); // Top
	Buffer_DrawHorizontalLine(buffer, x + r, y + height - 1, width - 2 * r, bw); // Bottom
	Buffer_DrawVerticalLine(buffer, x, y + r, height - 2 * r, bw); // Left
	Buffer_DrawVerticalLine(buffer, x + width - 1, y + r, height - 2 * r, bw); // Right

	Buffer_DrawRC(buffer, x + r, y + r, r, 1, bw);
	Buffer_DrawRC(buffer, x + width - r - 1, y + r, r, 2, bw);
	Buffer_DrawRC(buffer, x + width - r - 1, y + height - r - 1, r, 4, bw);
	Buffer_DrawRC(buffer, x + r, y + height - r - 1, r, 8, bw);
}
/*
	Change a Buffer.

	Draw a filled Rectangle with Rounded Corners.
	0 = White.
	1 = Black.
	(Which Buffer, Top Left X, Top Left Y, Width, Height, Radius, Black or White)
*/
void NokiaEyes::Buffer_DrawFilledRectRC(byte buffer[], int8_t x, int8_t y, int8_t width, int8_t height, int8_t r, uint16_t bw) {

	Buffer_DrawFilledRect(buffer, x + r, y, width - 2 * r, height, bw);

	Buffer_DrawFilledRectRC(buffer, x + width - r - 1, y + r, r, 1, height - 2 * r - 1, bw);
	Buffer_DrawFilledRectRC(buffer, x + r, y + r, r, 2, height - 2 * r - 1, bw);
}
/*
	Change a Buffer.

	Draw a Circle.
	0 = White.
	1 = Black.
	Thickness = 1 to Radius of circle (Use as Fill) max 255
	(Which Buffer, Centre X, Centre Y, Radius max 255, Black or White, Line Thickness)
*/
void NokiaEyes::Buffer_DrawCircle(byte buffer[], byte x0, byte y0, int8_t radius, boolean bw, int8_t lineThickness) {

	for (int r = 0; r < lineThickness; r++) {

		int8_t f = 1 - radius;
		int8_t ddF_x = 0;
		int8_t ddF_y = -2 * radius;
		int8_t x = 0;
		int8_t y = radius;

		Buffer_ChangePixel(buffer, x0, y0 + radius, bw);	//	Change Pixel.
		Buffer_ChangePixel(buffer, x0, y0 - radius, bw);	//	Change Pixel.
		Buffer_ChangePixel(buffer, x0 + radius, y0, bw);	//	Change Pixel.
		Buffer_ChangePixel(buffer, x0 - radius, y0, bw);	//	Change Pixel.

		while (x < y) {

			if (f >= 0) {
				y--;
				ddF_y += 2;
				f += ddF_y;
			}
			x++;
			ddF_x += 2;
			f += ddF_x + 1;

			Buffer_ChangePixel(buffer, x0 + x, y0 + y, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 - x, y0 + y, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 + x, y0 - y, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 - x, y0 - y, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 + y, y0 + x, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 - y, y0 + x, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 + y, y0 - x, bw);	//	Change Pixel.
			Buffer_ChangePixel(buffer, x0 - y, y0 - x, bw);	//	Change Pixel.
		}
		radius--;
	}
}
/*
	Change a Buffer.

	Draw a Rounded Corners. Used by Rectangle with Rounded Corners.

	0 = White.
	1 = Black.
	(Which Buffer, Position X, Position Y, Radius, Corner, Black or White)
*/
void NokiaEyes::Buffer_DrawRC(byte buffer[], int8_t x0, int8_t y0, int8_t r, uint8_t corner, uint16_t bw) {

	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;

		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (corner & 0x4) {
			Buffer_ChangePixel(buffer, x0 + x, y0 + y, bw);
			Buffer_ChangePixel(buffer, x0 + y, y0 + x, bw);

		}
		if (corner & 0x2) {
			Buffer_ChangePixel(buffer, x0 + x, y0 - y, bw);
			Buffer_ChangePixel(buffer, x0 + y, y0 - x, bw);

		}
		if (corner & 0x8) {
			Buffer_ChangePixel(buffer, x0 - y, y0 + x, bw);
			Buffer_ChangePixel(buffer, x0 - x, y0 + y, bw);

		}
		if (corner & 0x1) {
			Buffer_ChangePixel(buffer, x0 - y, y0 - x, bw);
			Buffer_ChangePixel(buffer, x0 - x, y0 - y, bw);

		}
	}
}
/*
	Change a Buffer.

	Draw a filled Rounded Corners. Used by Filled Rectangle with Rounded Corners.

	0 = White.
	1 = Black.
	(Which Buffer, Position X, Position Y, Radius, Corner, Delata, Black or White)
*/
void NokiaEyes::Buffer_DrawFilledRC(byte buffer[], int8_t x0, int8_t y0, int8_t r, uint8_t corner, int8_t delta, uint16_t bw) {

	int8_t f = 1 - r;
	int8_t ddF_x = 1;
	int8_t ddF_y = -2 * r;
	int8_t x = 0;
	int8_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;

		}

		x++;
		ddF_x += 2;
		f += ddF_x;

		if (corner & 0x1) {
			Buffer_DrawVerticalLine(buffer, x0 + x, y0 - y, 2 * y + 1 + delta, bw);
			Buffer_DrawVerticalLine(buffer, x0 + y, y0 - x, 2 * x + 1 + delta, bw);

		}
		if (corner & 0x2) {
			Buffer_DrawVerticalLine(buffer, x0 - x, y0 - y, 2 * y + 1 + delta, bw);
			Buffer_DrawVerticalLine(buffer, x0 - y, y0 - x, 2 * x + 1 + delta, bw);

		}
	}
}
/*
	Change a Buffer.

	Draw a Charecter.
	0 = White.
	1 = Black.
	Charecters limited to those in Charecter table, BasicChar5x8[][5]
	(Which Buffer, Charecter, Top Left X, Top Left Y, Black or White)
*/
void NokiaEyes::Buffer_DrawCharecter(byte buffer[], char character, byte x, byte y, boolean bw) {

	byte column;	//	Temp byte to store character's column bitmap.

	for (int i = 0; i < 5; i++) {	//	5 columns (x) per character.

		column = pgm_read_byte(&BasicChar5x8[character - 0x20][i]);

		for (size_t j = 0; j < 8; j++) {	//	8 rows (y) per character.

			if (column & (0x01 << j)) { Buffer_ChangePixel(buffer, x + i, y + j, bw); }	//	test bits to Change Pixel.
			else { Buffer_ChangePixel(buffer, x + i, y + j, !bw); }						//	Change Pixel.
		}
	}
}
/*
	Change a Buffer.

	Print String, uses Buffer_DrawCharecter, max 14 charecters.
	0 = White.
	1 = Black.
	(Which Buffer, String, Top Left X, Top Left Y, Black or White)
*/
void NokiaEyes::Buffer_Print(byte buffer[], char thisString[], byte x, byte y, boolean bw) {

	char* dString = thisString;

	while (*dString != 0x00) {	//	Untill String Terminator.

		Buffer_DrawCharecter(buffer, *dString++, x, y, bw);
		x += 5;

		for (int i = y; i < y + 8; i++) {

			Buffer_ChangePixel(buffer, x, i, !bw);
		}

		x++;

		if (x > (PCD8544_WIDTH - 5)) {	//	Go next line.

			x = 0;
			y += 8;
		}
	}
}
/*
		Change a Buffer.

	Insert small array from Program Memory into a ScreenBuffer.
	0 = White.
	1 = Black.
	Change the Background = True or False.
	(Which Buffer, A const horizontal 8bit array, Top Left X, Top Left Y, Width, Height, Black or White, Change Background?)
*/
void NokiaEyes::Buffer_DrawSprite_F(byte buffer[], const uint8_t* bitArray, int8_t x, int8_t y, int8_t sWidth, int8_t sHeight, uint16_t bw, uint16_t bg) {

	int8_t bitCount = 7;
	int16_t byteCount = 0;
	int8_t l = 0;
	int8_t p = 0;

	for (l = 0; l < sHeight; l++) {		//	Each Line.
		for (p = 0; p < sWidth; p++) {	//	Pixel on each line.
			if (x + p < PCD8544_WIDTH && y + l < PCD8544_HEIGHT) {	//	Is it in the Bounds of the screen?.

				if (bg) {	//	Change Forground and Background.
					if (bw) {
						Buffer_ChangePixel(buffer, x + p, y + l, bitRead(pgm_read_byte(&bitArray[byteCount]), bitCount));
					}
					else {
						Buffer_ChangePixel(buffer, x + p, y + l, !bitRead(pgm_read_byte(&bitArray[byteCount]), bitCount));
					}
				}
				else {	//	Change just the Forground.
					if (bitRead(pgm_read_byte(&bitArray[byteCount]), bitCount)) {
						Buffer_ChangePixel(buffer, x + p, y + l, bw);
					}
				}
			}
			bitCount--;
			if (bitCount < 0) {
				bitCount = 7;
				byteCount++;
			}

		}
	}
}
/*
		Change a Buffer.

	Insert small array from SRAM into a ScreenBuffer.
	0 = White.
	1 = Black.
	Change the Background = True or False.
	(Which Buffer, A const horizontal 8bit array, Top Left X, Top Left Y, Width, Height, Black or White, Change Background?)
*/
void NokiaEyes::Buffer_DrawSprite_S(byte buffer[], uint8_t bitArray[], int8_t x, int8_t y, int8_t sWidth, int8_t sHeight, uint16_t bw, uint16_t bg) {

	int8_t bitCount = 7;
	int16_t byteCount = 0;
	int8_t l = 0;
	int8_t p = 0;

	for (l = 0; l < sHeight; l++) {		//	Each Line.
		for (p = 0; p < sWidth; p++) {	//	Pixel on each line.
			if (x + p < PCD8544_WIDTH && y + l < PCD8544_HEIGHT) {	//	Is it in the Bounds of the screen?.

				if (bg) {	//	Change Forground and Background.
					if (bw) {
						Buffer_ChangePixel(buffer, x + p, y + l, bitRead(bitArray[byteCount], bitCount));
					}
					else {
						Buffer_ChangePixel(buffer, x + p, y + l, !bitRead(bitArray[byteCount], bitCount));
					}
				}
				else {	//	Change just the Forground.
					if (bitRead(bitArray[byteCount], bitCount)) {
						Buffer_ChangePixel(buffer, x + p, y + l, bw);
					}
				}
			}
			bitCount--;
			if (bitCount < 0) {
				bitCount = 7;
				byteCount++;
			}

		}
	}
}
/*
	Change a Buffer.

	Copy One Buffer to the Other Buffer.
	(Buffer to Copy From, Buffer to Copy To,)
*/
void NokiaEyes::Buffer_ToBuffer(byte buffer01[], byte buffer02[]) {

	for (int i = 0; i < (BUFFER_SIZE); i++) {
		buffer02[i] = buffer01[i];
	}
}
/*
	Change a Buffer.

	Fill Secreen Buffer with Black or White Pixels.
	0 = White.
	1 = Black.
	(Which Buffer, Black or White)
*/
void NokiaEyes::Buffer_FillScreen(byte buffer[], boolean bw) {

	for (int i = 0; i < (PCD8544_WIDTH * PCD8544_HEIGHT / 8); i++) {

		if (bw) { buffer[i] = 0xFF; }
		else { buffer[i] = 0x00; }
	}
}

/*
	Uses Arduino Print.
	Used to Print Text Direct to PCD8544 Nokia screen.
*/
size_t NokiaEyes::write(uint8_t character) {

	Serial.println(character, HEX);

	//if (_Buffer_IsDefault) { this->Buffer_DrawCharecter{Buffer01, character, _CurrentX, _CurrentY, 1}; }
	//else { this->Buffer_DrawCharecter{Buffer02, character, _CurrentX, _CurrentY, 1}; }

	if (character > 0x7F) { return 0; }	//	Does not exceed our Charecters Set.

	uint8_t Char5x8[5];			//	A Charecter 5x8.
	const uint8_t* charPointer;	//	One 8 bit column of a Charecter 5x8.

	if (character > 0x1F) {	//	Greater than the lower bounds of our charector set.
		memcpy_P(Char5x8, &BasicChar5x8[character - 0x20], sizeof(Char5x8));
		charPointer = Char5x8;

		for (uint8_t i = 0; i < 5; i++) {	//	Print Charecter.
			CheckCharPosition(_CurrentX, _CurrentY);
			this->PCF8574_I2C_Write(_CurrentEye, PCD8544_DATA, _TextInverted ? ~charPointer[i] : charPointer[i]);
			_CurrentX++;	//	Track position.
		}

		this->PCF8574_I2C_Write(_CurrentEye, PCD8544_DATA, _TextInverted ? 0xff : 0x00);// Charecter Spacing.
		_CurrentX++;	//	Track position.
	}

	if (character == 0x0A) {	//	Next Line.

		_CurrentX = 0;
		_CurrentY++;

		this->PCD8544_GoToXY(_CurrentEye, _CurrentX, _CurrentY);
	}

	return 1;

}
/*
	Check Printed charector will be within bounds of Screen.
*/
void NokiaEyes::CheckCharPosition(byte x, byte y) {

	if (x > PCD8544_WIDTH - 6) {	//	Screen width - Charecter width - Carecter Spaceing width.
		_CurrentX = 0;
		_CurrentY += 1;
		if (y > 5) {				//	Y = 0 to 5, lines of text each being 8 bits.
			_CurrentY = 0;
		}
	}
}
/*
	Invert Print Text.
	Used with write, to Print Text Direct to PCD8544 Nokia screen.
	(True or False)
*/
void NokiaEyes::TextInverted(boolean invert) { _TextInverted = invert; }
