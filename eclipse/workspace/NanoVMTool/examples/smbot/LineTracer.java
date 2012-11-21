	import nanovm.smbot.drivers.*;
	import java.io.*;
	
class LineTracer {
// set status led state and wait a second
	public static void main(String[] args) throws IOException {
		char test=0;
		int servo_position=6;

		System.out.println("NanoVM - console echo demo");
		System.out.println("Please press some keys ...");

		// adc_value150 이상일경우 흰색 150이하 일경우 검은색 
		ADC.setEnable(1,ADC.ENABLE);
		ADC.setEnable(2,ADC.ENABLE);
		ADC.setEnable(3,ADC.ENABLE);
		ADC.setEnable(4,ADC.ENABLE);
		
		Servo.release(0);
		
		int adc_value1,adc_value2,adc_value3,adc_value4;
		boolean enabled;
		while(true)
		{
			if(System.in.available() != 0)
			{
				test = ((char) System.in.read());
				System.out.print(test);
			} 

			else 
			{
				test = 0;
			}
		
			if(test == 'w')
			{
				Motor.setSpeed(0,80);
			} 
			
			else if(test == 's')
			{
				Motor.stop();
			}
		//Servo.setPosition(0,12);// 12
			enabled=ADC.getEnable(1);
			adc_value1=ADC.getADC(1);
			System.out.println("adc1 " + adc_value1);
			
			enabled=ADC.getEnable(2);
			adc_value2=ADC.getADC(2);
			System.out.println("adc2 " + adc_value2);
			
			enabled=ADC.getEnable(3);
			adc_value3=ADC.getADC(3);
			System.out.println("adc3 " + adc_value3);
			
			enabled=ADC.getEnable(4);
			adc_value4=ADC.getADC(4);
			System.out.println("adc4 " + adc_value4);
			
			if(adc_value2 < 150 || adc_value3 < 150)
			{
				Servo.setPosition(0,6);
			}
			
			else if(adc_value1 < 150 && adc_value2 < 150)
			{
				Servo.setPosition(0,9);
			}
			
			else if(adc_value4 < 150 && adc_value3 < 150)
			{
				Servo.setPosition(0,3);
			}
			
			else if(adc_value1 < 150)
			{
				Servo.setPosition(0,9);
			}
			
			else if(adc_value4 < 150)
			{
				Servo.setPosition(0,3);
			}
			
			Clock.delay_ms(10);
		}
	}
}

