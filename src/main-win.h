/***************************************************************************
 *   Copyright (C) 2007 by PCMan (Hong Jen Yee)   *
 *   pcman.tw@gmail.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef MAINWIN_H
#define MAINWIN_H

#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>

#include "image-view.h"
#include "image-list.h"

/**
    @author PCMan (Hong Jen Yee) <pcman.tw@gmail.com>
*/

#define MAIN_WIN_TYPE        (main_win_get_type ())
#define MAIN_WIN(obj)        (G_TYPE_CHECK_INSTANCE_CAST ((obj), MAIN_WIN_TYPE, MainWin))
#define MAIN_WIN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MAIN_WIN_TYPE, MainWinClass))
#define IS_MAIN_WIN(obj)     (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MAIN_WIN_TYPE))
#define IS_MAIN_WIN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), MAIN_WIN_TYPE))
#define MAIN_WIN_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MAIN_WIN_TYPE, MainWinClass))

typedef enum
{
     ZOOM_FIT,
     ZOOM_SCALE
} ZoomMode;

typedef struct _MainWinClass
{
    GtkWindowClass parent_class;
} MainWinClass;

typedef struct _MainWin
{
    GtkWindow parent;

    GdkPixbuf* pix;
    GdkPixbufAnimation* animation;
    GdkPixbufAnimationIter* animation_iter;
    guint animation_timeout;

    GtkWidget* img_view;
    GtkWidget* scroll;
    GtkWidget* evt_box;

    GtkWidget* nav_bar_alignment;
    GtkWidget* nav_bar;

    GtkWidget* btn_prev;
    GtkWidget* btn_next;
    GtkWidget* btn_play_stop;

    GtkWidget* btn_zoom_out;
    GtkWidget* btn_zoom_in;
    GtkWidget* btn_zoom_fit;
    GtkWidget* btn_zoom_orig;

    GtkWidget* btn_full_screen;

    GtkWidget* btn_rotate_cw;
    GtkWidget* btn_rotate_ccw;
    GtkWidget* btn_flip_v;
    GtkWidget* btn_flip_h;

    GtkWidget* btn_open;
    GtkWidget* btn_save_file;
    GtkWidget* btn_save_copy;
    GtkWidget* btn_delete_file;

    GtkWidget* btn_preference;
    GtkWidget* btn_quit;

    GtkWidget* img_play_stop;
    GtkWidget* percent;
    GdkCursor* hand_cursor;
    GdkCursor* busy_cursor;

    int ui_disabled;
    gboolean file_action_enabled;
    gboolean prev_action_enabled;
    gboolean next_action_enabled;
    gboolean play_stop_action_enabled;
    gboolean zoom_out_action_enabled;
    gboolean zoom_in_action_enabled;
    gboolean zoom_fit_action_enabled;
    gboolean zoom_orig_action_enabled;
    gboolean rotate_cw_action_enabled;
    gboolean rotate_ccw_action_enabled;
    gboolean flip_v_action_enabled;
    gboolean flip_h_action_enabled;
    gboolean save_file_action_enabled;
    gboolean save_copy_action_enabled;
    gboolean delete_file_action_enabled;

    GtkAllocation scroll_allocation;

    ZoomMode zoom_mode;
    gboolean full_screen;
    gboolean slideshow_running;
    gboolean slideshow_cancelled;
    guint slide_timeout;
    gboolean dragging;
    double scale;
    int drag_old_x;
    int drag_old_y;
    int rotation_angle;
    ImageList* image_list;

    gboolean saving_is_in_progress;

    guint preload_next_timeout;
    guint preload_prev_timeout;
    guint image_cache_yield_timeout;

    GdkColor background_color_from_image;
    gboolean background_color_from_image_valid;
} MainWin;

GtkWidget* main_win_new();

gboolean main_win_open( MainWin* mw, const char* file_path, ZoomMode zoom );

void main_win_start_slideshow( MainWin* mw );

void main_win_close( MainWin* mw );

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm );

void main_win_show_error( MainWin* mw, const char* message );

void main_win_fit_size( MainWin* mw, int width, int height, gboolean can_strech);

void main_win_fit_window_size( MainWin* mw, gboolean can_strech);

void main_win_center_image( MainWin* mw );

void main_win_set_scale(MainWin * mw, double new_scale);
void main_win_on_scale_preferences_changed(MainWin * mw);

void main_win_update_background_color(MainWin* mw);


GType main_win_get_type();

#endif
