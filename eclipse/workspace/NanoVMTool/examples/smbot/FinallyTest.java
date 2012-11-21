
public class FinallyTest {
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
		finally{
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
			System.out.println("catch3 ");
		}
		System.out.println("do I print?");
		//System.out.printf("%d/%d=%d\n",a,b,c);
		System.out.println("main finish!!");
	}
}