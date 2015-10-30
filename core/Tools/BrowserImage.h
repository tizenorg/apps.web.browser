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

#ifndef __BROWSER_IMAGE_H__
#define __BROWSER_IMAGE_H__

#include <string>
//#include <Evas.h>
namespace tizen_browser
{
namespace tools
{

struct BrowserImage
{
    enum ImageType{
        ImageTypeNoImage = 1,
        ImageTypeSerializedEvas = 2,
        ImageTypeEvasObject = 3,
        ImageTypePNG = 4
    };
    BrowserImage();
    ~BrowserImage();
    int id;
    std::string url;
    int width;
    int height;
    int dataSize;
    ImageType imageType;
    void * imageData;
};


} /* end of namespace tools */
} /* end of namespace tizen_browser */


#endif /* __BROWSER_IMAGE_H__ */
