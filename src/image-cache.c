/**
 * Copyright (c) 2012 Vadim Ushakov
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

#include "image-cache.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define CACHE_SIZE 20

static ImageCacheItem cache[CACHE_SIZE];


static unsigned get_cache_limit(void)
{
    static unsigned value = 0;

    if (value)
        return value;

    /* some heuristics to estimate cache size limit */
    unsigned long long totalmem = sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE);
    totalmem /= 1024 * 1024; // to MiB
    totalmem /= 5; // 1/5 of total memory
    unsigned limit = 1 + totalmem / 20; // guess each cache item is about 20MiB

    if (limit > CACHE_SIZE)
        limit = CACHE_SIZE;

    value = limit;

    //g_print("cache size limit is %u items\n", value);

    return value;
}


static ImageCacheItem* lookup_item(ImageCacheItem* item)
{
    unsigned i;
    unsigned limit = get_cache_limit();
    for (i = 0; i < limit; i++)
    {
        ImageCacheItem* item2 = &cache[i];
        if (item->mtime == item2->mtime &&
            item->size == item2->size &&
            item2->name &&
            strcmp(item->name, item2->name) == 0)
        {
            return item2;
        }
    }
    return NULL;
}

gboolean image_cache_get(ImageCacheItem* item)
{
    if (!item || !item->name)
        return FALSE;

    ImageCacheItem* cache_item = lookup_item(item);
    if (!cache_item)
        return FALSE;

    item->pix = cache_item->pix;
    item->animation = cache_item->animation;

    if (item->pix)
        g_object_ref(item->pix);
    if (item->animation)
        g_object_ref(item->animation);

    return TRUE;
}

void image_cache_put(ImageCacheItem* item)
{
    if (!item || !item->name)
        return;

    ImageCacheItem* cache_item = lookup_item(item);

    if (!cache_item)
    {
        unsigned index =
            (((unsigned)rand()) ^ ((unsigned)time(NULL)) ^ (unsigned)item->mtime) % get_cache_limit();

        cache_item = &cache[index];
        if (cache_item->name)
            g_free(cache_item->name);
        cache_item->name = g_strdup(item->name);
        cache_item->mtime = item->mtime;
        cache_item->size = item->size;
    }

    if (cache_item->pix)
        g_object_unref(cache_item->pix);
    if (cache_item->animation)
        g_object_unref(cache_item->animation);

    cache_item->pix = item->pix;
    cache_item->animation = item->animation;

    if (cache_item->pix)
        g_object_ref(cache_item->pix);
    if (cache_item->animation)
        g_object_ref(cache_item->animation);

}

unsigned image_cache_get_limit(void)
{
    return get_cache_limit();
}

