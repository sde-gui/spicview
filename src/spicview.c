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

#include "libsmfm_utils.h"
#include "spicview.h"
#include "spicview-gresource.h"
#include "pref.h"
#include "main-win.h"

gchar * get_about_message(void)
{
    const char * divider =
        "\n------------------------------------------------------\n\n";

    GString * text = g_string_new ("");

    g_string_append (text, _(
        "SPicView - Lightweight image viewer for SDE project\n"
    ));

    g_string_append_printf (text, _("Version: %s\n"), VERSION);

    g_string_append (text, divider);

    g_string_append_printf (text, _("Homepage: %s\n"), "https://sde-gui.github.io/spicview/");
    g_string_append_printf (text, _("You can take the latest sources at %s\n"), "https://github.com/sde-gui/spicview");
    g_string_append_printf (text, _("Please report bugs and feature requests to %s.\n"), "https://github.com/sde-gui/spicview/issues");

    g_string_append (text, divider);

    g_string_append (text, _(
        "Copyright (C) 2013-2018 Vadim Ushakov <igeekless@gmail.com>\n"
        "Copyright (C) 2007 洪任諭 Hong Jen Yee <pcman.tw@gmail.com>\n"
        "Copyright (C) Martin Siggel <martinsiggel@googlemail.com>\n"
        "Copyright (C) Hialan Liu <hialan.liu@gmail.com>\n"
        "Copyright (C) Marty Jack <martyj19@comcast.net>\n"
        "Copyright (C) Louis Casillas <oxaric@gmail.com>\n"
        "Copyright (C) Will Davies\n"
    ));

    g_string_append (text, divider);

    g_string_append (text, _(
        "This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation; either version 2 of the License, or (at your option) any later version.\n\n"
        "This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more details.\n\n"
        "You should have received a copy of the GNU General Public License along with this program; if not, write to the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.\n"
    ));

    /* TRANSLATORS: Replace this string with your names, one name per line. */
    const char * translator_credits = _( "translator-credits" );
    if (g_strcmp0(_( "translator-credits" ), "translator-credits") != 0)
    {
        g_string_append (text, divider);

        g_string_append (text, _(
            "Translators:\n\n"
        ));

        g_string_append (text, translator_credits);

        if (translator_credits[strlen(translator_credits)] != '\n')
            g_string_append ( text, "\n");
    }

    g_string_append (text, divider);

    g_string_append (text, _(
        "Supported Image Formats:\n\n"
    ));


    GSList* formats = gdk_pixbuf_get_formats ();
    GSList* format_entry;
    for (format_entry = formats; format_entry != NULL; format_entry = format_entry->next )
    {
        GdkPixbufFormat * format = (GdkPixbufFormat *) format_entry->data;

        if (gdk_pixbuf_format_is_disabled (format))
            continue;

        gchar * format_name = gdk_pixbuf_format_get_name (format);
        gchar * format_description = gdk_pixbuf_format_get_description (format);
        gchar * format_license = gdk_pixbuf_format_get_license (format);
        gboolean format_writable = gdk_pixbuf_format_is_writable (format);
        gboolean format_scalable = gdk_pixbuf_format_is_scalable (format);
        gchar ** format_mime_types_v = gdk_pixbuf_format_get_mime_types (format);
        gchar ** format_extensions_v = gdk_pixbuf_format_get_extensions (format);
        gchar * format_mime_types = g_strjoinv (", ", format_mime_types_v);
        gchar * format_extensions = g_strjoinv (", ", format_extensions_v);


        g_string_append_printf (text, _(
            "Image loader: GdkPixBuf::%s (%s)\n"
            "    License:    %s\n"
            "    Writable:   %s\n"
            "    Scalable:   %s\n"
            "    MIME types: %s\n"
            "    Extensions: %s\n"
            "\n"),
            format_name, format_description,
            format_license,
            format_writable ? _("Yes") : _("No"),
            format_scalable ? _("Yes") : _("No"),
            format_mime_types,
            format_extensions);

        g_free (format_name);
        g_free (format_description);
        g_free (format_license);
        g_strfreev (format_mime_types_v);
        g_strfreev (format_extensions_v);
        g_free (format_mime_types);
        g_free (format_extensions);
    }

    g_string_append (text, divider);

    g_string_append (text, _(
        "Compile-time environment:\n"
    ));

    g_string_append_printf (text, "glib %d.%d.%d\n", GLIB_MAJOR_VERSION, GLIB_MINOR_VERSION, GLIB_MICRO_VERSION);
    g_string_append_printf (text, "gtk %d.%d.%d\n", GTK_MAJOR_VERSION, GTK_MINOR_VERSION, GTK_MICRO_VERSION);
#ifdef ENABLE_LIBSMFM
    g_string_append_printf (text, "libsmfm-core %d.%d.%d\n", FM_VERSION_MAJOR, FM_VERSION_MINOR, FM_VERSION_MICRO);
#endif
    g_string_append_printf (text, "Configured with: %s\n", CONFIGURE_ARGUMENTS);

    return g_string_free (text, FALSE);
}

static char** files = NULL;
static gboolean option_version = FALSE;
static gboolean option_about = FALSE;
static gboolean option_slideshow = FALSE;

static GOptionEntry opt_entries[] =
{
    {G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, &files, NULL, N_("[FILE]")},
    {"version", 'v', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &option_version,
                 N_("Print version information and exit"), NULL },
    {"about",   'V', G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &option_about,
                 N_("Print copyright notice, license and other information about the program"), NULL },
    {"slideshow", 0, G_OPTION_FLAG_IN_MAIN, G_OPTION_ARG_NONE, &option_slideshow,
                 N_("Start slideshow"), NULL },
    { NULL }
};

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

    if (option_version)
    {
        printf( "%s %s\n", PACKAGE_NAME_STR, VERSION );
        return 0;
    }

    if (option_about)
    {
        gchar * message = get_about_message ();
        printf ("%s", message);
        g_free (message);
        return 0;
    }

#ifdef ENABLE_LIBSMFM
    fm_gtk_init(NULL);
#endif

    spicview_register_resource();

    #define BUILTIN_ICON(icon) do {\
        GdkPixbuf * pixbuf = gdk_pixbuf_new_from_resource(SPICVIEW_RESOURCE_PATH "icons/" icon ".png", NULL);\
        if (pixbuf)\
            gtk_icon_theme_add_builtin_icon(icon, gdk_pixbuf_get_width(pixbuf), pixbuf);\
    } while (0)

    BUILTIN_ICON("spicview");
    BUILTIN_ICON("object-rotate-right");
    BUILTIN_ICON("object-rotate-left");
    BUILTIN_ICON("object-flip-horizontal");
    BUILTIN_ICON("object-flip-vertical");

    #undef BUILTIN_ICON

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
        gchar * local_path = translate_uri_to_local_path (files[0]);
        main_win_open(win, local_path, ZOOM_FIT);
        g_free(local_path);

        if (option_slideshow)
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
