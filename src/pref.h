/*
 *      pref.h
 *
 *      Copyright (C) 2007 PCMan <pcman.tw@gmail.com>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 */

#ifndef _PREF_H_
#define _PREF_H_

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

G_BEGIN_DECLS

typedef struct _Pref
{
    gboolean auto_save_rotated; /* Save rotated images */
    gboolean ask_before_save;
    gboolean rotate_exif_only;
    gboolean ask_before_delete;
    gboolean open_maximized;
    int slide_delay;
    GdkColor background_color;
    GdkColor background_color_fullscreen;
    gboolean background_color_auto_adjust;
    gboolean colorize_scrollbars;

    int jpg_quality;
    int png_compression;

    gboolean quit_on_escape;
    gboolean show_toolbar;
    gboolean show_toolbar_fullscreen;
    gboolean toolbar_on_top;
    gboolean show_scrollbars;
    gboolean preload_images;

    GdkInterpType downscale_interpolation_mode;
    GdkInterpType upscale_interpolation_mode;
}Pref;

extern Pref pref; /* global variable holding user prerefences */

void load_preferences(); /* load user preferences */
void save_preferences(); /* save user preference s*/
void edit_preferences( GtkWindow* parent );

G_END_DECLS

#endif
