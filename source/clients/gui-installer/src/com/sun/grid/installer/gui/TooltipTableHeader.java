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
package com.sun.grid.installer.gui;

import java.awt.event.MouseEvent;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableColumnModel;

/**
 * Class extending JTableHeader with tooltip functionality
 *
 * @see JTableHeader
 */
public class TooltipTableHeader extends JTableHeader {
    private String[] toolTips;

    /**
     * Cunstructor
     * @param model The table column model
     * @param toolTips Tooltips for column headers
     */
    public TooltipTableHeader(TableColumnModel model, String[] toolTips) {
        super(model);

        this.toolTips = toolTips;
    }

    @Override
    public String getToolTipText(MouseEvent event) {
        int col = columnAtPoint(event.getPoint());
        int colIndex = getTable().convertColumnIndexToModel(col);
        String tooltip = null;

        try {
            if (toolTips.length > colIndex && !toolTips[colIndex].equals("")) {
                tooltip = toolTips[colIndex];
            }
        } catch (NullPointerException e) {
            tooltip = null;
        } catch (ArrayIndexOutOfBoundsException e) {
            tooltip = null;
        }

        return tooltip;
    }
}
