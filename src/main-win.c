/***************************************************************************
 *   Copyright (c) 2012-2016 Vadim Ushakov <igeekless@gmail.com>           *
 *   Copyright (C) 2007, 2008 by PCMan (Hong Jen Yee)                      *
 *   pcman.tw@gmail.com                                                    *
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "main-win.h"

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkkeysyms-compat.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "pref.h"

#include "gtkcompat.h"
#include "image-cache.h"
#include "image-view.h"
#include "image-list.h"
#include "ptk-menu.h"
#include "file-dlgs.h"
#include "jpeg-tran.h"
#include "libsmfm_utils.h"

/*
   REFACTORING IN PROGRESS!

  UI <------------------------------+
  |                                 |
  v                                 |
  on_someevent() handlers           UI state control handlers
  |                                 ^
  v                                 |
  main_win_action_*() handlers ---> internal functions

*/

/* For drag & drop */
static GtkTargetEntry drop_targets[] =
{
    {"text/uri-list", 0, 0},
    {"text/plain", 0, 1}
};

extern int ExifRotate(const char * fname, int new_angle);
// defined in exif.c
extern int ExifRotateFlipMapping[9][9];

static void main_win_init( MainWin*mw );
static void main_win_finalize( GObject* obj );

static void create_nav_bar( MainWin* mw, GtkWidget* box);
// GtkWidget* add_menu_item(  GtkMenuShell* menu, const char* label, const char* icon, GCallback cb, gboolean toggle=FALSE );
static void rotate_image( MainWin* mw, int angle );
static void show_popup_menu( MainWin* mw, GdkEventButton* evt );

/* signal handlers */

static gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt );
static void on_size_allocate( GtkWidget* widget, GtkAllocation    *allocation );
static gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state );
static void on_scroll_size_allocate(GtkWidget* widget, GtkAllocation* allocation, MainWin* mv);
static void on_full_screen( GtkWidget* widget, MainWin* mw );

static void on_next( GtkWidget* widget, MainWin* mw );
static void on_prev( GtkWidget* widget, MainWin* mw );

static void on_zoom_in( GtkWidget* btn, MainWin* mw );
static void on_zoom_out( GtkWidget* btn, MainWin* mw );
static void on_zoom_fit( GtkWidget* widget, MainWin* mw );
static void on_zoom_fit_menu( GtkWidget* widget, MainWin* mw );
static void on_zoom_orig( GtkWidget* widget, MainWin* mw );
static void on_zoom_orig_menu( GtkWidget* widget, MainWin* mw );

static void on_rotate_auto_save( GtkWidget* btn, MainWin* mw );
static void on_rotate_clockwise( GtkWidget* btn, MainWin* mw );
static void on_rotate_counterclockwise( GtkWidget* btn, MainWin* mw );
static void on_save_copy( GtkWidget* btn, MainWin* mw );
static void on_save( GtkWidget* btn, MainWin* mw );
static void cancel_slideshow(MainWin* mw);
static gboolean next_slide(MainWin* mw);
static void on_slideshow_menu( GtkMenuItem* item, MainWin* mw );
static void on_slideshow( GtkToggleButton* btn, MainWin* mw );
static void on_open( GtkWidget* btn, MainWin* mw );
static void on_preference( GtkWidget* btn, MainWin* mw );
static void on_toggle_toolbar( GtkMenuItem* item, MainWin* mw );
static void on_quit( GtkWidget* btn, MainWin* mw );
static gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw );
static gboolean on_button_release( GtkWidget* widget, GdkEventButton* evt, MainWin* mw );
static gboolean on_mouse_move( GtkWidget* widget, GdkEventMotion* evt, MainWin* mw );
static gboolean on_scroll_event( GtkWidget* widget, GdkEventScroll* evt, MainWin* mw );
static gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key);
static gboolean save_confirm( MainWin* mw, const char* file_path );
static void on_drag_data_received( GtkWidget* widget, GdkDragContext *drag_context,
                int x, int y, GtkSelectionData* data, guint info, guint time, MainWin* mw );
static void on_delete( GtkWidget* btn, MainWin* mw );
static void on_about( GtkWidget* menu, MainWin* mw );
static gboolean on_animation_timeout( MainWin* mw );

void on_flip_vertical( GtkWidget* btn, MainWin* mw );
void on_flip_horizontal( GtkWidget* btn, MainWin* mw );
static int trans_angle_to_id(int i);
static int get_new_angle( int orig_angle, int rotate_angle );

/* action handlers */
static void main_win_action_prev(MainWin* mw);
static void main_win_action_next(MainWin* mw);
static void main_win_action_zoom_in(MainWin* mw);
static void main_win_action_zoom_out(MainWin* mw);
static void main_win_action_zoom_orig(MainWin* mw);
static void main_win_action_zoom_fit(MainWin* mw);

static void main_win_set_zoom_scale(MainWin* mw, double scale);
static void main_win_set_zoom_mode(MainWin* mw, ZoomMode mode);

/* UI state */
static void main_win_update_zoom_buttons_state(MainWin* mw);
static void main_win_update_sensitivity(MainWin* mw);
static void update_title(const char *filename, MainWin *mw );
static void main_win_update_toolbar_visibility(MainWin *mw );
static void update_toolbar_position(MainWin *mw);

static gboolean on_preload_next_timeout(MainWin* mw);
static gboolean on_preload_prev_timeout(MainWin* mw);

// Begin of GObject-related stuff

G_DEFINE_TYPE( MainWin, main_win, GTK_TYPE_WINDOW )

void main_win_class_init( MainWinClass* klass )
{
    GObjectClass * obj_class;
    GtkWidgetClass *widget_class;

    obj_class = ( GObjectClass * ) klass;
//    obj_class->set_property = _set_property;
//   obj_class->get_property = _get_property;
    obj_class->finalize = main_win_finalize;

    widget_class = GTK_WIDGET_CLASS ( klass );
    widget_class->delete_event = on_delete_event;
    widget_class->size_allocate = on_size_allocate;
    widget_class->key_press_event = on_key_press_event;
    widget_class->window_state_event = on_win_state_event;
}

void main_win_finalize( GObject* obj )
{
    MainWin *mw = (MainWin*)obj;

    main_win_close(mw);

    if( G_LIKELY(mw->image_list) )
        image_list_free( mw->image_list );

    gdk_cursor_unref(mw->hand_cursor);
    gdk_cursor_unref(mw->busy_cursor);

    if (mw->preload_next_timeout)
        g_source_remove(mw->preload_next_timeout);
    if (mw->preload_prev_timeout)
        g_source_remove(mw->preload_prev_timeout);
}

GtkWidget* main_win_new()
{
    return (GtkWidget*)g_object_new ( MAIN_WIN_TYPE, NULL );
}

// End of GObject-related stuff

void main_win_init( MainWin*mw )
{
    gtk_window_set_title( (GtkWindow*)mw, _("Image Viewer"));
    gtk_window_set_icon_from_file( (GtkWindow*)mw, PACKAGE_DATA_DIR"/pixmaps/spicview.png", NULL );
    gtk_window_set_default_size( (GtkWindow*)mw, 640, 480 );

    g_signal_connect(G_OBJECT(mw), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget* box = gtk_vbox_new(FALSE, 0);
    gtk_container_add( (GtkContainer*)mw, box);

    // image area
    mw->evt_box = gtk_event_box_new();
    gtk_widget_set_can_focus(mw->evt_box, TRUE);
    gtk_widget_add_events( mw->evt_box,
                           GDK_POINTER_MOTION_MASK|GDK_BUTTON_PRESS_MASK|
                           GDK_BUTTON_RELEASE_MASK|GDK_SCROLL_MASK );
    g_signal_connect( mw->evt_box, "button-press-event", G_CALLBACK(on_button_press), mw );
    g_signal_connect( mw->evt_box, "button-release-event", G_CALLBACK(on_button_release), mw );
    g_signal_connect( mw->evt_box, "motion-notify-event", G_CALLBACK(on_mouse_move), mw );
    g_signal_connect( mw->evt_box, "scroll-event", G_CALLBACK(on_scroll_event), mw );

    main_win_update_background_color(mw);

    mw->img_view = image_view_new();
    gtk_container_add( (GtkContainer*)mw->evt_box, (GtkWidget*)mw->img_view);

    const char scroll_style[]=
            "style \"gpicview-scroll\" {"
            "GtkScrolledWindow::scrollbar-spacing=0"
            "}"
            "class \"GtkScrolledWindow\" style \"gpicview-scroll\"";
    gtk_rc_parse_string( scroll_style );

    mw->scroll = gtk_scrolled_window_new( NULL, NULL );
    g_signal_connect(G_OBJECT(mw->scroll), "size-allocate", G_CALLBACK(on_scroll_size_allocate), (gpointer) mw);
    gtk_scrolled_window_set_shadow_type( (GtkScrolledWindow*)mw->scroll, GTK_SHADOW_NONE );
    gtk_scrolled_window_set_policy((GtkScrolledWindow*)mw->scroll,
                                    GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    GtkAdjustment *hadj, *vadj;
    hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)mw->scroll);
    gtk_adjustment_set_page_increment(hadj, 10);
    gtk_adjustment_changed(hadj);

    vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)mw->scroll);
    gtk_adjustment_set_page_increment(vadj, 10);
    gtk_adjustment_changed(vadj);

    image_view_set_adjustments( IMAGE_VIEW(mw->img_view), hadj, vadj );    // dirty hack :-(
    gtk_scrolled_window_add_with_viewport( (GtkScrolledWindow*)mw->scroll, mw->evt_box );
    GtkWidget* viewport = gtk_bin_get_child( (GtkBin*)mw->scroll );
    gtk_viewport_set_shadow_type( (GtkViewport*)viewport, GTK_SHADOW_NONE );
    gtk_container_set_border_width( (GtkContainer*)viewport, 0 );

    gtk_box_pack_start( (GtkBox*)box, mw->scroll, TRUE, TRUE, 0 );

    // build toolbar
    create_nav_bar( mw, box );
    update_toolbar_position(mw);
    gtk_widget_show_all(box);
    main_win_update_toolbar_visibility(mw);

    mw->hand_cursor = gdk_cursor_new_for_display(gtk_widget_get_display((GtkWidget*)mw), GDK_FLEUR);
    mw->busy_cursor = gdk_cursor_new_for_display(gtk_widget_get_display((GtkWidget*)mw), GDK_WATCH);

    mw->zoom_mode = ZOOM_FIT;

    // Set up drag & drop
    gtk_drag_dest_set( (GtkWidget*)mw, GTK_DEST_DEFAULT_ALL,
                                                    drop_targets,
                                                    G_N_ELEMENTS(drop_targets),
                                                    GDK_ACTION_COPY | GDK_ACTION_ASK );
    g_signal_connect( mw, "drag-data-received", G_CALLBACK(on_drag_data_received), mw );

    mw->image_list = image_list_new();

    // rotation angle is zero on startup
    mw->rotation_angle = 0;
}

static void add_nav_btn_img(MainWin * mw, const char * icon, const char * tip, GCallback cb, gboolean toggle, GtkWidget ** ret_button, GtkWidget ** ret_img)
{
    if (tip && strcmp(tip, "separator") == 0)
    {
        gtk_box_pack_start((GtkBox*)mw->nav_bar, gtk_vseparator_new(), FALSE, FALSE, 0);
        return;
    }

    GtkWidget* img;
    if( g_str_has_prefix(icon, "gtk-") )
        img = gtk_image_new_from_stock(icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
    else
        img = gtk_image_new_from_icon_name(icon, GTK_ICON_SIZE_SMALL_TOOLBAR);
    GtkWidget* btn;
    if( G_UNLIKELY(toggle) )
    {
        btn = gtk_toggle_button_new();
        g_signal_connect( btn, "toggled", cb, mw );
    }
    else
    {
        btn = gtk_button_new();
        g_signal_connect( btn, "clicked", cb, mw );
    }
    gtk_button_set_relief( (GtkButton*)btn, GTK_RELIEF_NONE );
    gtk_button_set_focus_on_click( (GtkButton*)btn, FALSE );
    gtk_container_add( (GtkContainer*)btn, img );
    gtk_widget_set_tooltip_text( btn, tip );
    gtk_box_pack_start( (GtkBox*)mw->nav_bar, btn, FALSE, FALSE, 0 );

    if (ret_button)
        *ret_button = btn;
    if (ret_img)
        *ret_img = img;
}

static void add_nav_btn(MainWin * mw, const char * icon, const char * tip, GCallback cb, gboolean toggle, GtkWidget ** ret_button)
{
    add_nav_btn_img(mw, icon, tip, cb, toggle, ret_button, NULL);
}

void create_nav_bar( MainWin* mw, GtkWidget* box )
{
    mw->nav_bar = gtk_hbox_new( FALSE, 0 );

    #define ADD_BUTTON(icon, name, handler, toggle, widget) \
        add_nav_btn(mw, icon, name, G_CALLBACK(handler), toggle, &mw->btn_##widget)
    #define ADD_BUTTON_IMG(icon, name, handler, toggle, widget) \
        add_nav_btn_img(mw, icon, name, G_CALLBACK(handler), toggle, &mw->btn_##widget, &mw->img_##widget)
    #define ADD_SEPATAROR() \
        add_nav_btn(mw, NULL, "separator", NULL, FALSE, NULL)
    ADD_BUTTON(GTK_STOCK_GO_BACK, _("Previous"), on_prev, FALSE, prev);
    ADD_BUTTON(GTK_STOCK_GO_FORWARD, _("Next"), on_next, FALSE, next);
    ADD_BUTTON_IMG(GTK_STOCK_MEDIA_PLAY, _("Start Slideshow"), on_slideshow, TRUE, play_stop);

    ADD_SEPATAROR();

    ADD_BUTTON(GTK_STOCK_ZOOM_OUT, _("Zoom Out"), on_zoom_out, FALSE, zoom_out);
    ADD_BUTTON(GTK_STOCK_ZOOM_IN, _("Zoom In"), on_zoom_in, FALSE, zoom_in);

    ADD_BUTTON(GTK_STOCK_ZOOM_FIT, _("Fit Image To Window Size"), on_zoom_fit, TRUE, zoom_fit);
    ADD_BUTTON(GTK_STOCK_ZOOM_100, _("Original Size"), on_zoom_orig, TRUE, zoom_orig);
    gtk_toggle_button_set_active( (GtkToggleButton*)mw->btn_zoom_fit, TRUE );
    ADD_BUTTON(GTK_STOCK_FULLSCREEN, _("Full Screen"), on_full_screen, FALSE, full_screen);

    ADD_SEPATAROR();

    ADD_BUTTON("object-rotate-left", _("Rotate Counterclockwise"), on_rotate_counterclockwise, FALSE, rotate_ccw);
    ADD_BUTTON("object-rotate-right", _("Rotate Clockwise"), on_rotate_clockwise, FALSE, rotate_cw);

    ADD_BUTTON("object-flip-horizontal", _("Flip Horizontal"), on_flip_horizontal, FALSE, flip_h);
    ADD_BUTTON("object-flip-vertical", _("Flip Vertical"), on_flip_vertical, FALSE, flip_v);

    ADD_SEPATAROR();

    ADD_BUTTON(GTK_STOCK_OPEN, _("Open File"), G_CALLBACK(on_open), FALSE, open);
    ADD_BUTTON(GTK_STOCK_SAVE, _("Save File"), G_CALLBACK(on_save), FALSE, save_file);
    ADD_BUTTON(GTK_STOCK_SAVE_AS, _("Save a Copy"), G_CALLBACK(on_save_copy), FALSE, save_copy);
    ADD_BUTTON(GTK_STOCK_DELETE, _("Delete File"), G_CALLBACK(on_delete), FALSE, delete_file);

    ADD_SEPATAROR();

    ADD_BUTTON(GTK_STOCK_PREFERENCES, _("Preferences"), G_CALLBACK(on_preference), FALSE, preference);
    ADD_BUTTON(GTK_STOCK_QUIT, _("Quit"), G_CALLBACK(on_quit), FALSE, quit);

    #undef ADD_BUTTON
    #undef ADD_BUTTON_IMG
    #undef ADD_SEPATAROR

    mw->nav_bar_alignment = gtk_alignment_new( 0.5, 0, 0, 0 );
    gtk_container_add( (GtkContainer*)mw->nav_bar_alignment, mw->nav_bar);
    gtk_box_pack_start( (GtkBox*)box, mw->nav_bar_alignment, FALSE, TRUE, 2);
}

gboolean on_delete_event( GtkWidget* widget, GdkEventAny* evt )
{
    gtk_widget_destroy( widget );
    return TRUE;
}

gboolean on_animation_timeout( MainWin* mw )
{
    int delay;
    if ( gdk_pixbuf_animation_iter_advance( mw->animation_iter, NULL ) )
    {
        mw->pix = gdk_pixbuf_animation_iter_get_pixbuf( mw->animation_iter );
        image_view_set_pixbuf( (ImageView*)mw->img_view, mw->pix );
    }
    delay = gdk_pixbuf_animation_iter_get_delay_time( mw->animation_iter );
    mw->animation_timeout = g_timeout_add(delay, (GSourceFunc) on_animation_timeout, mw );
    return FALSE;
}


void main_win_start_slideshow( MainWin* mw )
{
    on_slideshow_menu(NULL, mw);
}

void main_win_show_error( MainWin* mw, const char* message )
{
    GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                                              GTK_DIALOG_MODAL,
                                              GTK_MESSAGE_ERROR,
                                              GTK_BUTTONS_OK,
                                              "%s", message );
    gtk_dialog_run( (GtkDialog*)dlg );
    gtk_widget_destroy( dlg );
}

void on_size_allocate( GtkWidget* widget, GtkAllocation    *allocation )
{
    GTK_WIDGET_CLASS(main_win_parent_class)->size_allocate( widget, allocation );
    if(gtk_widget_get_realized (widget) )
    {
        MainWin* mw = (MainWin*)widget;

        if( mw->zoom_mode == ZOOM_FIT )
        {
            while(gtk_events_pending ())
                gtk_main_iteration(); // makes it more fluid

            main_win_fit_window_size( mw, FALSE, GDK_INTERP_BILINEAR );
        }
    }
}

gboolean on_win_state_event( GtkWidget* widget, GdkEventWindowState* state )
{
    MainWin* mw = (MainWin*)widget;

    if ((state->new_window_state &= GDK_WINDOW_STATE_FULLSCREEN) != 0)
        mw->full_screen = TRUE;
    else
        mw->full_screen = FALSE;

    main_win_update_toolbar_visibility(mw);
    main_win_update_background_color(mw);

    int previous = pref.open_maximized;
    pref.open_maximized = ((state->new_window_state & GDK_WINDOW_STATE_MAXIMIZED) != 0);
    if (previous != pref.open_maximized)
        save_preferences();
    return TRUE;
}

void on_scroll_size_allocate(GtkWidget* widget, GtkAllocation* allocation, MainWin* mv)
{
    mv->scroll_allocation = *allocation;
}

void cancel_slideshow(MainWin* mw)
{
    mw->slideshow_cancelled = TRUE;
    mw->slideshow_running = FALSE;
    if (mw->slide_timeout != 0)
        g_source_remove(mw->slide_timeout);
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(mw->btn_play_stop), FALSE );
}

gboolean next_slide(MainWin* mw)
{
    /* Timeout causes an implicit "Next". */
    if (mw->slideshow_running)
        main_win_action_next(mw);

    return mw->slideshow_running;
}

void on_slideshow_menu( GtkMenuItem* item, MainWin* mw )
{
    if (!mw->play_stop_action_enabled)
        return;

    gtk_button_clicked( (GtkButton*)mw->btn_play_stop );
}

void on_slideshow( GtkToggleButton* btn, MainWin* mw )
{
    if (!mw->play_stop_action_enabled)
        return;

    if ((mw->slideshow_running) || (mw->slideshow_cancelled))
    {
        mw->slideshow_running = FALSE;
        mw->slideshow_cancelled = FALSE;
        gtk_image_set_from_stock( GTK_IMAGE(mw->img_play_stop), GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_SMALL_TOOLBAR );
        gtk_widget_set_tooltip_text( GTK_WIDGET(btn), _("Start Slideshow") );
        gtk_toggle_button_set_active( btn, FALSE );
    }
    else
    {
        gtk_toggle_button_set_active( btn, TRUE );
        mw->slideshow_running = TRUE;
        gtk_image_set_from_stock( GTK_IMAGE(mw->img_play_stop), GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_SMALL_TOOLBAR );
        gtk_widget_set_tooltip_text( GTK_WIDGET(btn), _("Stop Slideshow") );
        mw->slide_timeout = g_timeout_add(1000 * pref.slide_delay, (GSourceFunc) next_slide, mw);
    }

    main_win_update_sensitivity(mw);
}

void on_prev( GtkWidget* widget, MainWin* mw )
{
    main_win_action_prev(mw);
}

void on_next( GtkWidget* widget, MainWin* mw )
{
    main_win_action_next(mw);
}

void on_zoom_in( GtkWidget* widget, MainWin* mw )
{
    main_win_action_zoom_in(mw);
}

void on_zoom_out( GtkWidget* widget, MainWin* mw )
{
    main_win_action_zoom_out(mw);
}

void on_zoom_fit_menu( GtkWidget* widget, MainWin* mw )
{
    main_win_action_zoom_fit(mw);
}

void on_zoom_fit( GtkWidget* widget, MainWin* mw )
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_fit)))
        main_win_action_zoom_fit(mw);
    else
        main_win_update_zoom_buttons_state(mw);
}

void on_zoom_orig_menu( GtkWidget* widget, MainWin* mw )
{
    main_win_action_zoom_orig(mw);
}

void on_zoom_orig( GtkWidget* widget, MainWin* mw )
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_orig)))
        main_win_action_zoom_orig(mw);
    else
        main_win_update_zoom_buttons_state(mw);
}

void on_full_screen( GtkWidget* widget, MainWin* mw )
{
    if( ! mw->full_screen )
        gtk_window_fullscreen( (GtkWindow*)mw );
    else
        gtk_window_unfullscreen( (GtkWindow*)mw );
}


//////////////////// rotate & flip

static int trans_angle_to_id(int i)
{
    if(i == 0) 		return 1;
    else if(i == 90)	return 6;
    else if(i == 180)	return 3;
    else if(i == 270)	return 8;
    else if(i == -45)	return 7;
    else if(i == -90)	return 2;
    else if(i == -135)	return 5;
    else     /* -180 */ return 4;
}

static int get_new_angle( int orig_angle, int rotate_angle )
{
    // defined in exif.c
    static int angle_trans_back[] = {0, 0, -90, 180, -180, -135, 90, -45, 270};

    orig_angle = trans_angle_to_id(orig_angle);
    rotate_angle = trans_angle_to_id(rotate_angle);

    return angle_trans_back[ ExifRotateFlipMapping[orig_angle][rotate_angle] ];
}

void on_rotate_auto_save( GtkWidget* btn, MainWin* mw )
{
    if(pref.auto_save_rotated){
//      gboolean ask_before_save = pref.ask_before_save;
//      pref.ask_before_save = FALSE;
        on_save(btn,mw);
//      pref.ask_before_save = ask_before_save;
    }
}

void on_rotate_clockwise( GtkWidget* btn, MainWin* mw )
{
    if (!mw->rotate_cw_action_enabled)
        return;

    cancel_slideshow(mw);
    rotate_image( mw, GDK_PIXBUF_ROTATE_CLOCKWISE );
    mw->rotation_angle = get_new_angle(mw->rotation_angle, 90);
    on_rotate_auto_save(btn, mw);
}

void on_rotate_counterclockwise( GtkWidget* btn, MainWin* mw )
{
    if (!mw->rotate_ccw_action_enabled)
        return;

    cancel_slideshow(mw);
    rotate_image( mw, GDK_PIXBUF_ROTATE_COUNTERCLOCKWISE );
    mw->rotation_angle = get_new_angle(mw->rotation_angle, 270);
    on_rotate_auto_save(btn, mw);
}

void on_flip_vertical( GtkWidget* btn, MainWin* mw )
{
    if (!mw->flip_v_action_enabled)
        return;

    cancel_slideshow(mw);
    rotate_image( mw, -180 );
    mw->rotation_angle = get_new_angle(mw->rotation_angle, -180);
    on_rotate_auto_save(btn, mw);
}

void on_flip_horizontal( GtkWidget* btn, MainWin* mw )
{
    if (!mw->flip_h_action_enabled)
        return;

    cancel_slideshow(mw);
    rotate_image( mw, -90 );
    mw->rotation_angle = get_new_angle(mw->rotation_angle, -90);
    on_rotate_auto_save(btn, mw);
}

/* end of rotate & flip */

void on_save_copy( GtkWidget* btn, MainWin* mw )
{
    if (!mw->save_copy_action_enabled)
        return;

    char *file, *type;

    cancel_slideshow(mw);
    if( ! mw->pix )
        return;

    file = get_save_filename( GTK_WINDOW(mw), image_list_get_dir( mw->image_list ), &type );
    if( file )
    {
        main_win_save( mw, file, type, TRUE );
        g_free( file );
        g_free( type );
    }
}

void on_save( GtkWidget* btn, MainWin* mw )
{
    if (!mw->save_file_action_enabled)
        return;

    cancel_slideshow(mw);
    if( ! mw->pix )
        return;

    char* file_name = g_build_filename( image_list_get_dir( mw->image_list ),
                                        image_list_get_current( mw->image_list ), NULL );
    GdkPixbufFormat* info;
    info = gdk_pixbuf_get_file_info( file_name, NULL, NULL );
    char* type = gdk_pixbuf_format_get_name( info );

    /* Confirm save if requested. */
    if ((pref.ask_before_save) && ( ! save_confirm(mw, file_name)))
        return;

    if(strcmp(type,"jpeg")==0)
    {
        if(!pref.rotate_exif_only || ExifRotate(file_name, mw->rotation_angle) == FALSE)
        {
            // hialan notes:
            // ExifRotate retrun FALSE when
            //   1. Can not read file
            //   2. Exif do not have TAG_ORIENTATION tag
            //   3. Format unknown
            // And then we apply rotate_and_save_jpeg_lossless() ,
            // the result would not effected by EXIF Orientation...
#ifdef HAVE_LIBJPEG
            int status = rotate_and_save_jpeg_lossless(file_name,mw->rotation_angle);
	    if(status != 0)
            {
                main_win_show_error( mw, g_strerror(status) );
            }
#else
            main_win_save( mw, file_name, type, pref.ask_before_save );
#endif
        }
    } else
        main_win_save( mw, file_name, type, pref.ask_before_save );
    mw->rotation_angle = 0;
    g_free( file_name );
    g_free( type );
}

void on_open( GtkWidget* btn, MainWin* mw )
{
    cancel_slideshow(mw);
    char* file = get_open_filename( (GtkWindow*)mw, image_list_get_dir( mw->image_list ) );
    if( file )
    {
        main_win_open(mw, file, mw->zoom_mode);
        g_free( file );
    }
}

void on_preference(GtkWidget * btn, MainWin * mw)
{
    edit_preferences((GtkWindow *) mw);
    update_toolbar_position(mw);
    main_win_update_toolbar_visibility(mw);
}

void on_quit( GtkWidget* btn, MainWin* mw )
{
    cancel_slideshow(mw);
    gtk_main_quit();
}

gboolean on_button_press( GtkWidget* widget, GdkEventButton* evt, MainWin* mw )
{
    if( ! gtk_widget_has_focus( widget ) )
        gtk_widget_grab_focus( widget );

    if( evt->type == GDK_BUTTON_PRESS)
    {
        if( evt->button == 1 )    // left button
        {
            if( ! mw->pix )
                return FALSE;
            mw->dragging = TRUE;
            gtk_widget_get_pointer( (GtkWidget*)mw, &mw->drag_old_x ,&mw->drag_old_y );
            gdk_window_set_cursor( gtk_widget_get_window(widget), mw->hand_cursor );
        }
        else if( evt->button == 3 )   // right button
        {
            show_popup_menu( mw, evt );
        }
    }
    else if( evt->type == GDK_2BUTTON_PRESS && evt->button == 1 )    // double clicked
    {
         on_full_screen( NULL, mw );
    }
    return FALSE;
}

gboolean on_mouse_move( GtkWidget* widget, GdkEventMotion* evt, MainWin* mw )
{
    if( ! mw->dragging )
        return FALSE;

    int cur_x, cur_y;
    gtk_widget_get_pointer( (GtkWidget*)mw, &cur_x ,&cur_y );

    int dx = (mw->drag_old_x - cur_x);
    int dy = (mw->drag_old_y - cur_y);

    GtkAdjustment *hadj, *vadj;
    hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)mw->scroll);
    vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)mw->scroll);

    GtkRequisition req;
    gtk_widget_size_request( (GtkWidget*)mw->img_view, &req );

    gdouble hadj_page_size = gtk_adjustment_get_page_size(hadj);
    gdouble hadj_lower = gtk_adjustment_get_lower(hadj);
    gdouble hadj_upper = gtk_adjustment_get_upper(hadj);

    if( ABS(dx) > 4 )
    {
        mw->drag_old_x = cur_x;
        if( req.width > hadj_page_size )
        {
            gdouble value = gtk_adjustment_get_value (hadj);
            gdouble x = value + dx;
            if( x < hadj_lower )
                x = hadj_lower;
            else if( (x + hadj_page_size) > hadj_upper )
                x = hadj_upper - hadj_page_size;

            if( x != value )
                gtk_adjustment_set_value (hadj, x );
        }
    }

    gdouble vadj_page_size = gtk_adjustment_get_page_size(vadj);
    gdouble vadj_lower = gtk_adjustment_get_lower(vadj);
    gdouble vadj_upper = gtk_adjustment_get_upper(vadj);

    if( ABS(dy) > 4 )
    {
        if( req.height > vadj_page_size )
        {
            mw->drag_old_y = cur_y;
            gdouble value = gtk_adjustment_get_value (vadj);
            gdouble y = value + dy;
            if( y < vadj_lower )
                y = vadj_lower;
            else if( (y + vadj_page_size) > vadj_upper )
                y = vadj_upper - vadj_page_size;

            if( y != value )
                gtk_adjustment_set_value (vadj, y );
        }
    }
    return FALSE;
}

gboolean on_button_release( GtkWidget* widget, GdkEventButton* evt, MainWin* mw )
{
    mw->dragging = FALSE;
    gdk_window_set_cursor( gtk_widget_get_window(widget), NULL );
    return FALSE;
}

gboolean on_scroll_event( GtkWidget* widget, GdkEventScroll* evt, MainWin* mw )
{
    guint modifiers = gtk_accelerator_get_default_mod_mask();
    switch( evt->direction )
    {
    case GDK_SCROLL_UP:
        if ((evt->state & modifiers) == GDK_CONTROL_MASK)
            main_win_action_zoom_in(mw);
        else
            main_win_action_prev(mw);
        break;
    case GDK_SCROLL_DOWN:
        if ((evt->state & modifiers) == GDK_CONTROL_MASK)
            main_win_action_zoom_out(mw);
        else
            main_win_action_next(mw);
        break;
    case GDK_SCROLL_LEFT:
        if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
            main_win_action_next(mw);
        else
            main_win_action_prev(mw);
        break;
    case GDK_SCROLL_RIGHT:
        if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
            main_win_action_prev(mw);
        else
            main_win_action_next(mw);
        break;
    }
    return TRUE;
}

gboolean on_key_press_event(GtkWidget* widget, GdkEventKey * key)
{
    MainWin* mw = (MainWin*)widget;
    switch( key->keyval )
    {
        case GDK_Right:
        case GDK_KP_Right:
        case GDK_rightarrow:
            if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
                main_win_action_prev(mw);
            else
                main_win_action_next(mw);
            break;
        case GDK_Return:
        case GDK_space:
        case GDK_Next:
        case GDK_KP_Down:
        case GDK_Down:
        case GDK_downarrow:
            main_win_action_next(mw);
            break;
        case GDK_Left:
        case GDK_KP_Left:
        case GDK_leftarrow:
            if( gtk_widget_get_default_direction () == GTK_TEXT_DIR_RTL )
                main_win_action_next(mw);
            else
                main_win_action_prev(mw);
            break;
        case GDK_Prior:
        case GDK_BackSpace:
        case GDK_KP_Up:
        case GDK_Up:
        case GDK_uparrow:
            main_win_action_prev(mw);
            break;
        case GDK_w:
        case GDK_W:
            on_slideshow_menu( NULL, mw );
            break;
        case GDK_KP_Add:
        case GDK_plus:
        case GDK_equal:
            main_win_action_zoom_in(mw);
            break;
        case GDK_KP_Subtract:
        case GDK_minus:
            main_win_action_zoom_out(mw);
            break;
        case GDK_s:
        case GDK_S:
            on_save( NULL, mw );
            break;
        case GDK_a:
        case GDK_A:
            on_save_copy( NULL, mw );
            break;
        case GDK_l:
        case GDK_L:
            on_rotate_counterclockwise( NULL, mw );
            break;
        case GDK_r:
        case GDK_R:
            on_rotate_clockwise( NULL, mw );
            break;
        case GDK_f:
        case GDK_F:
            main_win_action_zoom_fit(mw);
            break;
        case GDK_g:
        case GDK_G:
            main_win_action_zoom_orig(mw);
            break;
        case GDK_h:
        case GDK_H:
            on_flip_horizontal( NULL, mw );
            break;
        case GDK_v:
        case GDK_V:
            on_flip_vertical( NULL, mw );
            break;
        case GDK_o:
        case GDK_O:
            on_open( NULL, mw );
            break;
        case GDK_Delete:
        case GDK_d:
        case GDK_D:
            on_delete( NULL, mw );
            break;
        case GDK_p:
        case GDK_P:
            on_preference( NULL, mw );
            break;
        case GDK_t:
        case GDK_T:
            on_toggle_toolbar( NULL, mw );
            break;
        case GDK_Escape:
            if (mw->full_screen)
                on_full_screen(NULL, mw);
            else if (pref.quit_on_escape)
                on_quit(NULL, mw);
            break;
        case GDK_q:
        case GDK_Q:
            on_quit( NULL, mw );
            break;
        case GDK_F11:
            on_full_screen( NULL, mw );
            break;
        case GDK_F10:
        case GDK_Menu:
            show_popup_menu(mw, NULL);
            break;

        default:
            GTK_WIDGET_CLASS(main_win_parent_class)->key_press_event( widget, key );
    }
    return FALSE;
}

void main_win_center_image( MainWin* mw )
{
    GtkAdjustment *hadj, *vadj;
    hadj = gtk_scrolled_window_get_hadjustment((GtkScrolledWindow*)mw->scroll);
    vadj = gtk_scrolled_window_get_vadjustment((GtkScrolledWindow*)mw->scroll);

    GtkRequisition req;
    gtk_widget_size_request( (GtkWidget*)mw->img_view, &req );

    gdouble hadj_page_size = gtk_adjustment_get_page_size(hadj);
    gdouble hadj_upper = gtk_adjustment_get_upper(hadj);

    if( req.width > hadj_page_size )
        gtk_adjustment_set_value(hadj, ( hadj_upper - hadj_page_size ) / 2 );

    gdouble vadj_page_size = gtk_adjustment_get_page_size(vadj);
    gdouble vadj_upper = gtk_adjustment_get_upper(vadj);

    if( req.height > vadj_page_size )
        gtk_adjustment_set_value(vadj, ( vadj_upper - vadj_page_size ) / 2 );
}

void rotate_image( MainWin* mw, int angle )
{
    GdkPixbuf* rpix = NULL;

    if( ! mw->pix )
        return;

    if(angle > 0)
    {
        rpix = gdk_pixbuf_rotate_simple( mw->pix, angle );
    }
    else
    {
        if(angle == -90)
            rpix = gdk_pixbuf_flip( mw->pix, TRUE );
        else if(angle == -180)
            rpix = gdk_pixbuf_flip( mw->pix, FALSE );
    }

    if (!rpix) {
        return;
    }

    g_object_unref( mw->pix );

    mw->pix = rpix;
    image_view_set_pixbuf( (ImageView*)mw->img_view, mw->pix );

    if( mw->zoom_mode == ZOOM_FIT )
        main_win_fit_window_size( mw, FALSE, GDK_INTERP_BILINEAR );
}

gboolean main_win_scale_image( MainWin* mw, double new_scale, GdkInterpType type )
{
    if( G_UNLIKELY( new_scale == 1.0 ) )
    {
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(mw->btn_zoom_orig), TRUE );
        mw->scale = 1.0;
        return TRUE;
    }
    mw->scale = new_scale;
    image_view_set_scale( (ImageView*)mw->img_view, new_scale, type );

    update_title( NULL, mw );

    return TRUE;
}

gboolean save_confirm( MainWin* mw, const char* file_path )
{
    if( g_file_test( file_path, G_FILE_TEST_EXISTS ) )
    {
        GtkWidget* dlg = gtk_message_dialog_new( (GtkWindow*)mw,
                GTK_DIALOG_MODAL,
                GTK_MESSAGE_QUESTION,
                GTK_BUTTONS_YES_NO,
                _("The file name you selected already exists.\nDo you want to overwrite existing file?\n(Warning: The quality of original image might be lost)") );
        if( gtk_dialog_run( (GtkDialog*)dlg ) != GTK_RESPONSE_YES )
        {
            gtk_widget_destroy( dlg );
            return FALSE;
        }
        gtk_widget_destroy( dlg );
    }
    return TRUE;
}

gboolean main_win_save( MainWin* mw, const char* file_path, const char* type, gboolean confirm )
{
    gboolean result1,gdk_save_supported;
    GSList *gdk_formats;
    GSList *gdk_formats_i;
    if( ! mw->pix )
        return FALSE;

    /* detect if the current type can be save by gdk_pixbuf_save() */
    gdk_save_supported = FALSE;
    gdk_formats = gdk_pixbuf_get_formats();
    for (gdk_formats_i = gdk_formats; gdk_formats_i;
         gdk_formats_i = g_slist_next(gdk_formats_i))
    {
        GdkPixbufFormat *data;
        data = gdk_formats_i->data;
        if (gdk_pixbuf_format_is_writable(data))
        {
            if ( strcmp(type, gdk_pixbuf_format_get_name(data))==0)
            {
                gdk_save_supported = TRUE;
                break;
            }
        }
    }
    g_slist_free (gdk_formats);

    GError* err = NULL;
    if (!gdk_save_supported)
    {
        main_win_show_error( mw, _("Writing this image format is not supported.") );
        return FALSE;
    }

    { /* update UI before the long-running action */
        mw->saving_is_in_progress = TRUE;
        mw->ui_disabled++;
        main_win_update_sensitivity(mw);
        update_title(NULL, mw);
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(mw)), mw->busy_cursor);
        while(gtk_events_pending ())
            gtk_main_iteration();
    }

    if( strcmp( type, "jpeg" ) == 0 )
    {
        char tmp[32];
        g_sprintf(tmp, "%d", pref.jpg_quality);
        result1 = gdk_pixbuf_save( mw->pix, file_path, type, &err, "quality", tmp, NULL );
    }
    else if( strcmp( type, "png" ) == 0 )
    {
        char tmp[32];
        g_sprintf(tmp, "%d", pref.png_compression);
        result1 = gdk_pixbuf_save( mw->pix, file_path, type, &err, "compression", tmp, NULL );
    }
    else
        result1 = gdk_pixbuf_save( mw->pix, file_path, type, &err, NULL );

    { /* update UI after the long-running action */
        mw->saving_is_in_progress = FALSE;
        mw->ui_disabled--;
        main_win_update_sensitivity(mw);
        update_title(NULL, mw);
        gdk_window_set_cursor(gtk_widget_get_window(GTK_WIDGET(mw)), NULL);
    }

    if( ! result1 )
    {
        main_win_show_error( mw, err->message );
        return FALSE;
    }
    return TRUE;
}

void on_delete( GtkWidget* btn, MainWin* mw )
{
    if (!mw->delete_file_action_enabled)
        return;

    cancel_slideshow(mw);
    gchar* file_path = image_list_get_current_file_path( mw->image_list );
    if( file_path )
    {
        int resp = GTK_RESPONSE_YES;
	if ( pref.ask_before_delete )
	{
            GtkWidget * dialog = gtk_message_dialog_new( (GtkWindow*)mw,
                    GTK_DIALOG_MODAL,
                    GTK_MESSAGE_QUESTION,
                    GTK_BUTTONS_YES_NO,
                    _("Are you sure you want to delete file \"%s\"?\n\nWarning: Once deleted, the file cannot be recovered."),
                    image_list_get_current(mw->image_list));
            resp = gtk_dialog_run((GtkDialog *) dialog);
            gtk_widget_destroy(dialog);
        }

	if( resp == GTK_RESPONSE_YES )
        {
            const char* name = image_list_get_current( mw->image_list );

	    if( g_unlink( file_path ) != 0 )
		main_win_show_error( mw, g_strerror(errno) );
	    else
	    {
		const char* next_name = image_list_to_next( mw->image_list );
		if( ! next_name )
		    next_name = image_list_to_prev( mw->image_list );

		if( next_name )
		{
		    char* next_file_path = image_list_get_current_file_path( mw->image_list );
		    main_win_open( mw, next_file_path, ZOOM_FIT );
		    g_free( next_file_path );
		}

		image_list_remove ( mw->image_list, name );

		if ( ! next_name )
		{
		    main_win_close( mw );
		    image_list_close( mw->image_list );
		    image_view_set_pixbuf( (ImageView*)mw->img_view, NULL );
		    gtk_window_set_title( (GtkWindow*) mw, _("Image Viewer"));
		}
	    }
        }
	g_free( file_path );
    }
}

void on_toggle_toolbar( GtkMenuItem* item, MainWin* mw )
{
    if (mw->full_screen)
        pref.show_toolbar_fullscreen = !pref.show_toolbar_fullscreen;
    else
        pref.show_toolbar = !pref.show_toolbar;

    main_win_update_toolbar_visibility(mw);
    save_preferences();
}


static void popup_menu_position_handler(GtkMenu * menu, gint * x, gint * y, gboolean * push_in, MainWin * mw)
{
    gdk_window_get_origin(gtk_widget_get_window(mw->evt_box), x, y);
    *push_in = FALSE;
}

void show_popup_menu(MainWin * mw, GdkEventButton * evt)
{
    GtkMenu * file_submenu = get_fm_file_menu_for_path((GtkWindow *) mw, image_list_get_current_file_path(mw->image_list));

    static PtkMenuItemEntry menu_def[] =
    {
        /*  0 */ PTK_MENU_ITEM( N_( "File" ), NULL, 0, 0),
        /*  1 */ PTK_IMG_MENU_ITEM( N_( "Previous" ), GTK_STOCK_GO_BACK, on_prev, GDK_leftarrow, 0 ),
        /*  2 */ PTK_IMG_MENU_ITEM( N_( "Next" ), GTK_STOCK_GO_FORWARD, on_next, GDK_rightarrow, 0 ),
        /*  3 */ PTK_IMG_MENU_ITEM( N_( "Start/Stop Slideshow" ), GTK_STOCK_MEDIA_PLAY, on_slideshow_menu, GDK_W, 0 ),
        /*  4 */ PTK_SEPARATOR_MENU_ITEM,
        /*  5 */ PTK_IMG_MENU_ITEM( N_( "Zoom Out" ), GTK_STOCK_ZOOM_OUT, on_zoom_out, GDK_minus, 0 ),
        /*  6 */ PTK_IMG_MENU_ITEM( N_( "Zoom In" ), GTK_STOCK_ZOOM_IN, on_zoom_in, GDK_plus, 0 ),
        /*  7 */ PTK_IMG_MENU_ITEM( N_( "Fit Image To Window Size" ), GTK_STOCK_ZOOM_FIT, on_zoom_fit_menu, GDK_F, 0 ),
        /*  8 */ PTK_IMG_MENU_ITEM( N_( "Original Size" ), GTK_STOCK_ZOOM_100, on_zoom_orig_menu, GDK_G, 0 ),
        /*  9 */ PTK_SEPARATOR_MENU_ITEM,
        /* 10 */ PTK_IMG_MENU_ITEM( N_( "Full Screen" ), GTK_STOCK_FULLSCREEN, on_full_screen, GDK_F11, 0 ),
        /* 11 */ PTK_SEPARATOR_MENU_ITEM,
        /* 12 */ PTK_IMG_MENU_ITEM( N_( "Rotate Counterclockwise" ), "object-rotate-left", on_rotate_counterclockwise, GDK_L, 0 ),
        /* 13 */ PTK_IMG_MENU_ITEM( N_( "Rotate Clockwise" ), "object-rotate-right", on_rotate_clockwise, GDK_R, 0 ),
        /* 14 */ PTK_IMG_MENU_ITEM( N_( "Flip Horizontal" ), "object-flip-horizontal", on_flip_horizontal, GDK_H, 0 ),
        /* 15 */ PTK_IMG_MENU_ITEM( N_( "Flip Vertical" ), "object-flip-vertical", on_flip_vertical, GDK_V, 0 ),
        /* 16 */ PTK_SEPARATOR_MENU_ITEM,
        /* 17 */ PTK_IMG_MENU_ITEM( N_("Open File"), GTK_STOCK_OPEN, G_CALLBACK(on_open), GDK_O, 0 ),
        /* 18 */ PTK_IMG_MENU_ITEM( N_("Save File"), GTK_STOCK_SAVE, G_CALLBACK(on_save), GDK_S, 0 ),
        /* 19 */ PTK_IMG_MENU_ITEM( N_("Save a Copy..."), GTK_STOCK_SAVE_AS, G_CALLBACK(on_save_copy), GDK_A, 0 ),
        /* 20 */ PTK_IMG_MENU_ITEM( N_("Delete File"), GTK_STOCK_DELETE, G_CALLBACK(on_delete), GDK_Delete, 0 ),
        /* 21 */ PTK_SEPARATOR_MENU_ITEM,
        /* 22 */ PTK_IMG_MENU_ITEM( N_("Preferences"), GTK_STOCK_PREFERENCES, G_CALLBACK(on_preference), GDK_P, 0 ),
        /* 23 */ PTK_IMG_MENU_ITEM( N_("Show/Hide toolbar"), NULL, G_CALLBACK(on_toggle_toolbar), GDK_T, 0 ),
        /* 24 */ PTK_STOCK_MENU_ITEM( GTK_STOCK_ABOUT, on_about ),
        /* 25 */ PTK_SEPARATOR_MENU_ITEM,
        /* 26 */ PTK_IMG_MENU_ITEM( N_("Quit"), GTK_STOCK_QUIT, G_CALLBACK(on_quit), GDK_Q, 0 ),
        PTK_MENU_END
    };

    GtkWidget* file_menu_item = NULL;

    GtkWidget* prev_menu_item = NULL;
    GtkWidget* next_menu_item = NULL;
    GtkWidget* play_stop_menu_item = NULL;

    GtkWidget* zoom_out_menu_item = NULL;
    GtkWidget* zoom_in_menu_item = NULL;
    GtkWidget* zoom_fit_menu_item = NULL;
    GtkWidget* zoom_orig_menu_item = NULL;

    GtkWidget* rotate_cw_menu_item = NULL;
    GtkWidget* rotate_ccw_menu_item = NULL;
    GtkWidget* flip_v_menu_item = NULL;
    GtkWidget* flip_h_menu_item = NULL;

    GtkWidget* save_file_menu_item = NULL;
    GtkWidget* save_copy_menu_item = NULL;
    GtkWidget* delete_file_menu_item = NULL;

    menu_def[0].ret = &file_menu_item;

    menu_def[1].ret = &prev_menu_item;
    menu_def[2].ret = &next_menu_item;
    menu_def[3].ret = &play_stop_menu_item;

    menu_def[5].ret = &zoom_out_menu_item;
    menu_def[6].ret = &zoom_in_menu_item;
    menu_def[7].ret = &zoom_fit_menu_item;
    menu_def[8].ret = &zoom_orig_menu_item;

    menu_def[12].ret = &rotate_ccw_menu_item;
    menu_def[13].ret = &rotate_cw_menu_item;
    menu_def[14].ret = &flip_h_menu_item;
    menu_def[15].ret = &flip_v_menu_item;

    menu_def[18].ret = &save_file_menu_item;
    menu_def[19].ret = &save_copy_menu_item;
    menu_def[20].ret = &delete_file_menu_item;

    // mw accel group is useless. It's only used to display accels in popup menu
    GtkAccelGroup* accel_group = gtk_accel_group_new();
    GtkMenuShell* popup = (GtkMenuShell*)ptk_menu_new_from_data( menu_def, mw, accel_group );

    gtk_widget_set_sensitive( file_menu_item, mw->file_action_enabled );

    gtk_widget_set_sensitive( prev_menu_item, mw->prev_action_enabled );
    gtk_widget_set_sensitive( next_menu_item, mw->next_action_enabled );
    gtk_widget_set_sensitive( play_stop_menu_item, mw->play_stop_action_enabled );

    gtk_widget_set_sensitive( zoom_out_menu_item, mw->zoom_out_action_enabled );
    gtk_widget_set_sensitive( zoom_in_menu_item, mw->zoom_in_action_enabled );
    gtk_widget_set_sensitive( zoom_fit_menu_item, mw->zoom_fit_action_enabled );
    gtk_widget_set_sensitive( zoom_orig_menu_item, mw->zoom_orig_action_enabled );

    gtk_widget_set_sensitive( rotate_ccw_menu_item, mw->rotate_ccw_action_enabled );
    gtk_widget_set_sensitive( rotate_cw_menu_item, mw->rotate_cw_action_enabled );
    gtk_widget_set_sensitive( flip_h_menu_item, mw->flip_h_action_enabled );
    gtk_widget_set_sensitive( flip_v_menu_item, mw->flip_v_action_enabled );

    gtk_widget_set_sensitive( save_file_menu_item, mw->save_file_action_enabled );
    gtk_widget_set_sensitive( save_copy_menu_item, mw->save_copy_action_enabled );
    gtk_widget_set_sensitive( delete_file_menu_item, mw->delete_file_action_enabled );


    gtk_widget_show_all( (GtkWidget*)popup );

    if (file_submenu)
    {
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_menu_item), GTK_WIDGET(file_submenu));
    }
    else
    {
        gtk_widget_hide(file_menu_item);
    }

    g_signal_connect(popup, "selection-done", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_menu_popup( (GtkMenu*)popup, NULL, NULL,
        (GtkMenuPositionFunc) (evt ? NULL : popup_menu_position_handler), mw,
        evt ? evt->button : 0, evt ? evt->time : 0);
}

void on_about( GtkWidget* menu, MainWin* mw )
{
    GtkWidget * about_dlg;
    const gchar *authors[] =
    {
        "Vadim Ushakov <igeekless@gmail.com>",
        "洪任諭 Hong Jen Yee <pcman.tw@gmail.com>",
        "Martin Siggel <martinsiggel@googlemail.com>",
        "Hialan Liu <hialan.liu@gmail.com>",
        "Marty Jack <martyj19@comcast.net>",
        "Louis Casillas <oxaric@gmail.com>",
        "Will Davies",
        _(" * Refer to source code of EOG image viewer and GThumb"),
        NULL
    };
    /* TRANSLATORS: Replace this string with your names, one name per line. */
    gchar *translators = _( "translator-credits" );

    about_dlg = gtk_about_dialog_new ();

    gtk_container_set_border_width ( ( GtkContainer*)about_dlg , 2 );
    gtk_about_dialog_set_version ( (GtkAboutDialog*)about_dlg, VERSION );
    gtk_about_dialog_set_program_name ( (GtkAboutDialog*)about_dlg, _( "SPicView" ) );
    gtk_about_dialog_set_logo( (GtkAboutDialog*)about_dlg, gdk_pixbuf_new_from_file(  PACKAGE_DATA_DIR"/pixmaps/spicview.png", NULL ) );
    gtk_about_dialog_set_copyright ( (GtkAboutDialog*)about_dlg, _( "Copyright (C) 2007 - 2016" ) );
    gtk_about_dialog_set_comments ( (GtkAboutDialog*)about_dlg, _( "Lightweight image viewer from Stuurman project" ) );
    gtk_about_dialog_set_license ( (GtkAboutDialog*)about_dlg, "SPicView\n\n"
    "Copyright (C) 2013-2016 Vadim Ushakov\n"
    "Copyright (C) 2007 Hong Jen Yee (PCMan)\n\n"
    "This program is free software; you can redistribute it and/or\nmodify it under the terms of the GNU General Public License\nas published by the Free Software Foundation; either version 2\nof the License, or (at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program; if not, write to the Free Software\nFoundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA." );
    gtk_about_dialog_set_website ( (GtkAboutDialog*)about_dlg, "http://git.make-linux.org/" );
    gtk_about_dialog_set_authors ( (GtkAboutDialog*)about_dlg, authors );
    gtk_about_dialog_set_translator_credits ( (GtkAboutDialog*)about_dlg, translators );
    gtk_window_set_transient_for( (GtkWindow*) about_dlg, GTK_WINDOW( mw ) );

    gtk_dialog_run( ( GtkDialog*)about_dlg );
    gtk_widget_destroy( about_dlg );
}

void on_drag_data_received( GtkWidget* widget, GdkDragContext *drag_context,
                int x, int y, GtkSelectionData* data, guint info, guint time, MainWin* mw )
{
    if( !data)
        return;

    int data_length = gtk_selection_data_get_length(data);

    if( data_length <= 0)
        return;

    char* file = NULL;
    if( info == 0 )    // text/uri-list
    {
        char** uris = gtk_selection_data_get_uris( data );
        if( uris )
        {
            file = g_filename_from_uri(*uris, NULL, NULL);
            g_strfreev( uris );
        }
    }
    else if( info == 1 )    // text/plain
    {
        file = (char*)gtk_selection_data_get_text( data );
    }
    if( file )
    {
        main_win_open( mw, file, ZOOM_FIT );
        g_free( file );
    }
}

/****************************************************************************/

/* action handlers */

static void main_win_action_prev(MainWin* mw)
{
    const char* name;
    if( image_list_is_empty( mw->image_list ) )
        return;

    name = image_list_to_prev( mw->image_list );

    if( ! name && image_list_has_multiple_files( mw->image_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_to_last( mw->image_list );
    }

    if( name )
    {
        gchar* file_path = image_list_get_current_file_path( mw->image_list );
        main_win_open( mw, file_path, mw->zoom_mode );
        g_free( file_path );
    }
}

static void main_win_action_next(MainWin* mw)
{
    if( image_list_is_empty( mw->image_list ) )
        return;

    const char* name = image_list_to_next( mw->image_list );

    if( ! name && image_list_has_multiple_files( mw->image_list ) )
    {
        // FIXME: need to ask user first?
        name = image_list_to_first( mw->image_list );
    }

    if( name )
    {
        gchar* file_path = image_list_get_current_file_path( mw->image_list );
        main_win_open( mw, file_path, mw->zoom_mode );
        g_free( file_path );
    }
}

static void main_win_action_zoom_in(MainWin* mw)
{
    if (!mw->zoom_in_action_enabled)
        return;

    double scale = mw->scale;
    scale *= 1.05;
    main_win_set_zoom_scale(mw, scale);
}

static void main_win_action_zoom_out(MainWin* mw)
{
    if (!mw->zoom_out_action_enabled)
        return;

    double scale = mw->scale;
    scale /= 1.05;
    main_win_set_zoom_scale(mw, scale);
}

static void main_win_action_zoom_orig(MainWin* mw)
{
    if (!mw->zoom_orig_action_enabled)
        return;
    main_win_set_zoom_mode(mw, ZOOM_ORIG);
}

static void main_win_action_zoom_fit(MainWin* mw)
{
    if (!mw->zoom_fit_action_enabled)
        return;
    main_win_set_zoom_mode(mw, ZOOM_FIT);
}

/****************************************************************************/

/* image loading */

static void eval_background_color_for_image(MainWin* mw)
{
    mw->background_color_from_image_valid = FALSE;

    if (mw->animation)
        return;

    GdkPixbuf* pixbuf = mw->pix;
    if (!pixbuf)
        return;

    int n_channels = gdk_pixbuf_get_n_channels(pixbuf);

    if ((n_channels != 3 && n_channels != 4)  ||
        gdk_pixbuf_get_colorspace(pixbuf) != GDK_COLORSPACE_RGB ||
        gdk_pixbuf_get_bits_per_sample (pixbuf) != 8)
    {
        return;
    }

    int width = gdk_pixbuf_get_width (pixbuf);
    int height = gdk_pixbuf_get_height (pixbuf);
    int rowstride = gdk_pixbuf_get_rowstride (pixbuf);
    guchar* pixels = gdk_pixbuf_get_pixels (pixbuf);

#define P(x, y) pixels[(y) * rowstride + (x) * n_channels]
#define HANDLE_PIXEL \
    if (n_channels == 4 && p[3] < 20) \
    { \
        skipped_pixel_count++; \
    } \
    else \
    { \
       r += p[0]; g += p[1]; b += p[2]; \
       pixel_count++; \
    }

    gdouble r = 0, g = 0, b = 0;
    int pixel_count = 0;
    int skipped_pixel_count = 0;
    int i;

    for (i = 0; i < width; i++)
    {
        guchar* p = &P(i, 0);
        HANDLE_PIXEL

        p = &P(i, height - 1);
        HANDLE_PIXEL
    }

    for (i = 1; i < height - 1; i++)
    {
        guchar* p = &P(0, i);
        HANDLE_PIXEL

        p = &P(width - 1, i);
        HANDLE_PIXEL
    }

    if (skipped_pixel_count > pixel_count * 2)
        return;

#undef P
#undef HANDLE_PIXEL

    r /= pixel_count;
    g /= pixel_count;
    b /= pixel_count;

    //g_print("%f, %f, %f\n", (float)r, (float)g, (float)b);

    unsigned ur = r * 256;
    unsigned ug = g * 256;
    unsigned ub = b * 256;
    if (ur > 65535)
        ur = 65535;
    if (ug > 65535)
        ug = 65535;
    if (ub > 65535)
        ub = 65535;

    mw->background_color_from_image.red   = ur;
    mw->background_color_from_image.green = ug;
    mw->background_color_from_image.blue  = ub;

    mw->background_color_from_image_valid = TRUE;
}

static gboolean load_image(MainWin* mw, const char* file_path, GdkPixbuf** _pix, GdkPixbufAnimation** _animation, GError** _err)
{
    GdkPixbuf* pix = NULL;
    GdkPixbufAnimation* animation = NULL;

    /* trying read image from cache */

    GStatBuf stat_info;
    if (g_stat(file_path, &stat_info) == 0)
    {
        ImageCacheItem item;
        item.name = (char *) file_path;
        item.mtime = stat_info.st_mtime;
        item.size = stat_info.st_size;
        if (image_cache_get(&item))
        {
            pix = item.pix;
            animation = item.animation;
            //g_print("cache hit : %s\n", file_path);
        }
    }

    /* read image from file */
    if (!pix && !animation)
    {
        //g_print("cache miss: %s\n", file_path);

        GdkPixbufFormat* info;
        info = gdk_pixbuf_get_file_info( file_path, NULL, NULL );
        char* type = ((info != NULL) ? gdk_pixbuf_format_get_name(info) : "");

        if (strcmp(type,"jpeg") == 0 || strcmp(type,"tiff") == 0)
        {
            animation =  NULL;
            GdkPixbuf* tmp = gdk_pixbuf_new_from_file(file_path, _err);
            if( ! tmp )
            {
                return FALSE;
            }

            pix = gdk_pixbuf_apply_embedded_orientation(tmp);
            g_object_unref(tmp);
        }
        else
        {
            /* grabs a file as if it were an animation */
            animation = gdk_pixbuf_animation_new_from_file( file_path, _err );
            if( ! animation )
            {
                return FALSE;
            }

            /* tests if the file is actually just a normal picture */
            if ( gdk_pixbuf_animation_is_static_image( animation ) )
            {
                pix = gdk_pixbuf_animation_get_static_image( animation );
                g_object_ref(pix);
                g_object_unref(animation);
                animation = NULL;
            }
        }

        /* put image into cache */
        ImageCacheItem item;
        item.name = (char *) file_path;
        item.mtime = stat_info.st_mtime;
        item.size = stat_info.st_size;
        item.pix = pix;
        item.animation = animation;
        image_cache_put(&item);
    }

    if (_pix)
        *_pix = pix;
    else if (pix)
        g_object_unref(pix);

    if (_animation)
        *_animation = animation;
    else if (animation)
        g_object_unref(animation);

    return TRUE;
}

gboolean main_win_open( MainWin* mw, const char* file_path, ZoomMode zoom )
{
    if (g_file_test(file_path, G_FILE_TEST_IS_DIR))
    {
        image_list_open_dir( mw->image_list, file_path, NULL );
        image_list_sort_by_name( mw->image_list, GTK_SORT_DESCENDING );
        if (image_list_to_first(mw->image_list))
        {
            gchar* path = image_list_get_current_file_path(mw->image_list);
            main_win_open(mw, path, zoom);
            g_free(path);
        }
        main_win_update_sensitivity(mw);
        return TRUE;
    }

    main_win_close( mw );

    GError* err = NULL;
    GdkPixbuf* pix = NULL;
    GdkPixbufAnimation* animation = NULL;

    load_image(mw, file_path, &pix, &animation, &err);

    if (!pix && !animation)
    {
        if (err)
        {
            main_win_show_error(mw, err->message);
            g_error_free(err);
        }
        main_win_update_sensitivity(mw);
        main_win_update_background_color(mw);
        return FALSE;
    }

    if (mw->animation)
        g_object_unref(mw->animation);
    if (mw->pix)
        g_object_unref(mw->pix);
    mw->animation = animation;
    mw->pix = pix;

    /* initialize animation iterator and callback */
    if (mw->animation)
    {
        mw->animation_iter = gdk_pixbuf_animation_get_iter( mw->animation, NULL );
        mw->pix = gdk_pixbuf_animation_iter_get_pixbuf( mw->animation_iter );
        int delay = gdk_pixbuf_animation_iter_get_delay_time( mw->animation_iter );
        mw->animation_timeout = g_timeout_add( delay, (GSourceFunc) on_animation_timeout, mw );
    }

    main_win_update_sensitivity(mw);

    mw->zoom_mode = zoom;

    if (mw->zoom_mode == ZOOM_FIT)
    {
        main_win_fit_window_size( mw, FALSE, GDK_INTERP_BILINEAR );
    }
    else if (mw->zoom_mode == ZOOM_SCALE)
    {
        main_win_scale_image( mw, mw->scale, GDK_INTERP_BILINEAR );
    }
    else if (mw->zoom_mode == ZOOM_ORIG)
    {
        image_view_set_scale( (ImageView*)mw->img_view, mw->scale, GDK_INTERP_BILINEAR );
        main_win_center_image( mw );
    }

    eval_background_color_for_image(mw);

    image_view_set_pixbuf( (ImageView*)mw->img_view, mw->pix );

//    while (gtk_events_pending ())
//        gtk_main_iteration ();

    // build file list
    char* dir_path = g_path_get_dirname( file_path );
    image_list_open_dir( mw->image_list, dir_path, NULL );
    image_list_sort_by_name( mw->image_list, GTK_SORT_DESCENDING );
    g_free( dir_path );

    char* base_name = g_path_get_basename( file_path );
    image_list_set_current( mw->image_list, base_name );

    char* disp_name = g_filename_display_name( base_name );
    g_free( base_name );

    update_title( disp_name, mw );
    g_free( disp_name );

    main_win_update_sensitivity(mw);
    main_win_update_zoom_buttons_state(mw);
    main_win_update_background_color(mw);

    if (image_cache_get_limit() > 5)
    {
        if (mw->preload_next_timeout == 0)
            mw->preload_next_timeout = g_timeout_add(300, (GSourceFunc) on_preload_next_timeout, mw);
        if (mw->preload_prev_timeout == 0)
            mw->preload_prev_timeout = g_timeout_add(500, (GSourceFunc) on_preload_prev_timeout, mw);
    }

    return TRUE;
}

gboolean on_preload_next_timeout(MainWin* mw)
{
    gchar* path = NULL;

    if (!image_list_is_empty(mw->image_list))
    {
        const char* name = image_list_get_next(mw->image_list);
        path = image_list_get_file_path_for_item(mw->image_list, name);
    }

    if (path)
    {
        load_image(mw, path, NULL, NULL, NULL);
        g_free(path);
    }

    mw->preload_next_timeout = 0;
    return FALSE;
}

gboolean on_preload_prev_timeout(MainWin* mw)
{
    gchar* path = NULL;

    if (!image_list_is_empty(mw->image_list))
    {
        const char* name = image_list_get_prev(mw->image_list);
        path = image_list_get_file_path_for_item(mw->image_list, name);
    }

    if (path)
    {
        load_image(mw, path, NULL, NULL, NULL);
        g_free(path);
    }

    mw->preload_prev_timeout = 0;
    return FALSE;
}

void main_win_close( MainWin* mw )
{
    if( mw->animation )
    {
        g_object_unref( mw->animation );
        mw->animation = NULL;
        if (mw->animation_timeout)
        {
            g_source_remove( mw->animation_timeout );
            mw->animation_timeout = 0;
        }
        if (mw->animation_iter)
        {
            g_object_unref( mw->animation_iter );
            mw->animation_iter = NULL;
        }
    }
    else if( mw->pix )
    {
        g_object_unref( mw->pix );
    }
    mw->pix = NULL;

    mw->background_color_from_image_valid = FALSE;
}

/****************************************************************************/

/* zoom */

void main_win_fit_size( MainWin* mw, int width, int height, gboolean can_strech, GdkInterpType type )
{
    if( ! mw->pix )
        return;

    int orig_w = gdk_pixbuf_get_width( mw->pix );
    int orig_h = gdk_pixbuf_get_height( mw->pix );

    if( can_strech || (orig_w > width || orig_h > height) )
    {
        gdouble xscale = ((gdouble)width) / orig_w;
        gdouble yscale = ((gdouble)height)/ orig_h;
        gdouble final_scale = xscale < yscale ? xscale : yscale;

        main_win_scale_image( mw, final_scale, type );
    }
    else    // use original size if the image is smaller than the window
    {
        mw->scale = 1.0;
        image_view_set_scale( (ImageView*)mw->img_view, 1.0, type );

        update_title(NULL, mw);
    }
}

void main_win_fit_window_size(  MainWin* mw, gboolean can_strech, GdkInterpType type )
{
    mw->zoom_mode = ZOOM_FIT;

    if( mw->pix == NULL )
        return;
    main_win_fit_size( mw, mw->scroll_allocation.width, mw->scroll_allocation.height, can_strech, type );
}

static void main_win_set_zoom_scale(MainWin* mw, double scale)
{
    main_win_set_zoom_mode(mw, ZOOM_SCALE);

    if (scale > 20.0)
        scale = 20.0;
    if (scale < 0.02)
        scale = 0.02;

    if (mw->scale != scale)
        main_win_scale_image(mw, scale, GDK_INTERP_BILINEAR);

    main_win_update_sensitivity(mw);
}

static void main_win_set_zoom_mode(MainWin* mw, ZoomMode mode)
{
    if (mw->zoom_mode == mode)
       return;

    mw->zoom_mode = mode;

    main_win_update_zoom_buttons_state(mw);

    if (mode == ZOOM_ORIG)
    {
        mw->scale = 1.0;
        if (!mw->pix)
           return;

        update_title(NULL, mw);

        image_view_set_scale( (ImageView*)mw->img_view, 1.0, GDK_INTERP_BILINEAR );

        while (gtk_events_pending ())
            gtk_main_iteration ();

        main_win_center_image( mw ); // FIXME:  mw doesn't work well. Why?
    }
    else if (mode == ZOOM_FIT)
    {
        main_win_fit_window_size( mw, FALSE, GDK_INTERP_BILINEAR );
    }
}

/****************************************************************************/

/* UI state */

static void main_win_update_zoom_buttons_state(MainWin* mw)
{
    gboolean button_zoom_fit_active = mw->zoom_mode == ZOOM_FIT;
    gboolean button_zoom_orig_active = mw->zoom_mode == ZOOM_ORIG;

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_fit)) != button_zoom_fit_active)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_fit), button_zoom_fit_active);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_orig)) != button_zoom_orig_active)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(mw->btn_zoom_orig), button_zoom_orig_active);
}

static void main_win_update_sensitivity(MainWin* mw)
{
    gboolean image = mw->animation || mw->pix;
    gboolean multiple_images = image_list_has_multiple_files(mw->image_list);
    gboolean animation = mw->animation != NULL;
    gboolean slideshow = mw->slideshow_running;
    int ui_enabled = (mw->ui_disabled == 0);

    mw->file_action_enabled        = ui_enabled && image && !slideshow;

    mw->prev_action_enabled        = ui_enabled && image && multiple_images;
    mw->next_action_enabled        = ui_enabled && image && multiple_images;
    mw->play_stop_action_enabled   = ui_enabled && image && multiple_images;

    mw->zoom_out_action_enabled    = ui_enabled && image && mw->scale > 0.02;
    mw->zoom_in_action_enabled     = ui_enabled && image && mw->scale < 20.0;
    mw->zoom_fit_action_enabled    = ui_enabled && image;
    mw->zoom_orig_action_enabled   = ui_enabled && image;

    mw->rotate_cw_action_enabled   = ui_enabled && image && !animation && !slideshow;
    mw->rotate_ccw_action_enabled  = ui_enabled && image && !animation && !slideshow;
    mw->flip_v_action_enabled      = ui_enabled && image && !animation && !slideshow;
    mw->flip_h_action_enabled      = ui_enabled && image && !animation && !slideshow;
    mw->save_file_action_enabled   = ui_enabled && image && !slideshow;
    mw->save_copy_action_enabled   = ui_enabled && image && !slideshow;
    mw->delete_file_action_enabled = ui_enabled && image && !slideshow;


    gtk_widget_set_sensitive(mw->btn_prev, mw->prev_action_enabled);
    gtk_widget_set_sensitive(mw->btn_next, mw->next_action_enabled);
    gtk_widget_set_sensitive(mw->btn_play_stop, mw->play_stop_action_enabled);

    gtk_widget_set_sensitive(mw->btn_zoom_out, mw->zoom_out_action_enabled);
    gtk_widget_set_sensitive(mw->btn_zoom_in, mw->zoom_in_action_enabled);
    gtk_widget_set_sensitive(mw->btn_zoom_fit, mw->zoom_fit_action_enabled);
    gtk_widget_set_sensitive(mw->btn_zoom_orig, mw->zoom_orig_action_enabled);

    gtk_widget_set_sensitive(mw->btn_rotate_cw, mw->rotate_cw_action_enabled);
    gtk_widget_set_sensitive(mw->btn_rotate_ccw, mw->rotate_ccw_action_enabled);
    gtk_widget_set_sensitive(mw->btn_flip_v, mw->flip_v_action_enabled);
    gtk_widget_set_sensitive(mw->btn_flip_h, mw->flip_h_action_enabled);
    gtk_widget_set_sensitive(mw->btn_save_file, mw->save_file_action_enabled);
    gtk_widget_set_sensitive(mw->btn_save_copy, mw->save_copy_action_enabled);
    gtk_widget_set_sensitive(mw->btn_delete_file, mw->delete_file_action_enabled);

    gtk_widget_set_sensitive(mw->nav_bar, ui_enabled);
}

static void update_title(const char *filename, MainWin *mw )
{
    static char fname[50];
    static int wid, hei;

    char buf[100];

    if(filename != NULL)
    {
      strncpy(fname, filename, 49);
      fname[49] = '\0';

      wid = gdk_pixbuf_get_width( mw->pix );
      hei = gdk_pixbuf_get_height( mw->pix );
    }

    snprintf(buf, 100, "%s (%dx%d) %d%%%s",
        fname,
        wid, hei,
        (int)(mw->scale * 100),
        mw->saving_is_in_progress ? _(" [saving the file...]") : "");

    gtk_window_set_title( (GtkWindow*)mw, buf );

    return;
}

void main_win_update_background_color(MainWin* mw)
{
    GdkColor * color = NULL;

    if (!mw->evt_box)
        return;

    if (pref.background_color_auto_adjust && mw->background_color_from_image_valid)
        color = &mw->background_color_from_image;
    else if (mw->full_screen)
        color = &pref.background_color_fullscreen;
    else
        color = &pref.background_color;

    gtk_widget_modify_bg(mw->evt_box, GTK_STATE_NORMAL, color);
    gtk_widget_queue_draw(mw->evt_box);
}

static void main_win_update_toolbar_visibility(MainWin *mw)
{
    gboolean visible = FALSE;

    if (mw->full_screen)
        visible = pref.show_toolbar_fullscreen;
    else
        visible = pref.show_toolbar;

    gtk_widget_set_visible(mw->nav_bar_alignment, visible);
}

void update_toolbar_position(MainWin *mw)
{
    int position = pref.toolbar_on_top ? 0 : 1;
    gtk_box_reorder_child(GTK_BOX(gtk_widget_get_parent(mw->nav_bar_alignment)), mw->nav_bar_alignment, position);
}
