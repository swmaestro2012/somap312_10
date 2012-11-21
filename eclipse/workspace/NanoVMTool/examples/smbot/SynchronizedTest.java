import nanovm.smbot.drivers.*;
 
public class SynchronizedTest {

	public static void main(String[] args) {
		Object lockobj0 = new Object();
		Object lockobj1 = new Object();
		synchronized(lockobj0)
		{
			System.out.println("synchronized test - 0");
		}

		synchronized(lockobj1)
		{
			System.out.println("synchronized test - 1");
		}
	}
}


