#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <jpeglib.h>

#include <setjmp.h>

int32_t histogram[256];

void
put_scanline_someplace (JSAMPLE* ba, int row_stride)
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

        histogram[ba[i]]++;
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

    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
        put_scanline_someplace(buffer[0], row_stride);
    }

    (void) jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(infile);
    return 0;
}

int
main ()
{
    int32_t histogram1[256];
    int32_t histogram2[256];
    read_jpeg("test1.jpg");
    memcpy(histogram1, histogram, sizeof(histogram));
    memset(histogram, 0, sizeof(histogram));
    read_jpeg("test2.jpg");
    memcpy(histogram2, histogram, sizeof(histogram));

    int sum1 = 0;
    int sum2 = 0;
    for (int i = 0; i < 256; i++)
    {
        sum1 += histogram1[i];
        sum2 += histogram2[i];
    }

    float result = 0;
    for (int i = 0; i < 256; i++)
    {
        printf("%7u %7u %lf%%\n",
                histogram1[i], histogram2[i],
                (float)(histogram1[i] - histogram2[i]) / (float)(sum1 + sum2) * 100);
        result += fabsf((float)histogram1[i] - histogram2[i]) / (float)(sum1 + sum2) * 100;
    }
    printf("%lf\n", result);

    exit(0);
}

