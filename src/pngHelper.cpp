extern "C" {
    #include "ff.h" // FS functions and declarations.
    #include <setjmp.h>
    #include <png.h>
    #define PNG_READ_SUPPORTED

    #include "DEV_Config.c"
    #include "LCD_1in28.c"
    #include "font20.c"
    #include "GUI_Paint.h"
}

typedef struct {
    FIL *file;
} custom_file;

void custom_read_data(png_structrp png_ptr, png_bytep data, size_t length) {
    printf("Custom read data...\n");
    UINT bytesRead;
    custom_file *filep = (custom_file*)png_get_io_ptr(png_ptr);
    f_read(filep->file, data, length, &bytesRead);
}

static void error(png_structp png_ptr, const char *message)
{
    printf("Error from libpng: %s\n", message);
}

void DisplayPng(FIL &file) {
    printf("Creating read structure...\n");
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, error, NULL);

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
    custom_file filep;
    filep.file = &file;
    png_set_read_fn(png_ptr, &filep, custom_read_data);

    printf("Setting up LongJump...\n");

    if (setjmp(png_jmpbuf(png_ptr)) == 0) {
        printf("LongJump set...\n");
    } else {
        printf("We got a LongJump, destroying read struct...\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    printf("Reading info...\n");
    // The call to png_read_info() gives us all of the information from the
    // PNG file before the first IDAT (image data chunk). REQUIRED.
    png_read_info(png_ptr, info_ptr);

    printf("Parsing image info...\n");
    png_uint_32 width, height;
    int bit_depth, color_type, interlace_type, row;
    // Get IDAT (image data chunk).
    png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    printf("PNG info: width: %d, height: %d\n", width, height);

    // Allocate the memory to hold the image using the fields of info_ptr. *
    png_bytep row_pointers[height];

    for (row = 0; row < height; row++)
        row_pointers[row] = NULL; // Clear the pointer array *

    printf("Allocating memory to read image data...\n");

    for (row = 0; row < height; row++)
        row_pointers[row] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));

        // row_pointers[row] = png_malloc(
        //     static_cast<png_const_structrp>(png_ptr),
        //     png_get_rowbytes(static_cast<png_const_structrp>(png_ptr), static_cast<png_const_inforp>(info_ptr)));

    printf("Reading image...\n");
    // png_read_update_info(png_ptr, info_ptr);
    png_read_image(png_ptr, row_pointers);

    //** display row_pointers ***
    printf("Initialize display...\n");
    if (DEV_Module_Init() != 0) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return;
    }

    /* LCD Init */
    LCD_1IN28_Init(HORIZONTAL);
    LCD_1IN28_Clear(WHITE);

    /* Turn backlight on */
    printf("Turning on backlight...\n");
    EPD_BL_PIN = 25;
    DEV_GPIO_Mode(EPD_BL_PIN, GPIO_OUT);
    DEV_Digital_Write(EPD_CS_PIN, 1);
    DEV_Digital_Write(EPD_DC_PIN, 0);
    DEV_Digital_Write(EPD_BL_PIN, 1);

    // UDOUBLE Imagesize = width * height * 2;
    // UWORD *texture;

    // if ((texture = (UWORD *) malloc(Imagesize)) == NULL) {
    //     printf("Failed to allocate memory to display the image...\n");
    //     png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    // }

    // printf("Paint_NewImage...\n");
    // Paint_NewImage((UBYTE *)texture, width, height, ROTATE_0, WHITE);
    // printf("Paint_SetScale...\n");
    // Paint_SetScale(65);

    // while (true) {
    // printf("Paint_Clear...\n");
    // Paint_Clear(WHITE);
    // printf("Paint_DrawString_EN...\n");
    // Paint_DrawString_EN(100, 100, "Hello, World!", &Font20, BLACK, WHITE);
    printf("LCD_1IN28_Display...\n");
    // LCD_1IN28_Display(texture);


    LCD_1IN28_SetWindows(0, 0, width, height);
    DEV_Digital_Write(EPD_DC_PIN, 1);
    DEV_Digital_Write(EPD_CS_PIN, 0);

    for (row = 0; row < height; row++) {
        // DEV_SPI_Write_nByte((uint8_t *)row_pointers[row], width*2);
        // DEV_SPI_Write_nByte((uint8_t *)row_pointers[row], sizeof(row_pointers[row]));
        // DEV_SPI_Write_nByte((uint8_t *)row_pointers[row], png_get_rowbytes(png_ptr, info_ptr));
        DEV_SPI_Write_nByte((uint8_t *)row_pointers[row], png_get_rowbytes(png_ptr, info_ptr) * 2);
    }

    DEV_Digital_Write(EPD_CS_PIN, 1);
    LCD_1IN28_SendCommand(0x29);


    //     DEV_Delay_ms(100);
    // }

    printf("DEV_Module_Exit...\n");
    DEV_Module_Exit();

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

    printf("Done! Destroying read struct...\n");
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}