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
#include "partition_writer.h"
#include <chrono>
#include <iostream>

using namespace std::chrono;

JsonWriter::JsonWriter(std::string modelfile)
  : _stream(), _root(), _dumpfile(modelfile),
    _json_file(_dumpfile + "/partition_map.json", std::ofstream::out),
    _writer(_stream.newStreamWriter())
{
}



// void JsonWriter::add_simple_record(std::uint32_t op_id, std::int64_t time)
// {
//   Json::Value rec;
//   rec["op_id"] = op_id;
//   rec["ts"] = time;
//   _root["traceEvents"].append(rec);
// }

std::vector<int> JsonWriter::read_exectime(){
    std::string execfile = _dumpfile + "execution_timebig.json";
    std::ifstream exectime(execfile, std::ifstream::binary);
    // exectime >> myroot;
    Json::CharReaderBuilder cfg;
    Json::Value myroot;
    Json::Value optimes;
    JSONCPP_STRING errs;
    cfg["collectComments"] = true;

    std::vector<int> times;

    if (!parseFromStream(cfg, exectime, &myroot, &errs))
    {
        std::cout << errs << std::endl;
        return times;
    }

    optimes = myroot["traceEvents"];
    for(auto it = optimes.begin(); it != optimes.end(); ++it){
        auto opt = *it;
        for(auto itr = opt.begin(); itr != opt.end(); ++itr){
            if(itr.key() == "ts"){
                times.push_back((*itr).asInt());
            }
        }

    }


    // std::cout << myroot["traceEvents"].size() <<"\n";
    // int n = myroot["traceEvents"].size();
    // std::vector<int> times(n);
    // for(int i = 0; i<n; i++){
    //     std::cout << myroot["traceEvents"][i];
    //     // times[i] = myroot["traceEvents"][i]["ts"].asInt();
    //     // std::cout << times[i];
    // }
    exectime.close();
    return times;
    
}

std::vector<std::vector<int>> JsonWriter::read_dag(){
    std::string execfile = _dumpfile + "model_dagbig.json";
    Json::Value myroot;
    std::ifstream exectime(execfile, std::ifstream::binary);
    exectime >> myroot;
    int n = myroot["dag"].size();
    std::vector<std::vector<int>> mydag(n, std::vector<int>(n, 0));
    for(int i = 0; i<n; i++){
        for(int j = 0; j<n; j++){
            mydag[i][j] = (myroot["dag"][i][j]).asInt();
            
        }
    }
    exectime.close();
    return mydag;
}

void JsonWriter::add_partition_record(std::vector<int> partition_map, int k)
{
//   Json::Value rec;
//   rec["op_id"] = op_id;
//   rec["ts"] = time;
//   _root["traceEvents"].append(rec);
Json::Value vec(Json::arrayValue);
for(int i = 0; i<partition_map.size(); i++){
    vec.append(Json::Value(partition_map[i]));
}
_root["partition_map"] = vec;
_root["num_partitions"] = k;

}


void JsonWriter::open_file(void)
{
  std::ifstream _json_input("output_trace_" + _dumpfile + ".json", std::ifstream::binary);
  if (!_json_input)
  {
    return;
  }
  Json::CharReaderBuilder cfg;
  JSONCPP_STRING errs;
  cfg["collectComments"] = true;
  if (!parseFromStream(cfg, _json_input, &_root, &errs))
  {
    std::cout << errs << std::endl;
  }
  _json_input.close();
}

void JsonWriter::write_to_file(void) { _writer->write(_root, &_json_file); }

void JsonWriter::write_and_close_file(void)
{
  _writer->write(_root, &_json_file);
  _json_file.close();
}