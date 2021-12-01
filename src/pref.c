/*
 *      pref.c
 *
 *      Copyright (C) 2013 Vadim Ushakov
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glib/gi18n.h>
#include <glib/gstdio.h>
#include <gio/gio.h>

#include <stdio.h>
#include "pref.h"
#include "main-win.h"
#include "spicview.h"

#define CFG_DIR    PACKAGE_NAME_STR
#define CFG_FILE    CFG_DIR"/" PACKAGE_NAME_STR ".conf"

Pref pref = {0};

typedef enum {
    OTYPE_BOOLEAN,
    OTYPE_INT,
    OTYPE_COLOR,
    OTYPE_INTERP
} OptionType;

#define DEBUG_WRONG_OPTION_TYPE(option) g_error("Unknown option type %d (%s) in %s (%s:%d)", (int)option->type, option->name, __FUNCTION__, __FILE__, __LINE__)

typedef struct {
    char * name;
    char * group;
    void * pref_ptr;
    OptionType type;
} OptionDef;

#define DEF_OPTION(group, name, type) {#name, #group, &pref.name, OTYPE_##type},
static OptionDef option_defs[] = {
    DEF_OPTION(View, quit_on_escape, BOOLEAN)
    DEF_OPTION(View, open_maximized, BOOLEAN)
    DEF_OPTION(View, show_toolbar, BOOLEAN)
    DEF_OPTION(View, show_toolbar_fullscreen, BOOLEAN)
    DEF_OPTION(View, toolbar_on_top, BOOLEAN)
    DEF_OPTION(View, preload_images, BOOLEAN)
    DEF_OPTION(View, slide_delay, INT)
    DEF_OPTION(View, background_color_auto_adjust, BOOLEAN)
    DEF_OPTION(View, background_color, COLOR)
    DEF_OPTION(View, background_color_fullscreen, COLOR)
    DEF_OPTION(View, downscale_interpolation_mode, INTERP)
    DEF_OPTION(View, upscale_interpolation_mode, INTERP)

    DEF_OPTION(Edit, auto_save_rotated, BOOLEAN)
    DEF_OPTION(Edit, ask_before_save, BOOLEAN)
    DEF_OPTION(Edit, ask_before_delete, BOOLEAN)
    DEF_OPTION(Edit, rotate_exif_only, BOOLEAN)
    DEF_OPTION(Edit, jpg_quality, INT)
    DEF_OPTION(Edit, png_compression, INT)

    {NULL, NULL, NULL, 0}
};
#undef DEF_OPTION

static const char * interp_to_a(GdkInterpType interp)
{
    const char * interp_name = "Bilinear";
    switch (interp) {
        case GDK_INTERP_NEAREST:  interp_name = "Nearest"; break;
        case GDK_INTERP_BILINEAR: interp_name = "Bilinear"; break;
        case GDK_INTERP_HYPER:    interp_name = "Hyper"; break;
        default: /* nothing */ break;
    }
    return interp_name;
}

static GdkInterpType a_to_interp(const char * interp_name)
{
    GdkInterpType interp;
    if (g_strcmp0(interp_name, "Nearest") == 0) {
        interp = GDK_INTERP_NEAREST;
    } else if (g_strcmp0(interp_name, "Bilinear") == 0) {
        interp = GDK_INTERP_BILINEAR;
    } else if (g_strcmp0(interp_name, "Hyper") == 0) {
        interp = GDK_INTERP_HYPER;
    } else {
        interp = GDK_INTERP_BILINEAR;
    }
    return interp;
}

static gboolean kf_get_bool(GKeyFile* kf, const char* group, const char* name, gboolean* ret )
{
    GError* err = NULL;
    gboolean val = g_key_file_get_boolean(kf, group, name, &err);
    if( G_UNLIKELY(err) )
    {
        g_error_free(err);
        return FALSE;
    }
    if(G_LIKELY(ret))
        *ret = val;
    return TRUE;
}

static gboolean kf_get_int(GKeyFile* kf, const char* group, const char* name, int* ret )
{
    GError* err = NULL;
    int val = g_key_file_get_integer(kf, group, name, &err);
    if( G_UNLIKELY(err) )
    {
        g_error_free(err);
        return FALSE;
    }
    if(G_LIKELY(ret))
        *ret = val;
    return TRUE;
}

static gboolean kf_get_color(GKeyFile* kf, const char* group, const char* name, GdkColor* ret )
{
    gchar * color = g_key_file_get_string(kf, group, name, NULL);
    if (color)
    {
        gdk_color_parse(color, ret);
        g_free(color);
        return TRUE;
    }
    return FALSE;
}

static gboolean kf_get_interp(GKeyFile* kf, const char* group, const char* name, GdkInterpType* ret )
{
    gchar * interp_name = g_key_file_get_string(kf, group, name, NULL);
    if (interp_name)
    {
        *ret = a_to_interp(interp_name);
        g_free(interp_name);
        return TRUE;
    }
    return FALSE;
}


void load_preferences()
{
    GKeyFile* kf;
    char* path;

    pref.auto_save_rotated = FALSE;
    pref.ask_before_save = TRUE;
    pref.ask_before_delete = TRUE;
    pref.rotate_exif_only = TRUE;
    pref.open_maximized = FALSE;
    pref.background_color.red = 65535;
    pref.background_color.green = 65535;
    pref.background_color.blue = 65535;
    pref.background_color_fullscreen.red = 0;
    pref.background_color_fullscreen.green = 0;
    pref.background_color_fullscreen.blue = 0;
    pref.background_color_auto_adjust = TRUE;

    pref.downscale_interpolation_mode = GDK_INTERP_BILINEAR;
    pref.upscale_interpolation_mode = GDK_INTERP_BILINEAR;

    pref.jpg_quality = 90;
    pref.png_compression = 9;

    pref.quit_on_escape = TRUE;
    pref.show_toolbar = TRUE;
    pref.show_toolbar_fullscreen = FALSE;
    pref.preload_images = TRUE;

    kf = g_key_file_new();
    path = g_build_filename( g_get_user_config_dir(),  CFG_FILE, NULL );
    if( g_key_file_load_from_file( kf, path, 0, NULL ) )
    {
        OptionDef * option;
        for (option = option_defs; option->name; option++)
        {
            switch (option->type)
            {
                case OTYPE_BOOLEAN:
                    kf_get_bool(kf, option->group, option->name, (gboolean *) option->pref_ptr);
                    break;
                case OTYPE_INT:
                    kf_get_int(kf, option->group, option->name, (int *) option->pref_ptr);
                    break;
                case OTYPE_COLOR:
                    kf_get_color(kf, option->group, option->name, (GdkColor *) option->pref_ptr);
                    break;
                case OTYPE_INTERP:
                    kf_get_interp(kf, option->group, option->name, (GdkInterpType *) option->pref_ptr);
                    break;
                default:
                    DEBUG_WRONG_OPTION_TYPE(option);
            }
        }
    }
    g_free( path );
    g_key_file_free( kf );

    if (pref.slide_delay == 0)
        pref.slide_delay = 5;
}

void save_preferences()
{
    FILE* f;
    char* dir = g_build_filename( g_get_user_config_dir(), CFG_DIR, NULL );
    char* path = g_build_filename( g_get_user_config_dir(),  CFG_FILE, NULL );
    if( ! g_file_test( dir, G_FILE_TEST_IS_DIR ) )
    {
        g_mkdir( g_get_user_config_dir(), 0766 );
        g_mkdir( dir, 0766 );
    }
    g_free( dir );

    if(  (f = fopen( path, "w" )) )
    {
        fprintf(f, "# Generated by %s %s\n", PACKAGE_NAME_STR, VERSION);

        const char * prev_group = "";
        OptionDef * option;
        for (option = option_defs; option->name; option++)
        {
            if (strcmp(prev_group, option->group) != 0)
                fprintf(f, "\n[%s]\n", option->group);
            prev_group = option->group;

            switch (option->type)
            {
                case OTYPE_BOOLEAN:
                case OTYPE_INT:
                    fprintf(f, "%s=%d\n", option->name, *(int *) option->pref_ptr);
                    break;
                case OTYPE_COLOR:
                {
                    GdkColor * color = (GdkColor *) option->pref_ptr;
                    fprintf(f, "%s=#%02x%02x%02x\n", option->name,
                        color->red / 256, color->green / 256, color->blue / 256);
                    break;
                }
                case OTYPE_INTERP:
                {
                    GdkInterpType * interp = (GdkInterpType *) option->pref_ptr;
                    fprintf(f, "%s=%s\n", option->name, interp_to_a(*interp));
                    break;
                }
                default:
                    DEBUG_WRONG_OPTION_TYPE(option);
            }
        }
        fclose( f );
    }
    g_free( path );
}

static void on_set_default( GtkButton* btn, gpointer user_data )
{
    GtkWindow* parent=(GtkWindow*)user_data;
    GtkWidget* dlg=gtk_message_dialog_new_with_markup( parent, 0,
            GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
            _("SPicView will become the default viewer for all supported image files on your system.\n"
                "(This will be done through \'xdg-mime\' program)\n\n"
                "<b>Are you sure you really want to do this?</b>") );
    if( gtk_dialog_run( (GtkDialog*)dlg ) == GTK_RESPONSE_OK )
    {
        const char cmd[]="xdg-mime default spicview.desktop image/bmp image/gif image/jpeg image/jpg image/png image/tiff image/x-bmp image/x-pcx image/x-tga image/x-portable-pixmap image/x-portable-bitmap image/x-targa image/x-portable-greymap application/pcx image/svg+xml image/svg-xml";
        g_spawn_command_line_sync( cmd, NULL, NULL, NULL, NULL );
    }
    gtk_widget_destroy( dlg );
}

static void on_set_color(GtkColorButton * widget, gpointer user_data )
{
    MainWin * parent=(MainWin * ) user_data;
    GdkColor * color = (GdkColor * ) g_object_get_data(G_OBJECT(widget), "pref_ptr");
    if (color)
        gtk_color_button_get_color(GTK_COLOR_BUTTON(widget), color);
    main_win_update_background_color(parent);
}

static void on_background_color_auto_adjust(GtkCheckButton * widget, gpointer user_data )
{
    MainWin * parent = (MainWin *) user_data;
    gboolean * value = (gboolean *) g_object_get_data(G_OBJECT(widget), "pref_ptr");
    if (value)
        *value = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    main_win_update_background_color(parent);
}

static void on_interp_changed(GtkComboBox * widget, gpointer user_data )
{
    MainWin * parent=(MainWin * ) user_data;
    GdkInterpType * p_interp = (GdkInterpType * ) g_object_get_data(G_OBJECT(widget), "pref_ptr");
    if (p_interp)
    {
        GtkTreeIter iter;
        if (gtk_combo_box_get_active_iter (widget, &iter))
        {
            *p_interp = a_to_interp("_invalid_name_");
            gtk_tree_model_get (GTK_TREE_MODEL(gtk_combo_box_get_model(widget)), &iter,
                1, p_interp,
                -1);
            main_win_on_scale_preferences_changed(parent);
        }
    }
}

void edit_preferences( GtkWindow* parent )
{
    GtkBuilder * builder = gtk_builder_new();
    GtkDialog * dlg = NULL;
    guint result;

    GBytes * ui_bytes = g_resources_lookup_data(
        SPICVIEW_RESOURCE_PATH "ui/pref-dlg.ui",
        G_RESOURCE_LOOKUP_FLAGS_NONE,
        NULL);

    if (ui_bytes == NULL)
        goto end;

    result = gtk_builder_add_from_string(
        builder, 
        g_bytes_get_data(ui_bytes, NULL),
        g_bytes_get_size(ui_bytes),
        NULL);

    if (result == 0)
        goto end;

    dlg = (GtkDialog*)gtk_builder_get_object(builder, "dlg");
    gtk_window_set_transient_for((GtkWindow*)dlg, parent);

    OptionDef * option;
    for (option = option_defs; option->name; option++)
    {
        GtkWidget * widget = (GtkWidget *) gtk_builder_get_object(builder, option->name);
        if (!widget)
            continue;
        g_object_set_data(G_OBJECT(widget), "pref_ptr", option->pref_ptr);
        switch (option->type)
        {
            case OTYPE_BOOLEAN:
            {
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), * (gboolean *) option->pref_ptr);
                if (strcmp(option->name, "background_color_auto_adjust") == 0)
                    g_signal_connect(widget, "clicked", G_CALLBACK(on_background_color_auto_adjust), parent);
                break;
            }
            case OTYPE_INT:
            {
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), * (int *) option->pref_ptr);
                break;
            }
            case OTYPE_COLOR:
            {
                gtk_color_button_set_color(GTK_COLOR_BUTTON(widget), (GdkColor *) option->pref_ptr);
                g_signal_connect(widget, "color-set", G_CALLBACK(on_set_color), parent);
                break;
            }
            case OTYPE_INTERP:
            {
                GtkListStore * list_store = gtk_list_store_new(2, G_TYPE_STRING, GDK_TYPE_INTERP_TYPE);
                gtk_list_store_insert_with_values(list_store, NULL, -1,
                    0, "Nearest",
                    1, GDK_INTERP_NEAREST,
                    -1);
                gtk_list_store_insert_with_values(list_store, NULL, -1,
                    0, "Bilinear",
                    1, GDK_INTERP_BILINEAR,
                    -1);
                gtk_list_store_insert_with_values(list_store, NULL, -1,
                    0, "Hyper",
                    1, GDK_INTERP_HYPER,
                    -1);

                gtk_combo_box_set_model(GTK_COMBO_BOX(widget), GTK_TREE_MODEL(list_store));

                g_object_unref(list_store);

                GtkCellRenderer * column = gtk_cell_renderer_text_new();
                gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(widget), column, TRUE);

                gtk_cell_layout_set_attributes(GTK_CELL_LAYOUT(widget), column,
                    "text", 0,
                    NULL);

                GtkTreeIter iter;
                gboolean valid = gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store), &iter);
                while (valid)
                {
                    GdkInterpType interp;
                    gtk_tree_model_get (GTK_TREE_MODEL(list_store), &iter,
                        1, &interp,
                        -1);
                    if (interp == * (GdkInterpType *) option->pref_ptr)
                    {
                        gtk_combo_box_set_active_iter(GTK_COMBO_BOX(widget), &iter);
                        break;
                    }
                    valid = gtk_tree_model_iter_next (GTK_TREE_MODEL(list_store), &iter);
                }

                g_signal_connect(widget, "changed", G_CALLBACK(on_interp_changed), parent);

                break;
            }
            default:
                DEBUG_WRONG_OPTION_TYPE(option);
        }
    }

    {
        GtkWidget * set_default_btn = (GtkWidget*)gtk_builder_get_object(builder, "make_default");
        g_signal_connect(set_default_btn, "clicked", G_CALLBACK(on_set_default), parent);
    }

    gtk_dialog_run( dlg );

    for (option = option_defs; option->name; option++)
    {
        GtkWidget * widget = (GtkWidget *) gtk_builder_get_object(builder, option->name);
        if (!widget)
            continue;
        switch (option->type)
        {
            case OTYPE_BOOLEAN:
            {
                * (gboolean *) option->pref_ptr = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
                break;
            }
            case OTYPE_INT:
            {
                * (int *) option->pref_ptr = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
                gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), * (int *) option->pref_ptr);
                break;
            }
            case OTYPE_COLOR:
            case OTYPE_INTERP:
            {
                /* Nothing to do, */
                break;
            }
            default:
                DEBUG_WRONG_OPTION_TYPE(option);
        }
    }

end:
    g_bytes_unref(ui_bytes);
    g_object_unref(builder);
    gtk_widget_destroy((GtkWidget*)dlg);
}

