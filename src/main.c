#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <jpeglib.h>

#include <setjmp.h>

#define NUM_SAMPLE 10

int32_t histogram[NUM_SAMPLE][3][256];

void
put_scanline_someplace (JSAMPLE* ba, int row_stride, int sample_id)
{
    static int height;
    int i;

#if 1
    for (i=0; i < row_stride; i+=3)
    {
        /*
        printf("(%03d, ", ba[i]);
        printf("%03d, ", ba[i+1]);
        printf("%03d) ", ba[i+2]);
        */

        histogram[sample_id][0][ba[i]]++;
        histogram[sample_id][1][ba[i+1]]++;
        histogram[sample_id][2][ba[i+2]]++;
    }
    //printf("\n");
#endif


    //printf ("width: %3d height: %3d\n", row_stride, height++);
}

int read_jpeg(const char *filename)
{
    struct jpeg_error_mgr jerr; /* "public" fields */

    struct jpeg_decompress_struct cinfo;
    FILE * infile;      /* source file */
    JSAMPARRAY buffer;        /* Output row buffer */
    int row_stride;     /* physical row width in output buffer */

    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return 0;
    }

    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    (void) jpeg_read_header(&cinfo, TRUE);
    (void) jpeg_start_decompress(&cinfo);

    printf ("P3\n# CREATOR: The GIMP's PNM Filter Version 1.0\n");
    printf ("%d %d\n255\n", cinfo.output_width, cinfo.output_height);

    row_stride = cinfo.output_width * cinfo.output_components;
    printf("row_stride = %d\n", row_stride);
    buffer = (*cinfo.mem->alloc_sarray)
        ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    for (int i = 0; i < NUM_SAMPLE; i++)
    {
        while (cinfo.output_scanline < cinfo.output_height &&
               cinfo.output_scanline < (cinfo.output_height / NUM_SAMPLE) * (i + 1))
        {
            /* jpeg_read_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could ask for
             * more than one scanline at a time if that's more convenient.
             */
            (void) jpeg_read_scanlines(&cinfo, buffer, 1);
            /* Assume put_scanline_someplace wants a pointer and sample count. */
            put_scanline_someplace(buffer[0], row_stride, i);
        }
    }

    while (cinfo.output_scanline < cinfo.output_height)
    {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        put_scanline_someplace(buffer[0], row_stride, NUM_SAMPLE - 1);
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
}

int
main (int argc, char *argv[])
{
    int32_t histogram1[NUM_SAMPLE][3][256];
    int32_t histogram2[NUM_SAMPLE][3][256];
    read_jpeg(argv[1]);
    memcpy(histogram1, histogram, sizeof(int32_t) * NUM_SAMPLE * 3 * 256);
    memset(histogram, 0, sizeof(int32_t) * NUM_SAMPLE * 3 * 256);
    read_jpeg(argv[2]);
    memcpy(histogram2, histogram, sizeof(int32_t) * NUM_SAMPLE * 3 * 256);

    int sum1[NUM_SAMPLE][3] = {0, };
    int sum2[NUM_SAMPLE][3] = {0, };

    for (int i = 0; i < NUM_SAMPLE; i++)
    {
        for (int k = 0; k < 256; k++)
        {
            for (int c = 0; c < 3; c++)
            {
                sum1[i][c] += histogram1[i][c][k];
                sum2[i][c] += histogram1[i][c][k];
            }
        }
    }

    float result[NUM_SAMPLE][3] = {0, };
    float result_sum[NUM_SAMPLE];
    for (int i = 0; i < NUM_SAMPLE; i++)
    {
        printf("SAMEPLE %d\n", i);
        for (int k = 0; k < 256; k++)
        {
#if 0
            printf("(%u,%u,%u) (%u,%u,%u) (%lf,%lf,%lf)\n",
                    histogram1[i][0][k],
                    histogram1[i][1][k],
                    histogram1[i][2][k],
                    histogram2[i][0][k],
                    histogram2[i][1][k],
                    histogram2[i][2][k],
                    (float)(histogram1[i][0][k] - histogram2[i][0][k]) / (float)(sum1[i][0] + sum2[i][0]) * 100,
                    (float)(histogram1[i][1][k] - histogram2[i][1][k]) / (float)(sum1[i][1] + sum2[i][1]) * 100,
                    (float)(histogram1[i][2][k] - histogram2[i][2][k]) / (float)(sum1[i][2] + sum2[i][2]) * 100);
#endif

            for (int c = 0; c < 3; c++)
            {
                result[i][c] +=
                    (fabsf((float)histogram1[i][c][k] - histogram2[i][c][k])
                    / (float)(sum1[i][c] + sum2[i][c])) * 100 / 3;
            }
        }

        printf("result = ");
        for (int c = 0; c < 3; c++)
        {
            printf("%lf ", result[i][c]);
            result_sum[i] += result[i][c] / 3;
        }
        printf("\n");
        printf("sample result = %lf\n", result_sum[i]);
    }

    float total_result = 0;
    for (int i = 0; i < NUM_SAMPLE; i++)
    {
        total_result += result_sum[i] / 10;
    }
    printf("\n total_result = %lf\n", total_result);

    exit(0);
}

