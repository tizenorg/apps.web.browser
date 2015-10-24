/*
 * browser
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Flora License, Version 1.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
try {
     function getAppCapable() {
         var capable= new Boolean();
         var metaTags = document.getElementsByTagName("meta");
         for (var i = 0; i < metaTags.length; i++) {
             if (metaTags[i].name.toLowerCase() == "apple-mobile-web-app-capable") {
                if (metaTags[i].content.toLowerCase() == "yes") {

                    capable = true;
                     break;
                 }
             }
         }
         var jsonString = JSON.stringify(capable);
         return jsonString;
     }
     getAppCapable();
} catch(e) {}

