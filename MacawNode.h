#ifndef __MACAWNODE_H
#define __MACAWNODE_H

#include <omnetpp.h>

using namespace omnetpp;

enum State {IDLE, WAIT_CTS, WAIT_DS, WAIT_ACK, BUSY};

class MacawNode : public cSimpleModule
{
  protected:
    State state = IDLE;
    int sender = 0;
    int receiver = 1;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif