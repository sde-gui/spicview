/**
 * Copyright (c) 2012-2016 Vadim Ushakov <igeekless@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "info-window.h"
#include <gdk/gdkkeysyms.h>

static gboolean on_info_window_key_press_event(
    GtkWidget * widget,
    GdkEventKey * event,
    gpointer user_data)
{
    if (event->keyval == GDK_Escape)
    {
        gtk_widget_hide (widget);
        gtk_widget_destroy (widget);
    }
    return FALSE;
}


GtkWidget * create_info_window (GtkWindow * parent, const char * title, const char * text)
{
    GtkWidget * info_window;
    GtkWidget * scrolled_window;
    GtkWidget * text_view;

    info_window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_widget_set_size_request (info_window, 600, 400);
    gtk_window_set_modal (GTK_WINDOW (info_window), TRUE);
    gtk_window_set_destroy_with_parent (GTK_WINDOW (info_window), TRUE);
    gtk_window_set_skip_taskbar_hint (GTK_WINDOW (info_window), TRUE);
    gtk_window_set_skip_pager_hint (GTK_WINDOW (info_window), TRUE);

    if (parent)
        gtk_window_set_transient_for (GTK_WINDOW (info_window), parent);

    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_show (scrolled_window);
    gtk_container_add (GTK_CONTAINER (info_window), scrolled_window);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

    text_view = gtk_text_view_new ();
    gtk_widget_show (text_view);
    gtk_container_add (GTK_CONTAINER (scrolled_window), text_view);
    gtk_text_view_set_editable (GTK_TEXT_VIEW (text_view), FALSE);
    gtk_text_view_set_wrap_mode (GTK_TEXT_VIEW (text_view), GTK_WRAP_WORD);
    gtk_text_view_set_left_margin (GTK_TEXT_VIEW (text_view), 10);
    gtk_text_view_set_right_margin (GTK_TEXT_VIEW (text_view), 10);

    g_signal_connect ((gpointer) info_window, "key_press_event",
                      G_CALLBACK (on_info_window_key_press_event),
                      NULL);


    PangoFontDescription * font_desc = pango_font_description_from_string ("Monospace");
    gtk_widget_modify_font (text_view, font_desc);
    pango_font_description_free (font_desc);

    GtkTextBuffer *buffer = gtk_text_buffer_new (NULL);
    gtk_text_buffer_set_text (buffer, text, -1);
    gtk_text_view_set_buffer (GTK_TEXT_VIEW (text_view), buffer);
    g_object_unref (buffer);

    gtk_window_set_title (GTK_WINDOW (info_window), title);

    return info_window;
}
