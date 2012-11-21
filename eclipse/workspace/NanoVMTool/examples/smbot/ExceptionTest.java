//import nanovm.smbot.drivers.*;

class ExceptionTest {
	public boolean test(int i) throws Exception{
		try {
			System.out.println("111");
            if(i == 1)
                throw new Exception();
            return true;
        }
        catch (Exception e) {
            System.out.println("exception");
			return true;
            //throw e;
        }
        finally{
			System.out.println("test - finally");
        }
    }
    public static void main(String args[]) throws InterruptedException {
        ExceptionTest java = new ExceptionTest();
        try {
            java.test(0);
            System.out.println("-------------------------");
            java.test(1);
			throw new InterruptedException();
        } 
		catch (Exception e) {
            System.out.println("333");
        }
		catch (InterruptedException e) {
			System.out.println("InterruptedException");
		}
		finally{
			System.out.println("main - finally");
		}
		
		
		System.out.println("main finish");
    }
}