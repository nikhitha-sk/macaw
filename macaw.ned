// =============================================================
//  MACAW Protocol Simulation -- OMNeT++ NED File
//  File: macaw.ned
//  Topology: A(Sender) <-> B(Receiver), neighbors C,E near A, D,F near B
//  Protocol order: RTS -> CTS -> DS -> DATA -> ACK
// =============================================================

simple MacawNode
{
    parameters:
        string  role        = default("none");
        double  navDuration @unit(s) = default(0.5s);
        int     dataBytes   = default(1000);
        @display("i=device/laptop");

    gates:
        inout wireless[];
}

channel WirelessChannel extends ned.DatarateChannel
{
    // datarate, delay, ber are inherited from ned.DatarateChannel
    // values are set in omnetpp.ini
}

network MacawNetwork
{
    parameters:
        @display("bgb=640,480");

    submodules:

        A: MacawNode {
            parameters:
                role = "sender";
                @display("p=155,240;i=device/laptop;is=l;t=A Sender");
        }

        B: MacawNode {
            parameters:
                role = "receiver";
                @display("p=465,240;i=device/laptop;is=l;t=B Receiver");
        }

        C: MacawNode {
            parameters:
                role = "neighbor";
                @display("p=50,390;i=device/pc;t=C Neighbor");
        }

        E: MacawNode {
            parameters:
                role = "neighbor";
                @display("p=50,80;i=device/pc;t=E Neighbor");
        }

        D: MacawNode {
            parameters:
                role = "neighbor";
                @display("p=570,80;i=device/pc;t=D Neighbor");
        }

        F: MacawNode {
            parameters:
                role = "neighbor";
                @display("p=570,390;i=device/pc;t=F Neighbor");
        }

    connections allowunconnected:

        A.wireless++ <--> WirelessChannel <--> B.wireless++;
        A.wireless++ <--> WirelessChannel <--> C.wireless++;
        A.wireless++ <--> WirelessChannel <--> E.wireless++;
        B.wireless++ <--> WirelessChannel <--> D.wireless++;
        B.wireless++ <--> WirelessChannel <--> F.wireless++;
}
