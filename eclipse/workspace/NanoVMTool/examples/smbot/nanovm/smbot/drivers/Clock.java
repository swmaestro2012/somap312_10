package nanovm.smbot.drivers;

public class Clock
{
  public static native int getTickCount();
  //public static native int getMilliSeconds();
  public static native void delay_ms(int ms);
  public static native void delay_us(int us);
}


