/*
 * window-list.c - a GtkTreeModel reflecting windows currently open
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

#define WNCK_I_KNOW_THIS_IS_UNSTABLE wankfactor

#include <libwnck/libwnck.h>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <string.h>

#include "window-list.h"

struct WindowList
{
  GtkListStore store;
  char *filter;
};

typedef struct
{
  GtkListStoreClass parent_class;
} WindowListClass;

G_DEFINE_TYPE( WindowList, window_list, GTK_TYPE_LIST_STORE );

static void
window_list_class_init( WindowListClass *class )
{
}

static gboolean
window_belongs_to( WnckWindow *win, const char *program )
{
  char destination[1024];
  char *path;
  int pid;
  int len;

  pid = wnck_window_get_pid( win );

  if( pid < 0 || pid == getpid() ) // never list myself
    return FALSE;

  path = g_strdup_printf( "/proc/%d/exe", pid );
  len = readlink( path, destination, sizeof destination );
  g_free( path );

  if( len < 0 || len >= sizeof destination )
    return FALSE;

  destination[len] = '\0';

  return strstr( destination, program ) != NULL;
}

static void
window_renamed( WnckWindow *win, WindowList *wl )
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = GTK_TREE_MODEL( wl );

  if( !gtk_tree_model_get_iter_first( model, &iter ) )
    return;

  do
  {
    WnckWindow *window;

    gtk_tree_model_get( model, &iter, 0, &window, -1 );

    if( window == win )
    {
      gtk_list_store_set( GTK_LIST_STORE( wl ), &iter, 2,
                          wnck_window_get_name( win ), -1 );
      break;
    }
  } while( gtk_tree_model_iter_next( model, &iter ) );
}

static void
window_opened( WnckScreen *screen, WnckWindow *win, WindowList *wl )
{
  GtkTreeIter iter;
  gboolean active;

  g_signal_connect( win, "name_changed", G_CALLBACK( window_renamed ), wl );

  if( window_belongs_to( win, wl->filter ) )
  {
    active = window_belongs_to( win, "gnome-terminal" ) &&
             !strcmp( "Keyboardcast Spawn", wnck_window_get_name( win ) );

    gtk_list_store_insert_with_values( GTK_LIST_STORE( wl ), &iter, 0,
                                       0, win,
                                       1, active,
                                       2, wnck_window_get_name( win ),
                                       -1 );
  }
}

static void
window_closed( WnckScreen *screen, WnckWindow *win, WindowList *wl )
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = GTK_TREE_MODEL( wl );

  if( !gtk_tree_model_get_iter_first( model, &iter ) )
    return;

  do
  {
    WnckWindow *window;

    gtk_tree_model_get( model, &iter, 0, &window, -1 );

    if( window == win )
    {
      gtk_list_store_remove( GTK_LIST_STORE( wl ), &iter );
      break;
    }
  } while( gtk_tree_model_iter_next( model, &iter ) );
}

static void
window_list_fill( WindowList *wl )
{
  GList *winlist, *item;
  WnckScreen *screen;

  gtk_list_store_clear( GTK_LIST_STORE( wl ) );

  screen = wnck_screen_get_default();

  winlist = wnck_screen_get_windows_stacked( screen );
  for( item = winlist; item; item = item->next )
  {
    WnckWindow *window = item->data;
    GtkTreeIter iter;

    if( window_belongs_to( window, wl->filter ) )
      gtk_list_store_insert_with_values( GTK_LIST_STORE( wl ), &iter, 0,
                                         0, window,
                                         1, FALSE,
                                         2, wnck_window_get_name( window ),
                                         -1 );
  }
}

static void
window_list_init( WindowList *wl )
{
  GType types[] = { G_TYPE_POINTER, G_TYPE_BOOLEAN, G_TYPE_STRING };
  WnckScreen *screen;

  wl->filter = g_strdup( "" );

  gtk_list_store_set_column_types( GTK_LIST_STORE( wl ), 3, types );

  screen = wnck_screen_get_default();

  g_signal_connect( screen, "window_opened",
                    G_CALLBACK( window_opened ), wl );
  g_signal_connect( screen, "window_closed",
                    G_CALLBACK( window_closed ), wl );

  window_list_fill( wl );
}

static void
cell_toggled_callback( GtkCellRendererToggle *cell, const char *path, GtkListStore *store )
{
  GtkTreeIter iter;
  gboolean active;

  gtk_tree_model_get_iter_from_string( GTK_TREE_MODEL( store ), &iter, path );
  gtk_tree_model_get( GTK_TREE_MODEL( store ), &iter, 1, &active, -1 );
  gtk_list_store_set( store, &iter, 1, !active, -1 );
}

GtkCellRenderer *
window_list_toggle_renderer( WindowList *wl )
{
  GtkCellRenderer *render;

  render = gtk_cell_renderer_toggle_new();
  g_object_set( render, "activatable", TRUE, NULL );
  g_signal_connect( render, "toggled",
                    G_CALLBACK( cell_toggled_callback ), wl );

  return render;
}

void
window_list_foreach_selected( WindowList *wl, WindowListForeachFunc func,
                              gpointer user_data )
{
  GtkTreeModel *model;
  GtkTreeIter iter;

  model = GTK_TREE_MODEL( wl );

  if( !gtk_tree_model_get_iter_first( model, &iter ) )
    return;

  do
  {
    WnckWindow *window;
    gboolean active;

    gtk_tree_model_get( model, &iter, 0, &window, 1, &active, -1 );

    if( active )
      if( func( wl, wnck_window_get_xid( window ), user_data ) )
        break;

  } while( gtk_tree_model_iter_next( model, &iter ) );
}

void
window_list_filter_by_process( WindowList *wl, const char *process )
{
  if( process == NULL )
    process = "";

  if( !strcmp( process, wl->filter ) )
    return;

  g_free( wl->filter );
  wl->filter = g_strdup( process );

  window_list_fill( wl );
}

static gboolean
select_by_xid( GtkTreeModel *model, GtkTreePath *path,
               GtkTreeIter *iter, gpointer user_data )
{
  int xid = (int) user_data;
  WnckWindow *win;

  gtk_tree_model_get( model, iter, 0, &win, -1 );

  if( wnck_window_get_xid( win ) != xid )
    return FALSE;

  gtk_list_store_set( GTK_LIST_STORE( model ), iter, 1, TRUE, -1 );
  return TRUE;
}

void
window_list_select_xid( WindowList *wl, int xid )
{
  gtk_tree_model_foreach( GTK_TREE_MODEL( wl ), select_by_xid, (gpointer) xid );
}


WindowList*
window_list_new (void)
{
  return g_object_new (TYPE_WINDOW_LIST, NULL);
}
