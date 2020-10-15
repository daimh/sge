#ifndef __CL_XML_PARSING_H
#define __CL_XML_PARSING_H

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
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2001 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 ************************************************************************/
/*___INFO__MARK_END__*/

#include <netinet/in.h>         /* OpenBSD */
#include <arpa/inet.h>

#define CL_DEFINE_MAX_MESSAGE_LENGTH                 1024 * 1024 * 1024 /* 1GB max message length */



/* (1) gmsh message (GMSH) 
 
      Format: 

      <gmsh>
         <dl>GMSH_DL</dl>
      </gmsh>

      GMSH_DL:     Data Length of the following XML struct in byte. (1 Byte = 8 bit)

 */
#define CL_GMSH_MESSAGE      "<gmsh><dl>%ld</dl></gmsh>"
#define CL_GMSH_MESSAGE_SIZE 22         /* size in byte of raw CL_GMSH_MESSAGE without parameter */


/* (2) Message Information Header (MIH) 
 
      Format:

      <mih version="MIH_VERSION">
         <mid>MIH_MID</mid>
         <dl>MIH_DL</dl>
         <df>MIH_DF</df>
         <mat>MIH_MAT</mat>
         <tag>MIH_TAG</tag>
         <rid>MIH_RID</rid>
      </mih>

      MIH_VERSION: Version number of MIH (e.g. "0.4")
      MIH_MID:     Message ID (unsigned)
      MIH_DL:      Data Length of the following XML struct in byte
      MIH_DF:      Data Format of the following message:
                      "bin"  = binary message  "xml" (Binary or XML data encoding)
                      "xml"  = general xml message
                      "am"   = acknowledge message (AM)
                      "sim"  = status information message (SIM)
                      "sirm" = status information response message (SIRM)
                      "ccm"  = connection close message (CCM)
                      "ccrm" = connection close response message (CCRM) 
      MIH_MAT:     Message Acknowledge Type:
                      "nak"  = not acknowledged
                      "ack"  = acknowledged after commlib has read the message
                      "sync" = acknowledged after application has read the message from NGC AND processed the message
      MIH_TAG:     User defined
      MIH_RID:     Response ID

      

 */
#define CL_MIH_MESSAGE          "<mih version=\"%s\"><mid>%ld</mid><dl>%ld</dl><df>%s</df><mat>%s</mat><tag>%ld</tag><rid>%ld</rid></mih>"
#define CL_MIH_MESSAGE_VERSION  "0.1"
#define CL_MIH_MESSAGE_SIZE     87 /* sizeof(CL_MIH_MESSAGE) + sizeof(CCL_MIH_MESSAGE_VERSION */
#define CL_MIH_MESSAGE_ACK_TYPE_NAK     "nak"
#define CL_MIH_MESSAGE_ACK_TYPE_ACK     "ack"
#define CL_MIH_MESSAGE_ACK_TYPE_SYNC    "sync"
#define CL_MIH_MESSAGE_DATA_FORMAT_BIN  "bin"
#define CL_MIH_MESSAGE_DATA_FORMAT_XML  "xml"
#define CL_MIH_MESSAGE_DATA_FORMAT_AM   "am"
#define CL_MIH_MESSAGE_DATA_FORMAT_SIM  "sim"
#define CL_MIH_MESSAGE_DATA_FORMAT_SIRM "sirm"
#define CL_MIH_MESSAGE_DATA_FORMAT_CCM  "ccm"
#define CL_MIH_MESSAGE_DATA_FORMAT_CCRM "ccrm"
#define CL_MIH_MESSAGE_DATA_FORMAT_CM   "cm"
#define CL_MIH_MESSAGE_DATA_FORMAT_CRM  "crm"


/* (3) Acknowledge Message (AM) 
      
      Format:

      <am version="AM_VERSION">
         <mid>AM_MID</mid>
      </am>

      AM_VERSION: Version number of AM (e.g. "0.4")
      AM_MID:     Acknowledged message id


*/
#define CL_AM_MESSAGE       "<am version=\"%s\"><mid>%ld</mid></am>"
#define CL_AM_MESSAGE_VERSION "0.1"
#define CL_AM_MESSAGE_SIZE  34 /* sizeof(CL_AM_MESSAGE) + sizeof(CL_AM_MESSAGE_VERSION) */

/* (4) connect message (CM) 

      Format:
  
      <cm version="CM_VERSION">
         <df>CM_DF</df>
         <ct>CM_CT</ct>
         <src host="CM_SRC_HOST" comp="CM_SRC_COMP" id="CM_SRC_ID"></src>
         <dst host="CM_DST_HOST" comp="CM_DST_COMP" id="CM_DST_ID"></dst>
         <rdata host="CM_RDATA_HOST" comp="CM_RDATA_COMP" id="CM_RDATA_ID"></rdata>
         <port>CM_PORT</port>
         <ac>CM_AC</ac>
      </cm>
      
      CM_VERSION:     Version number of CM (e.g. "0.4")
      CM_DF:          "bin" or "xml" (connection Binary or XML data encoding)
      CM_CT:          "stream" or "message" (connection data flow control)
      CM_SRC_HOST:    Host name of source endpoint
      CM_SRC_COMP:    Component name of source endpoint
      CM_SRC_ID:      Component id of source endpoint
      CM_DST_HOST:    Host name of destination endpoint
      CM_DST_COMP:    Component name of destination endpoint
      CM_DST_ID:      Component id of destination endpoint
      CM_RDATA_HOST:  Host name of routing host endpoint
      CM_RDATA_COMP:  Component name of routing host endpoint
      CM_RDATA_ID:    Component id of routing host endpoint
      CM_PORT:        port where client is reachable (e.g. "0" not reachable, "5000" for port 5000 )
      CM_AC:          "enabled" or "disabled", used for auto close of connected clients when max. connection
                      count is reached at service component.

*/
#define CL_CONNECT_MESSAGE         "<cm version=\"%s\"><df>%s</df><ct>%s</ct><dst host=\"%s\" comp=\"%s\" id=\"%ld\"></dst><rdata host=\"%s\" comp=\"%s\" id=\"%ld\"></rdata><port>%ld</port><ac>%s</ac></cm>"
#define CL_CONNECT_MESSAGE_VERSION "0.4"
#define CL_CONNECT_MESSAGE_SIZE   100 + 33 /* sizeof(CL_CONNECT_MESSAGE) + sizeof(CL_CONNECT_MESSAGE_VERSION) */
#define CL_CONNECT_MESSAGE_DATA_FORMAT_BIN    "bin"
#define CL_CONNECT_MESSAGE_DATA_FORMAT_XML    "xml"
#define CL_CONNECT_MESSAGE_DATA_FLOW_STREAM   "stream"
#define CL_CONNECT_MESSAGE_DATA_FLOW_MESSAGE  "message"
#define CL_CONNECT_MESSAGE_AUTOCLOSE_ENABLED  "enabled"
#define CL_CONNECT_MESSAGE_AUTOCLOSE_DISABLED "disabled"



/* (5) connect response message (CRM) 

      Format:

      <crm version="CRM_VERSION">
         <cs condition="CRM_CS_CONDITION">CRM_CS_TEXT</cs>
         <src host="CRM_SRC_HOST" comp="CRM_SRC_COMP" id="CRM_SRC_ID"></src>
         <dst host="CRM_DST_HOST" comp="CRM_DST_COMP" id="CRM_DST_ID"></dst>
         <rdata host="CRM_RDATA_HOST" comp="CRM_RDATA_COMP" id="CRM_RDATA_ID"></rdata>
         <params>CRM_QMASTER_PARAMS</params>
      </crm>

      CRM_VERSION:        Version number of CRM (e.g. "0.4")
      CRM_CS_CONDITION:   Connection Status: 
                             "connected"                -> No Errors
                             "access denied"            -> Service doesn't allow client to connect
                             "unsupported data format"  -> Message Format error
                             "endpoint not unique"      -> Client is already connected
      CRM_CS_TEXT:        User defined connection status error text
      CRM_SRC_HOST:       Host name of source endpoint
      CRM_SRC_COMP:       Component name of source endpoint
      CRM_SRC_ID:         Component id of source endpoint
      CRM_DST_HOST:       Host name of destination endpoint
      CRM_DST_COMP:       Component name of destination endpoint
      CRM_DST_ID:         Component id of destination endpoint
      CRM_RDATA_HOST:     Host name of routing host endpoint
      CRM_RDATA_COMP:     Component name of routing host endpoint
      CRM_RDATA_ID:       Component id of routing host endpoint
      CRM_QMASTER_PARAMS: Qmaster params eg: name=value:name=value:.....
      


*/
#define CL_CONNECT_RESPONSE_MESSAGE                              "<crm version=\"%s\"><cs condition=\"%s\">%s</cs><rdata host=\"%s\" comp=\"%s\" id=\"%ld\"></rdata><params>%s</params></crm>"
#define CL_CONNECT_RESPONSE_MESSAGE_VERSION                      "0.3"
#define CL_CONNECT_RESPONSE_MESSAGE_SIZE                         101 /* sizeof(CL_CONNECT_RESPONSE_MESSAGE) + sizeof(CL_CONNECT_RESPONSE_MESSAGE_VERSION) */
#define CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_OK         "connected"
#define CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_DENIED     "access denied"
#define CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_STATUS_NOT_UNIQUE "endpoint not unique"
#define CL_CONNECT_RESPONSE_MESSAGE_CONNECTION_UNSUP_DATA_FORMAT "unsupported data format"

/* (6) status information message (SIM) 
   
      Format:

      <sim version="SIM_VERSION">
      </sim>

      SIM_VERSION: version number of SIM (e.g. "0.4")


*/
#define CL_SIM_MESSAGE               "<sim version=\"%s\"></sim>"
#define CL_SIM_MESSAGE_VERSION       "0.1"
#define CL_SIM_MESSAGE_SIZE          25 /* sizeof(CL_SIM_MESSAGE) + sizeof(CL_SIM_MESSAGE_VERSION */

/* (7) status information response message (SIRM)
   
      Format:

      <sirm version="SIRM_VERSION">
         <mid>SIRM_MID/mid>
         <starttime>SIRM_STARTTIME</starttime>
         <runtime>SIRM_RUNTIME</runtime>
         <application>
            <messages>
               <brm>SIRM_APPLICATION_MESSAGES_BRM</brm>
               <bwm>SIRM_APPLICATION_MESSAGES_BWM</bwm>
            </messages>
            <connections>
               <noc>SIRM_APPLICATION_CONNECTIONS_NOC</noc>
            </connections>
            <status>SIRM_APPLICATION_STATUS</status>
         </application>
         <info>SIRM_INFO</info>
      </sirm>

      SIRM_VERSION:                       Version number of CCM (e.g. "0.4")
      SIRM_MID:                           Message Id of SIRM
      SIRM_STARTTIME:                     (unsigned Integer) Starttime of service (Unix timestamp)
      SIRM_RUNTIME:                       (unsigned Integer) Runtime since starttime (in seconds)
      SIRM_APPLICATION_MESSAGES_BRM:      Buffered read messages for application (service)
      SIRM_APPLICATION_MESSAGES_BWM:      Buffered write messages from application (service)
      SIRM_APPLICATION_CONNECTIONS_NOC:   No. of connected clients
      SIRM_APPLICATION_STATUS:            (Unsigned Integer) Application status
      SIRM_INFO:                          Application status information string
      

 */
#define CL_SIRM_MESSAGE            "<sirm version=\"%s\"><mid>%ld</mid><starttime>%ld</starttime><runtime>%ld</runtime><application><messages><brm>%ld</brm><bwm>%ld</bwm></messages><connections><noc>%ld</noc></connections><status>%ld</status></application><info>%s</info></sirm>"
#define CL_SIRM_MESSAGE_VERSION    "0.1" 
#define CL_SIRM_MESSAGE_SIZE       218  /* sizeof(CL_SIRM_MESSAGE) + sizeof(CL_SIRM_MESSAGE_VERSION) */


/* (8) connection close message (CCM) 
      
      Format:

      <ccm version="CCM_VERSION"></ccm>

      CCM_VERSION: version number of CCM (e.g. "0.4")


 */
#define CL_CCM_MESSAGE                              "<ccm version=\"%s\"></ccm>" 
#define CL_CCM_MESSAGE_VERSION                      "0.1"
#define CL_CCM_MESSAGE_SIZE                         25 /* sizeof(CL_CCM_MESSAGE) + sizeof(CL_CCM_MESSAGE_VERSION) */

/* (9) connection close response message (CCRM) 

      Format:

      <ccrm version="CCRM_VERSION"></ccrm>

      CCRM_VERSION: version number of CCRM (e.g. "0.4")

*/
#define CL_CCRM_MESSAGE                              "<ccrm version=\"%s\"></ccrm>"
#define CL_CCRM_MESSAGE_VERSION                      "0.1"
#define CL_CCRM_MESSAGE_SIZE                         27 /* sizeof(CL_CCRM_MESSAGE) + sizeof(CL_CCRM_MESSAGE_VERSION) */



typedef struct cl_com_endpoint {
   /* internal identification tripple */
   char*         comp_host;           
   char*         comp_name;
   unsigned long comp_id;
   struct in_addr addr;
   char*         hash_id;
} cl_com_endpoint_t ;



typedef enum cl_xml_data_format_def {
   CL_CM_DF_UNDEFINED = 1,
   CL_CM_DF_BIN,
   CL_CM_DF_XML
} cl_xml_data_format_t;
typedef enum cl_xml_mih_data_format_def {
   CL_MIH_DF_UNDEFINED = 1,
   CL_MIH_DF_BIN,
   CL_MIH_DF_XML,
   CL_MIH_DF_AM,
   CL_MIH_DF_SIM,
   CL_MIH_DF_SIRM,
   CL_MIH_DF_CCM,
   CL_MIH_DF_CCRM,
   CL_MIH_DF_CM,
   CL_MIH_DF_CRM
} cl_xml_mih_data_format_t;

typedef enum cl_xml_ack_type_def {
   CL_MIH_MAT_UNDEFINED = 1,
   CL_MIH_MAT_NAK,
   CL_MIH_MAT_ACK,
   CL_MIH_MAT_SYNC
} cl_xml_ack_type_t;

typedef enum cl_xml_connection_type_def {
   CL_CM_CT_UNDEFINED = 1,
   CL_CM_CT_STREAM,
   CL_CM_CT_MESSAGE
}cl_xml_connection_type_t ;

typedef enum cl_xml_connection_autoclose_def {
   CL_CM_AC_UNDEFINED = 1,
   CL_CM_AC_ENABLED,
   CL_CM_AC_DISABLED
}cl_xml_connection_autoclose_t;

typedef enum cl_xml_connection_status_def {
   CL_CRM_CS_UNDEFINED = 1,         /* 1 */
   CL_CRM_CS_CONNECTED,             /* 2 */
   CL_CRM_CS_DENIED,                /* 3 */
   CL_CRM_CS_ENDPOINT_NOT_UNIQUE,   /* 4 */
   CL_CRM_CS_UNSUPPORTED            /* 5 */
}cl_xml_connection_status_t ;


/* XML data types */
typedef struct cl_com_GMSH_type {
   unsigned long   dl;
} cl_com_GMSH_t;

typedef struct cl_com_CM_type {
   char*                          version;
   cl_xml_data_format_t           df;
   cl_xml_connection_type_t       ct;
   cl_xml_connection_autoclose_t  ac;
   unsigned long                  port;  
   cl_com_endpoint_t*             dst;   /* destination (=endpoint where clients want to connect to) */
   cl_com_endpoint_t*             rdata; /* remote */
} cl_com_CM_t;


typedef struct cl_com_CRM_type {
   char*                         version;
   cl_xml_connection_status_t    cs_condition;
   char*                         cs_text;
   char*                         formats;   /* each format is seperated with "," not supported TODO  */
   cl_com_endpoint_t*            rdata;
   char*                         params;
} cl_com_CRM_t;

typedef struct cl_com_MIH_type {
   char*                      version;
   unsigned long              mid;
   unsigned long              dl;
   cl_xml_mih_data_format_t   df;
   cl_xml_ack_type_t          mat;
   unsigned long              tag;
   unsigned long              rid;
} cl_com_MIH_t;

typedef struct cl_com_AM_type {
   char*            version;
   unsigned long    mid;
} cl_com_AM_t;

typedef struct cl_com_SIM_type {
   char*            version;
} cl_com_SIM_t;

typedef struct cl_com_SIRM_type {
   char*            version;
   unsigned long    mid;
   unsigned long    starttime;
   unsigned long    runtime;
   unsigned long    application_messages_brm;
   unsigned long    application_messages_bwm;
   unsigned long    application_connections_noc;
   unsigned long    application_status;
   char*            info;
} cl_com_SIRM_t;

typedef struct cl_com_CCM_type {
   char*            version;
} cl_com_CCM_t;

typedef struct cl_com_CCRM_type {
   char*            version;
} cl_com_CCRM_t;



const char* cl_com_get_mih_df_string(cl_xml_mih_data_format_t df);
const char* cl_com_get_mih_mat_string(cl_xml_ack_type_t mat);


/* endpoint helper functions */
cl_com_endpoint_t* cl_com_create_endpoint(const char* comp_host,
                                          const char* comp_name,
                                          unsigned long comp_id,
                                          const struct in_addr *in_addr);
cl_com_endpoint_t* cl_com_dup_endpoint(cl_com_endpoint_t* endpoint);
int  cl_com_free_endpoint(cl_com_endpoint_t** endpoint);



/* message functions */
int cl_com_free_gmsh_header(cl_com_GMSH_t** header);
int cl_com_free_cm_message(cl_com_CM_t** message);
int cl_com_free_crm_message(cl_com_CRM_t** message);
int cl_com_free_mih_message(cl_com_MIH_t** message);
int cl_com_free_am_message(cl_com_AM_t** message);
int cl_com_free_sim_message(cl_com_SIM_t** message);
int cl_com_free_sirm_message(cl_com_SIRM_t** message);
int cl_com_free_ccm_message(cl_com_CCM_t** message);
int cl_com_free_ccrm_message(cl_com_CCRM_t** message);


/* xml parsing functions */
int cl_xml_parse_GMSH(unsigned char* buffer, unsigned long buffer_length, cl_com_GMSH_t* header,unsigned long *used_buffer_length );
int cl_xml_parse_CM(unsigned char* buffer, unsigned long buffer_length, cl_com_CM_t** connection_message );
int cl_xml_parse_CRM(unsigned char* buffer, unsigned long buffer_length, cl_com_CRM_t** connection_message );
int cl_xml_parse_MIH(unsigned char* buffer, unsigned long buffer_length, cl_com_MIH_t** message );
int cl_xml_parse_AM(unsigned char* buffer, unsigned long buffer_length, cl_com_AM_t** message );
int cl_xml_parse_SIM(unsigned char* buffer, unsigned long buffer_length, cl_com_SIM_t** message );
int cl_xml_parse_SIRM(unsigned char* buffer, unsigned long buffer_length, cl_com_SIRM_t** message );
int cl_xml_parse_CCM(unsigned char* buffer, unsigned long buffer_length, cl_com_CCM_t** message );
int cl_xml_parse_CCRM(unsigned char* buffer, unsigned long buffer_length, cl_com_CCRM_t** message );

int cl_com_transformString2XML(const char* input, char** output);
int cl_com_transformXML2String(const char* input, char** output);
#endif /* __CL_XML_PARSING_H */

