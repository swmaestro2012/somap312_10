import nanovm.smbot.drivers.*;

public class ThreadTest {
	public static void main(String[] args) {
		Runnable r1 = new RunnableClass1();// 鸥百 按眉
		Thread t1 = new Thread(r1);
		t1.start();
		
		
		Runnable r2 = new RunnableClass2();// 鸥百 按眉
		Thread t2 = new Thread(r2);
		t2.start();
		
		while(true)
		{
			System.out.println("Thread of Main Class is running now...");
			Clock.delay_ms(1000);
		}
	}
}
  
class RunnableClass1 implements Runnable{
	public void run() {	
		while(true)
		{
			System.out.println("Thread of Class 1 is running now...");
			Clock.delay_ms(1000);			
		}
	}
}


class RunnableClass2 implements Runnable{
	public void run() {
		while(true)
		{
			System.out.println("Thread of Class 2 is running now...");
			Clock.delay_ms(1000);
		}
	}
}
