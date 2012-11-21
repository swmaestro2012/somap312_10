class SynkThread {
 private int PrtUsedNo = 0;
 
 //int usingPrinter() {//임계 영역을 지정하는 synchronized메소드
 synchronized int usingPrinter() {//임계 영역을 지정하는 synchronized메소드
  return PrtUsedNo++;
 }
}
 
class PC extends Thread {
 String name;
 SynkThread printer;
 PC(SynkThread printer, String name) {
  this.name = name;
  this.printer = printer;
 }
  
 public void run() {//스레드가 할 일을 정의
  try{
   for(int i=0; i<3; i++) {
    System.out.println(this.name + "--printer : " + printer.usingPrinter() + "use");
    sleep(500);
   }
  }catch(InterruptedException e){
   System.err.println(e.getMessage());
  }
 }
  
}

 
public class SynchronizedTest {
 
 public static void main(String[] args) {
  
  SynkThread printer = new SynkThread();
  
  //3개의 스레드 객체 생성
  PC p1 = new PC(printer, "com1");
  PC p2 = new PC(printer, "com2");
  PC p3 = new PC(printer, "com3");
  
  //스레드 스케줄링 : 우선순위 부여
  //p1.setPriority(Thread.MAX_PRIORITY);
  //p2.setPriority(Thread.NORM_PRIORITY);
  //p3.setPriority(Thread.MIN_PRIORITY);
  
  System.out.println("sychronized O");
  System.out.println("-----------------------");
  
  //스레드 시작 
  p1.start();//start()메소드는 run()메소드를 호출한다.
  p2.start();
  p3.start();
  
 }

}







/*public class FinallyTest {
	public static void main(String[]args){
		int a=10;
		int b=0;
		int c=0;
		try{
			c=a/b;
		}
		catch(NumberFormatException e){
			System.out.println("catch2 ");
		}
		catch(ArithmeticException e){
			b=1;
			c=a/b;
			System.out.println("catch1");
		}
		catch(Exception e){
			System.out.println("catch3 ");
		}
		System.out.println("do I print?");
		System.out.printf("%d/%d=%d\n",a,b,c);
		System.out.println("main finish!!");
	}
}*/