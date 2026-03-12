#ifndef __MACAWNODE_H
#define __MACAWNODE_H

#include <omnetpp.h>

using namespace omnetpp;

class MacawNode : public cSimpleModule
{
  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
};

#endif