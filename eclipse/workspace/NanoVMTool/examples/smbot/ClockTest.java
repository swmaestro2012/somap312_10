import nanovm.smbot.drivers.*;

class ClockTest {

  // set status led state and wait a second
  public static void main(String[] args) {
	int count;
	while(true)
	{
		Clock.delay_ms(100);
		count = Clock.getTickCount();
		System.out.println("count = " + count);
		Clock.delay_us(100);
		count = Clock.getTickCount();
		System.out.println("count = " + count);
	}
  }
}

     
