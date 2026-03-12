#include "MacawNode.h"

Define_Module(MacawNode);

void MacawNode::initialize()
{
    if (getIndex() == 0) {
        scheduleAt(simTime() + 1, new cMessage("RTS"));
    }
}

void MacawNode::handleMessage(cMessage *msg)
{
    EV << "Node " << getIndex() << " received " << msg->getName() << endl;

    if (strcmp(msg->getName(), "RTS") == 0) {
        bubble("RTS received");

        cMessage *cts = new cMessage("CTS");
        send(cts, "out");
    }
    else if (strcmp(msg->getName(), "CTS") == 0) {
        bubble("CTS received");

        cMessage *ds = new cMessage("DS");
        send(ds, "out");
    }
    else if (strcmp(msg->getName(), "DS") == 0) {
        bubble("DS received");

        cMessage *data = new cMessage("DATA");
        send(data, "out");
    }
    else if (strcmp(msg->getName(), "DATA") == 0) {
        bubble("DATA received");

        cMessage *ack = new cMessage("ACK");
        send(ack, "out");
    }
    else if (strcmp(msg->getName(), "ACK") == 0) {
        bubble("ACK received");
    }

    delete msg;
}