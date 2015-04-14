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
#include "browser_config.h"
#include "BrowserImage.h"

namespace tizen_browser
{
namespace tools
{

BrowserImage::BrowserImage()
    :id(0)
    ,url()
    ,width(0)
    ,height(0)
    ,dataSize(0)
    ,imageType(ImageTypeNoImage)
    ,imageData(0)
{

}

BrowserImage::~BrowserImage()
{
    switch(imageType){
        case ImageTypeEvasObject:
            //free evas structure
            ;
            break;
        case ImageTypeSerializedEvas:
	case ImageTypeNoImage:
	case ImageTypePNG:
	    break;
	default:
	    break;

    }
}


} /* end of namespace tools */
} /* end of namespace tizen_browser */
