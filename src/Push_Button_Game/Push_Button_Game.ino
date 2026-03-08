/*
 * Push_Button_Game.c
 *
 * Created: 18.08.2016 16:43:31
 *  Author: fmetzler
 */

// Uses Adafruit NeoPixel, NeoMatrix and GFX libraries (BSD / LGPL licensed)
// Uses TM1637 display library (MIT/LGPL depending on version)

#include "TM1637.h"
#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Enum for the game state machine
enum { SLEEP,
	     SLEEP_WAIT,
	     STANDBY,
	     STANDBY_WAIT,
	     GAME_START,
	     GAME_START_WAIT,
	     GAME_RUNNING,
	     GAME_RUNNING_WAIT,
	     GAME_FINISHED,
	     GAME_FINISHED_WAIT };

// Enum for the game speed selection
enum { SPEED_1,
	     SPEED_2,
	     SPEED_3 };

// Enum for the game mode selection
enum { MODE_1,
	     MODE_2,
	     MODE_3,
	     MODE_4 };

// Some game definitions
#define MAX_BUTTON_PRESSED_COUNTER_VALUE 2  // Treshhold value for the button detection. (Debouncing)
#define MAX_PRESHOW_PATTERNS 11             // Maximum number of light preshow patterns
#define MAX_GAME_TIME 300                   // Maximum game time. 30 seconds at 100 ms.
#define MAX_SLEEP_TIME 3000                 // Maximum time before the game goes to sleep mode when no button is pressed. 5 minutes at 100 ms.

// Pin definitions for TM1637-1
#define CLK_1 21
#define DIO_1 20

// Pin definitions for TM1637-2
#define CLK_2 8
#define DIO_2 9

// Pin definition for the led matrix
#define LED_MATRIX_PIN 25

// Pin definitions for the big push buttons
#define BUTTON_1_PIN 22  // 0
#define BUTTON_2_PIN 24  // 1
#define BUTTON_3_PIN 26  // 2
#define BUTTON_4_PIN 28  // 3

// Pin definitions for the setup buttons
#define BUTTON_5_PIN 32  // 4
#define BUTTON_6_PIN 34  // 5
#define BUTTON_7_PIN 36  // 6
#define BUTTON_8_PIN 30  // 7

// Definition of which button is connected to which bit in the button variables. 0-7, each bit position only once.
#define PLAY_BUTTON_1 0
#define PLAY_BUTTON_2 1
#define PLAY_BUTTON_3 2
#define PLAY_BUTTON_4 3
#define MODE_BUTTON 4
#define SPEED_BUTTON 5
#define START_BUTTON 6
#define SLEEP_BUTTON 7

// Pin definitions for the big push button leds
#define LED_1_PIN 46
#define LED_2_PIN 48
#define LED_3_PIN 50
#define LED_4_PIN 52

// Pin definitions for the setup button leds
#define LED_5_PIN 40
#define LED_6_PIN 42
#define LED_7_PIN 44
#define LED_8_PIN 38

// Definitions for the digital led matrix
#define LED_MATRIX_LED_COUNT 64
#define LED_MATRIX_MATRIX_COUNT 4

//Color numbering definitions.
#define BLACK 0x00
#define YELLOW 0x01
#define ORANGE 0x02
#define RED 0x03
#define GREEN 0x04
#define CYAN 0x05
#define BLUE 0x06
#define MAGENTA 0x07
#define WHITE 0x08

#define COLOR_COUNT 9       // Number of available colors
#define MAX_LED_VALUE 0x50  // Led light intensity value (0 - 255)

// Init color array. [line][row]
static volatile uint8_t colorDefinitions[COLOR_COUNT][3] = { { 0x00, 0x00, 0x00 },
	                                                           { MAX_LED_VALUE, MAX_LED_VALUE, 0x00 },
	                                                           { MAX_LED_VALUE, (MAX_LED_VALUE / 2), 0x00 },
	                                                           { MAX_LED_VALUE, 0x00, 0x00 },
	                                                           { 0x00, MAX_LED_VALUE, 0x00 },
	                                                           { 0x00, MAX_LED_VALUE, MAX_LED_VALUE },
	                                                           { 0x00, 0x00, MAX_LED_VALUE },
	                                                           { MAX_LED_VALUE, 0x00, MAX_LED_VALUE },
	                                                           { MAX_LED_VALUE, MAX_LED_VALUE, MAX_LED_VALUE } };

// Pin definitions for the TM1637
TM1637 tm1637_1(CLK_1, DIO_1);
TM1637 tm1637_2(CLK_2, DIO_2);

// Button array
uint8_t button_counter_array[8];  // Counts up if a button is pressed until MAX_BUTTON_PRESSED_COUNTER_VALUE is reached

// Button variables
uint8_t buttons_locked;   // Each bit stays for a button and indicate if a button is locked.
uint8_t buttons_trigger;  // Each bit stays for a button and indicate if a button is triggered (With debouncing). Must be released before it could be triggered again. Will be cleared after reading.
// Array that contains the pin definitions for the 8 input buttons.
uint8_t button_pin_array[8] = { BUTTON_1_PIN, BUTTON_2_PIN, BUTTON_3_PIN, BUTTON_4_PIN, BUTTON_5_PIN, BUTTON_6_PIN, BUTTON_7_PIN, BUTTON_8_PIN };
// Array that contains the pin definitions for the 8 output leds.
uint8_t led_pin_array[8] = { LED_1_PIN, LED_2_PIN, LED_3_PIN, LED_4_PIN, LED_5_PIN, LED_6_PIN, LED_7_PIN, LED_8_PIN };

uint8_t game_mode = MODE_1;       // Actual game mode
uint8_t game_mode_copy = MODE_1;  // Saved game mode
uint8_t speed = SPEED_1;          // Actual game speed
uint16_t time_counter = MAX_GAME_TIME;
bool time_over = false;  // Flag to signal when game time is over
uint16_t points = 0;     // Score counter

uint8_t step = 0;
uint8_t delay_counter = 0;  // Counter for the delay
uint8_t delay_value = 5;    // Delay value for the 100 ms main loop. Will be set depending on the selected speed.
uint8_t state = STANDBY;    // Game state
uint16_t sleep_counter = 0;
bool go_to_sleep = false;  // Flag to signal if system should go to sleep
uint8_t pattern_count_standby = 0;
uint8_t pattern_direction_standby = 0;
uint8_t pattern_count_preshow = 0;
uint8_t pattern_toggle_preshow = 0;
uint8_t pattern_base_color_preshow = BLACK;
uint8_t pattern_base_color_standby = YELLOW;

uint8_t button_to_hit = 0;       // The button that should be hit (0-3)
uint8_t game_delay_counter = 0;  // Counter for the game delay

// Neopixel Matrix 32 x 8
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(32, 8, LED_MATRIX_PIN, NEO_MATRIX_BOTTOM + NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE, NEO_GRB + NEO_KHZ800);

// Some variables for the led matrix
const uint16_t colors[] = { matrix.Color(255, 0, 0), matrix.Color(0, 255, 0), matrix.Color(0, 0, 255) };  //
String text = "Push start button";
int16_t x1, y1;
uint16_t w, h;
int x = 32;

// Some game variables
unsigned long previousMillisCounter1 = 0;    // Saves the timestamp from the last run
unsigned long previousMillisCounter2 = 0;    // Saves the timestamp from the last run
const unsigned long intervalCounter1 = 10;   // Interval in milliseconds
const unsigned long intervalCounter2 = 100;  // Interval in milliseconds
int gameTime = 0;                            // Time left to play

void setup() {
	Serial.begin(115200);

	// Init first 7 digit display
	tm1637_1.init();
	tm1637_1.set(BRIGHT_TYPICAL);  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
	delay(500);
	tm1637_1.display(3, 0);

	// Init second 7 digit display
	tm1637_2.init();
	tm1637_2.set(BRIGHT_TYPICAL);  //BRIGHT_TYPICAL = 2,BRIGHT_DARKEST = 0,BRIGHTEST = 7;
	delay(500);
	tm1637_2.display(3, 0);

	//Init input buttons
	for (int i = 0; i < 8; i++) {
		pinMode(button_pin_array[i], INPUT_PULLUP);
	}
	//Init leds
	for (int i = 0; i < 8; i++) {
		pinMode(led_pin_array[i], OUTPUT);

		srand(analogRead(A0));  // Init random generator

		// Init the led matrix
		matrix.begin();
		matrix.setTextWrap(false);
		matrix.setTextSize(1);  // for 8px high
		matrix.setBrightness(40);
		matrix.setTextColor(matrix.Color(255, 0, 0));
		matrix.getTextBounds(text.c_str(), 0, 0, &x1, &y1, &w, &h);
		delay(500);
	}
}

// Led and Button connection:
// Led 1 - 4: Play
// Led 5 + 6: Mode
// Led 7: Play
// Led 8: Sleep
// Button 1 - 4 Play
// Button 5 Mode
// Button 6 Speed
// Button 7 Start/Reset
// Button 8 Sleep
void loop() {
	unsigned long currentMillis = millis();  // Get the actual millis

	// Check if 10 ms are reached
	if (currentMillis - previousMillisCounter1 >= intervalCounter1) {
		previousMillisCounter1 = currentMillis;  // Save the actual millis
		update_button_state();                   // Check the buttons
	}

	currentMillis = millis();  // Refresh the actual millis

	// Check if 100 ms are reached
	if (currentMillis - previousMillisCounter2 >= intervalCounter2) {
		previousMillisCounter2 = currentMillis;  // Save the actual millis

		//button_test(); // Uncomment this test if you need to check the buttons

		// Check if mode button (4) has been pressed and change mode
		if (check_button_trigger(MODE_BUTTON)) {
			switch (game_mode) {
				case MODE_1:
					game_mode = MODE_2;
					digitalWrite(LED_5_PIN, LOW);
					digitalWrite(LED_6_PIN, HIGH);
					break;

				case MODE_2:
					game_mode = MODE_3;
					digitalWrite(LED_5_PIN, HIGH);
					digitalWrite(LED_6_PIN, LOW);
					break;

				case MODE_3:
					game_mode = MODE_4;
					digitalWrite(LED_5_PIN, HIGH);
					digitalWrite(LED_6_PIN, HIGH);
					break;

				case MODE_4:
					game_mode = MODE_1;
					digitalWrite(LED_5_PIN, LOW);
					digitalWrite(LED_6_PIN, LOW);
					break;

				default:
					game_mode = MODE_1;
					digitalWrite(LED_5_PIN, LOW);
					digitalWrite(LED_6_PIN, LOW);
					break;
			}
		}

		// Counts up the sleep counter and sets the flag go_to_sleep to true if time is over
		if (sleep_counter < MAX_SLEEP_TIME) sleep_counter++;
		else {
			//Serial.println("Go to sleep");
			go_to_sleep = true;
		}

		// Check if the game time is over if state is GAME_RUNNING_WAIT
		if (state == GAME_RUNNING_WAIT) {
			if (time_counter < MAX_GAME_TIME) time_counter++;
			else {
				//Serial.println("Game over");
				time_over = true;
			}
		}

		// Set the matrix for the state STANDBY_WAIT before the state machine, because we don´t want the dalay.
		if (state == STANDBY_WAIT) {
			// Matrix section
			matrix.fillScreen(0);
			matrix.setCursor(x, 0);
			uint32_t color = rgbRainbow((millis() / 10) % 256);
			matrix.setTextColor(color);
			matrix.print(text);
			matrix.show();
			// Scroll the text
			x--;
			if (x < -((int)w)) {
				x = matrix.width();
			}
		}

		// Delay to be able to have various speeds in the loop.
		if (delay_counter < delay_value) delay_counter++;
		else {
			delay_counter = 0;

			switch (state) {
				case STANDBY:
					Serial.println("State: Standby");

					if (points <= 0) tm1637_1.display(3, points);
					else tm1637_1.displayNum(points);

					gameTime = (MAX_GAME_TIME - time_counter) / 10;
					if (gameTime <= 0) tm1637_2.display(3, gameTime);
					else tm1637_2.displayNum(gameTime, 0, false);

					sleep_counter = 0;
					go_to_sleep = false;
					delay_value = 5;             // Loop time 500 ms
					text = "Push start button";  // text to display
					x = 32;                      // Start position for the text
					state = STANDBY_WAIT;        // Next state

					// Clear all leds
					clear_all_leds();

					// Set the game mode leds
					switch (game_mode) {
						case MODE_1:
							Serial.println("Mode: 1");
							digitalWrite(LED_5_PIN, LOW);
							digitalWrite(LED_6_PIN, LOW);
							break;

						case MODE_2:
							Serial.println("Mode: 2");
							digitalWrite(LED_5_PIN, LOW);
							digitalWrite(LED_6_PIN, HIGH);
							break;

						case MODE_3:
							Serial.println("Mode: 3");
							digitalWrite(LED_5_PIN, HIGH);
							digitalWrite(LED_6_PIN, LOW);
							break;

						case MODE_4:
							Serial.println("Mode: 4");
							digitalWrite(LED_5_PIN, HIGH);
							digitalWrite(LED_6_PIN, HIGH);
							break;

						default:
							Serial.println("Mode: Default");
							digitalWrite(LED_5_PIN, LOW);
							digitalWrite(LED_6_PIN, LOW);
							break;
					}
					break;

				case STANDBY_WAIT:
					led_toggle(LED_7_PIN);  // Show standby state.

					//Check if game should be started. Start button (6)
					if (check_button_trigger(START_BUTTON)) {
						Serial.println("Hit: Start button");
						state = GAME_START;
					}

					//Go to sleep if the sleep timer run out or if the sleep button was pressed
					if ((go_to_sleep) || (check_button_trigger(SLEEP_BUTTON))) {
						//state = SLEEP;
						//Serial.println("Should go to sleep");
					}
					break;

				case GAME_START:
					Serial.println("State: Game Start");
					digitalWrite(LED_7_PIN, HIGH);
					pattern_count_preshow = 0;
					delay_value = 5;          // Loop time 500 ms
					state = GAME_START_WAIT;  // Next state
					break;

				case GAME_START_WAIT:
					//Serial.println("State: Game Start Wait");
					if (pattern_count_preshow < MAX_PRESHOW_PATTERNS) {
						// Show info on Matrix
						if (pattern_count_preshow <= 1) {
							text = "5";
							x = 14;
						} else if (pattern_count_preshow <= 3) {
							text = "4";
							x = 13;
						} else if (pattern_count_preshow <= 5) {
							text = "3";
							x = 13;
						} else if (pattern_count_preshow <= 7) {
							text = "2";
							x = 13;
						} else if (pattern_count_preshow <= 9) {
							text = "1";
							x = 13;
						} else if (pattern_count_preshow <= 11) {
							text = "Go";
							x = 10;
						}

						matrix.fillScreen(0);
						matrix.setCursor(x, 0);
						uint32_t color = matrix.Color(255, 255, 255);
						;
						matrix.setTextColor(color);
						matrix.print(text);
						matrix.show();

						if (check_button_trigger(START_BUTTON)) state = STANDBY;  // Go to state standby if the standby button is clicked
						pattern_count_preshow++;                                  // Count up the pattern counter
					} else {
						state = GAME_RUNNING;  // Go to the next state after the preshow
					}
					break;

				case GAME_RUNNING:
					Serial.println("State: Game Running");
					delay_value = 1;                                            // Change delay time to minimum value
					game_mode_copy = game_mode;                                 // Save the actual game mode
					points = 0;                                                 // Reset the points
					clear_all_triggers();                                       // Clear the game button trigger
					state = GAME_RUNNING_WAIT;                                  // Next state
					time_counter = 0;                                           // Reset the time counter
					time_over = false;                                          // Reset flag
					button_to_hit = generate_random_hit_button(PLAY_BUTTON_1);  // Generate the first button that should be hit
					game_delay_counter = 0;                                     // Reset the game delay counter
					break;

				case GAME_RUNNING_WAIT:
					//Check for the reset button.
					if (check_button_trigger(START_BUTTON)) state = STANDBY;

					// Change hit pattern every x seconds. random 0 - 3. But not the same.
					// MODE_1 == 2 seconds and 1 lamp.
					// MODE_2 == 1 second and 1 lamp.
					// MODE_3 == 500 ms and 1 lamp.
					// MODE_4 == 200 ms and 1 lamp.
					// Check if lamp on  == button hit.
					// If lamp is hit directly show new lamp.
					//Count points up each time the correct button was hit.
					//Play game. Check buttons. Set points. Set new hit lights (random patterns).

					gameTime = (MAX_GAME_TIME - time_counter) / 10;
					if (gameTime <= 0) tm1637_2.display(3, gameTime);
					else tm1637_2.displayNum(gameTime);

					if (check_button_trigger(button_to_hit)) {
						//Hit! Points++ and new hit button.
						points++;
						if (points <= 0) tm1637_1.display(3, points);
						else tm1637_1.displayNum(points);
						Serial.print("Score: ");
						Serial.println(points);

						clear_all_triggers();  // Clear all existing triggers
						button_to_hit = generate_random_hit_button(button_to_hit);
						set_hit_button_led(button_to_hit);
						//Reset time.
						switch (game_mode_copy) {
							case MODE_1:
								game_delay_counter = 20;  // 2 seconds delay
								break;

							case MODE_2:
								game_delay_counter = 10;  // 1 seconds delay
								break;

							case MODE_3:
								game_delay_counter = 5;  // 0,5 seconds delay
								break;

							case MODE_4:
								game_delay_counter = 2;  // 0,2 seconds delay
								break;

							default:
								game_delay_counter = 10;  // 1 seconds delay
								break;
						}
					} else {
						if (game_delay_counter-- == 0) {  //	Time is over and the button was not hit -> generate a new hit button
							clear_all_triggers();           // Clear all existing triggers
							button_to_hit = generate_random_hit_button(button_to_hit);
							set_hit_button_led(button_to_hit);
							//No hit! No points! New hit button and reset time.
							switch (game_mode_copy) {
								case MODE_1:
									game_delay_counter = 20;  // 2 seconds delay
									break;

								case MODE_2:
									game_delay_counter = 10;  // 1 seconds delay
									break;

								case MODE_3:
									game_delay_counter = 5;  // 0,5 seconds delay
									break;

								case MODE_4:
									game_delay_counter = 2;  // 0,2 seconds delay
									break;

								default:
									game_delay_counter = 10;  // 1 seconds delay
									break;
							}
						}
					}

					if (time_over) state = GAME_FINISHED;  // End game if time is over
					break;

				case GAME_FINISHED:
					Serial.println("State: Game Finished");
					//Prepare finished state.
					digitalWrite(LED_1_PIN, LOW);
					digitalWrite(LED_2_PIN, LOW);
					digitalWrite(LED_3_PIN, LOW);
					digitalWrite(LED_4_PIN, LOW);
					digitalWrite(LED_7_PIN, LOW);
					//state = GAME_FINISHED_WAIT;
					state = STANDBY;  // Directly go to standby - No sleep mode wanted
					delay_value = 5;  // Loop time 500 ms
					break;

				case GAME_FINISHED_WAIT:
					//Stay in this state and wait for start button or the sleep timer. Show points of player.
					if (check_button_trigger(START_BUTTON)) state = STANDBY;

					//Go to sleep after 30 seconds or when sleep button is hit.
					if ((go_to_sleep) || (check_button_trigger(SLEEP_BUTTON))) state = SLEEP;
					break;

				case SLEEP:
					Serial.println("State: Sleep");
					sleep_counter = 0;              // Reset the sleep counter
					go_to_sleep = false;            // Reset the sleep flag
					digitalWrite(LED_8_PIN, HIGH);  //
					delay_value = 5;                // Loop time 500 ms
					state = SLEEP_WAIT;             // Next state
					break;

				case SLEEP_WAIT:
					//Check if we should return back to normal mode.
					if ((check_button_trigger(START_BUTTON)) || (check_button_trigger(SLEEP_BUTTON))) state = STANDBY;
					break;
			}
		}
	}
}

uint32_t rgbRainbow(byte pos) {
	if (pos < 85) {
		return matrix.Color(pos * 3, 255 - pos * 3, 0);  // Rot → Gelb → Grün
	} else if (pos < 170) {
		pos -= 85;
		return matrix.Color(255 - pos * 3, 0, pos * 3);  // Grün → Cyan → Blau
	} else {
		pos -= 170;
		return matrix.Color(0, pos * 3, 255 - pos * 3);  // Blau → Magenta → Rot
	}
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Toggels the led
 * @param		Led pin that should be toogled.
 * @return	-
 */
void led_toggle(uint8_t led_pin) {
	if (digitalRead(led_pin)) digitalWrite(led_pin, LOW);
	else digitalWrite(led_pin, HIGH);
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Clears all leds
 * @param		-
 * @return	-
 */
void clear_all_leds(void) {
	for (int i = 0; i < 8; i++) {
		digitalWrite(led_pin_array[i], LOW);
	}
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	?
 * @param		?
 * @return	-
 */
void set_led_stripe_buffer_to_moving_dot(uint8_t color, uint8_t dot_color, uint8_t step1, uint8_t step2) {
	//Prepare buffer.
	/*
	for (int i = 0; i < LED_MATRIX_LED_COUNT; i++) {
		if ((i != step1) && (i != step2)) {
			led_stripe_buffer[i][0] = colorDefinitions[color][0];
			led_stripe_buffer[i][1] = colorDefinitions[color][1];
			led_stripe_buffer[i][2] = colorDefinitions[color][2];
		} else {
			led_stripe_buffer[i][0] = colorDefinitions[dot_color][0];
			led_stripe_buffer[i][1] = colorDefinitions[dot_color][1];
			led_stripe_buffer[i][2] = colorDefinitions[dot_color][2];
		}
	}
	*/
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Checks if the trigger for the button is set and returns true if set.
 *					Also the trigger value is cleared after reading.
 * @param		Button for which the trigger should be checked.
 * @return	True if trigger is set, false if trigger is not set.
 */
bool check_button_trigger(uint8_t button) {
	if (bitRead(buttons_trigger, button)) {
		bitClear(buttons_trigger, button);
		return true;
	} else return false;
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Clears all button triggers.
 * @param		-
 * @return	-
 */
void clear_all_triggers() {
	buttons_trigger = 0;
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Checks all 8 button inputs and counts up a counter for each pressed button.
 *					If a treshhold value is reached, the trigger is set in the "buttons_trigger" variable.
 * @param		-
 * @return	-
 */
void update_button_state() {
	for (int i = 0; i < 8; i++) {
		//Check input and count up counter if button is pressed.
		if (!digitalRead(button_pin_array[i])) {
			if (button_counter_array[i] < MAX_BUTTON_PRESSED_COUNTER_VALUE) button_counter_array[i]++;
			else {
				if (!bitRead(buttons_locked, i)) {
					bitSet(buttons_trigger, i);  // Set button trigger only if once, then lock the button
					bitSet(buttons_locked, i);
				}
			}
		} else {
			button_counter_array[i] = 0;
			bitClear(buttons_locked, i);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	This is a test function for the buttons.
 *					If a button is pressed the led will go on.
 * @param		-
 * @return	-
 */
void button_test() {
	for (int i = 0; i < 8; i++) {
		if (check_button_trigger(i)) {
			digitalWrite(led_pin_array[i], HIGH);
			Serial.print("Button: ");
			Serial.print(i);
			Serial.println("high");
		} else digitalWrite(led_pin_array[i], LOW);
	}
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Generates a random number between 0 and 3 that is different to the old value.
 *					This function is used to generate the random button to be hit in the game.
 * @param 	The last pressed button.
 * @return	A random value between 0 and 3 that is different to the old value.
 */
uint8_t generate_random_hit_button(uint8_t old_button) {
	uint8_t r;
	do {
		r = rand() % 4;
	} while (old_button == r);
	return r;
}

//////////////////////////////////////////////////////////////////////////
/**
 * @brief 	Sets the led of the game button that is the actual target to be hit.
 * @param 	The actual button to be hit.
 * @return	-
 */
bool set_hit_button_led(uint8_t hit_button) {
	// Check if the input value is between 0 and 3
	if (hit_button >= 0 && hit_button <= 3) {
		for (int i = 0; i < 8; i++) {
			digitalWrite(led_pin_array[i], LOW);
		}
		digitalWrite(led_pin_array[hit_button], HIGH);
		return true;
	} else return false;
}