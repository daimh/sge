/*___INFO__MARK_BEGIN__*/
/*************************************************************************
 *
 *  The Contents of this file are made available subject to the terms of
 *  the Sun Industry Standards Source License Version 1.2
 *
 *  Sun Microsystems Inc., March, 2001
 *
 *
 *  Sun Industry Standards Source License Version 1.2
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.2 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://gridengine.sunsource.net/Gridengine_SISSL_license.html
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *   The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *   Copyright: 2001 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/
package com.sun.grid.jgdi;

import com.sun.grid.jgdi.configuration.ClusterQueue;
import com.sun.grid.jgdi.configuration.xml.XMLUtil;
import java.io.StringReader;
import java.io.StringWriter;
import java.util.List;
import junit.framework.Test;
import junit.framework.TestSuite;
import org.xml.sax.SAXParseException;

/**
 *
 */
public class MapAttributeTestCase extends BaseTestCase {
    
    /** Creates a new instance of MapAttributeTestCase */
    public MapAttributeTestCase(String name) {
        super(name);
    }
    
    public static Test suite() {
        TestSuite suite = new TestSuite(MapAttributeTestCase.class);
        return suite;
    }
    
    public void testClusterQueueRerun() throws Exception {
        
        JGDI jgdi = createJGDI();
        try {
            List<ClusterQueue> queueList = jgdi.getClusterQueueList();
            for (ClusterQueue queue : queueList) {
                logger.info(queue.getName() + "------------");
                for (String key : queue.getRerunKeys()) {
                    logger.info("rerun: " + key + "=" + queue.getRerun(key));
                    queue.putRerun(key, !queue.getRerun(key));
                }
                queue.setName("blubber_" + queue.getName());
                queue.addPe("@/", "make");
                StringWriter sw = new StringWriter();
                XMLUtil.write(queue, sw);
                sw.flush();
                logger.fine(sw.getBuffer().toString());
                
                try {
                    queue = (ClusterQueue) XMLUtil.read(new StringReader(sw.getBuffer().toString()));
                    XMLUtil.write(queue, System.out);
                } catch (SAXParseException spe) {
                    logger.severe("Error in [" + spe.getLineNumber() + ":" + spe.getColumnNumber() + "]:" + spe.getMessage());
                    throw spe;
                }
                
                //         pe = new ST();
                //         pe.setName("make");
                //         queue.addPe("oin", pe);
                //         jgdi.addClusterQueue(queue);
                //jgdi.deleteClusterQueue(queue);
            }
        } finally {
            jgdi.close();
        }
    }
}
