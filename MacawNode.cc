// =============================================================
//  MACAW Protocol Simulation — OMNeT++ C++ Source
//  File: MacawNode.cc
//  Protocol order: RTS → CTS → DS → DATA → ACK
//  Each step is logged to the OMNeT++ event log (Sequence Chart)
// =============================================================

#include <omnetpp.h>
using namespace omnetpp;

// ── Frame-type constants ──────────────────────────────────────
enum MacawFrameType { FRAME_RTS=0, FRAME_CTS, FRAME_DS, FRAME_DATA, FRAME_ACK };
static const char* FRAME_NAMES[] = {"RTS","CTS","DS","DATA","ACK"};

// ── Frame colours for Qtenv animation ────────────────────────
// We set a "kind" field that maps to a colour in omnetpp.ini
static const int KIND_RTS  = 1;
static const int KIND_CTS  = 2;
static const int KIND_DS   = 3;
static const int KIND_DATA = 4;
static const int KIND_ACK  = 5;

// ── Self-message kinds (timers) ───────────────────────────────
static const int TIMER_SEND_RTS  = 100;
static const int TIMER_SEND_DS   = 101;
static const int TIMER_SEND_DATA = 102;
static const int TIMER_SEND_CTS  = 103;
static const int TIMER_SEND_ACK  = 104;
static const int TIMER_NAV_CLEAR = 105;

// ─────────────────────────────────────────────────────────────
class MacawNode : public cSimpleModule
{
protected:
    // Parameters
    std::string role;
    double      navDuration;
    int         dataBytes;

    // State
    bool        navActive  = false;
    bool        waitingCTS = false;
    bool        waitingACK = false;

    // Statistics
    cOutVector  txVector, rxVector;

    void initialize() override;
    void handleMessage(cMessage *msg) override;
    void finish() override;

private:
    // Senders
    void sendRTS();
    void sendCTS();
    void sendDS();
    void sendDATA();
    void sendACK();

    // Broadcast a frame to all connected gates
    void broadcastFrame(MacawFrameType type, int bytes, int kind);

    // Set node display label / colour in Qtenv
    void setDisplay(const char* text, const char* bgCol);

    // Print step banner to EV log
    void logStep(int step, const char* desc, const char* dir);
};

Define_Module(MacawNode);

// =============================================================
void MacawNode::initialize()
{
    role        = par("role").stringValue();
    navDuration = par("navDuration").doubleValue();
    dataBytes   = par("dataBytes");

    txVector.setName("TX-bytes");
    rxVector.setName("RX-bytes");

    EV << "[" << getName() << "] role=" << role << "  initialized\n";

    if (role == "sender")
    {
        // A fires the first RTS after a short warm-up
        cMessage *timer = new cMessage("send-RTS", TIMER_SEND_RTS);
        scheduleAt(1.0, timer);
        setDisplay("Waiting...", "#2b2f5a");
    }
    else if (role == "receiver")
    {
        setDisplay("Listening", "#1a3a5c");
    }
    else
    {
        setDisplay("Idle", "#2d3d6a");
    }
}

// =============================================================
void MacawNode::handleMessage(cMessage *msg)
{
    // ── Self timers ──────────────────────────────────────────
    if (msg->isSelfMessage())
    {
        int k = msg->getKind();
        delete msg;

        if      (k == TIMER_SEND_RTS)  sendRTS();
        else if (k == TIMER_SEND_CTS)  sendCTS();
        else if (k == TIMER_SEND_DS)   sendDS();
        else if (k == TIMER_SEND_DATA) sendDATA();
        else if (k == TIMER_SEND_ACK)  sendACK();
        else if (k == TIMER_NAV_CLEAR)
        {
            navActive = false;
            EV << "[" << getName() << "] NAV CLEARED\n";
            setDisplay("NAV clear", "#2d3d6a");
        }
        return;
    }

    // ── Incoming frame ───────────────────────────────────────
    int frameType = msg->par("frameType").longValue();
    int frameBytes= msg->par("frameBytes").longValue();
    std::string src = msg->par("src").stringValue();

    EV << "[" << getName() << "] RECV " << FRAME_NAMES[frameType]
       << " from " << src << "  bytes=" << frameBytes << "\n";

    rxVector.record(frameBytes);
    delete msg;

    // ── Neighbor: set NAV ────────────────────────────────────
    if (role == "neighbor")
    {
        if (!navActive)
        {
            navActive = true;
            EV << "[" << getName() << "] NAV SET for " << navDuration << "s\n";
            setDisplay("NAV SET", "#c41414");
            // Show NAV bubble
            cMessage *navClr = new cMessage("nav-clear", TIMER_NAV_CLEAR);
            scheduleAt(simTime() + navDuration, navClr);
        }
        return;
    }

    // ── Receiver (B) reactions ───────────────────────────────
    if (role == "receiver")
    {
        if (frameType == FRAME_RTS)
        {
            EV << "[B] RTS received → scheduling CTS\n";
            setDisplay("Got RTS", "#1a5c3a");
            cMessage *t = new cMessage("send-CTS", TIMER_SEND_CTS);
            scheduleAt(simTime() + 0.05, t);
        }
        else if (frameType == FRAME_DATA)
        {
            EV << "[B] DATA received → scheduling ACK\n";
            setDisplay("Got DATA", "#1a5c3a");
            cMessage *t = new cMessage("send-ACK", TIMER_SEND_ACK);
            scheduleAt(simTime() + 0.05, t);
        }
        return;
    }

    // ── Sender (A) reactions ─────────────────────────────────
    if (role == "sender")
    {
        if (frameType == FRAME_CTS && waitingCTS)
        {
            waitingCTS = false;
            EV << "[A] CTS received → scheduling DS\n";
            setDisplay("Got CTS", "#5a1a5c");
            cMessage *t = new cMessage("send-DS", TIMER_SEND_DS);
            scheduleAt(simTime() + 0.05, t);
        }
        else if (frameType == FRAME_ACK && waitingACK)
        {
            waitingACK = false;
            EV << "[A] ACK received → exchange COMPLETE\n";
            setDisplay("DONE ✓", "#0a7a28");
            bubble("MACAW Exchange Complete!");
        }
    }
}

// =============================================================
//  Sender steps
// =============================================================
void MacawNode::sendRTS()
{
    logStep(1, "RTS (Request To Send)", "A --[RTS]--> B + {C,E}");
    setDisplay("Sending RTS", "#1a5c8a");
    broadcastFrame(FRAME_RTS, 50, KIND_RTS);
    waitingCTS = true;
}
void MacawNode::sendDS()
{
    logStep(3, "DS (Data Sending notice)", "A --[DS]--> B + {C,E}");
    setDisplay("Sending DS", "#6e1aa0");
    broadcastFrame(FRAME_DS, 30, KIND_DS);
    // Schedule DATA immediately after DS propagation
    cMessage *t = new cMessage("send-DATA", TIMER_SEND_DATA);
    scheduleAt(simTime() + 0.10, t);
}
void MacawNode::sendDATA()
{
    logStep(4, "DATA (Payload)", "A --[DATA]--> B + {C,E}");
    setDisplay("Sending DATA", "#0a6e28");
    broadcastFrame(FRAME_DATA, dataBytes, KIND_DATA);
    waitingACK = true;
}

// =============================================================
//  Receiver steps
// =============================================================
void MacawNode::sendCTS()
{
    logStep(2, "CTS (Clear To Send)", "B --[CTS]--> A + {D,F}");
    setDisplay("Sending CTS", "#8a5a00");
    broadcastFrame(FRAME_CTS, 50, KIND_CTS);
}
void MacawNode::sendACK()
{
    logStep(5, "ACK (Acknowledgement)", "B --[ACK]--> A + {D,F}");
    setDisplay("Sending ACK", "#8a0a0a");
    broadcastFrame(FRAME_ACK, 30, KIND_ACK);
}

// =============================================================
//  broadcastFrame — send one copy per connected gate
// =============================================================
void MacawNode::broadcastFrame(MacawFrameType type, int bytes, int kind)
{
    int nGates = gateSize("wireless");
    EV << "[" << getName() << "] Broadcasting " << FRAME_NAMES[type]
       << "  bytes=" << bytes << "  gates=" << nGates << "\n";

    for (int g = 0; g < nGates; ++g)
    {
        cGate *out = gate("wireless$o", g);
        if (!out->isConnected()) continue;

        // Plain cMessage — colour is driven by message kind via omnetpp.ini,
        // which is the correct OMNeT++ 6.x way to colour packets in Qtenv.
        cMessage *frame = new cMessage(FRAME_NAMES[type], kind);
        frame->addPar("frameType") = (long)type;
        frame->addPar("frameBytes")= (long)bytes;
        frame->addPar("src")       = getName();

        send(frame, "wireless$o", g);
        txVector.record(bytes);
    }
}

// =============================================================
void MacawNode::setDisplay(const char* text, const char* bgCol)
{
    getDisplayString().setTagArg("t", 0, text);
    getDisplayString().setTagArg("t", 2, "#ffffff");
    getDisplayString().setTagArg("b", 4, bgCol);
}

void MacawNode::logStep(int step, const char* desc, const char* dir)
{
    EV << "\n========================================\n"
       << "  STEP " << step << "/5 :  " << desc << "\n"
       << "  " << dir << "\n"
       << "  Time = " << simTime() << "\n"
       << "========================================\n\n";
    bubble(desc);
}

// =============================================================
void MacawNode::finish()
{
    EV << "[" << getName() << "] simulation ended.\n";
}
