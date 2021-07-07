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
// #include <bits/stdc++.h>
#include <vector>
#include "GraphSplitting.h"
#include "partition_writer.h"

void printv(std::vector<int> v){
    for(int i = 0; i<v.size(); i++){
        std::cout << v[i] << " ";
    }
    std::cout << std::endl;
}

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
//CHAGE ALERT
    std::reverse(top.begin(), top.end());
    _top_sort = top;
    // for(auto i : top){
    //     std::cout << i << " ";
    // }

}


void GraphTopology::generate_session_graph(int k){
    int n = _top_sort.size();
    std::vector<std::vector<int>> my_session_graph(k, std::vector<int>(k, 0));
    for(int i = 0; i<n-1; i++){
        for (int j = i+1; j < n; j++)
        {
            //FEEEL  PROBLEM
            //DISCUSS loop
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
                    my_session_graph[idx1][idx2] = 1;
                }
            }
        }
        
    }
    _session_graph = my_session_graph;
    // printv(_session_graph[0]);
    // printv(_session_graph[1]);
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
        int min1 = INT_MAX;
        int temp, minind;
        for (int j = 0; j < n; j++){
            temp = abs(c_sorted_vw[j] - pivot[i]);
            if(temp < min1){
                minind = j;
                min1 = temp;
            }
        } 
        _indx.push_back(minind);
    }
    // printv(_indx);
    // printv(sorted_vwgt);
    // printv(c_sorted_vw);
    // printv(pivot);

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
    // printv(_session_weights);
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
            neighbours.push_back(std::make_pair(i, _session_weights[i]));
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
    
    // std::cout << forward_direction << maxval <<n<<"\n";

    std::vector<int> s1, s2;
    while(improvement){
        s1 = _session_ids[bot_id];
        s2 = _session_ids[neighbour];

        //DISCUSS USE OF THIS
        //SESSION EDGES CODE STARTS
        std::unordered_map<int, std::vector<int>> sdict;
        if(forward_direction){
            // std::sort(s1.begin(), s1.end());
            std::sort(s2.begin(), s2.end()); //NEW INCLUDED
            for(auto t: s1){
                std::vector<int> tvec, tmp_s(n), tmp2;
                for(int i = 0; i<n; i++){
                    if(_dag[t][i]==1){
                        tvec.push_back(i);
                    }
                }
                //CHANGE: Include intersection
                // std::set_difference(tvec.begin(), tvec.end(), s1.begin(), s1.end(), tmp_s.begin());
                auto ls = std::set_intersection(tvec.begin(), tvec.end(), s2.begin(), s2.end(), tmp_s.begin());
                if((ls - tmp_s.begin()) > 0){
                    for(auto iter = tmp_s.begin(); iter != ls; iter++){
                        tmp2.push_back(*iter);
                        // std::cout << *iter;
                    }
                    sdict[t] = tmp2;
                }
            }
        }
        else{
            std::sort(s1.begin(), s1.end());
            for(auto t: s2){
                std::vector<int> tvec, tmp_s(n);
                for(int i = 0; i<n; i++){
                    if(_dag[t][i]==1){
                        tvec.push_back(i);
                    }
                }
                // std::cout << "C3 ";
                // printv(tvec);
                auto ls  = std::set_intersection(tvec.begin(), tvec.end(), s1.begin(), s1.end(), tmp_s.begin());
                if((ls - tmp_s.begin()) > 0){
                    //PROBLEMATIC _ CHECK
                    for(auto iter = tmp_s.begin(); iter != ls; iter++){
                        sdict[*iter] = {t};
                        // std::cout << *iter;
                    }
                }
                // printv(tmp_s);
            }
        }
        //SESSION EDGES CODE ENDS
        //DISCUSS - NEED FOR MARKED
        // std::unordered_map<int, bool> marked;

        // std::cout << "C1 \n";

        found_node = false;
        cnt = 0;
        auto itr = sdict.begin();

        //DISCUSS - INCLUDE RANDOMNESS

        while(!found_node && cnt < sdict.size()){
            int rnd_key = itr->first;
            // marked[rnd_key] = true;

            // std::cout << rnd_key <<"\n";
            // std::cout << sdict.size() <<"\n";
            
            found_node = true;
            if(forward_direction){
                //CHANGED RANGE TO SESSION1 to SESSION2
                for(int i = bot_id; i<neighbour; i++){
                    std::vector<int> tvec, tmp_s(n);
                    for(int j = 0; j<n; j++){
                        if(_dag[rnd_key][j]==1){
                            tvec.push_back(j);
                        }
                    }
                    // printv(tvec);
                    std::sort(_session_ids[i].begin(), _session_ids[i].end());
                    auto ls = std::set_intersection(tvec.begin(), tvec.end(), _session_ids[i].begin(), _session_ids[i].end(), tmp_s.begin());
                    if((ls - tmp_s.begin()) > 0){
                        found_node = false;
                        cnt++; itr++;
                        // std::cout << "C2 \n";
                        break;
                    }
                }
            }
            else{
                //CHANGED RANGE TO SESSION2 to SESSION1
                for(int i = neighbour+1; i<=bot_id; i++){
                    std::vector<int> tvec, tmp_s(n);
                    for(int j = 0; j<n; j++){
                        if(_dag[j][rnd_key]==1){
                            tvec.push_back(j);
                        }
                    }
                    // printv(tvec);
                    std::sort(_session_ids[i].begin(), _session_ids[i].end());
                    auto ls = std::set_intersection(tvec.begin(), tvec.end(), _session_ids[i].begin(), _session_ids[i].end(), tmp_s.begin());
                    if((ls - tmp_s.begin()) > 0){
                        found_node = false;
                        cnt++; itr++;
                        break;
                    }
                }
            }

            if(found_node){
                // std::cout << "C3 \n";
                cnt++; itr++;
                int new_maxval = std::max(_session_weights[bot_id] - _weights[rnd_key], _session_weights[neighbour]+ _weights[rnd_key]);
                if(new_maxval < maxval){
                    // if type(sdict[rnd_key]) is list:
                    //         rnd_val = np.random.choice(sdict[rnd_key])
                    //     else:
                    //         rnd_val = sdict[rnd_key]
                    //     if forward_direction == True:
                    //         if np.where(s2 == rnd_val)[0].size > 0:
                    //             s2 = np.insert(s2, np.where(s2 == rnd_val)[0], rnd_key)
                    //         else:
                    //             s2 = np.insert(s2, 0, rnd_key)
                    //     else:
                    //         if np.where(s2 == sdict[rnd_key])[0].size > 0:
                    //             s2 = np.insert(s2,
                    //                            np.where(s2 == sdict[rnd_key])[0] + 1,
                    //                            rnd_key)
                    //         else:
                    //             s2 = np.insert(s2, len(s2), rnd_key)
                    //DISCUSS: NEED FOR SDICT
                    s2.push_back(rnd_key);
                    s1.erase(std::find(s1.begin(), s1.end(), rnd_key));
                    _session_ids[bot_id] = s1;
                    _session_ids[neighbour] = s2;
                    _session_weights[bot_id] -= _weights[rnd_key];
                    _session_weights[neighbour] += _weights[rnd_key];
                    maxval = new_maxval;
                    generate_session_graph(k);
                    move_success = true;
                    // std::cout << "C3" << maxval<< " \n";

                }
                else{
                    improvement = false;
                }
            }
            else{
                improvement = false;
            }
            // itr++;            
        }
    }
    
    return move_success;
}

void GraphTopology::partition_minmax(int k){
    bool improvement = true;
    while(improvement){
        improvement = false;
        auto bot_info = get_bottleneck_info(k);


        // std::cout << bot_info.first <<"\n";
        
        
        int bottleneck_id = bot_info.first;
        std::vector<std::pair<int, int>> neighbour_list = bot_info.second;
        
        
        // std::cout << neighbour_list[0].first <<"\n";
        // std::cout << neighbour_list[0].second <<"\n";
        
        for(int i = 0; i< neighbour_list.size(); i++){
            bool ret_success = partition_move(bottleneck_id, neighbour_list[i].first, k);
            // std::cout << "C pm \n";
            if(ret_success){
                improvement = true;
                std::cout << "C partition move success \n";
                break;
            }
        }
    }
    return;
}

void GraphTopology::partition_minmax_multiple(int k, int nruns){

    // std::cout << "Here";
    JsonReadWrite* json_in = new JsonReadWrite("../trial/");
    _weights =  json_in->read_exectime();
    _dag = json_in->read_dag();

    int minval = INT_MAX;
    std::vector<std::vector<int>> my_session_ids;
    std::vector<int> my_session_weights;
    for (int i = 0; i < nruns; i++)
    {
        topological_sort();
        initial_partition(k);
        partition_minmax(k);
        int maxval = *std::max_element(_session_weights.begin(), _session_weights.end());
        if(maxval < minval){
            minval = maxval;
            // std::cout << minval << "lol\n";
            my_session_ids = _session_ids;
            my_session_weights = _session_weights;
        }
    }
    
    std::vector<int> partition_map(_top_sort.size(), 0);
    for(int i =0; i<k; i++){
        for(int op_idx: my_session_ids[i]){
            partition_map[op_idx] = i;
        }
    }

    JsonReadWrite* json_out = new JsonReadWrite("../trial/");
    json_out->add_partition_record(partition_map, k);
    json_out->write_and_close_file();

    for(int i:partition_map){
        std::cout << i << " "; 
    }
    std::cout << "\n";
    for(int i:my_session_weights){
        std::cout << i << " ";
    }
    std::cout << "\n";

}

int main(){
    return 0;
}
