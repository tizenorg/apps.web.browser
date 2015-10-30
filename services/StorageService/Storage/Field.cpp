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

#include <sqlite3.h>

#include "Field.h"
#include "Blob.h"

namespace storage
{

Field::Field()
{
    this->type = SQLITE_NULL;
}

/*private*/Field::Field(const Field &)
{
}
/*private*/Field & Field::operator=(const Field &)
{
    return *this;
}

Field::Field(int sqlInt)
{
    this->sqlInt = sqlInt;
    this->type = SQLITE_INTEGER;
}

Field::Field(double sqlDouble)
{
    this->sqlDouble = sqlDouble;
    this->type = SQLITE_FLOAT;
}

Field::Field(const std::string & sqlText)
{
    this->sqlText = sqlText;
    this->type = SQLITE3_TEXT;
}

Field::Field(std::shared_ptr<tizen_browser::tools::Blob> blob)
{
    this->blob = blob;
    this->type = SQLITE_BLOB;
}

Field::~Field()
{
}

int Field::getInt() const
{
    return this->sqlInt;
}

double Field::getDouble() const
{
    return this->sqlDouble;
}

std::string Field::getString() const
{
    return this->sqlText;
}

const std::shared_ptr<tizen_browser::tools::Blob> Field::getBlob() const
{
    return this->blob;
}

int Field::getType() const
{
    return this->type;
}

}
