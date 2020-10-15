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
 ************************************************************************/
/*___INFO__MARK_END__*/

package com.sun.grid.herd;

import com.sun.grid.loadsensor.LoadSensor;
import com.sun.grid.loadsensor.LoadSensorManager;
import java.io.IOException;
import java.net.InetSocketAddress;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.TimeUnit;
import java.util.logging.Logger;
import javax.security.auth.login.LoginException;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.conf.Configured;
import org.apache.hadoop.hdfs.DFSClient;
import org.apache.hadoop.hdfs.protocol.DatanodeInfo;
import org.apache.hadoop.hdfs.protocol.FSConstants.DatanodeReportType;
import org.apache.hadoop.hdfs.server.namenode.NameNode;
import org.apache.hadoop.hdfs.server.protocol.BlocksWithLocations;
import org.apache.hadoop.hdfs.server.protocol.NamenodeProtocol;
import org.apache.hadoop.io.retry.RetryPolicies;
import org.apache.hadoop.io.retry.RetryPolicy;
import org.apache.hadoop.io.retry.RetryProxy;
import org.apache.hadoop.ipc.RPC;
import org.apache.hadoop.ipc.VersionedProtocol;
import org.apache.hadoop.net.NetUtils;
import org.apache.hadoop.security.UserGroupInformation;
import org.apache.hadoop.util.StringUtils;
import org.apache.hadoop.util.Tool;
import org.apache.hadoop.util.ToolRunner;

/**
 * The HerdLoadSensor is a load sensor that reports the local HDFS blocks
 * and the name of the rack.  The rack name is reported twice: once as
 * hdfs_primary_rack and once as hdfs_secondary_rack.  This double
 * reported is needed by the HerdJsv.  The blocks are reported as 256
 * aggregate block resources named hdfs_blk*, where the * represents the
 * first two hex digits of the block's id.  Multiple blocks that share the
 * same first two hex id digits are reported via a single block resource.
 * Block resources for which no block is locally present are not reported.
 */
public class HerdLoadSensor extends Configured implements Tool, LoadSensor {
    // The prefix for the block resources
    private static final String BLOCK_KEY = "hdfs_blk";
    // The host name if none is specified
    private static final String DEFAULT_HOST = "localhost";
    // The rack name if none is specified
    private static final String DEFAULT_RACK = "/default-rack";
    // The primary rack resource
    private static final String RACK_KEY1 = "hdfs_primary_rack";
    // The secondary rack resource
    private static final String RACK_KEY2 = "hdfs_secondary_rack";
    // Used to zero-pad block ids
    private static final String ZEROS = "0000000000000000";
    // The logger for this class
    private static final Logger log = Logger.getLogger(HerdLoadSensor.class.getName());
    // The Hadoop configuration
    private Configuration conf;
    // The Hadoop DFSClient
    private DFSClient client = null;
    // A handle to the Hadoop Namenode
    private NamenodeProtocol namenode = null;
    // This node's datanode info
    private DatanodeInfo node = null;
    // This host's name
    private String hostName = null;
    // This host's rack's name
    private String rackName = null;
    // The list of found blocks as block load values
    private Map<String,String> blockStrings = null;
    
    /**
     * Run this load sensor.
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        int exit = 0;

        try {
            exit = ToolRunner.run(new HerdLoadSensor(), args);
        } catch (Exception e) {
            Throwable cause = e;

            while (cause.getCause() != null) {
                cause = cause.getCause();
            }

            log.warning("Error while running command: " + StringUtils.stringifyException(cause));
            exit = 1;
        }

        System.exit(exit);
    }

    public int run(String[] args) throws Exception {
        log.info("Started Herd load sensor");

        conf = getConf();
        client = new DFSClient(conf);
        namenode = createNamenode(conf);

        if (args.length > 0) {
            hostName = args[0];
        } else {
            throw new IllegalArgumentException("Usage: java com.sun.grid.herd.HerdLoadSensor hostname");
        }
        
        findDatanode();

        LoadSensorManager mgr = new LoadSensorManager(this);
        
        mgr.parse();
        
        return 0;
    }

    /**
     * Get the info object for this datanode.
     * @throws IOException Thrown if there is an error while communicating
     * with the namenode.
     */
    private void findDatanode() throws IOException {
        DatanodeInfo[] datanodes = client.namenode.getDatanodeReport(DatanodeReportType.LIVE);

        for (DatanodeInfo info : datanodes) {
            if (hostName.equals(info.getHostName())) {
                node = info;
            }
        }
    }

    /**
     * Create a connection to the namenode.
     * @param conf the Hadoop configuration
     * @return a handle to the namenode
     * @throws IOException Thrown if there is an error while communicating
     * with the namenode.
     */
    private static NamenodeProtocol createNamenode(Configuration conf) throws IOException {
        InetSocketAddress address = NameNode.getAddress(conf);
        RetryPolicy timeoutPolicy =
                RetryPolicies.retryUpToMaximumCountWithFixedSleep(3, 200,
                    TimeUnit.MILLISECONDS);
        Map<String, RetryPolicy> policyMap =
                Collections.singletonMap("getBlocks", timeoutPolicy);

        UserGroupInformation info = null;

        try {
            info = UserGroupInformation.getCurrentUser();
	} catch (Exception e) {
            throw new IOException(StringUtils.stringifyException(e));
        }

        VersionedProtocol proxy = RPC.getProxy(NamenodeProtocol.class,
            NamenodeProtocol.versionID, address, info, conf,
            NetUtils.getDefaultSocketFactory(conf));
        NamenodeProtocol ret =
                (NamenodeProtocol)RetryProxy.create(NamenodeProtocol.class,
                    proxy, policyMap);

        return ret;
    }

    public int getMeasurementInterval() {
        return MEASURE_ON_DEMAND;
    }

    public void doMeasurement() {
        try {
            rackName = node.getNetworkLocation();

            BlocksWithLocations blocks = namenode.getBlocks(node, Long.MAX_VALUE);

            List<Long> blockIds = new ArrayList<Long>(100);

            for (BlocksWithLocations.BlockWithLocations block : blocks.getBlocks()) {
                blockIds.add(block.getBlock().getBlockId());
            }

            blockStrings = buildBlockStrings(blockIds);

            log.fine("Rack is " + rackName);
            log.fine("Blocks are: " + blockStrings);
        } catch (IOException e) {
            log.warning("load sensor threw I/O exception while communicating with namenode: " + e.getMessage());
        }
    }

    /**
     * Take the list of block ids and create block load values.
     * @param blockIds a list of HDFS block ids
     * @return a map of block load values where the key is the block
     * resource name, and the value is the resource value
     */
    private static Map<String,String> buildBlockStrings(List<Long> blockIds) {
        Map<String,String> ret = new HashMap<String, String>();
        StringBuilder blk = new StringBuilder();
        String previous = "";

        if (blockIds != null) {
            Collections.sort(blockIds);

            for (Long id : blockIds) {
                String idString = Long.toHexString(id);

                // Zero-pad if needed
                idString = ZEROS.substring(idString.length()) + idString;

                if (!idString.substring(0, 2).equals(previous)) {
                    if (!previous.equals("")) {
                        blk.append('/');
                        ret.put(BLOCK_KEY + previous, blk.toString());
                    }

                    previous = idString.substring(0, 2);
                    blk.setLength(0);
                }

                blk.append('/');
                blk.append(idString.substring(2));
            }

            // Clean up last block
            if (!previous.equals("")) {
                blk.append("/");
                ret.put(BLOCK_KEY + previous, blk.toString());
            }
        }

        return ret;
    }

    public Map<String, Map<String, String>> getLoadValues() {
        Map<String, Map<String, String>> ret = null;
        Map<String,String> loadValues = new HashMap<String, String>();
        String loadHostName = this.hostName;
        String loadRackName = this.rackName;

        if (loadHostName == null) {
            loadHostName = DEFAULT_HOST;
        }

        if (loadRackName == null) {
            loadRackName = DEFAULT_RACK;
        }

        loadValues.put(RACK_KEY1, loadRackName);
        loadValues.put(RACK_KEY2, loadRackName);

        if (blockStrings != null) {
            loadValues.putAll(blockStrings);
        }

        ret = Collections.singletonMap(loadHostName, loadValues);

        log.fine("Returning load values: " + ret);

        return ret;
    }
}
