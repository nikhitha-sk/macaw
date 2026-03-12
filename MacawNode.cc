
#include "MacawNode.h"

Define_Module(MacawNode);

void MacawNode::initialize()
{
    if (getIndex() == sender) {
        // Find which out[] index leads to receiver
        int outIndex = (receiver > sender) ? receiver - 1 : receiver;
        cMessage *rts = new cMessage("RTS");
        scheduleAt(simTime() + 1, rts);
        // Send RTS to receiver after 1s
        cMessage *rtsNet = new cMessage("RTS");
        sendDelayed(rtsNet, 1.0, "out", outIndex);
        state = WAIT_CTS;
    }
}

void MacawNode::handleMessage(cMessage *msg)
{
    EV << "Node " << getIndex() << " received " << msg->getName() << endl;

    int n = gateSize("out");

    if (strcmp(msg->getName(), "RTS") == 0) {
        if (getIndex() == receiver && state == IDLE) {
            bubble("RTS received, sending CTS");
            cMessage *cts = new cMessage("CTS");
            send(cts, "out", sender); // send CTS to sender only
            state = WAIT_DS;
        } else if (getIndex() != sender && getIndex() != receiver) {
            bubble("Neighbor heard RTS, backing off");
            state = BUSY;
        }
    }
    else if (strcmp(msg->getName(), "CTS") == 0) {
        if (getIndex() == sender && state == WAIT_CTS) {
            bubble("CTS received, sending DS");
            // DS is broadcast to both neighbors (out[0] and out[1])
            for (int i = 0; i < n; ++i) {
                cMessage *ds = new cMessage("DS");
                send(ds, "out", i);
            }
            state = WAIT_ACK;
        } else if (getIndex() != sender && getIndex() != receiver) {
            bubble("Neighbor heard CTS, staying quiet");
            state = BUSY;
        }
    }
    else if (strcmp(msg->getName(), "DS") == 0) {
        if (getIndex() == sender) {
            bubble("Sender heard DS, sending DATA");
            // Find which out[] index leads to receiver
            int outIndex = (receiver > sender) ? receiver - 1 : receiver;
            cMessage *data = new cMessage("DATA");
            send(data, "out", outIndex);
        } else if (getIndex() == receiver) {
            bubble("Receiver heard DS, waiting for DATA");
        } else {
            bubble("Neighbor heard DS, staying quiet");
            state = BUSY;
        }
    }
    else if (strcmp(msg->getName(), "DATA") == 0) {
        if (getIndex() == receiver && state == WAIT_DS) {
            bubble("DATA received, sending ACK");
            // Find which out[] index leads to sender
            int outIndex = (sender > receiver) ? sender - 1 : sender;
            cMessage *ack = new cMessage("ACK");
            send(ack, "out", outIndex);
            state = IDLE;
        }
    }
    else if (strcmp(msg->getName(), "ACK") == 0) {
        if (getIndex() == sender && state == WAIT_ACK) {
            bubble("ACK received, transmission complete");
            state = IDLE;
        }
    }

    delete msg;
}