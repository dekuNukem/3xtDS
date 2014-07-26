#include <stdint.h>
#include <stdio.h>
#define BUTTON_A_PIN 0
#define BUTTON_B_PIN 1
#define BUTTON_X_PIN 2
#define BUTTON_Y_PIN 3
#define BUTTON_RIGHT_SHOULDER_PIN 4
#define BUTTON_LEFT_SHOULDER_PIN 5
#define BUTTON_UP_PIN 6
#define BUTTON_DOWN_PIN 7
#define BUTTON_LEFT_PIN 14
#define BUTTON_RIGHT_PIN 15
#define BUTTON_START_PIN 16
#define BUTTON_SELECT_PIN 17
#define BUTTON_HOME_PIN 18
#define TOUCH_SCREEN_X_PIN 23
#define TOUCH_SCREEN_Y_PIN A14
#define TOUCH_SCREEN_Y_SENSE_PIN 19
#define C_PAD_X_PIN 21
#define C_PAD_Y_PIN 22
#define DEBUG_BUTTON_PIN 8
#define LED_PIN 13
#define C_PAD_DEFAULT_POTENTIAL 140
#define DAC_9_BIT_1V8 279
#define RECV_BUF_SIZE 64
#define BUTTON_PRESSED LOW
#define MINIMAL_DELAY_MS 10

char recv_buf[RECV_BUF_SIZE];
int32_t c_pad_nudge_delay_ms;
int32_t button_click_delay_ms;
int32_t touch_screen_click_delay_ms;

void setup()
{
	// Teensy USB serial always run at full USB speed of 12 Mbit/sec,
	// regardless of the argument.
    Serial.begin(9600);
    analogWriteResolution(9);
	analogReadResolution(10);
	// PWM frequency at 90KHz
	analogWriteFrequency(TOUCH_SCREEN_X_PIN, 90000);
	pinMode(DEBUG_BUTTON_PIN, INPUT_PULLUP);
	delay(1);
	pinMode(LED_PIN, OUTPUT);
	pinMode(BUTTON_A_PIN, OUTPUT);
	pinMode(BUTTON_B_PIN, OUTPUT);
	pinMode(BUTTON_X_PIN, OUTPUT);
	pinMode(BUTTON_Y_PIN, OUTPUT);
	pinMode(BUTTON_RIGHT_SHOULDER_PIN, OUTPUT);
	pinMode(BUTTON_LEFT_SHOULDER_PIN, OUTPUT);
	pinMode(BUTTON_UP_PIN, OUTPUT);
	pinMode(BUTTON_DOWN_PIN, OUTPUT);
	pinMode(BUTTON_LEFT_PIN, OUTPUT);
	pinMode(BUTTON_RIGHT_PIN, OUTPUT);
	pinMode(BUTTON_START_PIN, OUTPUT);
	pinMode(BUTTON_SELECT_PIN, OUTPUT);
	pinMode(BUTTON_HOME_PIN, OUTPUT);
	button_release_all();
	pinMode(C_PAD_X_PIN, OUTPUT);
	pinMode(C_PAD_Y_PIN, OUTPUT);
	c_pad_reset();
	button_click_delay_ms = 250;
    c_pad_nudge_delay_ms = 500;
    touch_screen_click_delay_ms = 100;
    digitalWrite(LED_PIN, HIGH);
    while(digitalRead(DEBUG_BUTTON_PIN) == HIGH)
    	Serial.read();
    digitalWrite(LED_PIN, LOW);
    pinMode(TOUCH_SCREEN_X_PIN, OUTPUT);
	pinMode(TOUCH_SCREEN_Y_PIN, OUTPUT);
    analogWrite(TOUCH_SCREEN_X_PIN, 0);
    disable_touch_screen();
}

// main loop, wait for commands from PC
void loop()
{
	if(get_serial_command(recv_buf, RECV_BUF_SIZE) != -1)
	{
		// c-pad nudge:  cn 127 240
		if(strncmp(recv_buf, "cn ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t arg2_pos = goto_next_arg(arg1_pos, recv_buf, RECV_BUF_SIZE);
			int16_t x = atoi(recv_buf + arg1_pos);
			int16_t y = atoi(recv_buf + arg2_pos);
			c_pad_nudge(x, y, c_pad_nudge_delay_ms);
		}
		// c-pad hold: ch 127 240
		if(strncmp(recv_buf, "ch ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t arg2_pos = goto_next_arg(arg1_pos, recv_buf, RECV_BUF_SIZE);
			int16_t x = atoi(recv_buf + arg1_pos);
			int16_t y = atoi(recv_buf + arg2_pos);
			c_pad_hold(x, y);
		}
		// c-pad release: cr
		if(strncmp(recv_buf, "cr\n", 3) == 0)
			c_pad_reset();
		/*	
		c-pad set nudge delay, aka how long the c-pad
		is held before releasing when c_pad_nudge is called
		cd 100
		*/
		if(strncmp(recv_buf, "cd ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t new_delay = atoi(recv_buf + arg1_pos);
			new_delay > MINIMAL_DELAY_MS ? c_pad_nudge_delay_ms = new_delay : c_pad_nudge_delay_ms;
		}
		// touch screen click: tc 120 50
		if(strncmp(recv_buf, "tc ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t arg2_pos = goto_next_arg(arg1_pos, recv_buf, RECV_BUF_SIZE);
			int16_t x = atoi(recv_buf + arg1_pos);
			int16_t y = atoi(recv_buf + arg2_pos);
			touch_screen_click(x, y, touch_screen_click_delay_ms);
		}
		// touch screen set click delay in ms: td 100
		if(strncmp(recv_buf, "td ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t new_delay = atoi(recv_buf + arg1_pos);
			new_delay > MINIMAL_DELAY_MS ? touch_screen_click_delay_ms = new_delay : touch_screen_click_delay_ms;
		}
		// button click: bc a
		if(strncmp(recv_buf, "bc ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			button_click(get_button_pin(recv_buf + arg1_pos), button_click_delay_ms);
		}
		// button hold: bh a
		if(strncmp(recv_buf, "bh ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			button_press(get_button_pin(recv_buf + arg1_pos));
		}
		// button release: br a
		if(strncmp(recv_buf, "br ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			button_release(get_button_pin(recv_buf + arg1_pos));
		}
		// button release all: br\n
		if(strncmp(recv_buf, "br\n", 3) == 0)
			button_release_all();

		// button set click delay in ms: bd 100
		if(strncmp(recv_buf, "bd ", 3) == 0)
		{
			int16_t arg1_pos = goto_next_arg(0, recv_buf, RECV_BUF_SIZE);
			int16_t new_delay = atoi(recv_buf + arg1_pos);
			new_delay > MINIMAL_DELAY_MS ? button_click_delay_ms = new_delay : button_click_delay_ms;
		}
	}
}

void button_click(int8_t button_pin, int16_t button_delay)
{
	button_press(button_pin);
	delay(button_delay);
	button_release(button_pin);
	delay(20);
}

void button_press(int8_t button_pin)
{
	pinMode(button_pin, OUTPUT);
	digitalWrite(button_pin, BUTTON_PRESSED);
}

void button_release(int8_t button_pin)
{
	pinMode(button_pin, INPUT);
}

void button_release_all()
{
	button_release(BUTTON_A_PIN);
	button_release(BUTTON_B_PIN);
	button_release(BUTTON_X_PIN);
	button_release(BUTTON_Y_PIN);
	button_release(BUTTON_RIGHT_SHOULDER_PIN);
	button_release(BUTTON_LEFT_SHOULDER_PIN);
	button_release(BUTTON_UP_PIN);
	button_release(BUTTON_DOWN_PIN);
	button_release(BUTTON_LEFT_PIN);
	button_release(BUTTON_RIGHT_PIN);
	button_release(BUTTON_START_PIN);
	button_release(BUTTON_SELECT_PIN);
	button_release(BUTTON_HOME_PIN);
}

int32_t goto_next_arg(int16_t current_pos, char* buf, int16_t size)
{
	while(current_pos < size && buf[current_pos] != ' ')
		current_pos++;

	while(current_pos < size && buf[current_pos] == ' ')
		current_pos++;
	return current_pos;
}

// get a complete command from serial
int8_t get_serial_command(char buf[], int16_t size)
{
	if(Serial.available())
	{
		int16_t count = 0;
		memset(buf, 0, size);
		char c;
		while(1)
			if(Serial.available())
			{
				c = Serial.read();
				// ignore carriage return
				if(c == '\r')
					continue;
				if(count < size - 1)
					buf[count] = c;
				count++;
				if(c == '\n')
					return 0;
			}
	}
	else
		return -1;
}

void c_pad_nudge(uint16_t x, uint16_t y, int16_t duration_ms)
{
	c_pad_hold(x, y);
	delay(duration_ms);
	c_pad_reset();
}

void c_pad_hold(uint16_t x, uint16_t y)
{
	if(x > 255 || y > 255)
		return;
	analogWrite(C_PAD_X_PIN, x);
	analogWrite(C_PAD_Y_PIN, y);
}

void c_pad_reset()
{
	analogWrite(C_PAD_X_PIN, C_PAD_DEFAULT_POTENTIAL);
	analogWrite(C_PAD_Y_PIN, C_PAD_DEFAULT_POTENTIAL);
}

void touch_screen_click(uint16_t x, uint16_t y, int16_t duration_ms)
{
	if(x <= 0 || y <= 0 || x > 320 || y > 240)
		return;
	int16_t x_potential = ((double)x / 320) * 320;
	int16_t y_potential = ((double)y / 240) * 320;
	
	y_potential -= (((double)(y_potential)) * 0.257 - 29);
	x_potential -= (((double)(x_potential)) * 0.066 - 7);

	analogWrite(TOUCH_SCREEN_X_PIN, x_potential);
	enable_touch_screen();
	int32_t start = millis();
	while(millis() - start < duration_ms)
	{
		// wait until Y+ pin is in output mode
		while(analogRead(TOUCH_SCREEN_Y_SENSE_PIN) < 512);
		delayMicroseconds(490);
		// now Y+ pin is in input mode, write the value
		analogWrite(TOUCH_SCREEN_Y_PIN, y_potential);
		delayMicroseconds(180);
		// pull Y+ down again to keep interrupt going
		enable_touch_screen();
	}
	disable_touch_screen();
	analogWrite(TOUCH_SCREEN_X_PIN, 0);
	delay(50);
}

void disable_touch_screen()
{
	analogWrite(TOUCH_SCREEN_Y_PIN, DAC_9_BIT_1V8);
}

// pull Y+ pin low to initialize 3DS's touch interrupt
void enable_touch_screen()
{
	analogWrite(TOUCH_SCREEN_Y_PIN, 0);
}

int8_t get_button_pin(char* buf)
{
	if(strncmp(buf, "a\n", 2) == 0)
		return BUTTON_A_PIN;
	if(strncmp(buf, "b\n", 2) == 0)
		return BUTTON_B_PIN;
	if(strncmp(buf, "x\n", 2) == 0)
		return BUTTON_X_PIN;
	if(strncmp(buf, "y\n", 2) == 0)
		return BUTTON_Y_PIN;
	// right shoulder
	if(strncmp(buf, "rs\n", 3) == 0)
		return BUTTON_RIGHT_SHOULDER_PIN;
	// left shoulder
	if(strncmp(buf, "ls\n", 3) == 0)
		return BUTTON_LEFT_SHOULDER_PIN;
	if(strncmp(buf, "u\n", 2) == 0)
		return BUTTON_UP_PIN;
	if(strncmp(buf, "d\n", 2) == 0)
		return BUTTON_DOWN_PIN;
	if(strncmp(buf, "l\n", 2) == 0)
		return BUTTON_LEFT_PIN;
	if(strncmp(buf, "r\n", 2) == 0)
		return BUTTON_RIGHT_PIN;
	// start
	if(strncmp(buf, "st\n", 3) == 0)
		return BUTTON_START_PIN;
	// select
	if(strncmp(buf, "sl\n", 3) == 0)
		return BUTTON_SELECT_PIN;
	// home
	if(strncmp(buf, "h\n", 2) == 0)
		return BUTTON_HOME_PIN;
	else
		return -1;
}