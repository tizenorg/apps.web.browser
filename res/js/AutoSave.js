/*
 * Copyright 2013  Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// KEY,VALUE will be replaced in the _auto_fill_every_form()
var keys = [$KEY];
var values = [$VALUE];

var forms = document.getElementsByTagName('form');
for (var formIdx = 0; formIdx < forms.length; formIdx++) {
    var inputs = forms[formIdx].getElementsByTagName('input');
    for (var inputIdx = 0; inputIdx < inputs.length; inputIdx++) {
        for (var keyIdx = 0; keyIdx < keys.length; keyIdx++) {
            if (inputs[inputIdx].name == keys[keyIdx]) {
                inputs[inputIdx].value = values[keyIdx];
                break;
            }
        }
    }
}

