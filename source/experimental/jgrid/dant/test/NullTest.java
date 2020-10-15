/*
 * NullTest.java
 *
 * Created on October 27, 2003, 1:49 PM
 */

package dant.test;

/**
 *
 * @author  dant
 */
public class NullTest {
	
	/** Creates a new instance of NullTest */
	public NullTest () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) {
		String str1 = "test";
		String str2 = null;
		String str3 = str1 + str2;
		
		System.out.println(str3);
	}
	
}
