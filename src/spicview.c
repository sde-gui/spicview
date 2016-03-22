/***************************************************************************
 *   Copyright (c) 2012-2016 Vadim Ushakov <igeekless@gmail.com>           *
 *   Copyright (C) 2007 by PCMan (Hong Jen Yee) <pcman.tw@gmail.com>       *
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
#include <config.h>
#endif

#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <string.h>
#include <libsmfm-core/fm.h>
#include <libsmfm-gtk/fm-gtk.h>

#include "pref.h"
#include "main-win.h"

static char** files = NULL;
static gboolean should_display_version = FALSE;
static gboolean should_start_slideshow = FALSE;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &should_display_version,
                 N_("Print version information and exit"), NULL },
    {"slideshow", 0, G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &should_start_slideshow,
                 N_("Start slideshow"), NULL },
    { NULL }
};

#define PIXMAP_DIR        PACKAGE_DATA_DIR "/" PACKAGE_NAME_STR "/pixmaps/"

int main(int argc, char *argv[])
{
    GError *error = NULL;
    GOptionContext *context;
    MainWin* win;

#ifdef ENABLE_NLS
    bindtextdomain ( GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR );
    bind_textdomain_codeset ( GETTEXT_PACKAGE, "UTF-8" );
    textdomain ( GETTEXT_PACKAGE );
#endif

    context = g_option_context_new ("- simple image viewer");
    g_option_context_add_main_entries (context, opt_entries, GETTEXT_PACKAGE);
    g_option_context_add_group (context, gtk_get_option_group (TRUE));
    if ( !g_option_context_parse (context, &argc, &argv, &error) )
    {
        g_print( "option parsing failed: %s\n", error->message);
        return 1;
    }

    if( should_display_version )
    {
        printf( "%s %s\n", PACKAGE_NAME_STR, VERSION );
        return 0;
    }

    fm_gtk_init(NULL);

    gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), PIXMAP_DIR);

    load_preferences();

    /* Allocate and show the window.
     * We must show the window now in case the file open needs to put up an error dialog. */
    win = (MainWin*)main_win_new();
    gtk_widget_show( GTK_WIDGET(win) );

    if ( pref.open_maximized )
        gtk_window_maximize( (GtkWindow*)win );

    // FIXME: need to process multiple files...
    if( files )
    {
        gchar * local_path_str = NULL;
        FmPath * path = fm_path_new_for_str(files[0]);
        if(fm_path_is_native(path))
            local_path_str = fm_path_to_str(path);
        else
        {
            GFile * gf = fm_path_to_gfile(path);
            local_path_str = g_file_get_path(gf);
            g_object_unref(gf);
        }

        main_win_open(win, local_path_str, ZOOM_FIT);

        g_free(local_path_str);
        fm_path_unref(path);

        if (should_start_slideshow)
            main_win_start_slideshow ( win );
    }
    else
    {
        main_win_open(win, ".", ZOOM_FIT);
    }

    gtk_main();

    save_preferences();

    return 0;
}
