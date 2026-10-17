# Battery allocation update — selected, validation pending

October 17, 2026. This decision supersedes the earlier allocation of three 1.5 V motor cells and the larger rechargeable cell for electronics. Bench-powered movement success is separate from portable integration.

| Branch | Selected battery | Regulated rail | Status |
|---|---|---|---|
| Motors | Existing 1,000 mAh rechargeable cell; nominal voltage/chemistry to confirm from label | Dedicated boost, 6 V -> L293D pin 8 | New allocation; not yet installed or load-validated |
| Electronics | Smaller rechargeable cell, previously 500 mAh; latest capacity wording ambiguous | Separate boost, 5 V -> verified ESP32 board input and L293D pin 16 | Label/discharge and charging limits to confirm |

mAh is charge capacity; it is not a discharge-current guarantee. The bench display's approximately 550 mA is at the 6 V motor rail, not automatically the required cell current.

Illustrative power balance: Icell = Vout * Iout / (efficiency * Vcell). At 6 V, 0.55 A, assumed 85% efficiency and 3.7 V input, Icell is approximately 1.05 A. At 0.60 A output it is approximately 1.14 A; input rises as cell voltage falls. Starting pulses, converter heating, wiring and protection trip behavior require checking. Source: https://e2e.ti.com/cfs-file/__key/communityserver-discussions-components-files/196/3678.snva731.pdf .

## Intended load topology, not a completed wiring plan

Motor cell protected load positive -> motor switch -> motor boost IN+.
Motor protected load return -> motor boost IN-.
Motor boost OUT+ -> 6 V rail / pin 8.
Electronics cell protected charger OUT+ -> electronics switch -> electronics boost IN+.
Electronics protected charger OUT- -> electronics boost IN-.
Electronics boost OUT+ -> 5 V rail / verified board input / pin 16.
Both branch protected load returns and boost OUT- join SYSTEM_GND; all L293D GND pins and ESP32 GND share it. Keep motor return current out of ESP32 board traces.

Protection/charger arrangement for the newly reassigned motor cell is not identified yet. A raw negative terminal is not interchangeable with protected OUT- on a low-side-protected cell/charger. Do not externally bypass protection by joining raw B- into common load ground. Do not connect the two cells together or charge both with one single-cell module. Confirm compatible charging for each cell separately, with load switches off; no simultaneous charge-and-run design is established.

Before implementation: identify both cell labels and protection boards, allowable discharge/charge currents, both converter models and their 6 V/5 V load ratings, switching, polarity and charging return paths. Check for heating/odor and previous unexplained power paths before reuse. The earlier power-pinout document retains historical three-cell details; those are not instructions to wire the new rechargeable motor branch without these checks.
