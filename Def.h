#ifndef DEF_
#define DEF_

RelativeMove whitePawnEat[2] = { { 1, 1 }, { -1, 1 } };
RelativeMove whitePawnMove = { 0, 1 };
RelativeMove blackPawnEat[2] = { { 1, -1 }, { -1, -1 } };
RelativeMove blackPawnMove = { 0, -1 };

RelativeMove bishopMove[28]= { { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 }, { 7, 7 }, 
							{ 1, -1 }, { 2, -2 }, { 3, -3 }, { 4, -4 }, { 5, -5 }, { 6, -6 }, { 7, -7 },
							{ -1, 1 }, { -2, 2 }, { -3, 3 }, { -4, 4 }, { -5, 5 }, { -6, 6 }, { -7, 7 },
							{ -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 } }; //todo:cant leap

RelativeMove rookMove [28] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
								{ 0, -1 }, { 0, -2 }, { 0, -3 }, { 0, -4 }, { 0, -5 }, { 0, -6 }, { 0, -7 },
								{ 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 },
								{ -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 }, { -5, 0 }, { -6, 0 }, { -7, 0 } }; //todo:cant leap

RelativeMove knightMove [8] = { { 2, 1 }, { -2, 1 }, { 2, -1 }, { -2, -1 }, { 1, 2 }, { -1, 2 }, { 1, -2 }, { -1, -2 } };

RelativeMove queenMove[56] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 }, { 0, 6 }, { 0, 7 },
								{ 0, -1 }, { 0, -2 }, { 0, -3 }, { 0, -4 }, { 0, -5 }, { 0, -6 }, { 0, -7 },
								{ 1, 0 }, { 2, 0 }, { 3, 0 }, { 4, 0 }, { 5, 0 }, { 6, 0 }, { 7, 0 },
								{ -1, 0 }, { -2, 0 }, { -3, 0 }, { -4, 0 }, { -5, 0 }, { -6, 0 }, { -7, 0 },  { 1, 1 }, { 2, 2 }, { 3, 3 }, { 4, 4 }, { 5, 5 }, { 6, 6 }, { 7, 7 },
								{ 1, -1 }, { 2, -2 }, { 3, -3 }, { 4, -4 }, { 5, -5 }, { 6, -6 }, { 7, -7 },
								{ -1, 1 }, { -2, 2 }, { -3, 3 }, { -4, 4 }, { -5, 5 }, { -6, 6 }, { -7, 7 },
								{ -1, -1 }, { -2, -2 }, { -3, -3 }, { -4, -4 }, { -5, -5 }, { -6, -6 }, { -7, -7 } };  //todo:cant leap

RelativeMove kingMove[8] = { { 1, 1 }, { -1, 1 }, { 1, -1 }, { -1, -1 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, -1 } };

int mailbox[120] = {
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1,  0,  1,  2,  3,  4,  5,  6,  7, -1,
	 -1,  8,  9, 10, 11, 12, 13, 14, 15, -1,
	 -1, 16, 17, 18, 19, 20, 21, 22, 23, -1,
	 -1, 24, 25, 26, 27, 28, 29, 30, 31, -1,
	 -1, 32, 33, 34, 35, 36, 37, 38, 39, -1,
	 -1, 40, 41, 42, 43, 44, 45, 46, 47, -1,
	 -1, 48, 49, 50, 51, 52, 53, 54, 55, -1,
	 -1, 56, 57, 58, 59, 60, 61, 62, 63, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int mailbox64[64] = {
	21, 22, 23, 24, 25, 26, 27, 28,
	31, 32, 33, 34, 35, 36, 37, 38,
	41, 42, 43, 44, 45, 46, 47, 48,
	51, 52, 53, 54, 55, 56, 57, 58,
	61, 62, 63, 64, 65, 66, 67, 68,
	71, 72, 73, 74, 75, 76, 77, 78,
	81, 82, 83, 84, 85, 86, 87, 88,
	91, 92, 93, 94, 95, 96, 97, 98
};


int slide[6] = {
	0, 0, 1, 1, 1, 0 /* 0 = move one square, 1 = slide many squares */
};

int offsets[6] = {
	0, 8, 4, 4, 8, 8 /* number of directions we can move in */
};

int offset[6][8] = {
	{ 0, 0, 0, 0, 0, 0, 0, 0 }, 			/*PAWN */
	{ -21, -19, -12, -8, 8, 12, 19, 21 }, 	/* KNIGHT */
	{ -11, -9, 9, 11, 0, 0, 0, 0 },			/* BISHOP */
	{ -10, -1, 1, 10, 0, 0, 0, 0 },			/* ROOK */
	{ -11, -10, -9, -1, 1, 9, 10, 11 },		/* QUEEN */
	{ -11, -10, -9, -1, 1, 9, 10, 11 }		/* KING */
};



/*    ******************************************************** */

int piece_value[6] = {
	100, 300, 300, 500, 900, 0
};


int pawn_score[64] = {
	  0,   0,   0,   0,   0,   0,   0,   0,
	  5,  10,  15,  20,  20,  15,  10,   5,
	  4,   8,  12,  16,  16,  12,   8,   4,
	  3,   6,   9,  12,  12,   9,   6,   3,
	  2,   4,   6,   8,   8,   6,   4,   2,
	  1,   2,   3, -10, -10,   3,   2,   1,
	  0,   0,   0, -40, -40,   0,   0,   0,
	  0,   0,   0,   0,   0,   0,   0,   0
};

int knight_score[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -30, -10, -10, -10, -10, -30, -10
};

int bishop_score[64] = {
	-10, -10, -10, -10, -10, -10, -10, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,  10,  10,   5,   0, -10,
	-10,   0,   5,   5,   5,   5,   0, -10,
	-10,   0,   0,   0,   0,   0,   0, -10,
	-10, -10, -20, -10, -10, -20, -10, -10
};

int king_score[64] = {
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-40, -40, -40, -40, -40, -40, -40, -40,
	-20, -20, -20, -20, -20, -20, -20, -20,
	  0,  20,  40, -20,   0, -20,  40,  20
};

int king_endgame_score[64] = {
	  0,  10,  20,  30,  30,  20,  10,   0,
	 10,  20,  30,  40,  40,  30,  20,  10,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 30,  40,  50,  60,  60,  50,  40,  30,
	 20,  30,  40,  50,  50,  40,  30,  20,
	 10,  20,  30,  40,  40,  30,  20,  10,
	  0,  10,  20,  30,  30,  20,  10,   0
};


int flip[64] = {
	 56,  57,  58,  59,  60,  61,  62,  63,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 16,  17,  18,  19,  20,  21,  22,  23,
	  8,   9,  10,  11,  12,  13,  14,  15,
	  0,   1,   2,   3,   4,   5,   6,   7
};


#endif
DEF_
