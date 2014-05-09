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
#define LEVEL_SHIFTER_POWER_PIN 20
#define DEBUG_BUTTON_PIN 8
#define C_PAD_DEFAULT_POTENTIAL 132
#define DAC_9_BIT_1V8 279
#define RECV_BUF_SIZE 64
#define BUTTON_PRESSED HIGH
#define BUTTON_RELEASED LOW
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
	analogWriteFrequency(LEVEL_SHIFTER_POWER_PIN, 90000);
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
	pinMode(TOUCH_SCREEN_X_PIN, OUTPUT);
	pinMode(TOUCH_SCREEN_Y_PIN, OUTPUT);
	pinMode(C_PAD_X_PIN, OUTPUT);
	pinMode(C_PAD_Y_PIN, OUTPUT);
	pinMode(LEVEL_SHIFTER_POWER_PIN, OUTPUT);
	c_pad_reset();
	analogWrite(LEVEL_SHIFTER_POWER_PIN, DAC_9_BIT_1V8);
	button_click_delay_ms = 200;
    c_pad_nudge_delay_ms = 200;
    touch_screen_click_delay_ms = 100;
    // don't change the order of following two lines
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
			c_pad_nudge(x, y, c);
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
			button_hold(get_button_pin(recv_buf + arg1_pos));
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
	digitalWrite(button_pin, BUTTON_PRESSED);
	delay(button_delay);
	digitalWrite(button_pin, BUTTON_RELEASED);
	delay(20);
}

void button_hold(int8_t button_pin)
{
	digitalWrite(button_pin, BUTTON_PRESSED);
}

void button_release(int8_t button_pin)
{
	digitalWrite(button_pin, BUTTON_RELEASED);
}

void button_release_all()
{
	digitalWrite(BUTTON_A_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_B_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_X_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_Y_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_RIGHT_SHOULDER_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_LEFT_SHOULDER_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_UP_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_DOWN_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_LEFT_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_RIGHT_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_START_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_SELECT_PIN, BUTTON_RELEASED);
	digitalWrite(BUTTON_HOME_PIN, BUTTON_RELEASED);
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
	if(x > 255 || y > 255)
		return;
	analogWrite(C_PAD_X_PIN, x);
	analogWrite(C_PAD_Y_PIN, y);
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
	if(x > 320 || y > 240)
		return;
	int16_t x_potential = ((double)x / 320) * DAC_9_BIT_1V8;
	int16_t y_potential = ((double)y / 240) * DAC_9_BIT_1V8;
	
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
}


void disable_touch_screen()
{
	analogWrite(TOUCH_SCREEN_Y_PIN, DAC_9_BIT_1V8);
}

// pull Y+ pin down to initialize touch sreen interrupt
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
	if(strncmp(buf, "rs\n", 3) == 0)
		return BUTTON_RIGHT_SHOULDER_PIN;
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
	if(strncmp(buf, "st\n", 3) == 0)
		return BUTTON_START_PIN;
	if(strncmp(buf, "sl\n", 3) == 0)
		return BUTTON_SELECT_PIN;
	if(strncmp(buf, "h\n", 2) == 0)
		return BUTTON_HOME_PIN;
	else
		return -1;
}