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
 *   Copyright: 2009 by Sun Microsystems, Inc.
 *
 *   All Rights Reserved.
 *
 *  Portions of this software are Copyright (c) 2011 Univa Corporation.
 ************************************************************************/
/*___INFO__MARK_END__*/

package com.sun.grid.herd;

import com.sun.grid.jsv.JobDescription;
import com.sun.grid.jsv.Jsv;
import com.sun.grid.jsv.JsvManager;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.TreeSet;
import java.util.logging.Logger;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.fs.FileStatus;
import org.apache.hadoop.fs.FileSystem;
import org.apache.hadoop.fs.Path;
import org.apache.hadoop.hdfs.DFSClient;
import org.apache.hadoop.hdfs.protocol.HdfsFileStatus;
import org.apache.hadoop.hdfs.protocol.DatanodeInfo;
import org.apache.hadoop.hdfs.protocol.DirectoryListing;
import org.apache.hadoop.hdfs.protocol.LocatedBlock;
import org.apache.hadoop.util.StringUtils;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

/**
 * The HerdJsv class is a JSV that translates hard resource requests for the
 * hdfs_input resource into soft requests for the hdfs_primary_rack,
 * hdfs_secondary_rack, and hdfs_blk* resources.  In order to do the
 * conversion, this JSV must be able to contact the Namenode for the HDFS
 * cluster.
 * TODO: Make it possible to make the block and rack requests hard instead
 * of soft.
 */
public class HerdJsv extends Configured implements Tool, Jsv {
    private static final String BLOCK_KEY = "hdfs_B";
    private static final String PATH_KEY = "hdfs_input";
    private static final String PATH_KEY_SHORT = "hdfs_in";
    private static final String PRIMARY_RACK_KEY = "hdfs_R";
    private static final String SECONDARY_RACK_KEY = "hdfs_r";
    private static final String ZEROS = "0000000000000000";
    private static final Logger log;

    static {
        log = Logger.getLogger(HerdJsv.class.getName());
        log.setUseParentHandlers(false);
    }

    /**
     * The method to run the JSV.
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        int exit = 0;

        try {
            exit = ToolRunner.run(new HerdJsv(), args);
        } catch (Exception e) {
            Throwable cause = e;

            while (e.getCause() != null) {
                cause = e.getCause();
            }

            log.warning("Error while running command: " + StringUtils.stringifyException(cause));
            exit = 1;
        }

        System.exit(exit);
    }

    public int run(String[] arg0) throws Exception {
        log.info("Started Herd JSV");

        new JsvManager().parse(this);

        return 0;
    }

    public void onStart(JsvManager jsv) {
        log.info("Starting job verification");
    }

    public void onVerify(JsvManager jsv) {
        JobDescription job = jsv.getJobDescription();
        Map<String,String> res = job.getHardResourceRequirements();
        String path = null;

        if ((res != null) && res.containsKey(PATH_KEY)) {
            path = res.get(PATH_KEY);
        } else if ((res != null) && res.containsKey(PATH_KEY_SHORT)) {
            path = res.get(PATH_KEY_SHORT);
        }

        if (path != null) {
            if (verifyPath(path)) {
                Map<String,String> soft = job.getSoftResourceRequirements();

                try {
                    Collection<LocatedBlock> blocks = null;

                    blocks = getBlocks(path, getConf());

                    // Add new requests
                    soft.putAll(buildRackRequests(blocks));
                    soft.putAll(buildBlockRequests(blocks));
                    job.setSoftResourceRequirements(soft);

                    // Remove old request
                    res.remove(PATH_KEY);
                    res.remove(PATH_KEY_SHORT);
                    job.setHardResourceRequirements(res);
                } catch (FileNotFoundException e) {
                    jsv.reject("The requested data path does not exist: " + path);
                } catch (IOException e) {
                    log.warning("Unable to contact Namenode: " + StringUtils.stringifyException(e));
                    jsv.log(JsvManager.LogLevel.ERROR, "Unable to contact Namenode: " + e.getMessage());
                    jsv.rejectWait("Unable to process Hadoop jobs at this time.");
                } catch (RuntimeException e) {
                    log.warning("Exception while trying to contact Namenode: " + StringUtils.stringifyException(e));
                    jsv.log(JsvManager.LogLevel.ERROR, "Exception while trying to contact Namenode: " + e.getClass().getSimpleName() + " -- " + e.getMessage());
                    jsv.rejectWait("Unable to process Hadoop jobs at this time.");
                }
            } else {
                jsv.reject("The requested data path must start with a '/'");
            }
        }
    }

    /**
     * Verify that the given path is valid according to the apparent rules
     * imposed by the namenode, i.e. it must start with a /.
     * @param path the path to test
     * @return whether the path is valid
     */
    private static boolean verifyPath(String path) {
        boolean valid = false;

        if ((path != null) && (path.length() > 0)) {
            valid = path.charAt(0) == '/';
        }

        return valid;
    }

    /**
     * This method takes a list of HDFS data blocks and creates resources
     * requests for the hdfs_blk* resources.
     * @param blocks a list of HDFS data blocks
     * @return a map of resource requests.  The key is the name
     * of the requested resource, and the value is the requested value.
     */
    private static Map<String,String> buildBlockRequests(Collection<LocatedBlock> blocks) {
        Map<String,String> ret = new HashMap<String, String>();
        StringBuilder request = new StringBuilder();
        String previous = "";

        if (blocks != null) {
            for (LocatedBlock block : blocks) {
                if (block.getBlock() != null) {
                    String idString = toIdString(block.getBlock().getBlockId());

                    if (!idString.substring(0, 2).equals(previous)) {
                        if (!previous.equals("")) {
                            request.append("*");
                            ret.put(BLOCK_KEY + previous, request.toString());
                        }

                        previous = idString.substring(0, 2);
                        request.setLength(0);
                    }

                    request.append('*');
                    request.append(idString.substring(2));
                }
            }
        }

        if (!previous.equals("")) {
            request.append("*");
            ret.put(BLOCK_KEY + previous, request.toString());
        }

        return ret;
    }

    /**
     * Translate the given long into a 16-character hexidecimal string.
     * @param id a long value
     * @return a 16-character hexidecimal string
     */
    private static String toIdString(long id) {
        String idString = Long.toHexString(id);

        // Zero-pad if needed
        idString = ZEROS.substring(idString.length()) + idString;

        return idString;
    }

    /**
     * This method translates a list of HDFS data blocks into a set of rack
     * resource requests.
     * @param blocks a list of HDFS data blocks
     * @return a map of resource requests.  The key is the name of the
     * requested resource, and the value is the requested value.
     */
    private static Map<String,String> buildRackRequests(Collection<LocatedBlock> blocks) {
        Map<String,String> ret = new HashMap<String, String>();
        StringBuilder primary = new StringBuilder();
        StringBuilder secondary = new StringBuilder();

        Map<String, Integer> racks = collateRacks(blocks);
        // Sort the racks by number of blocks in each rack
        List<String> rackNames = getSortedRacks(racks);

        // Build primary from the top 20% and secondary from 100%
        int count = 0;
        int top = (int)Math.ceil(rackNames.size() * 0.2f);
        
        for (String rack : rackNames) {
            secondary.append('|');
            secondary.append(rack);

            if (count < top) {
                primary.append('|');
                primary.append(rack);
            }

            count++;
        }

        if (primary.length() > 1) {
            ret.put(PRIMARY_RACK_KEY, primary.toString().substring(1));
        }

        if (secondary.length() > 1) {
            ret.put(SECONDARY_RACK_KEY, secondary.toString().substring(1));
        }

        return ret;
    }

    /**
     * Take the list of HDFS data blocks and determine what racks house the
     * blocks and how many blocks are in each rack.
     * @param blocks a list of HDFS data blocks
     * @return a map of racks, where the key is the rack name, and the value
     * if the number of blocks in that rack
     */
    private static Map<String,Integer> collateRacks(Collection<LocatedBlock> blocks) {
        Map<String, Integer> racks = new HashMap<String, Integer>();

        if (blocks != null) {
            for (LocatedBlock block : blocks) {
                for (DatanodeInfo node : block.getLocations()) {
                    String loc = node.getNetworkLocation();

                    if (racks.containsKey(loc)) {
                        racks.put(loc, racks.get(loc) + 1);
                    } else {
                        racks.put(loc, 1);
                    }
                }
            }
        }

        return racks;
    }

    /**
     * Take of map of the number of blocks in a set of racks and return the
     * names of the racks sorted in descending order by the number of
     * blocks.
     * @param racks a map of the number of blocks in a set of racks
     * @return a list of the rack names sorted by the number of blocks
     */
    private static List<String> getSortedRacks(final Map<String,Integer> racks) {
        List<String> rackNames = Collections.EMPTY_LIST;

        if (racks != null) {
            rackNames = new ArrayList<String>(racks.keySet());

            Collections.sort(rackNames, new Comparator<String>() {
                public int compare(String o1, String o2) {
                    int ret = racks.get(o2).compareTo(racks.get(o1));

                    if (ret == 0) {
                        ret = o1.compareTo(o2);
                    }

                    return ret;
                }
            });
        }

        return rackNames;
    }

    /**
     * Convert the given HDFS path into a list of HDFS data blocks.  If the
     * path is a directory, it will be recursively processed to include the data
     * blocks for all files contained under the directory path.
     * @param path an HDFS path
     * @param conf the Hadoop configuration
     * @return a list of HDFS data blocks
     * @throws IOException Thrown if there is an error while communcating
     * with the HDFS Namenode
     */
    private static Collection<LocatedBlock> getBlocks(String path, Configuration conf) throws IOException {
        Set<LocatedBlock> blocks = new TreeSet<LocatedBlock>(new Comparator<LocatedBlock>() {
            public int compare(LocatedBlock o1, LocatedBlock o2) {
                return o1.getBlock().compareTo(o2.getBlock());
            }
        });

        DFSClient dfs = new DFSClient(conf);

        return getBlocks(path, conf, dfs, blocks);
    }

    /**
     * Convert the given HDFS path into a list of HDFS data blocks.  If the
     * path is a directory, it will be recursively processed to include the data
     * blocks for all files contained under the directory path.
     * @param path an HDFS path
     * @param conf the Hadoop configuration
     * @param dfs the DFSClient to use
     * @param blocks the list to populate with blocks
     * @return a list of HDFS data blocks
     * @throws IOException Thrown if there is an error while communcating
     * with the HDFS Namenode
     */
    private static Set<LocatedBlock> getBlocks(String path, Configuration conf, DFSClient dfs, Set<LocatedBlock> blocks) throws IOException {
        FileStatus s = FileSystem.get(conf).getFileStatus(new Path(path));
        
        if (!s.isDir()) {
            blocks.addAll(dfs.namenode.getBlockLocations(path, 0,
                    Long.MAX_VALUE).getLocatedBlocks());
        } else {
	    DirectoryListing drl = dfs.listPaths(path, HdfsFileStatus.EMPTY_NAME);
            for (HdfsFileStatus fileStatus : drl.getPartialListing()) {
		blocks.addAll(getBlocks(fileStatus.getFullName(path), conf, dfs, blocks));
            }
        }
        
        dfs.close();

        return blocks;
    }
}
