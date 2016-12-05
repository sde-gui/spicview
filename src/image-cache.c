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

#include "image-cache.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define CACHE_SIZE 20

#define DEBUG_PRINT 0

static ImageCacheItem cache[CACHE_SIZE];

static unsigned long long get_available_mem(void)
{
    return sysconf(_SC_AVPHYS_PAGES) * (unsigned long long) sysconf(_SC_PAGE_SIZE);
}

static unsigned long long get_total_mem(void)
{
    static unsigned long long totalmem = 0;

    if (!totalmem)
        totalmem = sysconf(_SC_PHYS_PAGES) * (unsigned long long) sysconf(_SC_PAGE_SIZE);

    return totalmem;
}

static void get_memory_limits(
    unsigned long long * p_mem_limit1,
    unsigned long long * p_mem_limit2)
{
    double f1 = 0.10;
    double f2 = 0.30;
    double fa = 0.70;

    double f1_max = 0.20;
    double f2_max = 0.60;

    unsigned long long total_mem = get_total_mem();
    unsigned long long available_mem = get_available_mem();

    unsigned long long mem_limit1 = total_mem * f1 + available_mem * fa;
    unsigned long long mem_limit2 = total_mem * f2 + available_mem * fa + 1;

    if (mem_limit1 > total_mem * f1_max)
        mem_limit1 = total_mem * f1_max;

    if (mem_limit2 > total_mem * f2_max)
        mem_limit2 = total_mem * f2_max;

    if (sizeof(NULL) < 8)
    {
        if (mem_limit1 > (unsigned long long) 800 * 1024 * 1024)
            mem_limit1 = (unsigned long long) 800 * 1024 * 1024;
        if (mem_limit2 > (unsigned long long) 1400 * 1024 * 1024)
            mem_limit2 = (unsigned long long) 1400 * 1024 * 1024;
    }

    if (p_mem_limit1)
        *p_mem_limit1 = mem_limit1;

    if (p_mem_limit2)
        *p_mem_limit2 = mem_limit2;
}

static unsigned get_cache_limit(void)
{
    static unsigned value = 0;

    if (value)
        return value;

    /* some heuristics to estimate cache size limit */
    unsigned long long total_mem = get_total_mem();
    total_mem /= 1024 * 1024; /* to MiB */
    total_mem /= 5; /* 1/5 of total memory */
    unsigned limit = 2 + total_mem / 30; /* guess each cache item is about 30MiB */

    if (limit > CACHE_SIZE)
        limit = CACHE_SIZE;

    value = limit;

    if (DEBUG_PRINT)
    {
        g_print("cache size limit is %u items\n", value);
    }

    return value;

}

static unsigned long long get_cache_memory(void)
{
    unsigned i;
    unsigned long long cache_memory = 0;

    for (i = 0; i < get_cache_limit(); i++)
    {
        ImageCacheItem* item = &cache[i];
        if (item->pix)
            cache_memory += gdk_pixbuf_get_byte_length (item->pix);
    }
    return cache_memory;
}

static unsigned get_nr_portected_items(void)
{
    unsigned long long mem_limit1;
    unsigned long long mem_limit2;
    get_memory_limits(&mem_limit1, &mem_limit2);

    unsigned long long cache_memory = get_cache_memory();

    unsigned limit = get_cache_limit();
    if (limit >= 10 && cache_memory < mem_limit1)
        return 5;
    else if (limit >= 5 && cache_memory < (mem_limit1 + mem_limit2) / 2)
        return 3;
    return 0;
}

static void clear_item(ImageCacheItem* item)
{
    if (item->pix)
        g_object_unref(item->pix);
    if (item->animation)
        g_object_unref(item->animation);
    if (item->name)
        g_free(item->name);
    item->pix = NULL;
    item->animation = NULL;
    item->name = NULL;
}

static void drop_items(ImageCacheItem* additional_protected_item)
{
    unsigned i;

    if (DEBUG_PRINT)
    {
        unsigned long long mem_limit1;
        unsigned long long mem_limit2;
        get_memory_limits(&mem_limit1, &mem_limit2);

        g_print("******************************\n");
        g_print("Phys memory:     %5llu M\n", get_total_mem() / (1024 * 1024));
        g_print("Avail memory:    %5llu M\n", get_available_mem() / (1024 * 1024));
        g_print("Memory limit 1:  %5llu M\n", mem_limit1 / (1024 * 1024));
        g_print("Memory limit 2:  %5llu M\n", mem_limit2 / (1024 * 1024));
        g_print("Memory used:     %5llu M\n", get_cache_memory() / (1024 * 1024));
    }

    for (i = 0; i < get_cache_limit() - get_nr_portected_items(); i++)
    {
        ImageCacheItem* item = &cache[i];

        unsigned long long mem_limit1;
        unsigned long long mem_limit2;
        get_memory_limits(&mem_limit1, &mem_limit2);

        unsigned long long cache_memory = get_cache_memory();
        if (cache_memory < mem_limit1)
            break;

        if (item == additional_protected_item)
            continue;

        if (!item->pix)
            continue;

        double f = (cache_memory - mem_limit1) / (double) (mem_limit2 - mem_limit1) + 0.20;
        double f_rand = (double)rand() / (double)RAND_MAX;

        gint64 interval = g_get_monotonic_time() - item->cache_atime;
        f += 0.10 * interval / (1000000.0 * 60.0); /* +10% for every minute*/

        if (DEBUG_PRINT)
            g_print("f = %f, f_rand = %f\n", (float) f, (float) f_rand);

        if (f_rand < f)
        {

            if (DEBUG_PRINT)
            {
                g_print("Drop:            %5lu M\n",
                    (unsigned long) gdk_pixbuf_get_byte_length (item->pix) / (1024 * 1024));
            }

            clear_item(item);
        }
    }
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
            g_strcmp0(item->name, item2->name) == 0)
        {
            drop_items(item2);
            return item2;
        }
    }
    drop_items(NULL);
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

    item->cache_atime = g_get_monotonic_time();

    return TRUE;
}

void image_cache_put(ImageCacheItem* item)
{
    if (!item || !item->name)
        return;

    ImageCacheItem* cache_item = lookup_item(item);

    if (!cache_item)
    {
        unsigned limit = get_cache_limit();
        unsigned nr_portected_items = get_nr_portected_items();
        unsigned index = (((unsigned)rand()) ^ ((unsigned)time(NULL)) ^ (unsigned)item->mtime) %
            (limit - get_nr_portected_items());

        if (nr_portected_items > 0)
        {
            unsigned index2 = limit - get_nr_portected_items();
            ImageCacheItem tmp = cache[index];
            cache[index] = cache[index2];
            for (; index2 < limit - 1; index2++)
            {
                cache[index2] = cache[index2 + 1];
            }
            cache[index2] = tmp;
            index = index2;
        }

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

    cache_item->cache_atime = g_get_monotonic_time();
}

unsigned int image_cache_get_limit(void)
{
    return get_cache_limit();
}

unsigned int image_cache_get_nr_portected_items(void)
{
    return get_nr_portected_items();
}

void image_cache_yield(void)
{
    drop_items(NULL);
}
