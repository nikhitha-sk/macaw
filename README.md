# MACAW Protocol Simulation — Build & Run Guide
Protocol order: **RTS → CTS → DS → DATA → ACK**
Topology: A (Sender) ↔ B (Receiver), with neighbors C, E near A and D, F near B

---

## Files Included

| File | Purpose |
|---|---|
| `macaw-ns3.cc` | NS-3 simulation with NetAnim output |
| `macaw.ned` | OMNeT++ network topology |
| `MacawNode.cc` | OMNeT++ node logic (all 5 MACAW steps) |
| `omnetpp.ini` | OMNeT++ run configuration |

---

## NS-3 + NetAnim

### Requirements
- NS-3 (≥ 3.38) built with NetAnim support
- NetAnim viewer (included in NS-3 distribution under `netanim/`)

### Build & Run
```bash
# 1. Copy the file into the NS-3 scratch directory
cp macaw-ns3.cc <path-to-ns3>/scratch/

# 2. Move into the NS-3 root directory
cd <path-to-ns3>

# 3. Build
./ns3 build

# 4. Run
./ns3 run scratch/macaw-ns3

# 5. Open the animation
#    Launch NetAnim:
./netanim/NetAnim &
#    Then File → Open → macaw-animation.xml
```

### What You See in NetAnim
| Node | Colour | Role |
|---|---|---|
| A | Dark navy | Sender |
| B | Dark navy | Receiver |
| C, E | Mid blue | Neighbors of A (overhear RTS, DS, DATA) |
| D, F | Mid blue | Neighbors of B (overhear CTS, ACK) |

Packets animate along edges. The NS-3 console prints every
PHY TX/RX event with timestamp and frame type.

### Console Output Format
```
1.001s  PHY-TX  [RTS]  ctx=/NodeList/0/...
1.002s  PHY-RX  [RTS]  ctx=/NodeList/1/...
1.052s  PHY-TX  [CTS]  ctx=/NodeList/1/...
...
```

---

## OMNeT++

### Requirements
- OMNeT++ 6.x (IDE or command-line)
- No extra INET framework needed — uses basic cMessage passing

### Project Setup (IDE)
1. File → New → OMNeT++ Project → name it `macaw`
2. Copy `macaw.ned`, `MacawNode.cc`, `omnetpp.ini` into the project root
3. Build: Project → Build All (Ctrl+B)
4. Run: Run → Run As → OMNeT++ Simulation → select **MacawNetwork**

### Command-Line Build
```bash
cd macaw_project/
opp_makemake -f --deep
make -j4
./macaw -u Qtenv omnetpp.ini          # GUI animation
./macaw -u Cmdenv omnetpp.ini         # console only
./macaw -u Cmdenv omnetpp.ini -c FastRun   # fast batch
```

### What You See in Qtenv
- **Animated packets** travel along wires with colour coding:

| Frame | Colour |
|---|---|
| RTS | Blue `#2e72d9` |
| CTS | Orange `#eb8008` |
| DS | Purple `#8c1fc8` |
| DATA | Green `#14a64b` |
| ACK | Red `#dc2424` |

- **Bubble popups** appear on each node announcing each step
- **NAV SET** bubble appears on neighbor nodes C, E, D, F
- **Sequence Chart** (Tools → Sequence Chart) shows the full
  event timeline after the run completes

### Qtenv Animation Speed
Edit `omnetpp.ini`:
```ini
**.animationSpeed = 0.2    # very slow (demo mode)
**.animationSpeed = 1      # normal
**.animationSpeed = 5      # fast
```

### Run Configurations
| Config | Use |
|---|---|
| `[General]` | Default — GUI, all recording on |
| `[Config FastRun]` | No animation, batch results |
| `[Config SlowAnim]` | 0.2× speed for classroom demo |
| `[Config Debug]` | Full debug logging + eventlog |

---

## Protocol Sequence (both simulators)

```
t=1.000s   A  ──[RTS]──►  B          (overheard by C, E → NAV SET)
t=1.050s   B  ──[CTS]──►  A          (overheard by D, F → NAV SET)
t=1.100s   A  ──[DS] ──►  B          (overheard by C, E)
t=1.200s   A  ──[DATA]──► B          (overheard by C, E)
t=1.250s   B  ──[ACK]──►  A          (overheard by D, F → NAV SET)
```

---

## Troubleshooting

**NS-3: `MacawTypeTag` not found**
- Ensure `ns3/packet.h` and `ns3/tag.h` are included (they are in `core-module.h`)

**NS-3: animation file empty**
- Check that `AnimationInterface` is constructed before `Simulator::Run()`

**OMNeT++: gate index out of range**
- Ensure `wireless[]` gate size in NED matches the number of connections

**OMNeT++: no Sequence Chart**
- Set `record-eventlog = true` in omnetpp.ini and re-run
