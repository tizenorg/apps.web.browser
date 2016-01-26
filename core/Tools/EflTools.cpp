/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 *
 * Created on: May, 2014
 *     Author: k.dobkowski
 */

#include "browser_config.h"
#include <BrowserAssert.h>
#include "BrowserLogger.h"
#include "EflTools.h"
#include <Evas.h>
#include <cstring>

namespace tizen_browser {
namespace tools {
namespace EflTools {

struct pngBufferData {
    char * ptr;
    int length;
    int capacity;
    EINA_MAGIC

    pngBufferData() : ptr(0), length(0), capacity(0) {
        EINA_MAGIC_SET(this, 0x23823972);
    }

    ~pngBufferData() {
        free(ptr);
        EINA_MAGIC_SET(this, EINA_MAGIC_NONE);
    }

    bool write(const void * data, int size) {
        if (EINA_MAGIC_CHECK(this, 0x23823972)) {} else {
            EINA_MAGIC_FAIL(this, 0x23823972);
            return false;
        }

        if ((length + size) > capacity) {
            char * new_ptr = (char *)realloc(ptr, capacity + std::max<int>(size, 8192));
            if (!new_ptr) {
                return false;
            }
            capacity += std::max<int>(size, 8192);
            ptr = new_ptr;
        }
        memcpy(ptr + length, data, size);
        length += size;
        return true;
    }

    void * release(int * p_length) {
        if (EINA_MAGIC_CHECK(this, 0x23823972)) {} else {
            EINA_MAGIC_FAIL(this, 0x23823972);
            return NULL;
        }

        void * result = (void *)ptr;
        if (p_length) {
            *p_length = length;
        }
        ptr = 0;
        length = 0;
        capacity = 0;
        return result;
    }
};

static void psPngWriteFn(png_structp png_ptr, png_bytep data, png_size_t length)
{
    pngBufferData * buffer = reinterpret_cast<pngBufferData *>(png_get_io_ptr(png_ptr));
    if (!buffer->write(data, length)) {
        png_error(png_ptr, "write error");
    }
}

static void psPngFlushFn(png_structp)
{
}


std::unique_ptr<Blob> getBlobPNG(std::shared_ptr<BrowserImage> browserImage, int level)
{
    BROWSER_LOGD("[%s]: HELO !!"  , __func__);
    int length = 0;
    void * mem_buffer = getBlobPNG(browserImage->getWidth(), browserImage->getHeight(), browserImage->getData(), &length, level);
    std::unique_ptr<Blob> image(new Blob(mem_buffer, length));
    BROWSER_LOGD("[%s]:length =%d"  , __func__, image->getLength());

    return std::move(image);
}

void * getBlobPNG(int width, int height, void * image_data, int * length, int level)
{
    EINA_SAFETY_ON_NULL_RETURN_VAL(image_data, NULL);

    pngBufferData write_buffer;
    png_color_8 sig_bit;
    int num_passes = 1;
    png_bytep row_ptr;

    if (length) {
        *length = 0;
    }

    if (!width || !height) {
        BROWSER_LOGW("[ps] Incorrect image type");
        return NULL;
    }

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    EINA_SAFETY_ON_NULL_RETURN_VAL(png_ptr, NULL);

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (EINA_UNLIKELY(!info_ptr)) {
        png_destroy_write_struct(&png_ptr, NULL);
        return NULL;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        BROWSER_LOGE("[ps] PNG compessor failed");
        png_destroy_write_struct(&png_ptr, &info_ptr);
        png_destroy_info_struct(png_ptr, &info_ptr);
        return NULL;
    }

    png_set_compression_level(png_ptr, level);
    png_set_write_fn(png_ptr, &write_buffer, psPngWriteFn, psPngFlushFn);
    png_set_IHDR(png_ptr,
                 info_ptr,
                 width,
                 height,
                 8,
                 PNG_COLOR_TYPE_RGB_ALPHA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

#ifdef WORDS_BIGENDIAN
    png_set_swap_alpha(png_ptr);
#else
    png_set_bgr(png_ptr);
#endif

    sig_bit.red = 8;
    sig_bit.green = 8;
    sig_bit.blue = 8;
    sig_bit.alpha = 8;
    png_set_sBIT(png_ptr, info_ptr, &sig_bit);

    png_write_info(png_ptr, info_ptr);
    png_set_shift(png_ptr, &sig_bit);
    png_set_packing(png_ptr);

#ifdef PNG_WRITE_INTERLACING_SUPPORTED
    num_passes = png_set_interlace_handling(png_ptr);
#endif

    row_ptr = (png_bytep)image_data;
    for (int pass = 0 ; pass < num_passes ; ++pass) {
        for (int y = 0 ; y < height ; ++y) {
            png_write_row(png_ptr, row_ptr);
            row_ptr += (width * 4);
        }
    }

    png_write_end(png_ptr, info_ptr);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    png_destroy_info_struct(png_ptr, &info_ptr);

    return write_buffer.release(length);
}



void setExpandHints(Evas_Object* toSet) {
    evas_object_size_hint_weight_set(toSet, EVAS_HINT_EXPAND,
    EVAS_HINT_EXPAND);
    evas_object_size_hint_align_set(toSet, EVAS_HINT_FILL,
    EVAS_HINT_FILL);
}


} /* end of EflTools */
} /* end of namespace tools */
} /* end of namespace tizen_browser */

