package nanovm.smbot.drivers;

public class Motor
{
  public static final int MAXIMUM_SPEED = 100;
  public static final int MINIMUM_SPEED = -100;
  public static native void setSpeed(int channel,int value);
  public static native void stop();
}

