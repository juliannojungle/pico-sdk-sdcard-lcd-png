#include "ff.h" // FS functions and declarations.


#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

extern "C" {
    #include <png.h>
    // #include "pngread.c"
    // #include <zlib.h>
    #define PNG_READ_SUPPORTED
}

struct custom_file {
    FIL file;
};

void custom_read_data(png_structrp png_ptr, png_bytep data, size_t length) {
    UINT bytesRead;
    f_read((FIL*)png_ptr, data, length, &bytesRead);
}

void DisplayPng(FIL &file) {
    printf("Creating read structure...\n");
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr == NULL) {
        printf("png_create_read_struct error\n");
        return;
    }

    printf("Allocating memory for image information...\n");
    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL) {
        printf("png_create_info_struct error\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return;
    }

    printf("Setting up the custom read function...\n");
    custom_file filep { file };
    png_set_read_fn(static_cast<png_structrp>(png_ptr), &filep, custom_read_data);

    // OK, you're doing it the hard way, with the lower-level functions. *

    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk). REQUIRED.
    png_read_info(png_ptr, info_ptr);

    // Get IDAT (image data chunk).
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, row;
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);

    // Allocate the memory to hold the image using the fields of info_ptr. *
    png_bytep row_pointers[height];

    for (row = 0; row < height; row++)
        row_pointers[row] = NULL; // Clear the pointer array *

    for (row = 0; row < height; row++)
        auto teste = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));
        // row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

        // row_pointers[row] = png_malloc(
        //     static_cast<png_const_structrp>(png_ptr),
        //     png_get_rowbytes(static_cast<png_const_structrp>(png_ptr), static_cast<png_const_inforp>(info_ptr)));

    // png_read_update_info(png_ptr, info_ptr);
    png_read_image(png_ptr, row_pointers);

    //** display row_pointers ***

    // int number_passes = png_set_interlace_handling(png_ptr);
    // int pass;

    // for (pass = 0; pass < number_passes; pass++) {
    //     for (row = 0; row < height; row++) {
    //         row_pointers[row] = NULL; // Clear the pointer array *
    //         row_pointers[row] = png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

    //         // Read the image a single row at a time *
    //         png_read_rows(png_ptr, &row_pointers[row], NULL, 1);

    //         // display row_pointers;
    //     }

    //     // display row_pointers;
    // }

    // // Read rest of file, and get additional chunks in info_ptr.  REQUIRED. *
    // png_read_end(png_ptr, info_ptr);

    // Done
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

}