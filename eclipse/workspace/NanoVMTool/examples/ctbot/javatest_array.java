
class JavaTest_array{
	public static void main(String[] args) {
		int buffer[];
		int bufsz = 100;
		int value = 12;
		buffer = new int[bufsz];
		buffer[10] = value;
		value = buffer[11];
	}
}