// Copyright 2009-2023 NTESS. Under the terms
// of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2023, NTESS
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// of the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.
#include <stdlib.h>
#include <fstream>
#include <climits>


#include <sst_config.h>
#include "like_neve.h"

using namespace SST::Ember;

typedef int64_t GraphElem;
typedef double GraphWeight;

static void test(void* a, void* b, int* len, PayloadDataType* ) {

	printf("%s() len=%d\n",__func__,*len);
}

EmberLikeNEVEGenerator::EmberLikeNEVEGenerator(SST::ComponentId_t id,
                                            Params& params) :
	EmberMessagePassingGenerator(id, params, "LikeNEVE"),
    m_loopIndex(0)
{
	m_iterations = (uint32_t) params.find("arg.iterations", 10);
	m_count      = (uint32_t) params.find("arg.count", 1);
	m_redRoot    = (uint32_t) params.find("arg.root", 0);

    memSetBacked();
    m_sendBuf = (char *)memAlloc(1024);
    m_recvBuf = (char *)memAlloc(1024);

    message_size = 1;
    last_time = getCurrentSimCycle();
    // shmemsBuf= (char *)memAlloc(32);
    // shmem_recv= (char *)memAlloc(32);

}

void ReadProcessGraph(char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf(" Error opening file! \n");
        // exit();
    }

    int64_t M_, N_;
    fread(&M_, sizeof(int64_t), 1, file);
    fread(&N_, sizeof(int64_t), 1, file);
    
    size_t tot_bytes=(M_+1)*sizeof(int64_t);
    size_t offset = 2*sizeof(int64_t);
}



Graph *ReadProcessGraph2(char *filename) {
    std::ifstream file;

    file.open(filename, std::ios::in | std::ios::binary); 

    if (!file.is_open()) 
    {
        std::cout << " Error opening file! " << std::endl;
        std::abort();
    }

    GraphElem M_, N_;

    // read the dimensions 
    file.read(reinterpret_cast<char*>(&M_), sizeof(GraphElem));
    file.read(reinterpret_cast<char*>(&N_), sizeof(GraphElem));
    // create local graph
    Graph *g = new Graph(M_, N_);

    uint64_t tot_bytes=(M_+1)*sizeof(GraphElem);
    ptrdiff_t offset = 2*sizeof(GraphElem);

    if (tot_bytes < INT_MAX)
        file.read(reinterpret_cast<char*>(&g->edge_indices_[0]), tot_bytes);
    else 
    {
        int chunk_bytes=INT_MAX;
        uint8_t *curr_pointer = (uint8_t*) &g->edge_indices_[0];
        uint64_t transf_bytes = 0;

        while (transf_bytes < tot_bytes)
        {
            file.read(reinterpret_cast<char*>(&curr_pointer[offset]), chunk_bytes);
            transf_bytes += chunk_bytes;
            offset += chunk_bytes;
            curr_pointer += chunk_bytes;

            if ((tot_bytes - transf_bytes) < INT_MAX)
                chunk_bytes = tot_bytes - transf_bytes;
        } 
    }    

    N_ = g->edge_indices_[M_] - g->edge_indices_[0];
    g->set_nedges(N_);
    tot_bytes = N_*(sizeof(Edge));
    offset = 2*sizeof(GraphElem) + (M_+1)*sizeof(GraphElem) + g->edge_indices_[0]*(sizeof(Edge));
    if (tot_bytes < INT_MAX)
        file.read(reinterpret_cast<char*>(&g->edge_list_[0]), tot_bytes);
    else 
    {
        int chunk_bytes=INT_MAX;
        uint8_t *curr_pointer = (uint8_t*)&g->edge_list_[0];
        uint64_t transf_bytes = 0;

        while (transf_bytes < tot_bytes)
        {
            file.read(reinterpret_cast<char*>(&curr_pointer[offset]), tot_bytes);
            transf_bytes += chunk_bytes;
            offset += chunk_bytes;
            curr_pointer += chunk_bytes;

            if ((tot_bytes - transf_bytes) < INT_MAX)
                chunk_bytes = (tot_bytes - transf_bytes);
        } 
    }   

    file.close();

    for(GraphElem i=1;  i < M_+1; i++)
        g->edge_indices_[i] -= g->edge_indices_[0];   
    g->edge_indices_[0] = 0;

    return g;
}

bool EmberLikeNEVEGenerator::generate( std::queue<EmberEvent*>& evQ) {
    
    if ( 0 == rank() && m_loopIndex > 0) {
        // printf("running iteration %d of %d\n", m_loopIndex, m_iterations);
        // printf("The simulation has been running for %" PRIu64 " cycles.\n", getCurrentSimCycle());
        printf("%d\t\t%d\n", message_size, getCurrentSimCycle() - last_time);
    }
    last_time = getCurrentSimCycle();
    if ( m_loopIndex == m_iterations ) {
        if ( 0 == rank() ) {
            int source = (rank() + size() - 1) % size();
            // printf("Received \"%s\" from process %d.\n", m_recvBuf, source);
        }
        return true;
    }

    if ( 0 == m_loopIndex ) {
        verbose(CALL_INFO, 1, 0, "rank=%d size=%d\n", rank(), size());
    }

    enQ_init( evQ );
    // enQ_malloc( evQ, &m_src, m_nelems * sizeof(TYPE) * 2);


	// char *sbuf = (char *)memAlloc(32);
	// char rbuf[32];
	int source = (rank() + size() - 1) % size();
	int dest = (rank() + 1) % size();
	// sprintf(m_sendBuf, "hello from process %d", rank());
	// printf("sbuf contains %s\n", sbuf);
	SST::UnitAlgebra time = getCoreTimeBase();
	sst_big_num time2 = time.getRoundedValue();
	// printf("time2 is %d\n", time2);
	// printf("The core is counting time in units of %s\n", getCoreTimeBase().toStringBestSI().c_str());
    // The core is counting time in units of 1 ps

    // send(Queue& q, const Hermes::MemAddr& payload, uint32_t count, PayloadDataType dtype, RankID dest, uint32_t tag, Communicator group)
	enQ_send( evQ, m_sendBuf, message_size, CHAR, dest, 0, GroupWorld );
	enQ_recv( evQ, m_recvBuf, message_size, CHAR, source, 0, GroupWorld );
	// printf("Received \"%s\" from process %d.\n", m_recvBuf, source);

    // enQ_compute( evQ, 11000 );
    // enQ_reduce( evQ, m_sendBuf, m_recvBuf, m_count, DOUBLE,
    //                                              MP::SUM, m_redRoot, GroupWorld );
    // if ( ++m_loopIndex == m_iterations ) {
    //     return true;
    // } else {
    //     return false;
    // }
    ++m_loopIndex;
    message_size *= 2;
    return false;
}
