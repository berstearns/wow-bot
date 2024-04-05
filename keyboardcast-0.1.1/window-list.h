/*
 * window-list.h - a GtkTreeModel reflecting windows currently open
 * Copyright © 2005 Ryan Lortie <desrt@desrt.ca>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110 USA
 */

#ifndef _window_list_h_
#define _window_list_h_

#include <gtk/gtk.h>

#define TYPE_WINDOW_LIST            (window_list_get_type())
#define WINDOW_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj),   \
                                     TYPE_WINDOW_LIST, WindowList))
#define WINDOW_LIST_CLASS(class)    (G_TYPE_CHECK_CLASS_CAST ((class),    \
                                     TYPE_WINDOW_LIST, WindowListwClass))
#define IS_WINDOW_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj),   \
                                     TYPE_WINDOW_LIST))
#define IS_WINDOW_LIST_CLASS(class) (G_TYPE_CHECK_CLASS_TYPE ((class),    \
                                     TYPE_WINDOW_LIST))
#define WINDOW_LIST_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),    \
                                     TYPE_WINDOW_LIST, WindowListwClass))

typedef struct WindowList WindowList;

typedef gboolean (*WindowListForeachFunc)( WindowList *wl, int xid, gpointer user_data );

GType window_list_get_type( void );
WindowList* window_list_new( void );
GtkCellRenderer *window_list_toggle_renderer( WindowList *wl );
void window_list_foreach_selected( WindowList *wl, WindowListForeachFunc func,
                                   gpointer user_data );
void window_list_filter_by_process( WindowList *wl, const char *process );
void window_list_select_xid( WindowList *wl, int xid );

#endif
