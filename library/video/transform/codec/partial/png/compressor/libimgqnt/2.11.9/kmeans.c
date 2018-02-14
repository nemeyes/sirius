/*
** © 2011-2016 by Kornel Lesiński.
** See COPYRIGHT file for license.
*/

#include "libimagequant.h"
#include "pam.h"
#include "kmeans.h"
#include "nearest.h"
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#else
#define omp_get_max_threads() 1
#define omp_get_thread_num() 0
#endif

/*
 * K-Means iteration: new palette color is computed from weighted average of colors that map to that palette entry.
 */
LIQ_PRIVATE void kmeans_init(const colormap *map, const unsigned int max_threads, kmeans_state average_color[])
{
    memset(average_color, 0, sizeof(average_color[0])*(KMEANS_CACHE_LINE_GAP+map->colors)*max_threads);
}

LIQ_PRIVATE void kmeans_update_color(const f_pixel acolor, const float value, const colormap *map, unsigned int match, const unsigned int thread, kmeans_state average_color[])
{
    match += thread * (KMEANS_CACHE_LINE_GAP+map->colors);
    average_color[match].a += acolor.a * value;
    average_color[match].r += acolor.r * value;
    average_color[match].g += acolor.g * value;
    average_color[match].b += acolor.b * value;
    average_color[match].total += value;
}

LIQ_PRIVATE void kmeans_finalize(colormap *map, const unsigned int max_threads, const kmeans_state average_color[])
{
    for (unsigned int i=0; i < map->colors; i++) {
        double a=0, r=0, g=0, b=0, total=0;

        // Aggregate results from all threads
        for(unsigned int t=0; t < max_threads; t++) {
            const unsigned int offset = (KMEANS_CACHE_LINE_GAP+map->colors) * t + i;

            a += average_color[offset].a;
            r += average_color[offset].r;
            g += average_color[offset].g;
            b += average_color[offset].b;
            total += average_color[offset].total;
        }

        if (total && !map->palette[i].fixed) {
            map->palette[i].acolor = (f_pixel){
                .a = a / total,
                .r = r / total,
                .g = g / total,
                .b = b / total,
            };
            map->palette[i].popularity = total;
        }
    }
}

LIQ_PRIVATE double kmeans_do_iteration(histogram *hist, colormap *const map, kmeans_callback callback)
{
    const unsigned int max_threads = omp_get_max_threads();
	kmeans_state *average_color = malloc((KMEANS_CACHE_LINE_GAP + map->colors) * max_threads * sizeof(kmeans_state));
    kmeans_init(map, max_threads, average_color);
    struct nearest_map *const n = nearest_init(map);
    hist_item *const achv = hist->achv;
    const int hist_size = hist->size;

    double total_diff=0;
    #pragma omp parallel for if (hist_size > 2000) \
        schedule(static) default(none) shared(average_color,callback) reduction(+:total_diff)
    for(int j=0; j < hist_size; j++) {
        float diff;
        unsigned int match = nearest_search(n, &achv[j].acolor, achv[j].tmp.likely_colormap_index, &diff);
        achv[j].tmp.likely_colormap_index = match;
        total_diff += diff * achv[j].perceptual_weight;

        kmeans_update_color(achv[j].acolor, achv[j].perceptual_weight, map, match, omp_get_thread_num(), average_color);

        if (callback) callback(&achv[j], diff);
    }

    nearest_free(n);
    kmeans_finalize(map, max_threads, average_color);

	free(average_color);
    return total_diff / hist->total_perceptual_weight;
}
