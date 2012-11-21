class SynkThread {
 private int PrtUsedNo = 0;
 
 //int usingPrinter() {//�Ӱ� ������ �����ϴ� synchronized�޼ҵ�
 synchronized int usingPrinter() {//�Ӱ� ������ �����ϴ� synchronized�޼ҵ�
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
  
 public void run() {//�����尡 �� ���� ����
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
  
  //3���� ������ ��ü ����
  PC p1 = new PC(printer, "com1");
  PC p2 = new PC(printer, "com2");
  PC p3 = new PC(printer, "com3");
  
  //������ �����ٸ� : �켱���� �ο�
  //p1.setPriority(Thread.MAX_PRIORITY);
  //p2.setPriority(Thread.NORM_PRIORITY);
  //p3.setPriority(Thread.MIN_PRIORITY);
  
  System.out.println("sychronized O");
  System.out.println("-----------------------");
  
  //������ ���� 
  p1.start();//start()�޼ҵ�� run()�޼ҵ带 ȣ���Ѵ�.
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