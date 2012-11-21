/*
  Fibonacci.java
 */
import nanovm.asuro.*;
class Fibonacci {

  static int fib(int n) {
    if(n < 2) 
      return 1;
    else
      return fib(n-2) + fib(n-1);
  } 

  public static void main(String[] args) {
	String temp;
    for(int i=0;i<=20;i++)
	{
	  temp = "Fibonacci of "+i+" is "+fib(i);
      System.out.println(temp);
	}
	  //System.out.println("Fibonacci of ");
  }
}

     
