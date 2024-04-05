/*
 * keyboardcast.c - broadcast keystrokes to multiple windows
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

#include <glade/glade.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <stdlib.h>
#include <string.h>

#include "grab-window.h"
#include "window-list.h"

static WindowList *wl;
GladeXML *xml;

static gboolean
window_list_foreach_callback( WindowList *wl, int xid, gpointer user_data )
{
  XKeyEvent *xevent = user_data;

  xevent->window = xid;
  gdk_error_trap_push();
  XSendEvent( xevent->display, xid, FALSE, KeyPressMask, (XEvent *) xevent );
  gdk_flush();
  gdk_error_trap_pop();

  return FALSE;
}

static gboolean
key_event( GtkWidget *input, GdkEventKey *event )
{
  XKeyEvent xevent;

  switch( event->type )
  {
    case GDK_KEY_PRESS:
      xevent.type = KeyPress;
      break;

    case GDK_KEY_RELEASE:
      xevent.type = KeyRelease;
      break;

    default:
      return FALSE;
  }

  xevent.display = gdk_x11_get_default_xdisplay();
  xevent.keycode = event->hardware_keycode;
  xevent.state = event->state;
  xevent.time = event->time;
  xevent.root = gdk_x11_get_default_root_xwindow();

  window_list_foreach_selected( wl, window_list_foreach_callback, &xevent );

  return TRUE;
}

static void
expanded( GtkExpander *expander, GParamSpec *ps, GtkWindow *window )
{
  gtk_window_set_resizable( window, gtk_expander_get_expanded( expander ) );
}

static gboolean
selection_foreach( GtkTreeModel *model, GtkTreePath *path,
                   GtkTreeIter *iter, gpointer data )
{
  const char *criteria = data;
  const char *name;
  gboolean active;

  gtk_tree_model_get( model, iter, 2, &name, -1 );

  active = criteria && strstr( name, criteria );

  gtk_list_store_set( GTK_LIST_STORE( model ), iter, 1, active, -1 );

  return FALSE;
}

static void
spawn_command( const char *cmd, const char *args )
{
  char **argv;
  int i;

  argv = g_strsplit( args, ",", 0 );

  for( i = 0; argv[i]; i++ )
  {
    char *command;
    command = g_strdup_printf( "gnome-terminal --window-with-profile=keyboardcast -t 'Keyboardcast Spawn' -e '%s %s'&", cmd, argv[i] );
    system( command );
    g_free( command );
  }

  g_strfreev( argv );
}

static void
update_label( GtkEntry *entry, GtkLabel *label )
{
  static char *original_label;
  const char *args;
  char *new_label;
  char **argv;
  int i;

  if( !original_label )
    original_label = g_strdup( gtk_label_get_label( label ) );

  if( entry )
  {
    args = gtk_entry_get_text( entry );

    /* do it this way so it's exactly the same as the other function */
    argv = g_strsplit( args, ",", 0 );
    for( i = 0; argv[i]; i++ );
    g_strfreev( argv );
  }
  else
    i = 4;

  new_label = g_strdup_printf( original_label, i );
  gtk_label_set_markup( label, new_label );
  g_free( new_label );
}

static void
spawn_clicked( GtkButton *button, GtkDialog *spawn_dialog )
{
  GtkWidget *command, *arguments;
  const char *cmd, *arg;
  int response;
 
  response = gtk_dialog_run( spawn_dialog );
  gtk_widget_hide( GTK_WIDGET( spawn_dialog ) );

  if( response != GTK_RESPONSE_OK )
    return;

  command = glade_xml_get_widget( xml, "command-entry" );
  arguments = glade_xml_get_widget( xml, "arguments-entry" );

  cmd = gtk_entry_get_text( GTK_ENTRY( command ) );
  arg = gtk_entry_get_text( GTK_ENTRY( arguments ) );

  spawn_command( cmd, arg );
}

static void
button_clicked( GtkButton *button, GtkEntry *select_entry )
{
  const char *label = gtk_button_get_label( button );
  const char *criteria;

  if( !strcmp( label, "_Grab" ) )
  {
    window_list_select_xid( wl, grab_window_xid() );
    return;
  }

  if( !strcmp( label, "_Select" ) )
    criteria = gtk_entry_get_text( select_entry );
  else if( !strcmp( label, "_None" ) )
    criteria = NULL;
  else if( !strcmp( label, "_All" ) )
    criteria = "";
  else
    return;

  gtk_tree_model_foreach( GTK_TREE_MODEL( wl ),selection_foreach,
                          (gpointer) criteria );
}

static void
terminal_toggled( GtkToggleButton *button )
{
  const char *process;

  if( gtk_toggle_button_get_active( button ) )
    process = "gnome-terminal";
  else
    process = NULL;

  window_list_filter_by_process( wl, process );
}

static int
setup_interface( void )
{
  GtkWidget *window, *treeview, *select_entry, *spawn_dialog, *spawn_label;
  GtkCellRenderer *renderer;

  gtk_window_set_default_icon_name( "gnome-dev-keyboard" );

  wl = window_list_new();

  xml = glade_xml_new( PREFIX "/share/keyboardcast/keyboardcast.glade",
                       NULL, NULL );

  if( xml == NULL )
    return 1;

  select_entry = glade_xml_get_widget( xml, "select-entry" );
  treeview = glade_xml_get_widget( xml, "treeview" );
  window = glade_xml_get_widget( xml, "window" );
  spawn_dialog = glade_xml_get_widget( xml, "spawn-dialog" );
  spawn_label = glade_xml_get_widget( xml, "spawn-label" );

  if( select_entry == NULL || treeview == NULL ||
      window == NULL || spawn_dialog == NULL || spawn_label == NULL )
    return 1;

  gtk_tree_view_set_model( GTK_TREE_VIEW( treeview ), GTK_TREE_MODEL( wl ) );

  renderer = window_list_toggle_renderer( wl );
  gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( treeview ), -1,
                                               "✓", renderer, // ☑☒✓✔
                                               "active", 1, NULL );

  renderer = gtk_cell_renderer_text_new();
  gtk_tree_view_insert_column_with_attributes( GTK_TREE_VIEW( treeview ), -1,
                                               "Window Title", renderer,
                                               "text", 2, NULL );

  glade_xml_signal_connect( xml, "key_event", G_CALLBACK( key_event ) );
  glade_xml_signal_connect( xml, "gtk_exit", G_CALLBACK( gtk_exit ) );
  glade_xml_signal_connect_data( xml, "expanded",
                                 G_CALLBACK( expanded ), window );
  glade_xml_signal_connect_data( xml, "button_clicked",
                                 G_CALLBACK( button_clicked ), select_entry );
  glade_xml_signal_connect_data( xml, "spawn_clicked",
                                 G_CALLBACK( spawn_clicked ), spawn_dialog );
  glade_xml_signal_connect( xml, "terminal_toggled",
                            G_CALLBACK( terminal_toggled ) );
  glade_xml_signal_connect_data( xml, "update_label",
                                 G_CALLBACK( update_label ), spawn_label );

  update_label( NULL, GTK_LABEL( spawn_label ) );

  window_list_filter_by_process( wl, "gnome-terminal" );

  gtk_widget_show_all( window );

  return 0;
}

int
main( int argc, char **argv )
{
  int ret;

  gtk_init( &argc, &argv );

  if( !(ret = setup_interface()) )
    gtk_main();

  return ret;
}
