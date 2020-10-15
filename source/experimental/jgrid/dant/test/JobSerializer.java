/*
 * JobSerializer.java
 *
 * Created on May 18, 2003, 8:01 PM
 */

package dant.test;

import java.io.*;

import com.sun.grid.jgrid.*;
import com.sun.grid.jgrid.proxy.*;

/**
 *
 * @author  dant
 */
public class JobSerializer {
	
	/** Creates a new instance of JobSerializer */
	public JobSerializer () {
	}
	
	/**
	 * @param args the command line arguments
	 */
	public static void main (String[] args) throws Exception {
		File file = new File ("MyJob");
		FileOutputStream fout = new FileOutputStream (file);
		ProxyOutputStream oout = new ProxyOutputStream (fout);
		Job job = new Job ("MyJob", new ComputeTest ());
		
		job.setFilename (file.getAbsolutePath ());
		oout.setAnnotation ("http://java.sun.com/");
		oout.writeObject (job);
		oout.close ();
	}	
}
