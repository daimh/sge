#ifndef DND_H
#define DND_H

void InitializeDropSite P_((Widget w));
void start_drag P_((Widget aw, XEvent *event, String *params, 
                        Cardinal *num_params));
#endif   /* DND_H */
