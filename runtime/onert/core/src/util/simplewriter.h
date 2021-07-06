/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd. All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @file  tracer.h
 * @brief This file describes API for dumping profiling in Simple format
 */

#include <fstream>
// #include <json.h>
#include "../../../../runtime/3rdparty/jsoncpp/json/json.h"
#include <memory>

#define TRACE_INTERVAL 10
class JsonWriter
{
private:
  Json::StreamWriterBuilder _stream;
  Json::Value _root;
  std::string _dumpfile;
  std::ofstream _json_file; // output trace file
  std::unique_ptr<Json::StreamWriter> _writer;

public:
  JsonWriter(std::string dumpfile);

  void add_simple_record(std::uint32_t op_id, std::int64_t time);

  void open_file(void);
  void write_to_file(void);
  void write_and_close_file(void);
};