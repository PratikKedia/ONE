/*
 * Copyright (c) 2019 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include "nnfw.h"
#include "nnfw_experimental.h"
#include "../../../../contrib/model_partition/GraphSplitting.h"
#include <vector>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <chrono>

clock_t begin_time, end_time;

std::chrono::_V2::system_clock::time_point t_start, t_end, t_mid, t_mid2;

uint64_t num_elems(const nnfw_tensorinfo *ti)
{
  uint64_t n = 1;
  for (uint32_t i = 0; i < ti->rank; ++i)
  {
    n *= ti->dims[i];
  }
  return n;
}

void* prodfun(void* session){
  std::vector<void *> inputs;
  std::vector<uint32_t> lengths;
  int N = 10;
  // std::cout <<"Here2\n";
  float *temp = (float *)malloc(299 * 299 * 3 * sizeof(float));

  for(int cnt = 0; cnt< 299 * 299 * 3 ; cnt++){
    temp[cnt] = (rand()/(double)RAND_MAX);
  }

  inputs.push_back((void *)temp);
  lengths.push_back(299 * 299 * 3 * sizeof(float));

  // for(int i = 0; i<299; i++){
  //   std::cout << ((float *)inputs[0])[i] << " ";
  // }
  begin_time = clock();
  t_start = std::chrono::high_resolution_clock::now();

  for (int i = 1; i <= N; i++) {
      nnfw_push_pipeline_input((nnfw_session *)session, (void *)&inputs, (void *)&lengths);
      printf("%dth push_pipeline\n", i);
      usleep(10000);
  }
  std::cout << float(clock() - begin_time)/CLOCKS_PER_SEC << std::endl;
  t_mid = std::chrono::high_resolution_clock::now();
  free(inputs[0]);
  inputs.clear();
  lengths.clear();
  nnfw_push_pipeline_input((nnfw_session *)session, (void *)&inputs, (void *)&lengths); // empty input
  return NULL;
}


void* confun(void* session){
  std::vector<void *> outputs;
  int cnt = 0;
  while ((nnfw_pop_pipeline_output((nnfw_session *)session, (void *)&outputs) == NNFW_STATUS_NO_ERROR))
  {
    printf("%dth pop pipeline\n", ++cnt);
    t_mid2 = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration<double, std::milli>(t_mid2-t_mid).count() << std::endl;
    t_mid = t_mid2;
    for (uint32_t i = 0; i < outputs.size(); i++) {
    free(outputs[i]);
    }
    outputs.clear();
  }
  t_end = std::chrono::high_resolution_clock::now();
  end_time = clock();
  return NULL;
}



int main(const int argc, char **argv)
{
  nnfw_session *session = nullptr;
  pthread_t producer, consumer;

  nnfw_create_session(&session);

  nnfw_load_model_from_file(session, argv[1]);

  nnfw_set_available_backends(session, "cpu");

  // Compile model
  std::string partitionpath = argv[1];
  partitionpath = partitionpath +  "/parition_map.json";
  // nnfw_prepare_pipeline(session, "./inception_v3/partition_map.json");
  nnfw_prepare_pipeline(session, partitionpath.c_str());  

  pthread_create(&producer, NULL, &prodfun, (void *)session);

  pthread_create(&consumer, NULL, &confun, (void *)session);
 
  pthread_join(consumer, NULL);
  pthread_join(producer, NULL);

  double elapsed_time_ms = std::chrono::duration<double, std::milli>(t_end-t_start).count();
  
  std::cout << float(end_time - begin_time)/CLOCKS_PER_SEC << std::endl;
  
  std::cout << elapsed_time_ms << std::endl;
  // TODO: Please print or compare the output value in your way.

  nnfw_close_session(session);

  std::cout << "nnpackage multisession " << argv[1] << " runs successfully." << std::endl;
  return 0;
}
