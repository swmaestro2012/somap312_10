import nanovm.smbot.drivers.*;

class TypeLongTest {

  // set status led state and wait a second
  public static void main(String[] args) {
	/*int i1 = 1;
	long l0 = 0;
	long l1 = 1;
	long l2 = 2;
	System.out.println("test\r\n");*/
	int i1,i2;
	long l;
	long l2;
	
	i1 = 2147483647;
	i2 = -2147483648;
	System.out.println(i1 + "\r\n" + i2  + "\r\n");
	
	l = 0x0111111111111111L;
	System.out.println(l);
	System.out.println("");
	l2 = 0x0333333333213333L;
	//l2 = 3L;
	
	l = l + l2;
	System.out.println("l + l2 = " + l);
	
	l = l - l2;
	System.out.println("l - l2 = " + l);	
	
	l = l * l2;
	System.out.println("l * l2 = " + l);
	
	l = 0x0444444444444444L;	
	l = l / l2;	
	System.out.println("l / l2 = " + l);
	
	l = l2 >> l;
	System.out.println("l >> l2 = " + l);
	
	l = 2;
	l = l2 << l;
	System.out.println("l << l2 = " + l);
	l = 154346345345643L;
	l2 = 10000000L;
	l = l % l2;
	System.out.println("l % l2 = " + l);
	l = -l;
	System.out.println("-l = " + l);
	l = ~l;
	System.out.println("~l = " + l);
	
	l = 0x56789abc;
	System.out.println(l);
	System.out.println("l = " + l);
  }
}

     
