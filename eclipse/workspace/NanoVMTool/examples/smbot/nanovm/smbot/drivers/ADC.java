package nanovm.smbot.drivers;

public class ADC
{
  public static final boolean ENABLE = true;
  public static final boolean DISABLE = false;
  public static final int MAXIMUM = 255;
  public static final int MINIMUM = 0;
  public static native int getADC(int source);
  public static native void setEnable(int source, boolean enable);
  public static native boolean getEnable(int source);
}


