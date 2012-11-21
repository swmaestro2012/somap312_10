/*
  LedTest.java

  testing ctbot leds by cycling through the six
  status leds and blinking all blue leds

  (c) 2007 Nils Springob <nils@nicai-systems.de>
*/

import nanovm.smbot.drivers.*;

class ADCTest {

  // set status led state and wait a second
  public static void main(String[] args) {
	boolean enabled;
	int adc_value;
	while(true)
	{
		ADC.setEnable(0,ADC.DISABLE);
		ADC.setEnable(0,ADC.ENABLE);
		enabled = ADC.getEnable(0);
		adc_value = ADC.getADC(0);
		System.out.println("adc0: "+adc_value);
		
		
		ADC.setEnable(1,ADC.DISABLE);
		ADC.setEnable(1,ADC.ENABLE);
		enabled = ADC.getEnable(1);
		adc_value = ADC.getADC(1);
		System.out.println("adc1: "+adc_value);
		
		
		ADC.setEnable(2,ADC.DISABLE);
		ADC.setEnable(2,ADC.ENABLE);
		enabled = ADC.getEnable(2);
		adc_value = ADC.getADC(2);
		System.out.println("adc2: "+adc_value);
		
		
		ADC.setEnable(3,ADC.DISABLE);
		ADC.setEnable(3,ADC.ENABLE);
		enabled = ADC.getEnable(3);
		adc_value = ADC.getADC(3);
		System.out.println("adc3: "+adc_value);
		
		
		ADC.setEnable(4,ADC.DISABLE);
		ADC.setEnable(4,ADC.ENABLE);
		enabled = ADC.getEnable(4);
		adc_value = ADC.getADC(4);
		System.out.println("adc4: "+adc_value);
		
		
		ADC.setEnable(5,ADC.DISABLE);
		ADC.setEnable(5,ADC.ENABLE);
		enabled = ADC.getEnable(5);
		adc_value = ADC.getADC(5);
		System.out.println("adc5: "+adc_value);
		
		
		ADC.setEnable(6,ADC.DISABLE);
		ADC.setEnable(6,ADC.ENABLE);
		enabled = ADC.getEnable(6);
		adc_value = ADC.getADC(6);
		System.out.println("adc6: "+adc_value);
		
		
		ADC.setEnable(7,ADC.DISABLE);
		ADC.setEnable(7,ADC.ENABLE);
		enabled = ADC.getEnable(7);
		adc_value = ADC.getADC(7);
		System.out.println("adc7: "+adc_value);
		
    }
  }
}

     
