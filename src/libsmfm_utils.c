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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "libsmfm_utils.h"

GtkMenu * get_fm_file_menu_for_path(GtkWindow* parent, const char * path)
{
#ifdef ENABLE_LIBSMFM
    GFile * gfile = NULL;
    GFileInfo * gfile_info = NULL;
    FmPath * fm_path = NULL;
    FmFileInfo * fm_file_info = NULL;
    GtkMenu * popup = NULL;

    gfile = g_file_new_for_path(path);
    if (!gfile)
        goto out;

    gfile_info = g_file_query_info(gfile, "standard::*,unix::*,time::*", G_FILE_QUERY_INFO_NONE, NULL, NULL);
    if (!gfile_info)
        goto out;

    fm_path = fm_path_new_for_path(path);
    if (!fm_path)
        goto out;

    fm_file_info = fm_file_info_new_from_gfileinfo(fm_path, gfile_info);
    if (!fm_file_info)
        goto out;

    FmFileMenu * fm_file_menu = fm_file_menu_new_for_file(parent,
                                                          fm_file_info,
                                                          /*cwd*/ NULL,
                                                          TRUE);
    if (!fm_file_menu)
        goto out;

    popup = fm_file_menu_get_menu(fm_file_menu);
 
out:

    if (fm_file_info)
        fm_file_info_unref(fm_file_info);
    if (fm_path)
        fm_path_unref(fm_path);
    if (gfile_info)
        g_object_unref(G_OBJECT(gfile_info));
    if (gfile)
        g_object_unref(G_OBJECT(gfile));

    return popup;
#else
    return NULL;
#endif /* ENABLE_LIBSMFM */
}


gchar * translate_uri_to_local_path(const char * uri)
{
#ifdef ENABLE_LIBSMFM
    gchar * local_path_str = NULL;
    FmPath * path = fm_path_new_for_str(uri);
    if (fm_path_is_native(path))
    {
        local_path_str = fm_path_to_str(path);
    }
    else
    {
        GFile * gf = fm_path_to_gfile(path);
        local_path_str = g_file_get_path(gf);
        g_object_unref(gf);
    }

    fm_path_unref(path);

    return local_path_str;

#else
    return g_strdup(uri);
#endif /* ENABLE_LIBSMFM */
}
