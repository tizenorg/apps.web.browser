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

#ifndef __EFL_TOOLS_H__
#define __EFL_TOOLS_H__ 1

#include <string>
#include <memory>
#include <Evas.h>
#include <png.h>
#include <vector>
#include "BrowserImage.h"
#include "Blob.h"

namespace tizen_browser
{
namespace tools
{

namespace EflTools
{
    std::shared_ptr<BrowserImage> getBrowserImage(Evas_Object * eo_image);

    /**
     * Crate BrowserImage based on data e.g. from bp_history_info_fmt.
     */
    std::shared_ptr<BrowserImage> createBrowserImage(const int width,
        const int height, const int length,
        const unsigned char* const imageData);

    Evas_Object * getEvasImage(std::shared_ptr<BrowserImage> b_image, Evas_Object * parent);

    std::vector< uint8_t > rawEvasImageData(std::shared_ptr<BrowserImage> browserImage);
    std::vector< uint8_t > rawEvasImageData(Evas_Object * eo_image);

    std::unique_ptr<Blob> getBlobPNG(std::shared_ptr<BrowserImage> browserImage, int level = 9);
    void * getBlobPNG(int width, int height, void * image_data, int * length, int level = 9);
    Evas_Object * getEvasPNG(Evas_Object * parent, const void * buffer, int length);

    void setExpandHints(Evas_Object* toSet);

} /* end of namespace EflTools */
} /* end of namespace tools */
} /* end of namespace tizen_browser */


#endif /* __EFL_TOOLS_H__ */
