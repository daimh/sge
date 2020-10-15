<?xml version="1.0" encoding="ISO-8859-1"?>
<!DOCTYPE jgdi.todo [
<!ENTITY whitespace "&nbsp;">
<!ENTITY trademark  "&trade;">
]>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
   <xsl:output method="html" version="1.0" encoding="ISO-8859-1" indent="yes"/>
   
   <xsl:template match="/">
      <html>
         <head>
            <title>Java Admin API - Overview</title>
            <LINK REL ="stylesheet" TYPE="text/css" HREF="stylesheet.css" TITLE="Style"/>
         </head>
         <body>
            <xsl:apply-templates/>
         </body>
      </html>
   </xsl:template>
   
   <xsl:template match="control">
      <p>
         Java API for all administrative tasks of the Sun Grid Engine
      </p>
      
      <h1>Milestones</h1>
      
      <table BORDER="0" WIDTH="100%" CELLPADDING="3" CELLSPACING="0" SUMMARY="">
         <tr BGCOLOR="#CCCCFF" CLASS="TableHeadingColor">
            <th align='left'><font CLASS="NavBarFont1">Milestone</font></th>
            <th align='left'><font CLASS="NavBarFont1">Release Date</font></th>
            <th align='left'><font CLASS="NavBarFont1">Description</font></th>
         </tr>
         <xsl:apply-templates select="/control/*/milestone"/>
      </table>
      
      <h1>Todo List</h1>
      
      <table BORDER="0" WIDTH="100%" CELLPADDING="3" CELLSPACING="0" SUMMARY="">
         <tr BGCOLOR="#CCCCFF" CLASS="TableHeadingColor">
            <th align='left'><font CLASS="NavBarFont1">Element</font></th>
            <th align='left'><font CLASS="NavBarFont1">Line</font></th>
            <th align='left'><font CLASS="NavBarFont1">TODO</font></th>
            <th align='left'><font CLASS="NavBarFont1">Target</font></th>
            <th align='left'><font CLASS="NavBarFont1">Duration</font></th>
            <th align='left'><font CLASS="NavBarFont1">Assigned To</font></th>
         </tr>         
         <xsl:apply-templates/>
      </table>
   </xsl:template>
   
   <xsl:template match="package">
      <tr CLASS="TableRowColor">
         <th align='left' colspan='6' class='NavBarCell1'>
            <font CLASS="NavBarFont1">Package <xsl:value-of select="@name"/></font>
         </th>
      </tr>
      <xsl:apply-templates select='todo'/>
      <xsl:apply-templates select="class"/>
   </xsl:template>
   
   <xsl:template match="class">
      <tr CLASS="TableRowColor">
         <td valign='top' colspan='6' bgcolor="#C0C0C0">
            <div style="margin-left:30px;">
               <font CLASS="NavBarFont1">Class <xsl:value-of select="@name"/></font>
            </div>
         </td>
      </tr>
      <xsl:apply-templates select='todo'/>
      <xsl:apply-templates select="constructor"/>
      <xsl:apply-templates select="method"/>
   </xsl:template>
   
   
   <xsl:template match="method">
      <tr CLASS="TableRowColor">
         <td valign='top' colspan="6" CLASS="NavBarCell3">
            <div style="margin-left:60px;">
               Method <xsl:value-of select="@name"/>
            </div>
         </td>
      </tr>
      <xsl:apply-templates select="todo"/>
   </xsl:template>
   
   <xsl:template match="constructor">
      <tr CLASS="TableRowColor">
         <td valign='top' colspan="6" CLASS="NavBarCell3">
            <div style="margin-left:60px;">
               Constructor <xsl:value-of select="@name"/>
            </div>
         </td>
      </tr>
      <xsl:apply-templates select="todo"/>
   </xsl:template>
   
   <xsl:template match="milestone">
      <tr>
         <td valign='top'><xsl:value-of select="@name"/></td>
         <td valign='top'><xsl:value-of select="@releaseDate"/></td>
         <td valign='top'><xsl:value-of select="."/></td>
      </tr>
   </xsl:template>
   
   <xsl:template match="todo">
      <tr>
         <td valign='top'>
            <xsl:text>&#xA0;  </xsl:text>
         </td>
         <td valign='top'>
            <a> <xsl:attribute name="href"><xsl:value-of select="@file"/></xsl:attribute>
               <xsl:value-of select="@line"/>           
            </a>
         </td>
         <td valign='top'><xsl:value-of select="."/><xsl:text>&#xA0;  </xsl:text></td>
         <td valign='top'><xsl:value-of select="@target"/><xsl:text>&#xA0;  </xsl:text></td>
         <td valign='top'><xsl:value-of select="@duration"/><xsl:text>&#xA0;  </xsl:text></td>
         <td valign='top'><xsl:value-of select="@assignedTo"/><xsl:text>&#xA0;  </xsl:text></td>
      </tr>         
   </xsl:template>
</xsl:stylesheet>
