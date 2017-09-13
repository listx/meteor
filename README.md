# METEOR

Meteor is a chess move generator.
It understands Shredder-FEN style strings to read in arbitrary game positions.
It also understands castling rules for Chess960.

Currently, its main function is for running a move tree, aka "perft", for determining all possible legal moves from the given arbitrary position up to some ply depth.
It is quite useful in this regard, as it allows you to check perhaps your own chess engine against bugs (en passant, castling, etc).

Some notable features:

- Hash tables (aka transposition hash tables) to help speed up the move count (if the same position has been seen before, it will use the hashed value for that position instead of recalculating the moves over again).
- pthreads for parallelism, although the granularity of parallelism can certainly be improved.
- Bitboards for handling game position information.

The project has been abandoned for some time, but it is perhaps of some interest to people looking to get into chess engine programming.

Meteor borrows heavily from Glaurung/Stockfish for some parts (move generation of sliding pieces in particular, and also Zobrist hashing). As such this project is also licensed under the GPLv3.

Please feel free to open a ticket or contact me for any questions/concerns.
