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


#ifndef _H_EMBER_LIKE_NEVE_MOTIF
#define _H_EMBER_LIKE_NEVE_MOTIF

#include "mpi/embermpigen.h"

namespace SST {
namespace Ember {

typedef int64_t GraphElem;
typedef double GraphWeight;

struct Edge
{
    GraphElem tail_;
    GraphWeight weight_;
    
    Edge(): tail_(-1), weight_(0.0) {}
    Edge(GraphElem tail_, GraphWeight weight_): tail_(tail_), weight_(weight_) {}
};

class Graph {
    public:
        Graph(GraphElem nv, GraphElem ne): 
            nv_(nv), ne_(ne) 
        {
            edge_indices_   = new GraphElem[nv_+1];
            edge_list_      = new Edge[ne_];
            // vertex_degree_  = new GraphWeight[nv_];
        }

        void set_nedges(GraphElem ne) 
        { 
            ne_ = ne; 
            edge_list_      = new Edge[ne_];
        }
        
        // public variables
        GraphElem *edge_indices_;
        Edge *edge_list_;
        GraphElem nv_, ne_;
};

class EmberLikeNEVEGenerator : public EmberMessagePassingGenerator {

public:
    SST_ELI_REGISTER_SUBCOMPONENT(
        EmberLikeNEVEGenerator,
        "ember",
        "LikeNEVEMotif",
        SST_ELI_ELEMENT_VERSION(1,0,0),
        "Performs a Allreduce operation with type set to float64 and operation SUM",
        SST::Ember::EmberGenerator
    )

    SST_ELI_DOCUMENT_PARAMS(
        {   "arg.iterations",   "Sets the number of allreduce operations to perform",   "10"},
        {   "arg.compute",      "Sets the time spent computing",        "1"},
        {   "arg.count",        "Sets the number of elements to reduce",        "1"},
        {   "arg.doUserFunc",   "Test reduce operation",        "false"},
    )

    SST_ELI_DOCUMENT_STATISTICS(
        { "time-Init", "Time spent in Init event",          "ns",  1},
        { "time-Finalize", "Time spent in Finalize event",  "ns", 1},
        { "time-Rank", "Time spent in Rank event",          "ns", 2},
        { "time-Size", "Time spent in Size event",          "ns", 2},
        { "time-Send", "Time spent in Recv event",          "ns", 1},
        { "time-Recv", "Time spent in Recv event",          "ns", 1},
        { "time-Irecv", "Time spent in Irecv event",        "ns", 2},
        { "time-Isend", "Time spent in Isend event",        "ns", 2},
        { "time-Wait", "Time spent in Wait event",          "ns", 2},
        { "time-Waitall", "Time spent in Waitall event",    "ns", 2},
        { "time-Waitany", "Time spent in Waitany event",    "ns", 2},
        { "time-Compute", "Time spent in Compute event",    "ns", 2},
        { "time-Barrier", "Time spent in Barrier event",    "ns", 2},
        { "time-Alltoallv", "Time spent in Alltoallv event", "ns", 2},
        { "time-Alltoall", "Time spent in Alltoall event",  "ns", 2},
        { "time-Allreduce", "Time spent in Allreduce event", "ns", 2},
        { "time-Reduce", "Time spent in Reduce event",      "ns", 2},
        { "time-Bcast", "Time spent in Bcast event",        "ns", 2},
        { "time-Gettime", "Time spent in Gettime event",    "ns", 2},
        { "time-Commsplit", "Time spent in Commsplit event", "ns", 2},
        { "time-Commcreate", "Time spent in Commcreate event", "ns", 2},
    )

public:
	EmberLikeNEVEGenerator(SST::ComponentId_t, Params& params);
    bool generate( std::queue<EmberEvent*>& evQ);

private:
    Graph *ReadProcessGraph(const char *filename);

    uint32_t m_iterations;
    uint32_t m_count;
    char*    m_sendBuf;
    char*    m_recvBuf;
    int      m_redRoot;
    uint32_t m_loopIndex;
    int message_size;
    SimTime_t last_time;

    std::vector<Edge> targets;
    std::vector<Edge> sources;
    Graph *g;
};

}
}

#endif