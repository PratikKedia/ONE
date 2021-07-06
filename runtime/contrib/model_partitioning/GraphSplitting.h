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

#include <bits/stdc++.h>

// class ModelInfo
// {
// public:
//   ModelInfo(std::string modelfile, const ir::Graph &graph)
//     : _et(std::move(et)), _graph(graph)
//   {
//   }
//   void handleJobBegin(IExecutor *, ir::SubgraphIndex, ir::OperationIndex,
//                       const backend::Backend *) override;
//   void handleJobEnd(IExecutor *, ir::SubgraphIndex, ir::OperationIndex,
//                     const backend::Backend *) override;

//   void handleSubgraphEnd(ir::SubgraphIndex) override { _et->storeOperationsExecTime(); }

// private:
//   std::unique_ptr<util::ITimer> _timer;
//   std::shared_ptr<ExecTime> _et;
//   const ir::Graph &_graph;
// };

class GraphTopology
{
public:
  GraphTopology()//std::string modelfile, std::string tracefile, std::string dagfile)
    //: _et(std::move(et)), _graph(graph)
  {
  }
  void topological_sort();
  void generate_session_graph(int);
  void initial_partition(int);
  std::pair<int, std::vector<std::pair<int,int>>> get_bottleneck_info(int);
  bool partition_move(int, int, int);
  void partition_minmax(int);
  void partition_minmax_multiple(int, int);

  // void handleJobEnd(IExecutor *, ir::SubgraphIndex, ir::OperationIndex,
  //                   const backend::Backend *) override;

  // void handleSubgraphEnd(ir::SubgraphIndex) override { _et->storeOperationsExecTime(); }


  std::vector<int> _weights;
  std::vector<std::vector<int>> _dag;
  std::vector<int> _top_sort;

  std::vector<std::vector<int>> _session_ids;
  std::vector<int> _session_weights;

  std::vector<int> _indx;
  std::vector<std::vector<int>> _session_graph;

};