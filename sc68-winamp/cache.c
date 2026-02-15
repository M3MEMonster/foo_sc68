/*
 * @file    cache.c
 * @brief   sc68-ng plugin for winamp 5.5 - disk cache
 * @author  http://sourceforge.net/users/benjihan
 *
 * Copyright (c) 1998-2015 Benjamin Gerard
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 *
 */

/* generated config header */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

/* winamp sc68 declarations */
#include "wasc68.h"

/* sc68 */
#include <sc68/sc68.h>

/* windows */
#include <windows.h>

enum {
  MAX_CACHED_DISK = 8
};

static struct {
  HANDLE lock;                          /* lock disk cache */
  struct cache_entry {
    int ref;                            /* reference count */
    char * uri;                         /* uri  (key)      */
    sc68_disk_t disk;                   /* disk (val)      */
  } e [MAX_CACHED_DISK];
} g_cache;

static inline int cache_lock(void) {
  return WaitForSingleObject(g_cache.lock, INFINITE) == WAIT_OBJECT_0;
}

static inline void cache_unlock(void) {
  ReleaseMutex(g_cache.lock);
}

int wasc68_cache_init(void)
{
  /* clear and init cacke */
  memset(&g_cache,0,sizeof(g_cache));
  g_cache.lock = CreateMutex(NULL, FALSE, NULL);
  return -!g_cache.lock;
}

void wasc68_cache_kill(void)
{
  /* Clean the cache */
  if (cache_lock()) {
    int i;
    for (i=0; i<MAX_CACHED_DISK; ++i) {
      if (g_cache.e[i].ref > 0) {
        DBG("quit -- cache #%d has %d references\n",
            i, g_cache.e[i].ref);
        continue;
      }
      g_cache.e[i].ref = 0;
      if (g_cache.e[i].uri) {
        DBG("quit -- cache - #%d %d <%p> '%s'",
            i, g_cache.e[i].ref, g_cache.e[i].disk,g_cache.e[i].uri);
        free(g_cache.e[i].uri);
        g_cache.e[i].uri = 0;
      }
      sc68_disk_free(g_cache.e[i].disk);
      g_cache.e[i].disk = 0;
    }
    cache_unlock();
    CloseHandle(g_cache.lock);
    memset(&g_cache,0,sizeof(g_cache));
  }
}

void wasc68_cache_release(void * disk, int dont_keep)
{
  if (!disk) return;                    /* safety net */

  if (cache_lock()) {
    int i;
    for (i = 0; i < MAX_CACHED_DISK; ++i) {
      if (disk != g_cache.e[i].disk)
        continue;
      if (--g_cache.e[i].ref <= 0) {
        if (dont_keep) {
          /* DBG("get_disk -- cache - #%d %d <%p> '%s'\n", */
          /*     i, g_cache.e[i].ref, g_cache.e[i].disk, g_cache.e[i].uri); */
          free(g_cache.e[i].uri);
          g_cache.e[i].uri = 0;
          sc68_disk_free(g_cache.e[i].disk);
          g_cache.e[i].disk = 0;
        }
        if (g_cache.e[i].ref != 0) {
          DBG("rel_disk -- !!! reference is %d !!! \n",
              g_cache.e[i].ref);
          g_cache.e[i].ref = 0;
        }
      } else {
        /* DBG("get_disk -- cache ~ #%d %d <%p> '%s'\n", */
        /*     i, g_cache.e[i].ref, g_cache.e[i].disk, g_cache.e[i].uri); */
      }
      break;
    }
    cache_unlock();
    if (i == MAX_CACHED_DISK) {
      /* Disk was not cached */
      /* DBG("get_disk -- cache miss <%p>\n", disk); */
      sc68_disk_free(disk);
    }
  } else {
    DBG("rel_disk -- %s\n", "cache lock failed");
  }
}

void * wasc68_cache_get(const char * uri)
{
  sc68_disk_t disk = 0;

  if (!uri || !*uri) {
    DBG("get_disk -- %s\n", "no uri");
    return 0;
  }

  if (cache_lock()) {
    int i, j, k ;
    for (i = 0, j = k = -1; i < MAX_CACHED_DISK; ++i) {
      if (!g_cache.e[i].disk) {
        if (j < 0) j = i;           /* keep track of 1st free entry */
      } else if (!strcmp(uri, g_cache.e[i].uri)) {
        disk = g_cache.e[i].disk;
        g_cache.e[i].ref++;
        /* DBG("::get_disk -- cache = #%d %d <%p> '%s'\n", */
        /*     i, g_cache.e[i].ref,g_cache.e[i].disk,g_cache.e[i].uri); */
        break;
      } else if (k < 0 && ! g_cache.e[i].ref)
        k = i;              /* keep track of 1st unreferenced entry */
    }

    /* Did not find this uri in cache, load the disk. */
    if (!disk) {
      disk = sc68_load_disk_uri(uri);
      if (disk) {
        /* Free or unreferenced entry ? */
        i = j >= 0 ? j : k;
        /* Have a free entry in the cache ? */
        if (i >= 0) {
          free(g_cache.e[i].uri);
          sc68_disk_free(g_cache.e[i].disk);
          g_cache.e[i].disk = 0;
          g_cache.e[i].uri = 0;
          g_cache.e[i].uri = strdup(uri);
          if (g_cache.e[i].uri) {
            g_cache.e[i].ref  = 1;
            g_cache.e[i].disk = disk;
            /* DBG("get_disk -- cache + #%d %d <%p> '%s'\n", */
            /*     i, g_cache.e[i].ref,g_cache.e[i].disk,g_cache.e[i].uri); */
          } else {
            DBG("cache alloc failed -- '%s'\n", uri);
          }
        } else {
          DBG("cache full -- '%s'\n", uri);
        }
      } else {
        DBG("'%s' -- %s\n",
            uri,
            "could not cache (load failed)");
      }
    }
    cache_unlock();
  } else {
    DBG("cache lock failed -- '%s'\n", uri);
    disk = sc68_load_disk_uri(uri);
  }
  /* DBG("get_disk -- '%s' -- <%p>\n", uri, disk); */
  return disk;
}
