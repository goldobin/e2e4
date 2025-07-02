
# Backlog

- [x] Test rook move
- [x] Test bishop move
- [x] Test queen move
- [x] Test knight move
- [ ] Test pawn move
- [ ] Test king move
- [ ] Implement pawn exchange move
- [ ] Implement king's castle move
- [ ] **-> Implement game loop**
  - [ ] **-> (file_rw) Read and write board to file**
- [ ] Implement reference initialization for the Arena
- [x] ~~Implement support for CharSlice in WriteF (PrintF)~~ As soon as I can always access CharSlice.arr field it is no  
      longer necessary 
- [ ] (MAYBE) Implement auto resize for CharSlice using Arena
- [x] ~~For CharSlice write zero where possible, so the necessity for to string method is eliminated. 
      Implement a method to terminate a string at a known length.~~ Decided to use asume that slice is allways 
      initialized with zeros. 
- [x] Replace CharSlice_Wrap with CharSlice_Make
- [ ] Check memory got cleaned with 0's in all cases where memory is allocated from arena.
