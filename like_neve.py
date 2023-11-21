import sst
from sst.merlin.base import *
from sst.merlin.endpoint import *
from sst.merlin.interface import *
from sst.merlin.topology import *

from sst.ember import *

def example():
    PlatformDefinition.setCurrentPlatform("firefly-defaults")
    # compA = sst.Component("statTestA", "ember.ExampleMotif")
    # compA.enableAllStatistics({"type" : "sst.AccumulatorStatistic", 
    #                        "rate" : "500ms" })




    ### Setup the topology
    topo = topoDragonFly()
    topo.hosts_per_router = 4
    topo.routers_per_group = 1
    topo.intergroup_links = 1
    topo.num_groups = 1
    topo.algorithm = ["minimal"]

    # Set up the routers
    router = hr_router()
    router.link_bw = "4GB/s"
    router.flit_size = "8B"
    router.xbar_bw = "6GB/s"
    router.input_latency = "20ns"
    router.output_latency = "20ns"
    router.input_buf_size = "4kB"
    router.output_buf_size = "4kB"
    router.num_vns = 1
    router.xbar_arb = "merlin.xbar_arb_lru"

    topo.router = router
    topo.link_latency = "20ns"

    ### set up the endpoint
    networkif = ReorderLinkControl()
    networkif.link_bw = "4GB/s"
    networkif.input_buf_size = "8kB"
    networkif.output_buf_size = "8kB"

    ep = EmberMPIJob(0,topo.getNumNodes())
    ep.network_interface = networkif
    ep.addMotif("Init")
    ep.addMotif("LikeOSU")
    # ep.addMotif("Allgather")
    ep.addMotif("Fini")
    ep.nic.nic2host_lat= "100ns"

    system = System()
    system.setTopology(topo)
    system.allocateNodes(ep,"linear")

    system.build()


    # sst.setStatisticLoadLevel(7)
    # sst.Component("c0", "ember")
    # sst.enableAllStatisticsForComponentName("ember.ExampleMotif")
    # params = { "rate" : "500ns" } # Report statistics every 500ns
    # sst.enableAllStatisticsForAllComponents(params)
    # sst.enableStatisticForComponentName("c0", "time-Send", params, True)
    # sst.setStatisticOutput("sst.statOutputConsole")
    # params = { "filepath" : "sim_output.csv", "separator" : ", " }
    # statOutput.addParams(params)

if __name__ == "__main__":
    example()