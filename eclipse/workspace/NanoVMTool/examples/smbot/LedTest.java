/*
  LedTest.java

  testing ctbot leds by cycling through the six
  status leds and blinking all blue leds

  (c) 2007 Nils Springob <nils@nicai-systems.de>
*/

import nanovm.smbot.drivers.*;

class LedTest {

  // set status led state and wait a second
  static int led(int state) {
    Leds.set(state);
	return 135;
  }

  public static void main(String[] args) {
	int temp;
	while(true)
	{
		temp = led(Leds.OFF);
		System.out.println("a" + temp);
		led(Leds.ORANGE);
		led(Leds.RED);
		led(Leds.GREEN);
		led(Leds.BLUE);
		led(Leds.ORANGE+Leds.RED+Leds.GREEN+Leds.BLUE);
    }
  }
}

     
