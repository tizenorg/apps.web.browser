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
     function getAppIconSizes() {
         var sizes = new Array();
         var linkTags = document.getElementsByTagName("link");
         for (var i = 0; i < linkTags.length; i++) {
             if (linkTags[i].rel.toLowerCase() in {"apple-touch-icon":1, "apple-touch-icon-precomposed":1}) {
                 if (!linkTags[i].href)
                     continue;
                 sizes.push(linkTags[i].sizes.value);
             }
         }
         var jsonString = JSON.stringify(sizes);
         return jsonString;
     }
     getAppIconSizes();
} catch(e) {}

