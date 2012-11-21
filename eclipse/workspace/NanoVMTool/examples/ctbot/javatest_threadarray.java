
class MyThread{
	static void nothing(int a,int b)
	{
	}
}

class JavaTest_threadarray{
	public static void main(String[] args) {
		MyThread threads[];
		int count = 10;
		threads = new MyThread[count];
		threads[0] = new MyThread();
	}
}