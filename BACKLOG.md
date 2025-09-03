# Backlog

- [x] Test rook move
- [x] Test bishop move
- [x] Test queen move
- [x] Test knight move
- [x] Wrap board into the game
- [x] Replace CharSlice_Wrap with CharSlice_Make
- [x] Simplify JSON interpretation by using JsonSource
- [x] Test pawn move
- [ ] Test king move
- [ ] Implement game loop
  - [x] Read board from a file
  - [x] Write board to JSON file
  - [x] Read the board from the JSON file
  - [ ] Check the "Checkmate" state
  - [ ] Implement pawn exchange move
  - [ ] Implement king's castle move
- [ ] Move history. Requires dynamically resizable **slice**
- [ ] Command history file persistence and scroll through history with arrow
  keys (probably tough without libs)

Arena:

- [ ] Implement reference initialization for the Arena
- [ ] Check memory got cleaned with 0's in all cases where memory is allocated
  from the arena.

Declined:

- [x] For CharSlice write zero where possible, so the necessity for to string
  method is eliminated. Implement a method to terminate a string at a known
  length. Decided to use char arr directly assuming that slice is allways
  initialized with zeros.
- [x] ~~Implement support for CharSlice in WriteF (PrintF)~~ (As soon as I can
  always access CharSlice.arr field it is no longer necessary)
- [ ] (MAYBE) Implement auto resize for CharSlice using Arena
