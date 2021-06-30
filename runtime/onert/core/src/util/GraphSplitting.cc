/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd. All Rights Reserved
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

#include <iostream>
#include <bits/stdc++.h>
#include "util/GraphSplitting.h"


void GraphTopology::topological_sort()
{
    auto adj(_dag);
    int n = adj.size();
    std::vector<int> top;
    std::vector<bool> marked(n, false);
    while (top.size()<n){
        for(int i = 0; i<n; i++){
            if((accumulate(adj[i].begin(), adj[i].end(), 0) == 0) && !marked[i]){
                top.push_back(i);
                marked[i] = true;
                for(int j = 0; j<n; j++){
                    adj[j][i] = 0;
                }
                continue;
            }
        }
    }
    _top_sort = top;
    // for(auto i : top){
    //     std::cout << i << " ";
    // }

}


void GraphTopology::generate_session_graph(int k){
    int n = _top_sort.size();
    std::vector<std::vector<int>> my_session_graph(n, std::vector<int>(n, 0));
    for(int i = 0; i<n-1; i++){
        for (int j = i+1; j < n; j++)
        {
            //FEEEL  PROBLEM
            //DISCUSS
            int idx1 = -1, idx2 = -1;
            if(_dag[i][j] == 1){
                int cnt = 0;
                for(int itr = 0; itr<k; itr++){
                    if(std::find(_session_ids[itr].begin(), _session_ids[itr].end(), i) != _session_ids[itr].end()){
                        idx1 = itr;
                        cnt++;
                    }
                    if(std::find(_session_ids[itr].begin(), _session_ids[itr].end(), j) != _session_ids[itr].end()){
                        idx2 = itr;
                        cnt++;
                    }
                    if(cnt>= 2) break;
                }
                if(idx1 == -1 || idx2 == -1) exit(1);
                if(idx1 != idx2){
                    _session_graph[idx1][idx2] = 1;
                }
            }
        }
        
    }
    return;
}

void GraphTopology::initial_partition(int k)
{
    int n = _top_sort.size();
    std::vector<int> sorted_vwgt, c_sorted_vw, pivot;
    int sum1 = 0;
    for(int i = 0; i<n; i++){
        sorted_vwgt.push_back(_weights[_top_sort[i]]);
        sum1 += _weights[_top_sort[i]];
        c_sorted_vw.push_back(sum1);
    }

    for (int i = 1; i < k; i++){
        pivot.push_back(int((i*c_sorted_vw[n-1])/k));
    }

    _indx = {0};
    for (int i = 0; i < k-1; i++){
        int min1 = INT64_MAX;
        int temp, minind;
        for (int j = 0; j < n; j++){
            temp = abs(c_sorted_vw[j] - pivot[i]);
            if(temp < min1){
                minind = j;
            }
            _indx.push_back(minind);
        } 
    }

    std::vector<int> sesvec;
    int tempsum;
    _session_ids = {};
    _session_weights = {};
    for (int i = 0; i < k-1; i++)
    {
        sesvec = {};
        tempsum = 0;
        for (int j = _indx[i]; j < _indx[i+1]; j++)
        {
            sesvec.push_back(_top_sort[j]);
            tempsum+=sorted_vwgt[j];
        }
        _session_ids.push_back(sesvec);
        _session_weights.push_back(tempsum);  
    }

    sesvec = {};
    tempsum = 0;
    for (int j = _indx[k-1]; j < n; j++)
    {
        sesvec.push_back(_top_sort[j]);
        tempsum+=sorted_vwgt[j];
    }
    _session_ids.push_back(sesvec);
    _session_weights.push_back(tempsum);

    generate_session_graph(k);

}

std::pair<int, std::vector<std::pair<int,int>>> GraphTopology::get_bottleneck_info(int k){
    int maxval = 0;
    int ret_id = -1;
    for(int i =0; i<k; i++){
        if (maxval < _session_weights[i]){
            maxval = _session_weights[i];
            ret_id = i;
        }
    }
    std::vector<std::pair<int, int>> neighbours = {};
    for(int i = 0; i<k; i++){
        if(_session_graph[ret_id][i] == 1 || _session_graph[i][ret_id]==1){
            neighbours.push_back(std::make_pair(_session_weights[i], i));
        }
    }
    std::sort(neighbours.begin(), neighbours.end());
    return std::make_pair(ret_id, neighbours);
}



bool GraphTopology::partition_move(int bot_id, int neighbour, int k){
    bool forward_direction, found_node, move_success = false, improvement = true;
    int n = _dag.size(), cnt = 0;

    if (_session_graph[bot_id][neighbour] ==1)
        forward_direction = true;
    else if(_session_graph[neighbour][bot_id] ==1)
        forward_direction = false;
    else
        return false;

    int maxval = std::max(_session_weights[bot_id], _session_weights[neighbour]);
    
    std::vector<int> s1, s2;
    while(improvement){
        s1 = _session_ids[bot_id];
        s2 = _session_ids[neighbour];

        //SESSION EDGES CODE STARTS
        std::unordered_map<int, std::vector<int>> sdict;
        if(forward_direction){
            std::sort(s1.begin(), s1.end());
            for(auto t: s1){
                std::vector<int> tvec, tmp_s;
                for(int i = 0; i<n; i++){
                    if(_dag[t][i]==1){
                        tvec.push_back(i);
                    }
                }
                std::set_difference(tvec.begin(), tvec.end(), s1.begin(), s1.end(), tmp_s.begin());
                if(tmp_s.size() > 0){
                    sdict[t] = tmp_s;
                }
            }
        }
        else{
            std::sort(s2.begin(), s2.end());
            for(auto t: s2){
                std::vector<int> tvec, tmp_s;
                for(int i = 0; i<n; i++){
                    if(_dag[t][i]==1){
                        tvec.push_back(i);
                    }
                }
                std::set_intersection(tvec.begin(), tvec.end(), s2.begin(), s2.end(), tmp_s.begin());
                if(tmp_s.size() > 0){
                    for(auto key: tmp_s)
                        sdict[key] = {t};
                }
            }
        }
        //SESSION EDGES CODE ENDS
        std::unordered_map<int, bool> marked;
        found_node = false;
        cnt = 0;
        auto itr = sdict.begin();
        while(!found_node && cnt < sdict.size()){
            int rnd_key = itr->first;
            marked[rnd_key] = true;
            found_node = true;
            if(forward_direction){
                //CHANGE RANGE TO SESSION1 to SESSION2
                for(int i = 0; i<neighbour; i++){
                    std::vector<int> tvec, tmp_s;
                    for(int j = 0; j<n; j++){
                        if(_dag[rnd_key][j]==1){
                            tvec.push_back(j);
                        }
                    }
                    std::sort(_session_ids[i].begin(), _session_ids[i].end());
                    std::set_difference(tvec.begin(), tvec.end(), _session_ids[i].begin(), _session_ids[i].end(), tmp_s.begin());
                    if(tmp_s.size() > 0){
                        found_node = false;
                        cnt++;
                        break;
                    }
                }
            }
            else{
                //CHANGE RANGE TO SESSION1 to SESSION2
                for(int i = neighbour+1; i<k; i++){
                    std::vector<int> tvec, tmp_s;
                    for(int j = 0; j<n; j++){
                        if(_dag[j][rnd_key]==1){
                            tvec.push_back(j);
                        }
                    }
                    std::sort(_session_ids[i].begin(), _session_ids[i].end());
                    std::set_difference(tvec.begin(), tvec.end(), _session_ids[i].begin(), _session_ids[i].end(), tmp_s.begin());
                    if(tmp_s.size() > 0){
                        found_node = false;
                        cnt++;
                        break;
                    }
                }
            }

            if(found_node){
                int new_maxval = std::max(_session_weights[bot_id] - _weights[rnd_key], _session_weights[neighbour]+ _weights[rnd_key]);
                if(new_maxval < maxval){
                    
                }
            }
            itr++;
            
        }





    }
    
    return true;
}
