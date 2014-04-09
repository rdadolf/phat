## Open tasks
- Isolate persistent state into a structure.
- Move to mmap persistence in Paxos
- Implement fuzzy.
- Implement random drop into modcomm.

## Questions
- How do we address the ABA problem?

## Reminders
- Check that RPC_Msg::validate() is used everywhere. `NACK` back as necessary.
- Unique ID on every FS object, because Chubby is not duck-typed.

## Bugs

