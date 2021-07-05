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
#include <vector>
#include <iostream>
#include <thread>
#include <pthread.h>
#include <unistd.h>

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
  int N = 100;

  float *temp = (float *)malloc(299 * 299 * 3 * sizeof(float));
  inputs.push_back((void *)temp);
  lengths.push_back(299 * 299 * 3 * sizeof(float));
  //GENERATE RANDOM CODE 

  for (int i = 1; i <= N; i++) {
      nnfw_push_pipeline_input((nnfw_session *)session, (void *)&inputs, (void *)&lengths);
      printf("%dth push_pipeline\n", i);
      usleep(10000);
  }

  free(inputs[0]);
  inputs.clear();
  lengths.clear();
  nnfw_push_pipeline_input((nnfw_session *)session, (void *)&inputs, (void *)&lengths); // empty input
 
}


void* confun(void* session){
  std::vector<void *> outputs;
  int cnt = 0;
  while ((nnfw_pop_pipeline_output((nnfw_session *)session, (void *)&outputs) == NNFW_STATUS_NO_ERROR))
  {
    printf("%dth pop pipeline\n", ++cnt);
    for (uint32_t i = 0; i < outputs.size(); i++) {
    free(outputs[i]);
    }
    outputs.clear();
  }
}



int main(const int argc, char **argv)
{
  nnfw_session *session = nullptr;
  pthread_t producer, consumer;

  nnfw_create_session(&session);

  // Loading nnpackage
  nnfw_load_model_from_file(session, argv[1]);

  // Use acl_neon backend for CONV_2D and acl_cl for otherwise.
  // Note that defalut backend is acl_cl
  //nnfw_set_op_backend(session, "CONV_2D", "cpu");

  nnfw_set_available_backends(session, "cpu");

  // Compile model
  nnfw_prepare_pipeline(session, "inception_v3/partition_map.json");

  pthread_create(&producer, NULL, &prodfun, (void *)session);

  pthread_create(&consumer, NULL, &confun, (void *)session);
 
  pthread_join(consumer, NULL);
  pthread_join(producer, NULL);
  
  pthread_exit(NULL);

  // TODO: Please print or compare the output value in your way.

  nnfw_close_session(session);

  std::cout << "nnpackage multisession " << argv[1] << " runs successfully." << std::endl;
  return 0;
}
