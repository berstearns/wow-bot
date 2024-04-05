/*
 * grab-window.c - return the XID of a window the user selects by clicking
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

#include <X11/Xmu/WinUtil.h>
#include <gdk/gdkx.h>

int
grab_window_xid( void )
{
  Cursor cursor;
  Display *dpy;
  int pressed;
  Window root;
  Window win;

  dpy = gdk_x11_get_default_xdisplay();
  root = gdk_x11_get_default_root_xwindow();
  cursor = XCreateFontCursor( dpy, GDK_CROSSHAIR );

  if( GrabSuccess != XGrabPointer( dpy, root, False,
                                   ButtonPressMask | ButtonReleaseMask,
                                   GrabModeSync, GrabModeAsync, None,
                                   cursor, CurrentTime ) )
  {
    return 0;
  }

  win = pressed = 0;

  while( win == 0 || pressed )
  {
    XEvent event;

    XAllowEvents( dpy, SyncPointer, CurrentTime );
    XWindowEvent( dpy, root, ButtonPressMask | ButtonReleaseMask, &event );

    switch( event.type )
    {
      case ButtonPress:
        win = event.xbutton.subwindow;
        pressed++;
        continue;

      case ButtonRelease:
        if( pressed )
          pressed--;
        continue;
    }
  }

  XUngrabPointer( dpy, CurrentTime );
  XFreeCursor( dpy, cursor );
  XSync( dpy, 0 );

  return XmuClientWindow( dpy, win );
}
