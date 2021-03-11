
/*
* 1
	Tims_I2C_Eyes_Library_Test.ino 02/02/2021

	The library is ported from:
	Part of TIM-01 Robot Project.

	It is for an eye module I have designed for my robot.
	Details are here: https://robot-tim01.blogspot.com/2021/02/eyes.html

	By Tim Jackson
	Use Freely, Just give me credit where due :)

	Tim-01_Eyes.
	The Head section of my Robot.

	To make its Eyes I am using:
	Two Nokia LPH7366 screens, with an internal PCD8544 driver (9 pin).
	Connecting to the Nokia LPH7366 screens via an I2C, PCF8574 (Ground, Vdd, SDA, SCL to 8 Pin).
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



	There are several types of command in my library.
		If starts with Buffer_??,  this alters the buffer and will require the command PCD8544_UpdateDisplay to display any changes to the buffer.
		If starts with PCD8544_??, this is written direct to the displays.
		The Print command uses the Arduino Print library. It writes direct to the screens, the text position will need to be set first.

Sketch uses 14536 bytes (45%) of program storage space. Maximum is 32256 bytes.
Global variables use 1478 bytes (72%) of dynamic memory, leaving 570 bytes for local variables. Maximum is 2048 bytes.

*/
#include <Wire.h>
#include <Tims_I2C_Nokia_Eyes.h>


#define PCF8574_ADDRESS	0x22	//	34	PCF8574  in Head.	(Actual)

NokiaEyes EyeControl(PCF8574_ADDRESS);

static const byte Logo[] PROGMEM = {
0x00, 0x01, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x1C, 0xE4, 0x01, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1B, 0x83, 0xA0, 0x1F, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x7D, 0x98, 0x00, 0x30, 0xCE, 0x1C, 0x00, 0x00, 0x00, 0x00,
0x41, 0x81, 0xF0, 0x03, 0x0C, 0xF3, 0xC2, 0xE8, 0xE0, 0x00, 0x04, 0x30, 0x07, 0x00, 0x30, 0xCF, 0xFC, 0x2A, 0x8A, 0x00, 0x00, 0x47, 0x00, 0x50, 0x03, 0x0C, 0xDE, 0xC2, 0xEE, 0xA0, 0x00, 0x0B, 0xD0, 0x00, 0x80, 0x30, 0xCC, 0xCC, 0x22, 0xAA, 0x00, 0x00, 0xBD, 0x00, 0x08, 0x03, 0x0C, 0xCC, 0xD2, 0xEE, 0xE0, 0x00, 0x0D, 0x50, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFE,
0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xE0, 0x01, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x0E, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xC7, 0xF1, 0x80, 0x2A, 0xAA, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x3F, 0xB8, 0x02, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0xAF, 0x80, 0x2A, 0xAA, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xD4, 0x0E,
0xCC, 0x02, 0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x0D, 0x40, 0xF4, 0xC0, 0x14, 0x51, 0x48, 0x00, 0x00, 0x00, 0x00, 0x78, 0x00, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0xB0, 0x02, 0x00, 0x00, 0x0E, 0xA2, 0xE9, 0x77, 0x00, 0x01, 0xAB, 0x00, 0x60, 0x00, 0x00, 0x4B, 0x68, 0xD4, 0x20, 0x00, 0x0B, 0xB0, 0x1E, 0x00, 0x00, 0x04, 0xAA, 0xEF, 0x62, 0x00, 0x0E, 0xBF, 0xFF, 0xC0,
0x00, 0x00, 0x4A, 0xA2, 0xB4, 0x20, 0x00, 0x4F, 0x7F, 0x94, 0x00, 0x00, 0x04, 0xAA, 0xE9, 0x72, 0x80, 0xB1, 0xEF, 0xDF, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x35, 0xFA, 0xF8, 0x00, 0x00, 0x00, 0x00, 0x31, 0x84, 0xA8, 0x40, 0x77, 0xBF, 0x00, 0x00, 0x00, 0x00, 0x04, 0xA4, 0x4A, 0x82, 0x02, 0x6E, 0x90, 0x00, 0x00, 0x00, 0x00, 0x42, 0x44, 0xB0, 0x10, 0x30, 0xBF, 0x00, 0x00,
0x00, 0x00, 0x04, 0xA4, 0x4A, 0x83, 0xC3, 0x3F, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x31, 0x93, 0x24, 0x32, 0x9C, 0xC1, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x06, 0xF9, 0x8A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x36, 0x18, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x20, 0x10, 0x0E, 0x01, 0x86, 0x7F, 0x30, 0x18, 0x07, 0xE0, 0x73, 0x03, 0x10, 0xA0, 0x18, 0x67,
0xF3, 0x01, 0x80, 0xFF, 0x01, 0x10, 0x30, 0xEA, 0x01, 0x86, 0x60, 0x30, 0x18, 0x0C, 0x30, 0x41, 0x22, 0xCF, 0xE0, 0x18, 0x66, 0x03, 0x01, 0x80, 0xC3, 0x06, 0x89, 0x24, 0x9C, 0x01, 0x86, 0x60, 0x30, 0x18, 0x0C, 0x30, 0x3C, 0x92, 0x45, 0xC0, 0x1F, 0xE7, 0xE3, 0x01, 0x80, 0xC3, 0x09, 0xD8, 0x15, 0x44, 0x01, 0xFE, 0x7E, 0x30, 0x18, 0x0C, 0x30, 0x43, 0x80, 0x74, 0x60, 0x18, 0x66, 0x03,
0x01, 0x80, 0xC3, 0x00, 0xB8, 0x03, 0x07, 0x01, 0x86, 0x60, 0x30, 0x18, 0x0C, 0x30, 0x29, 0x00, 0x32, 0x10, 0x18, 0x66, 0x03, 0x01, 0x80, 0xC3, 0x00, 0x8C, 0x8F, 0x61, 0x01, 0x86, 0x60, 0x30, 0x18, 0x0C, 0x30, 0x00, 0xC4, 0xE4, 0x18, 0x18, 0x67, 0xF3, 0xF9, 0xFC, 0xFF, 0x08, 0x00, 0x30, 0x00, 0x81, 0x86, 0x7F, 0x3F, 0x9F, 0xC7, 0xE0
};
byte glyph_5x8[] = {// A Smiley
  0x00, 0x14, 0x0F, 0xB8, 0x00
};

bool PCD8544_inverted = false;
unsigned long NextInterval1 = millis() + 1000;	//	Timing 1.
unsigned long NextInterval2 = millis() + 2000;	//	Timing 2.
unsigned long NextInterval3 = millis() + 500;	//	Timing 3.
unsigned long Seconds = 0;	//	Counter.
boolean Toggle1 = false;
boolean Toggle2 = false;

//	Coordinates.
byte CurrentX = (PCD8544_WIDTH / 2) - 16;
byte CurrentY = (PCD8544_HEIGHT / 2) - 15;

//Tests
uint8_t Eye_Left_Red = 200;
uint8_t Eye_Left_Green = 200;
uint8_t Eye_Left_Blue = 200;
uint8_t Eye_Right_Red = 200;
uint8_t Eye_Right_Green = 200;
uint8_t Eye_Right_Blue = 200;
uint8_t count = 0;

void setup() {

	Serial.begin(115200);
	Wire.begin();
	Wire.setClock(400000);

	EyeControl.PCD8544_Begin(BOTH);
	EyeControl.PCD8544_ChangeContrast(LEFT, 85);
	EyeControl.PCD8544_ChangeContrast(RIGHT, 55);

	StartUp();
}
void StartUp() {

	EyeControl.SK9822_EyeColour(30, 255, 255, 255, 30, 255, 255, 255);

	EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer01);
	EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer02);
	delay(3000);
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE); // Clear Screen
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);

	randomSeed(analogRead(A0));	//	For Random generator, uses noise on pin to pick a start number.

	TestPattern();
	delay(1000);
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE); // Clear Screen

	CountDown();

	TestGraphics();

	//TestPrintText();

	EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, BLACK, true);
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);

	//Rainbow();


}
void loop() {

	//TestI2C();	//	This is for testing the PCF8574 Module only.

	//TestText();

	TestIris();

	if (millis() > NextInterval3) {
		BGRW();
		NextInterval3 = millis() + 3000;

		//EyeControl.AUX(Toggle2);
		//Toggle2 = !Toggle2;
	}

}

/*
	Display Eye Iris randomly moving, Blinking periodically.
*/
void TestIris() {

	if (millis() > NextInterval1) {

		EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, WHITE, false);
		CurrentX = random(0, PCD8544_WIDTH - 32);
		CurrentY = random(0, PCD8544_HEIGHT - 30);
		EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, BLACK, true);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);

		NextInterval1 = millis() + random(2000, 5000);
	}

	if (millis() > NextInterval2) { //	Blink.

		int y = 0;

		EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, WHITE, false);
		CurrentX = (PCD8544_WIDTH / 2 - 16);
		CurrentY = PCD8544_HEIGHT - 35;
		EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, BLACK, true);

		for (y = 0; y < 6; y++) { //	Blink Close.
			EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, (y * 6), PCD8544_WIDTH, (6), BLACK);
			EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, PCD8544_HEIGHT - (y * 2), PCD8544_WIDTH, 2, BLACK);
			EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
		}

		for (y = 6; y > 0; y--) { //	Blink Open.
			EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, ((y - 1) * 6), PCD8544_WIDTH, (6), WHITE);
			EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, PCD8544_HEIGHT - (y * 2), PCD8544_WIDTH, 2, WHITE);
			EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Iris_32x30, CurrentX, CurrentY, 32, 30, BLACK, false);
			EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
		}
		NextInterval2 = millis() + random(30000, 80000);
		NextInterval1 = millis() + 1000;
	}
}
/*
	This is for testing the PCF8574 Module only.
	Connect low powered LED's with reistors to the eight output pins and ground.
	Four LED's should light LED's alternatly.
*/
void TestI2C() {

	Wire.beginTransmission(PCF8574_ADDRESS);
	Wire.write(B01010101);
	Wire.endTransmission();
	delay(2000);
	Wire.beginTransmission(PCF8574_ADDRESS);
	Wire.write(B10101010);
	Wire.endTransmission();
	delay(2000);

}
/*
	Test Text output.
	And a small Sprite
*/
void TestText() {
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE); // Clear Screen

	// Write the counter on the second line...
	int startColumn = 0;
	if (Seconds < 10000000000000000) { startColumn = float(42 - (3 * 16)); }
	if (Seconds < 1000000000000000) { startColumn = float(42 - (3 * 15)); }
	if (Seconds < 100000000000000) { startColumn = float(42 - (3 * 14)); }
	if (Seconds < 10000000000000) { startColumn = float(42 - (3 * 13)); }
	if (Seconds < 1000000000000) { startColumn = float(42 - (3 * 12)); }
	if (Seconds < 100000000000) { startColumn = float(42 - (3 * 11)); }
	if (Seconds < 10000000000) { startColumn = float(42 - (3 * 10)); }
	if (Seconds < 1000000000) { startColumn = float(42 - (3 * 9)); }
	if (Seconds < 100000000) { startColumn = float(42 - (3 * 8)); }
	if (Seconds < 10000000) { startColumn = float(42 - (3 * 7)); }
	if (Seconds < 1000000) { startColumn = float(42 - (3 * 6)); }
	if (Seconds < 100000) { startColumn = float(42 - (3 * 5)); }
	if (Seconds < 10000) { startColumn = float(42 - (3 * 4)); }
	if (Seconds < 1000) { startColumn = float(42 - (3 * 3)); }
	if (Seconds < 100) { startColumn = float(42 - (3 * 2)); }
	if (Seconds < 10) { startColumn = float(42 - (3 * 1)); }

	char secondsString[14];
	sprintf(secondsString, "%lu", Seconds);

	EyeControl.Buffer_Print(EyeControl.Buffer01, "Hello, World!", 5, 0, BLACK);
	EyeControl.Buffer_Print(EyeControl.Buffer01, secondsString, startColumn, 8, BLACK);
	EyeControl.Buffer_Print(EyeControl.Buffer01, "Seconds.", 20, 16, BLACK);

	if (millis() > NextInterval1) {

		//glyph
		EyeControl.Buffer_DrawSprite_S(EyeControl.Buffer01, glyph_5x8, CurrentX, CurrentY, 5, 8, WHITE, false);
		CurrentY = 24;
		if (Toggle1) { CurrentX = (PCD8544_WIDTH / 2) + 10; }
		else { CurrentX = (PCD8544_WIDTH / 2) - 15; }
		EyeControl.Buffer_DrawSprite_S(EyeControl.Buffer01, glyph_5x8, CurrentX, CurrentY, 5, 8, BLACK, true);

		int valOver = millis() - NextInterval1;
		NextInterval1 = millis() + 1000 - valOver;
		Seconds++;
		Toggle1 = !Toggle1;
	}
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
}
/*
	Display A Black and White squares.
	To set contrast.
	84x48
	+------------------------------------------------------------------------------------+
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|                              ************************
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|**********          **********                        **********          **********
	|          **********                                            **********
	|          **********                                            **********
	+------------------------------------------------------------------------------------+
	*/
void TestPattern() {

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE); // Clear Screen
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, 0, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 20, 0, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 54, 0, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 74, 0, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 30, 10, 24, 24, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0, 34, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 20, 34, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 54, 34, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 74, 34, 10, 10, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 10, 44, 10, 4, BLACK);
	EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 64, 44, 10, 4, BLACK);
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);

}
/*
	Test various Outputs.
*/
void TestGraphics() {

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.
	for (size_t i = 1; i < 22; i++) {
		EyeControl.Buffer_DrawRoundRect(
			EyeControl.Buffer01,
			(PCD8544_WIDTH / 2) - (i * 2) + 1,
			(PCD8544_HEIGHT / 2) - i,
			(i * 4) - 2,
			i * 2,
			i,
			BLACK
		);
		EyeControl.Buffer_DrawRoundRect(
			EyeControl.Buffer01,
			(PCD8544_WIDTH / 2) - (i * 2),
			(PCD8544_HEIGHT / 2) - i,
			i * 4,
			i * 2,
			i,
			BLACK
		);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	}
	for (size_t i = 22; i > 0; i--) {
		EyeControl.Buffer_DrawRoundRect(EyeControl.Buffer01, (PCD8544_WIDTH / 2) - (i * 2) + 1, (PCD8544_HEIGHT / 2) - i, (i * 4) - 2, i * 2, i, WHITE);
		EyeControl.Buffer_DrawRoundRect(EyeControl.Buffer01, (PCD8544_WIDTH / 2) - (i * 2), (PCD8544_HEIGHT / 2) - i, i * 4, i * 2, i, WHITE);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	}

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.
	EyeControl.Buffer_FillScreen(EyeControl.Buffer02, WHITE);	//	Clear display.

	EyeControl.Buffer_Print(EyeControl.Buffer01, "Random", 24, (PCD8544_HEIGHT / 2) - 4, BLACK);	//	50 Random cross using EyeControl.Buffer_ChangePixel.
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	for (int i = 0; i < 50; i++) {
		byte CurrentX = random(0, PCD8544_WIDTH);
		byte CurrentY = random(0, PCD8544_HEIGHT);
		for (size_t i = 0; i < 5; i++) {
			if (Toggle1) {
				EyeControl.Buffer_ChangePixel(EyeControl.Buffer01, CurrentX + i, CurrentY);
				EyeControl.Buffer_ChangePixel(EyeControl.Buffer01, CurrentX + 2, CurrentY - 2 + i);
			}
			else {
				EyeControl.Buffer_ChangePixel(EyeControl.Buffer02, CurrentX + i, CurrentY);
				EyeControl.Buffer_ChangePixel(EyeControl.Buffer02, CurrentX + 2, CurrentY - 2 + i);
			}
		}
		if (Toggle1) {
			EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer01);
		}
		else {
			EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer02);
		}

		Toggle1 = !Toggle1;
	}
	delay(500);
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);  //  Clear display.
	EyeControl.Buffer_FillScreen(EyeControl.Buffer02, WHITE); //  Clear display.

	for (int i = 0; i < 5; i++) {	//	Invert display 5 times.
		EyeControl.PCD8544_InvertDisplay(BOTH);	//	Invert.
		EyeControl.PCD8544_InvertDisplay(BOTH);	//	Change Back.
	}

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.
	int x0 = PCD8544_WIDTH / 2;
	int y0 = PCD8544_HEIGHT / 2;
	for (float i = 0; i < 2 * PI; i += PI / 8) {	//	Draw Raial Lines.
		const int lineLength = 24;
		int x1 = x0 + lineLength * sin(i);
		int y1 = y0 + lineLength * cos(i);
		EyeControl.Buffer_DrawLine(EyeControl.Buffer01, x0, y0, x1, y1, BLACK);
		EyeControl.Buffer_DrawLine(EyeControl.Buffer02, x0, y0, x1, y1, WHITE);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	}
	delay(1000);
	EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer02);
	delay(1000);
	for (size_t i = 0; i < 2; i++) {
		EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer02);
		EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer01);
		delay(1000);
		EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer01);
		EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer02);
		delay(1000);
	}

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.

	for (int x = 0; x < PCD8544_WIDTH; x += 4) { // Swipe right black
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, x, 0, 4, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer02, PCD8544_WIDTH - x, 0, 4, PCD8544_HEIGHT, BLACK);
		EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer01);
		EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer02);
	}
	for (int x = 0; x < PCD8544_WIDTH; x += 4) { // Swipe right white
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, x, 0, 4, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer02, PCD8544_WIDTH - 1 - x, 0, 4, PCD8544_HEIGHT, WHITE);
		EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer01);
		EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer02);
	}

	// Cube
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, BLACK);	//	Fill display.
	EyeControl.Buffer_DrawRect(EyeControl.Buffer01, 20, 2, 30, 29, WHITE);
	EyeControl.Buffer_DrawRect(EyeControl.Buffer01, 35, 17, 30, 29, WHITE);
	EyeControl.Buffer_DrawLine(EyeControl.Buffer01, 20, 2, 35, 17, WHITE);
	EyeControl.Buffer_DrawLine(EyeControl.Buffer01, 49, 2, 64, 17, WHITE);
	EyeControl.Buffer_DrawLine(EyeControl.Buffer01, 20, 31, 34, 45, WHITE);
	EyeControl.Buffer_DrawLine(EyeControl.Buffer01, 50, 31, 64, 45, WHITE);
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	delay(2000);

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.
	for (int x = 0; x < 12; x++) { // Shutter Fill
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 11 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 23 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 35 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 47 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 59 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 71 + x, 0, 1, PCD8544_HEIGHT, BLACK);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	}

	for (int x = 0; x < 12; x++) { // Shutter Clear
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 0 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 11 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 23 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 35 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 47 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 59 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.Buffer_DrawFilledRect(EyeControl.Buffer01, 71 + x, 0, 1, PCD8544_HEIGHT, WHITE);
		EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	}

	for (int i = 0; i < 40; i++) {	//	Random Circles.
		int x = random(0, PCD8544_WIDTH);
		int y = random(0, PCD8544_HEIGHT);
		int r = random(2, 15);

		if (Toggle1) {
			EyeControl.Buffer_DrawCircle(EyeControl.Buffer01, x, y, r, BLACK, 1);
			EyeControl.PCD8544_UpdateDisplay(LEFT, EyeControl.Buffer01);
		}
		else {
			EyeControl.Buffer_DrawCircle(EyeControl.Buffer02, x, y, r, BLACK, 1);
			EyeControl.PCD8544_UpdateDisplay(RIGHT, EyeControl.Buffer02);
		}
		Toggle1 = !Toggle1;
	}

	EyeControl.Buffer_Print(EyeControl.Buffer01, "Both Defferent", 0, (PCD8544_HEIGHT / 2) / 4, WHITE);
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	delay(500);

	EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Logo, 0, 0, PCD8544_WIDTH, PCD8544_HEIGHT, BLACK, true);	//	Insert Image.
	EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
	delay(10000);

	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE);	//	Clear display.
	EyeControl.Buffer_FillScreen(EyeControl.Buffer02, WHITE);	//	Clear display.

}
/*
	Countdown 9 to 0.
*/
void CountDown() {

	NextInterval1 = millis() + 1000;
	uint8_t i = 11;
	while (i > 0) {
		if (millis() > NextInterval1) {
			int valOver = millis() - NextInterval1;
			NextInterval1 = millis() + 1000 - valOver;
			if (i > 1) { EyeControl.Buffer_DrawSprite_F(EyeControl.Buffer01, Numbers_18x24[i - 2], 33, 12, 18, 24, BLACK, true); }
			EyeControl.PCD8544_UpdateDisplay(BOTH, EyeControl.Buffer01);
			i--;
		}
	}
	EyeControl.Buffer_FillScreen(EyeControl.Buffer01, WHITE); // Clear Screen

}
/*
	To test Print direct to screen using the Arduino Print Class.

	Need to set position first. Use: PCD8544_GoToXY(eye, X, Y,)
		Eye = LEFT, RIGHT or BOTH.
		X = 0 to 83. Pixels horozontaly.
		Y = 0 to  5. Lines of text verticaly. Each line is 8 pixels.

	(Text)
*/
void TestPrintText() {

	byte number = 2;

	EyeControl.PCD8544_Clear(BOTH);

	EyeControl.PCD8544_GoToXY(BOTH, 24, 0);
	EyeControl.print("TIM-01");

	EyeControl.PCD8544_GoToXY(LEFT, 18, 1);
	EyeControl.print("Left Eye");

	EyeControl.PCD8544_GoToXY(RIGHT, 15, 1);
	EyeControl.print("Right Eye");

	EyeControl.PCD8544_GoToXY(BOTH, 12, 2);
	EyeControl.print("Number = ");
	EyeControl.println(number);// test variable.
	number = 255;
	EyeControl.println(number, HEX);// test variable.
	EyeControl.print("Next Line");// test println


	delay(6000);
}
/*
	Random colours increasing in brightness in left Eye, then Random colours increasing in brightness in right Eye, then both on random colour at 30% Brightness.
*/
void Test_RandomEyeColour() {
	//BGR

	for (size_t i = 0; i < 20; i++) {
		EyeControl.SK9822_EyeColour(i, random(0, 255), random(0, 255), random(0, 255), 0, random(0, 255), random(0, 255), random(0, 255));
		delay(50);
	}
	for (size_t i = 0; i < 20; i++) {
		EyeControl.SK9822_EyeColour(0, random(0, 255), random(0, 255), random(0, 255), i, random(0, 255), random(0, 255), random(0, 255));
		delay(50);
	}

	uint8_t Blue = random(0, 255);
	uint8_t Green = random(0, 255);
	uint8_t Red = random(0, 255);

	EyeControl.SK9822_EyeColour(15, Blue, Green, Red, 15, Blue, Green, Red);

}
/*
	Test Eye Colours.
*/
void TestEyeColours() {

	//Serial.println("TestEyeColours");
	EyeControl.SK9822_EyeColour(20, Eye_Left_Red, Eye_Left_Green, Eye_Left_Blue, 20, Eye_Right_Red, Eye_Right_Green, Eye_Right_Blue);
	delayMicroseconds(10);
	Eye_Left_Red += 50;
	Eye_Right_Red += 50;
	if (Eye_Left_Red > 200) {
		Eye_Left_Red = 0;
		Eye_Right_Red = 0;
		Eye_Left_Green += 50;
		Eye_Right_Green += 50;
		if (Eye_Left_Green > 200) {
			Eye_Left_Green = 0;
			Eye_Right_Green = 0;
			Eye_Left_Blue += 50;
			Eye_Right_Blue += 50;
			if (Eye_Left_Blue > 200) {
				Eye_Left_Blue = 0;
				Eye_Right_Blue = 0;
			}
		}
	}
}
/*
	Show all veriants of colour.
*/
void AllColours() {
	uint8_t colour[3] = { 255,0,0 };

	for (int i = 0; i < 3; i += 1) {
		int next = i == 2 ? 0 : i + 1;

		for (int i = 0; i < 255; i += 1) {
			colour[i] -= 1;
			colour[next] += 1;

			EyeControl.SK9822_EyeColour(20, colour[0], colour[1], colour[2], 20, colour[0], colour[1], colour[2]);
			delay(20);
		}
	}

}
/*
	Show just 7 main colours.
*/
void Rainbow() {

	int _delay1 = 20;
	int _delay2 = 200;

	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, ii, 0, 0, 20, ii, 0, 0);	//	Red.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		delay(_delay1);
		EyeControl.SK9822_EyeColour(20, ii, 0, 0, 20, ii, 0, 0);	//	Red.
	}
	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, ii, ii, 0, 20, ii, ii, 0);	//	Red, Green = Yellow.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		EyeControl.SK9822_EyeColour(20, ii, ii, 0, 20, ii, ii, 0);	//	Red, Green = Yellow.
		delay(_delay1);
	}
	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, 0, ii, 0, 20, 0, ii, 0);	//	Green.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		EyeControl.SK9822_EyeColour(20, 0, ii, 0, 20, 0, ii, 0);	//	Green.
		delay(_delay1);
	}
	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, 0, ii, ii, 20, 0, ii, ii);	//	Green, Blue = Cyan.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		EyeControl.SK9822_EyeColour(20, 0, ii, ii, 20, 0, ii, ii);	//	Green, Blue = Cyan.
		delay(_delay1);
	}
	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, 0, 0, ii, 20, 0, 0, ii);	//	Blue.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		EyeControl.SK9822_EyeColour(20, 0, 0, ii, 20, 0, 0, ii);	//	Blue.
		delay(_delay1);
	}
	for (size_t ii = 1; ii < 256; ii++) {
		EyeControl.SK9822_EyeColour(20, ii, 0, ii, 20, ii, 0, ii);	//	Blue, Red = Magenta.
		delay(_delay1);
	}
	delay(_delay2);
	for (size_t ii = 255; ii > 0; ii--) {
		EyeControl.SK9822_EyeColour(20, ii, 0, ii, 20, ii, 0, ii);	//	Blue, Red = Magenta.
		delay(_delay1);
	}

}
/*
	Blue, Green, Red, White.
*/
void BGRW() {
	switch (count) {

	case 1:
		EyeControl.SK9822_EyeColour(30, 255, 10, 10, 30, 255, 10, 10);
		break;
	case 2:
		EyeControl.SK9822_EyeColour(30, 10, 255, 10, 30, 10, 255, 10);
		break;
	case 3:
		EyeControl.SK9822_EyeColour(30, 10, 10, 255, 30, 10, 10, 255);
		break;
	case 4:
		EyeControl.SK9822_EyeColour(30, 255, 255, 255, 30, 255, 255, 255);
		break;
	default:
		EyeControl.SK9822_EyeColour(30, 255, 255, 255, 30, 255, 255, 255);
		break;
	}

	count++;
	if (count > 4) { count = 1; }
}
