# ctc
ctc - Call Threaded Code

Call Threaded Code is an execution mechanism for programming language
interpreters, providing improved performance over direct threaded code,
while remaining much simpler to implement than JIT compilation.

Direct Threaded Code (DTC) suffers from high stall and branch misprediction
rates for two reasons.  First, the target of the dispatch code is dynamically
fetched from data (the DTC structure), requiring the processor to wait for
the result (stall) before it can proceed to the target.  The processor may
attempt to avoid the stall by predicting the target, but the dispatch code
after each instruction handler will have varying dynamic targets (depending
on the program being interpreted), reducing the effectiveness of conventional
branch prediction.

Call Threaded Code (CTC) is based on the observation that processors have a
special-purpose branch predictor for RETURN instructions, in the form of a
Return Address Stack (RAS).  A return instruction is an indirect jump, but
it's typically paired with a previous CALL instruction.  Processors track the
return addresses created by call instructions on a stack, and then predict the
jump target of a return from the top of this stack.  As a result a return can
often execute with no stall or misprediction penalty.

DTC represents the interpreted code as an array of pointers to the handlers
(in machine code) for the specific instructions.  This requires a virtual
program counter (vPC) for the interpreted code, as well as machine code to
dispatch the next instruction (this dispatch may be centralized, or copied
into each handler).  After the handler pointer there may also be a few words
describing the operands to that instruction, e.g. register numbers or constants
(immediates).

CTC represents the interpreted code as an array of CALL instructions to the
handlers.  As this is executable code, the processor's own program counter
is used to address it; no vPC is needed.  Dispatching the next interpreted
instruction is done by executing a RETURN instruction in the handler, which
brings control to the next CALL instruction in the array.  Thanks to the RAS
this will not stall or mispredict.  Operands to the handlers can be set up
by prefixing the CALL with MOVE instructions to load immediates into a few
dedicated argument registers.

This directory contains a snapshot of WIP research into CTC done around 2004
while the author was a researcher at Uppsala University, Sweden.  It has not
been updated since then, and may no longer work.

This README and all code included here is:

   Copyright 2018 Mikael Pettersson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
