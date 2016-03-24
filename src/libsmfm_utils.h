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

#ifndef _SPICVIEW_LIBSMFM_H
#define _SPICVIEW_LIBSMFM_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef ENABLE_LIBSMFM
#include <libsmfm-core/fm.h>
#include <libsmfm-gtk/fm-gtk.h>
#endif

#include <gtk/gtk.h>

GtkMenu * get_fm_file_menu_for_path(GtkWindow* parent, const char * path);

gchar * translate_uri_to_local_path(const char * uri);

#endif
