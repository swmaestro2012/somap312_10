package nanovm.smbot.drivers;

public class Servo
{
  public static final int MAXIMUM = 12;
  public static native void setPosition(int ch, int val);  
  public static native void release(int ch);
}

