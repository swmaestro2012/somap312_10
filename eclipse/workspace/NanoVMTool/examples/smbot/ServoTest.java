import nanovm.smbot.drivers.*;

class ServoTest {

  // set status led state and wait a second
  public static void main(String[] args) {
	int i;
	while(true)
	{		
		for(i=0;i<=Servo.MAXIMUM;i++)
		{
			System.out.println("Position = " + i);			
			Servo.setPosition(0,i);
			Servo.setPosition(1,i);
			Servo.setPosition(2,i);
			Servo.setPosition(3,i);
			Servo.setPosition(4,i);
			Servo.setPosition(5,i);
			Servo.setPosition(6,i);
			Servo.setPosition(7,i);
			Clock.delay_ms(500);
		}
		Clock.delay_ms(2000);
		System.out.println("servo release");
		Servo.release(0);
		Servo.release(1);
		Servo.release(2);
		Servo.release(3);
		Servo.release(4);
		Servo.release(5);
		Servo.release(6);
		Servo.release(7);
		Clock.delay_ms(2000);
		for(i=Servo.MAXIMUM;i>=0;i--)
		{
			System.out.println("Position = " + i);			
			Servo.setPosition(0,i);
			Servo.setPosition(1,i);
			Servo.setPosition(2,i);
			Servo.setPosition(3,i);
			Servo.setPosition(4,i);
			Servo.setPosition(5,i);
			Servo.setPosition(6,i);
			Servo.setPosition(7,i);
			Clock.delay_ms(500);
		}
	}
  }
}

