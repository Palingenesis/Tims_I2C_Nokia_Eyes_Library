# Tims_I2C_Nokia_Eyes_Library
An Arduino Library for controlling my TIM-01 Robot Eyes Module
<img src="https://github.com/Palingenesis/Tims_I2C_Nokia_Eyes_Library/blob/main/Images/TIM-01_Eyes_Front_Github.png" alt="Front">


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
			|     1 2 3 4 5 6 7 8 9    |	PCD8544 Driver Pins:
			|     # # # # # # # # #    |		1 .. VDD	3.3v (up to 7.4 mA)
			|  ===#=#=#=#=#=#=#=#=#==  |		2 .. SCLK	Clock.
			+--======================--+		3 .. SDIN	Serial In.
			|                          |		4 .. D/C	Data / Command.
			|                          |		5 .. SCE	(CS) Cable select (active low).
			|        rear view         |		6 .. Osc	Oscillator, attach to 3.3v rail.
			|   connector is visible   |		7 .. GND	Ground.
			|                          |		8 .. Vout	LCD supply voltage (from driver internal dc/dc converter).
			|  Display Name LPH7366    |		9 .. RES	Reset (active low).
			|                          |
			+--------------------------+


	PCF8574 Module:
			+----------------------------------------+	PCF8574 Pins:
			|		1 2 3 4 5 6 7 8 9	 |		I2C: VCC, Ground, SDA, SCL.
			|                                   	 |	Pins:
			| VCC				    VCC  |		1	P0	= SDIN
			| GND				    GND	 |		2	P1	= SCLK	(PCD8544) / CKI (SK9822)
			| SDA	    component side   	    SDA	 |		3	P2	= D/C
			| SCL				    SCL	 |		4	P3	= RES
			|					 |		5	P4	= SCE	(CS) Cable select Left  Nokia Display.
			+----------------------------------------+		6	P5	= SCE	(CS) Cable select Right Nokia Display.
										7	P6	= SDI	(SK9822)
										8	P7	= AUX
										9	INT = PCF8574 interupt.
	SK9822 Intelegent RGB LED:
			+-----------------------+	SK9822 Pins:
			|                       |		Vcc	=	5v Power.
			| VCC		    GND |		CKO	=	Clock Out.	
			| CKO   Top View    SCI	|		SDO	=	Data  Out.	
			| SDO		    SDI	|		SDI	=	Data  In.	
			|		        |		SCI	=	Clock In.	
			+-----------------------+		GND	=	Ground, Negative Power.	



	There are several types of command in my library.
		If starts with Buffer_??,  this alters the buffer and will require the command PCD8544_UpdateDisplay to display any changes to the buffer.
		If starts with PCD8544_??, this is written direct to the displays.
		The Print command uses the Arduino Print library. It writes direct to the screens, the text position will need to be set first.
