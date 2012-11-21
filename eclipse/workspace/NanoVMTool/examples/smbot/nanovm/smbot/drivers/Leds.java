package nanovm.smbot.drivers;

public class Leds
{
  public static final int ORANGE = 0x01;
  public static final int RED    = 0x02;
  public static final int BLUE   = 0x04;
  public static final int GREEN  = 0x08;
  public static final int OFF	 = 0x00;
  public static native void set(int val);
  public static native int get();
}


