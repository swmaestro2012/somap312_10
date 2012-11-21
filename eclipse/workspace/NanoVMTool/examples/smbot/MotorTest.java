import nanovm.smbot.drivers.*;

class MotorTest {

  // set status led state and wait a second
  public static void main(String[] args) {
	int i;
	while(true)
	{
		Motor.stop();
		Clock.delay_ms(2000);
		for(i=0;i<=Motor.MAXIMUM_SPEED;i++)
		{
			System.out.println("time = " + i);
			Clock.delay_ms(50);
			Motor.setSpeed(0,i);
			Motor.setSpeed(1,i);
			Motor.setSpeed(2,i);
			Motor.setSpeed(3,i);
		}
		
		Motor.stop();
		Clock.delay_ms(2000);
		for(i=0;i>=Motor.MINIMUM_SPEED;i--)
		{
			System.out.println("time = " + i);
			Clock.delay_ms(50);
			Motor.setSpeed(0,i);
			Motor.setSpeed(1,i);
			Motor.setSpeed(2,i);
			Motor.setSpeed(3,i);
		}
	}
  }
}

