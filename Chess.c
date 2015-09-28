#include "Chess.h"


/* global variables */

treeNode *gameTree = NULL;
treeNode *prevGameTree = NULL;
int current_num_of_nodes;
int minimaxDepth;
int game_mode;
color user_color;
color next_player = WHITE;
int gameState = 0;
int get_best_moves = 0;

int restartFlag = 0;


int GUI;
int done = 0;
int initDepth = 0;
int nodeNum = 0;



const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 800;
const int SCREEN_BPP = 0;

int SAVE_STATES = 7;
int LOAD = 0;
int gameBoardX = 20, gameBoardY = 20;
int setupBoardX = 100, setupBoardY = 50;

SDL_Surface *screen = NULL; /* main screen */
SDL_Rect clip[14]; /* sprite map handling */
SDL_Surface *tools = NULL; /* game tools sprite sheet */
SDL_Surface *guiBoard = NULL; /* game board */

struct memList* memoryAllocList;
static void* safeMalloc(size_t size)
{
	void *p = malloc(size);
	if (!p)
	{
		printf("Out of Memory\n");
		exit(EXIT_FAILURE);
	}
	struct memNode * tempNode;
	tempNode = malloc(sizeof(struct memNode));
	if (tempNode == NULL)
		perror("Out of memory");
	tempNode->p = p; tempNode->next = memoryAllocList->head;
	memoryAllocList->head = tempNode;
	return p;
}

static void safeFree(void * p)
{
	if (p == memoryAllocList->head->p)
	{
		free(memoryAllocList->head->p);
		free(memoryAllocList->head);
		memoryAllocList->head = NULL;
		return;
	}
	struct memNode * curr = (memoryAllocList->head->next);
	struct memNode * prev = (memoryAllocList->head);
	while (curr->p != p)
	{
		prev = prev->next;
		curr = curr->next;
		if (curr == NULL)
		{
			return;
		}
	}
	if (curr->p == p)
	{
		prev->next = curr->next;
		free(p);
		free(curr);
	}
}

static void bruteFree()
{
	struct memNode * tempNode = memoryAllocList->head;
	struct memNode * prev;
	while (1)
	{
		if (tempNode->next == NULL)
		{
			free(tempNode->p);
			free(tempNode);
			return;
		}
		prev = tempNode;
		tempNode = tempNode->next;
		free(prev->p);
		free(prev);
	}
}

int main(int argc, char ** argv)
{
	/* creating the board */
	char board[BOARD_SIZE][BOARD_SIZE];
	color current_player_color = WHITE; /*by default player starts */

	/* initializing the game with default values */
	current_num_of_nodes = 0;


	minimaxDepth = 1;
	initDepth = minimaxDepth;
	game_mode = 1;
	user_color = WHITE;
	init_board_(board);
	initArrayOfReps();
	if (SAVE_STATES > 10)
		SAVE_STATES = 10;

	if (argc == 0) /* restart from gui */
		goto GUI_RESTART;

	/* GUI flag */
	if (argc == 1)
		GUI = 0;
	else if (!strcmp(argv[1], "console") && argc == 2)
		GUI = 0;
	else if (!strcmp(argv[1], "gui") && argc == 2)
		GUI = 1;

GUI_RESTART:
	if (GUI == 1)
	{
		/* start of UI tree */
		guiMainMenu(board, current_player_color);
	}
	else if (GUI == 0){ /* cmd mode */
		/* settings and game states */
		while (1 && !gameState)
		analyzeCommand(board);
		//current_player_color = settings_state(board);
		board[0][6] = WHITE_P;
		board[0][7] = EMPTY;
		board[1][7] = EMPTY;
		board[1][6] = EMPTY;
		print_board_(board);
		game_state(board, current_player_color);
	}


	return 0;
}

/* *******************************************************/
/* ***************  prime methods   **********************/
/* *******************************************************/


/*
the function gets a board and runs the settings state as described in the PDF of this exercise.
It returns the color of the player who should play next (For new game its WHITE by default).
*/

checker getPoint(char *iter)
{
	checker c = { '0', 0 };
	char x = iter[1];
	int  y = iter[3] - 48;

	if (!(x<'a' || x>'h' || y<1 || y>BOARD_SIZE)){
		c.x = x;
		c.y = y;
	}
	return c;
	
}

char* getInput()
{

	long long count = 0;
	long long stringSize = 10;

	char *s;
	s = malloc(stringSize);
	char *tempS = s;
	//char c = getchar();
	//c = getchar();
	while (1)
	{
		char c = getchar();
		count += 1;
		if (c == '\n')
		{
			*tempS = '\0';
			break;
		}
		*tempS = c;
		tempS += 1;

		if (count == stringSize - 1)
		{
			stringSize += 10;
			s = (char *)realloc(s, stringSize);
			tempS = s + count;
		}
	}
	return s;
}
int legelPos(int x, int y)
{
	return !(((x % 2 == 1) && (y % 2 == 0)) || ((x % 2 == 0) && (y % 2 == 1)) || ((x<1) || (y>10) || (x<1) || (y>10)));
}
char getCharColor(char c)
{
	if (c == 'm' || c == 'k')
		return 'w';
	return'b';
}

char* processWordUntilSpecialSign(char * s, int flag)
{
	char *iter = s;
	//char *tempString = safeMalloc(sizeof(s) + 1);
	char *tempString =malloc(sizeof(s) + 1);
	char *tempStringiter = tempString;
	//skip spaces
	while (isspace(*iter))
	{
		iter += 1;
	}
	//handle first word
	//if ((*iter == '_') || (*iter == '<'))
	if ((*iter == '<'))
	{
		*tempStringiter++ = *iter++;

		if (!flag)
		{
			*tempStringiter = '\0';
			return tempString;
		}
	}
	//while (!(isspace(*iter) || (*iter == '_') || (*iter == '<') || (*iter == '\0')))
	while (!(isspace(*iter) || (*iter == '<') || (*iter == '\0')))
	{
		*tempStringiter++ = *iter++;
	}
	*tempStringiter = '\0';
	//safe_realloc(tempString, strlen(tempString) + 1);
	return tempString;
}



int analyzeCommand(char board[BOARD_SIZE][BOARD_SIZE])
{
	if (GUI != 1)
		print_board_(board); /* cmd mode, print board out */ // TODO: check if needed
	print_message(ENTER_SETTINGS);
	//struct List list;
	//struct List *lst = &list;
	
	char opCode = '0';
	checker srcChkr;	
	char *iter = getInput();
	char *in1;
	char *in2;
		
	color whitec = WHITE;
	color* whithcPointer = &whitec;

	

	
	in1 = processWordUntilSpecialSign(iter, 0);
	iter += strlen(in1);
	while (isspace(*iter))
	{
		iter += 1;
	}
	if (*iter == '\0') 
	{
		if (!strcmp(in1, "quit"))
		{
			opCode = 'q';
			freeAllRepositories();
			freeTree(gameTree);
			exit(0);

		}
		if (!strcmp(in1, "clear"))
		{
			//if (GameState)
				//print_message(ILLEGAL_COMMAND);
			
			clear_board(board);
		}
		if (!strcmp(in1, "print"))
		{
			opCode = 'p';
			print_board_(board);
		}
		if (!strcmp(in1, "start"))
		{
			opCode = 's';
			//if (GameState)
				//print_message(ILLEGAL_COMMAND);
			
			//start(piece);
			//TODO: CHACK IF CAN START-2 KINGS
			gameState = 1;
			return;
		}
		if (opCode != '0')
		{

			return; //TODO:call func
		}
		//maybe do an else, maybe return
	}
	if (*iter == '<')
	{
		if (!strcmp(in1, "rm"))
		{
			/*if (GameState)
			{
			print_message(ILLEGAL_COMMAND);
			return;
			} */
			opCode = 'r';
			srcChkr = stringToChecker(iter);
			if (srcChkr.y == 0){
				print_message(WRONG_POSITION);
				return;
			}
			board[srcChkr.x - 97][srcChkr.y-1] = EMPTY;
		}
		
		if (!strcmp(in1, "set"))
		{
			/*if (GameState == 1)
			{
				print_message(ILLEGAL_COMMAND);
				return;
			}*/
			opCode = 't';
			srcChkr = stringToChecker(iter);
			if (srcChkr.y == 0){
				print_message(WRONG_POSITION);
				return;
			}
			
			color clr;
			iter += 6;
			if (*iter == 'b')
				clr = BLACK;
			else
				clr = WHITE;
			iter += 6;
			char b = *iter; // k q r k b or p

			//setPiece(srcChkr, clr, b, board); //TODO: make func and check for "Setting this piece creates an invalid board"
		}
		/*if (!strcmp(in1, "move"))
		{
			opCode = 'g';
			Point p = getPoint(iter);
			if (!legelPos(p.x, p.y))
			{
				print_message(WRONG_POSITION);
				return;
			}
			struct Node node;
			struct Node *atnode = &node;

			node.p = p;
			lst->head = atnode;
			iter += 5;
			while (isspace(*iter) || *iter == 't' || *iter == 'o' || *iter == '>')
			{
				iter += 1;
			}
			while (*iter != '\0')
			{
				struct Node *next = (struct Node*) safeMalloc(sizeof(struct Node));
				//	struct Node *atnext=&next;
				next->p = getPoint(iter);
				if (!legelPos(next->p.x, next->p.y))
				{
					print_message(WRONG_POSITION);
					return;
				}
				atnode->next = next;
				atnode = next;
				iter += 5;
				if (next->p.y == 10)
					iter++;

			}
			if (getCharColor(piece[p.x - 1][p.y - 1].t) == opColor(userColor) || piece[p.x - 1][p.y - 1].t == EMPTY)
			{
				print_message(NO_DICS);
				return;
			}
			atnode->next = nullNode;
			//print_board(piece);
			int max = updateAllMovesByColor(userColor, piece);
			filterAllMovesList(piece, max);
			if (checkIfLegalMove(piece, *lst, max) == 1)
			{
				movePiece(*lst, piece);
				print_board(piece);
				if (Win(piece) == userColor)
				{
					GameState = 2;
					char* s = stringColorBig(userColor);
					printf("%s%s", s, " player wins!\n");
					return;

				}
				else if (Win(piece) == 't')
				{
					GameState = 2;
					print_message("Tie!\n");
					return;
				}
				else
				{
					minimax(piece, opColor(userColor), minimaxDepth, -101, 101, 1);
					movePiece(*bestmove, piece);
					printComputerMove(bestmove);
					print_board(piece);

					if (Win(piece) == opColor(userColor))
					{
						GameState = 2;
						char* s = stringColorBig(opColor(userColor));
						printf("%s%s", s, " player wins!\n");

					}
					else if (Win(piece) == 't')
					{
						GameState = 2;
						print_message("Tie!");
						return;
					}

				}
				return;
			}
			else
			{
				printf(ILLEGAL_MOVE);
				return;
			}

		}
		*/
	}
	else
	{
		
		if (!strcmp(in1, "load"))
		{
			if (!loadGame(board, iter, *whithcPointer))
			{
				print_message(WRONG_FILE);
				return;
			}
			LOAD = 1; /* game starts from load */
			print_board_(board);
			return;
		}
		else if (!strcmp(in1, "difficulty"))
		{
			if (!strcmp(iter, "best"))
			{
				minimaxDepth = -1;
				return;
			}
			in2 = processWordUntilSpecialSign(iter, 1);
			if (!strcmp(in2, "depth") ){
				int x = iter[6] - 48;
				if (x<1 || x>4){
					print_message(WRONG_MINIMAX_DEPTH);
					return;
				}
				minimaxDepth = x;
				initDepth = minimaxDepth;
				return;
			}

			print_message(ILLEGAL_COMMAND);

		}
		else if (!strcmp(in1, "user_color"))
		{
			if (!strcmp(iter, "black"))
				user_color = BLACK;
			else
				user_color = WHITE;
			return;

		}
		else if (!strcmp(in1, "game_mode"))
		{
			int x = iter[0] - 48;
			if (x != 1 && x != 2){
				print_message(WRONG_GAME_MODE);
				return;
			}
			game_mode = x;
			if (x == 1)
				printf("Running game in 2 players mode\n");
			else
				printf("Running game in player vs. AI mode\n");
			return;

		}
		else if (!strcmp(in1, "next_player"))
		{
			if (!strcmp(iter, "black"))
				next_player = BLACK;
			else
				next_player = WHITE;
			return;

		}
			
	
		
	//	while (isspace(*iter))
	//	{
	//		iter += 1;
	//	}
	//	iter += strlen(in2);
	//	while (isspace(*iter))
	//	{
	//		iter += 1;
	//	}
	//	if (!strcmp(in1, "get") && !strcmp(in2, "_moves"))
	//	{
	//		opCode = 'g';
	//		//int max = updateAllMovesByColor(userColor, piece); //updates possible moves and puts maximum moves in max
	//		filterAllMovesList(piece, max);
	//		printAllPossibleMoves(piece, max);
	//		//printf(ENTER_YOUR_MOVE);
	//		return; //TODO: call func
	//	}
	//	if (!strcmp(in1, "minimax") && !strcmp(in2, "_depth"))
	//	{
	//		if (GameState)
	//		{
	//			print_message(ILLEGAL_COMMAND);
	//			return;
	//		}
	//		opCode = 'd';
	//		int depth = *iter - 48;
	//		if (depth > 6 || depth < 1)
	//		{
	//			printf(WRONG_MINIMAX_DEPTH);
	//			return;
	//		}
	//		minimaxDepth = depth;
	//		return;

	//	}
	//	if (!strcmp(in1, "user") && !strcmp(in2, "_color"))
	//	{
	//		if (GameState)
	//		{
	//			print_message(ILLEGAL_COMMAND);
	//			return;
	//		}
	//		opCode = 'u';
	//		userColor = 'w';
	//		if (*iter == 'b')
	//			userColor = 'b';
	//	}
	//	if (opCode != '0')
	//	{
	//		iter++;
	//		return; //TODO: callfunc
	//	}
	//	
	}
	if (opCode == '0')
		printf(ILLEGAL_COMMAND);
		
}





// to here




color settings_state(char board[BOARD_SIZE][BOARD_SIZE])
{
	char input[STRING_SIZE];
	char arg[STRING_SIZE], a[STRING_SIZE];
	int x, flag = 0;
	color current_player_color = WHITE;

	color* current_player_color_pointer = &current_player_color;
	//
	char *iter = getInput();
	char *in1;
	char *in2;

	in1 = processWordUntilSpecialSign(iter, 0);
	iter += strlen(in1);
	while (isspace(*iter))
	{
		iter += 1;
	}
	if (GUI != 1)
		print_board_(board); /* cmd mode, print board out */


	input[0] = '\0';
	printf("Enter game setting:\n");

	while (1){
		/* clearing input buffer */
		if (flag)
		while (getchar() != '\n');

		flag = 1;
		scanfWrapper(scanf("%s", input));

		if (strcmp(input, "game_mode") == 0){
			scanfWrapper(scanf("%d", &x));
			if (x != 1 && x != 2){
				print_message(WRONG_GAME_MODE);
				continue;
			}
			game_mode = x;
			if (x == 1)
				printf("Running game in 2 players mode\n");
			else
				printf("Running game in player vs. AI mode\n");

		}
		else if (strcmp(input, "difficulty") == 0 && game_mode == 2){
			scanfWrapper(scanf("%s", a));

			if (strcmp(a, "best") == 0){
				minimaxDepth = -1;
				continue;
			}

			if (strcmp(a, "depth") == 0){
				scanfWrapper(scanf("%d", &x));
				if (x<1 || x>4){
					print_message(WRONG_MINIMAX_DEPTH);
					continue;
				}
				minimaxDepth = x;
				initDepth = minimaxDepth;
				continue;
			}

			print_message(ILLEGAL_COMMAND);

		}
		else if (strcmp(input, "user_color") == 0 && game_mode == 2){
			scanfWrapper(scanf("%s", arg));
			if (strcmp(arg, "black") == 0)
				user_color = BLACK;
			else
				user_color = WHITE;
		}
		else if (strcmp(input, "load") == 0){
			/* getting the filename  */
			scanfWrapper(scanf("%s", arg));

			if (loadGame(board, arg, *current_player_color_pointer) == 0){

				print_message(WRONG_FILE);
				continue;
			}
			LOAD = 1; /* game starts from load */
			print_board_(board);

		}


		else if (strcmp(input, "print") == 0){
			print_board_(board);
		}
		else if (strcmp(input, "quit") == 0){
			freeAllRepositories();
			freeTree(gameTree);
			exit(0);
		}
		else if (strcmp(input, "start") == 0)
		{

			break;
		}
		else{
			print_message(ILLEGAL_COMMAND);
		}


	}

	/* settings state is over */
	return current_player_color;
}




/*
the function gets a board and the current player color
and runs the game state as described in the PDF of this exercise.
*/
void game_state(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color)
{

	legalMovesRepository re;
	legalMovesRepository* rep = &re;


	LOAD = 0;


	/* initialize repository */
	rep->legalMoves = NULL;
	rep->pos = 0; rep->size = 0;
	addRepToArrayOfReps(rep);



	getAllLegalMoves(board, OPPOSITE_COLOR(current_player_color), rep, 1, 0);
	/* checking for a loaded win game */
	if (rep->size == 0){
		if (current_player_color == WHITE){
			if (GUI != 1)
				printf("Mate! White player wins the game\n");
			else
			{
				guiWinMessage(WHITE, board, current_player_color); /* black won */ //TODO: change guiWinMessage
			}
		}
		else{
			if (GUI != 1)
				printf("Mate! Black player wins the game\n");
			else
			{
				guiWinMessage(BLACK, board, current_player_color); /* white won  */
			}
		}
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}
	/* finished with this repository*/
	freeRepository(rep);

	/* initializing a new repository for inside the while loop */
	rep->legalMoves = NULL;
	rep->pos = 0; rep->size = 0;
	addRepToArrayOfReps(rep);

	while (1)
	{
		if (GUI == 1)
		{
			setMessageToUser(current_player_color,TURN_MESSAGE);
		}

		if (current_player_color == user_color || game_mode == 1)
		{
			user_turn(board, current_player_color);
		}
		else
		{
			computer_turn(board, current_player_color);
		}

		if (GUI != 1)
			print_board_(board); /* cmd mode, print board out */
		else
		{
			setBoard(board, 0,gameBoardX,gameBoardY); /* gui mode, show board on screen */
		}

		

		/* creating a repository of all legal available moves of the OPPONENT!!! */
		getAllLegalMoves(board, OPPOSITE_COLOR(current_player_color), rep, 1, 0);

		/* if the user has no moves to play, he lost */
		if (rep->size == 0){
			if (current_player_color == BLACK){
				if (GUI != 1)
					printf("Mate! Black player wins the game\n");
				else{
					guiWinMessage(BLACK, board, current_player_color); /* black won */
				}
			}
			else{
				if (GUI != 1)
					printf("Mate! White player wins the game\n");
				else{
					guiWinMessage(WHITE, board, current_player_color); /* white won  */
				}
			}


			freeAllRepositories();
			freeTree(gameTree);
			exit(0);
		}
		else{
			/* checking if it a check*/
			if (!isKingSafe(board, OPPOSITE_COLOR(current_player_color))){
				if (GUI != 1)
					printf("Check!\n");
				else
					guiCheckMessage(board);
			}
		}

		freeRepository(rep);
		/* initializing a new repository for inside the while loop */
		rep->legalMoves = NULL;
		rep->pos = 0; rep->size = 0;
		addRepToArrayOfReps(rep);



		/* changing the current player color to the color of the next player */
		current_player_color = OPPOSITE_COLOR(current_player_color);
	}

	freeRepository(rep);
}


/* this function simulates a user turn */
void user_turn(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color){
	char cmd[STRING_SIZE], str[STRING_SIZE], str2[STRING_SIZE], theMove[STRING_SIZE], promoteTo[STRING_SIZE];

	char *iter;
	char *in1;
	char *in2;
	char ch;
	checker srcChkr;
	checker destChkr;
	legalMovesRepository re;
	legalMovesRepository* rep = &re;
	int i, clearBufferFlag = 0;

	if (GUI == 1)
	{
		/* GUI mode */
		/* show game window and get move */
		guiUserTurn(board, current_player_color);

	}
	else
	{

		/* initialize repository */
		rep->legalMoves = NULL;
		rep->pos = 0; rep->size = 0;
		addRepToArrayOfReps(rep);

		/* creating a repository of all legal available moves */
		getAllLegalMoves(board, current_player_color, rep, 1, 0);


		while (1){


			if (current_player_color == BLACK)
				printf("Black player - enter your move:\n");
			else
				printf("White player - enter your move:\n");
			/* clearing input buffer */
			iter = getInput();
			in1 = processWordUntilSpecialSign(iter, 0);
			iter += strlen(in1);
			while (isspace(*iter))
			{
				iter += 1;
			}

			if (clearBufferFlag)
		//	while (getchar() != '\n');

			clearBufferFlag = 1;

			/* getting new command */
			

			if (!strcmp(in1, "move")){
				/* getting the source checker */
				
				strcpy(theMove, iter);
				//theMove[14] = '\0';
				srcChkr = stringToChecker(iter);
				if (srcChkr.y == 0){
					print_message(WRONG_POSITION);
					continue;
				}

				/* getting the 'to' */
				iter += 5;
				while (isspace(*iter))
				{
					iter += 1;
				}
				in2 = processWordUntilSpecialSign(iter, 1);
				while (isspace(*iter))
				{
					iter += 1;
				}
				if (strcmp(in2, "to") != 0)
				{
					print_message(ILLEGAL_COMMAND);
					continue;
				}

				/* getting the destination checker */
				//scanfWrapper(scanf("%s", str));
				//strcat(theMove, str);
				iter += 2;
				while (isspace(*iter))
				{
					iter += 1;
				}

				/* creating an array of destination checkers */
				destChkr = stringToChecker(iter);

				/* making sure the checker is legal */
				if (destChkr.y == 0){
					print_message(WRONG_POSITION);
					continue;
				}
				while (isspace(*iter))
				{
					iter += 1;
				}
				iter += 5;
				/* checking if the user has a tool on the source checker */
				if (!(((current_player_color == WHITE) &&
					WHITE_TOOL(board[srcChkr.x - 97][srcChkr.y - 1])) ||
					((current_player_color == BLACK) &&
					BLACK_TOOL(board[srcChkr.x - 97][srcChkr.y - 1]))))
				{
					print_message(NO_DICS);
					continue;
				}

				/* getting the extra string if a pawn has to be promoted */
				if (((board[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P && destChkr.y == BOARD_SIZE) ||
					(board[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P && destChkr.y == 1)) && isItInRepository(rep, theMove))
				{
					//i = 0;
					//while ((ch = getchar()) != '\n')
					//	promoteTo[i++] = ch;
					//promoteTo[i] = '\0';
					//clearBufferFlag = 0;
					//

					//if (strcmp(promoteTo, "") == 0){
					//	strcpy(promoteTo, " queen");	 /* queen by default*/
					//}
					//else if (strcmp(promoteTo, " knight") != 0 && strcmp(promoteTo, " bishop") != 0 &&
					//	strcmp(promoteTo, " rook") != 0 && strcmp(promoteTo, " queen") != 0) {
					//	print_message(ILLEGAL_COMMAND);
					//	continue;
					//}
					//
					if (strcmp(iter, "") == 0){
						strcat(theMove, " queen\0");	 /* queen by default*/
					}
					/*else if (strcmp(promoteTo, "knight") != 0 && strcmp(promoteTo, "bishop") != 0 &&
						strcmp(promoteTo, "rook") != 0 && strcmp(promoteTo, "queen") != 0) {
						print_message(ILLEGAL_COMMAND);
						continue;
					}
					
					strcat(theMove, promoteTo);*/

				}

				/* checking if it a legal move by searching in the repository */
				if (isItInRepository(rep, theMove) == 0){
					print_message(ILLEGAL_MOVE);
					continue;

				}

				/* the move is legal - update the board */
				simulateTurn(board, theMove, board);


				break;

			}
			else if (strcmp(in1, "get_moves") == 0){
				//scanfWrapper(scanf("%s", str));
				srcChkr = stringToChecker(iter);

				if (srcChkr.y == 0){
					print_message(WRONG_POSITION);
					continue;
				}

				/* checking if the user has a tool on the source checker */
				if (!(((current_player_color == WHITE) &&
					WHITE_TOOL(board[srcChkr.x - 97][srcChkr.y - 1])) ||
					((current_player_color == BLACK) &&
					BLACK_TOOL(board[srcChkr.x - 97][srcChkr.y - 1]))))
				{
					print_message(NO_DICS);
					continue;
				}

				for (i = 0; i < rep->size; i++){
					substringFromString(str2, 0, 4, rep->legalMoves[i]);

					if (strcmp(iter, str2) == 0 && strlen(rep->legalMoves[i]) > 7)
					{
						if (((srcChkr.y - 1) == 6 && board[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P) || ((srcChkr.y - 1) == 1 && board[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P))
							if (strlen(rep->legalMoves[i]) < 15)
								continue;					
						printf("%s\n", rep->legalMoves[i]); /* print regular move */
					}
				}


			}
			else if (!strcmp(in1, "get_best_moves"))
			{
				//todo: make func
			}
			else if (!strcmp(in1, "get_score"))
			{
				//todo: make func
			}
			else if (!strcmp(in1, "castle"))
			{
				srcChkr = stringToChecker(iter);
				if (srcChkr.y == 0){
					print_message(WRONG_POSITION);
					continue;
				}
				if (!(((current_player_color == WHITE) &&
					board[srcChkr.x - 97][srcChkr.y - 1]==WHITE_R) ||
					((current_player_color == BLACK) &&
					board[srcChkr.x - 97][srcChkr.y - 1]==BLACK_R)))
				{
					print_message(WROND_ROOK_POS);
					continue;
				}
				
				if (current_player_color == WHITE)
				{
					if (board[0][4] != WHITE_K || ((strcmp(iter, "<a,1>") && strcmp(iter, "<h,1>"))))
					{
						print_message(ILLEGAL_CASTLING);
						continue;
					}
					if (!strcmp(iter, "<a,1>"))
						if (board[1][0] != EMPTY || board[2][0] != EMPTY || board[3][0] != EMPTY)
						{
							print_message(ILLEGAL_CASTLING);
							continue;
						}
					if (!strcmp(iter, "<h,1>"))
						if (board[5][0] != EMPTY || board[6][0] != EMPTY )
						{
							print_message(ILLEGAL_CASTLING);
							continue;
						}
				}
				if (current_player_color == BLACK)
				{
					if (board[0][4] != WHITE_B || ((strcmp(iter, "<h,8>") && strcmp(iter, "<a,8>"))))
					{
						print_message(ILLEGAL_CASTLING);
						continue;
					}
					if (!strcmp(iter, "<a,8>"))
						if (board[1][7] != EMPTY || board[2][7] != EMPTY || board[3][7] != EMPTY)
						{
							print_message(ILLEGAL_CASTLING);
							continue;
						}
					if (!strcmp(iter, "<h,8>"))
						if (board[5][7] != EMPTY || board[6][7] != EMPTY)
						{
							print_message(ILLEGAL_CASTLING);
							continue;
						}
				}
				//todo: castle function
			}
			//else if (strcmp(cmd, "restart") == 0){  //todo: remove

			//	main(1, NULL); /* restart and go to console mode */
			//}
			else if (strcmp(in1, "save") == 0){
				/* getting the filename  */
				

				if (!saveGame(board, iter, current_player_color)){
					print_message(WRONG_FILE);
					continue;
				}

			}
			else if (!strcmp(in1, "quit")){
				freeAllRepositories();
				freeTree(gameTree);
				exit(0);
			}
			else{
				print_message(ILLEGAL_COMMAND);

			}
		}

		freeRepository(rep);
	}
}



/* this function simulates a computer turn */
void computer_turn(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color){
	color PC_Color;
	char *chosenMove;
	int bestValue, prevBest = 0;
	int depth = 1;


	nodeNum = 0;

	if (minimaxDepth == -1){

		PC_Color = OPPOSITE_COLOR(user_color);

		while (nodeNum < MAX_NODES){

			initDepth = depth;
			bestValue = getBestScoreBybuildingGameTree(board, depth, PC_Color, PC_Color, NULL, NULL, 0, -1 * INF, INF, 1);
			if (nodeNum < MAX_NODES){
				prevBest = bestValue;
				depth++;

				freeTree(prevGameTree);
				prevGameTree = gameTree;
				gameTree = NULL;

				if (prevBest == INF)
					break;/* only one move option, just stop */
			}
			else{
				freeTree(gameTree);
				gameTree = NULL;
			}


		}

		gameTree = prevGameTree;
		chosenMove = chooseMove(prevBest, gameTree, 0);


		initDepth = depth;

	}
	else{

		depth = minimaxDepth;

		PC_Color = OPPOSITE_COLOR(user_color);

		bestValue = getBestScoreBybuildingGameTree(board, depth, PC_Color, PC_Color, NULL, NULL, 0, -1 * INF, INF, 1);
		chosenMove = chooseMove(bestValue, gameTree, 0);
	}

	simulateTurn(board, chosenMove, board); /* update board */

	if( GUI != 1)
		printf("move %s\n",chosenMove);

	freeTree(gameTree);
	gameTree = NULL;
	prevGameTree = NULL;

}


/* gets an old board, a move string and a new board
at the end -  the new board looks like the old board after playing the given move
*/
void simulateTurn(char oldBoard[BOARD_SIZE][BOARD_SIZE], char* move, char newBoard[BOARD_SIZE][BOARD_SIZE]){
	char srcStr[STRING_SIZE], destStr[STRING_SIZE];
	char ch;
	checker srcChkr;
	checker destChkr;


	/* source checker */
	substringFromString(srcStr, 0, 4, move); //todo: check if can bring thr srchrr and the destchkr in the function arguments
	srcChkr = stringToChecker(srcStr);


	/* destination checker */
	substringFromString(destStr, 9, 13, move);
	destChkr = stringToChecker(destStr);

	/* the new board should be identical to the old one at the beggining */
	copyBoard(oldBoard, newBoard);

	/* if a pawn got to the far side it is promoted */
	if ((oldBoard[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P && destChkr.y == BOARD_SIZE) ||
		(oldBoard[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P && destChkr.y == 1))
	{
		if (strlen(move) < 15)
			ch = (oldBoard[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P) ? 'q' : 'Q';
		else
			ch = move[15];

		if (ch == 'k' && oldBoard[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P) //todo: mybe can remove this
			ch = 'n';
		else if (ch == 'k' && oldBoard[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P)
			ch = 'N';
		else if (ch == 'q' && oldBoard[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P)
			ch = 'Q';
		else if (ch == 'r' && oldBoard[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P)
			ch = 'R';
		else if (ch == 'b' && oldBoard[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P)
			ch = 'B';

	}
	else
		ch = newBoard[srcChkr.x - 97][srcChkr.y - 1];
		//ch = oldBoard[srcChkr.x - 97][srcChkr.y - 1];

	/* simulate the move */
	newBoard[destChkr.x - 97][destChkr.y - 1] = ch;
	newBoard[srcChkr.x - 97][srcChkr.y - 1] = EMPTY;
	//oldBoard[destChkr.x - 97][destChkr.y - 1] = ch;
//	oldBoard[srcChkr.x - 97][srcChkr.y - 1] = EMPTY;

}




/* *******************************************************/
/* **************  game tree methods   *******************/
/* *******************************************************/
/*
The method gets:
1) board             - a board game
2) depth             - the depth of the tree
3) PlayerA           - the color of the player which this tree is being created for
4) currentColor      - the color of the current player (in this level of the tree)
5) curMove		     - used to transfer the move from a father to his child (in order to store it in the child)
6) father		     - pointer to the father of the current node created
7) whichSon		     - indicates what is the serial number of this child regarding to his father
8) alpha		     - used for alpha beta pruning
9) beta			     - used for alpha beta pruning
10) maximizingPlayer - indicated whether this level we should max or min (it is a version of minimax algorithm afterall)


The methods returns the best score (of the best board) for PlayerA (using alpha beta pruning).

*/

int getBestScoreBybuildingGameTree(char board[BOARD_SIZE][BOARD_SIZE], int depth, color PlayerA, color currentColor,
	char* curMove, treeNode* father, int whichSon, int alpha, int beta, int maximizingPlayer){

	char nextBoard[BOARD_SIZE][BOARD_SIZE];
	treeNode *root = NULL; treeNode *son = NULL;
	legalMovesRepository re;
	legalMovesRepository *rep = &re;
	int i, amountOfMoves, temp;
	color nextColor;

	/* set next level color */
	nextColor = OPPOSITE_COLOR(currentColor);

	/* initialize repository */
	rep->legalMoves = NULL;
	rep->pos = 0; rep->size = 0;
	addRepToArrayOfReps(rep);

	/* get all possible moves */
	if (depth > 0)
		getAllLegalMoves(board, currentColor, rep, 1, 0);
	else
		getAllLegalMoves(board, currentColor, rep, 1, 1);/* depth == 0 check if leaf */
	amountOfMoves = rep->size;


	/* allocate current node */
	root = allocWrapper(calloc(1, sizeof(treeNode)), "calloc");
	nodeNum++;



	/* attach this node to his father */
	if (father != NULL)
		father->branch[whichSon] = root;

	/* attach the main root to gameTree */
	if (gameTree == NULL)
		gameTree = root;

	/* how we got to this node */

	root->move = allocWrapper(malloc(STRING_SIZE * sizeof(char)), "malloc");

	if (curMove != NULL)
		strcpy(root->move, curMove);

	root->boardValue = 0; /* for now, minimax will change this */
	if (depth == 0)
		root->boardValue = boardScore(board, PlayerA); /* give actual value to leaves */
	root->branch = NULL; /* start with no kids */
	root->childAmount = 0;

	if (amountOfMoves == 0) {
		if (currentColor == user_color)
		if (minimaxDepth == -1)
			root->boardValue = 10000 + depth;
		else
			root->boardValue = SCORE_K + depth;
		else
		if (minimaxDepth == -1)
			root->boardValue = -10000;
		else
			root->boardValue = -SCORE_K;

	}
	if (nodeNum >= MAX_NODES && minimaxDepth == -1){
		freeRepository(rep);
		return 0;
	}

	/* if there are more moves to be added and room in the tree */
	if ((depth > 0) && (amountOfMoves > 0)){

		root->branch = allocWrapper(calloc(amountOfMoves, sizeof(treeNode *)), "calloc");
		/* Initialize all children to null as default */
		for (i = 0; i < amountOfMoves; i++)
			root->branch[i] = NULL;
		root->childAmount = amountOfMoves;
		root->boardValue = boardScore(board, PlayerA);

		/* if there is only one option */
		if ((depth == initDepth) && (amountOfMoves == 1)){
			son = allocWrapper(calloc(1, sizeof(treeNode)), "calloc");
			son->move = allocWrapper(malloc(STRING_SIZE * sizeof(char)), "malloc");
			strcpy(son->move, rep->legalMoves[0]);
			son->boardValue = INF;
			root->branch[0] = son;
			freeRepository(rep);
			return INF;
		}


		for (i = 0; i < amountOfMoves; i++) {

			simulateTurn(board, rep->legalMoves[i], nextBoard);


			/* recursive call*/
			temp = getBestScoreBybuildingGameTree(nextBoard, depth - 1, PlayerA, nextColor, rep->legalMoves[i],
				root, i, alpha, beta, !maximizingPlayer);


			if (maximizingPlayer){
				alpha = MAX(alpha, temp);
				root->boardValue = alpha;
				if (beta <= alpha)
					break;
			}
			else{
				beta = MIN(beta, temp);
				root->boardValue = beta;
				if (beta <= alpha)
					break;
			}
		}

		freeRepository(rep);
		if (maximizingPlayer)
			return alpha;

		return beta;
	}
	/* if there are no more moves or we got to depth 0 */
	freeRepository(rep);
	return root->boardValue;

}




/* gets a tree and a best value , and returns a move from the first level of moves with this value */
/* (if randomly = 1 then a move will be chosen randomly, o.w the first move found will be returned) */
char* chooseMove(int bestValue, treeNode *tree, int randomly){

	int i = 0, countBestMoves = 0;
	treeNode *child;
	randomly = 0;
	while (i < tree->childAmount){
		child = tree->branch[i++];

		if (child->boardValue == bestValue){
			if (!randomly){
				return child->move;
			}
			/* counting how many options we got to choose from*/
			countBestMoves++;
		}
	}


	/* should never get here */
	fprintf(stderr, "Error in choosing computer move!\n");
	return NULL;
}







/* ****************************************************************************** */
/*                                   GUI GUI GUI                                  */
/* ****************************************************************************** */

void guiEnterGameState(char board[BOARD_SIZE][BOARD_SIZE], color nextColor)
{
	guiSetForGameState(board, nextColor, 1);
	if (LOAD != 1) /* starting new game */
		game_state(board, nextColor);
	else
	{ /* LOAD == 1 */
		LOAD = 0;
		game_state(board, nextColor);/* when loading we always load users turn */
	}
}


void guiSetupState(char board[BOARD_SIZE][BOARD_SIZE], color nextColor)
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i< 2; i++){
		for (j = 0; j < 6; j++){
			clip[k].x = j * 60;
			clip[k].y = i * 60;
			clip[k].w = 60;
			clip[k].h = 60;
			k++;
		}
	}

	int removeSelected = 1;
	SDL_Surface *backgroud = NULL;
	SDL_Surface *btnAdd = NULL;
	SDL_Surface *btnRemove = NULL;
	SDL_Surface *btnDone = NULL;
	backgroud = load_image("defaultBackground.png", 0);
	btnAdd = load_image("Buttons/btnAdd.png", 0);
	btnRemove = load_image("Buttons/btnRemoveSelected.png", 0);
	btnDone = load_image("Buttons/btnDone.png", 0);
	apply_surface(0, 0, backgroud, screen, NULL);
	apply_surface(150, 600, btnAdd, screen, NULL);
	apply_surface(280, 600, btnRemove, screen, NULL);
	apply_surface(215, 670, btnDone, screen, NULL);
	SDL_FreeSurface(backgroud);
	SDL_FreeSurface(btnAdd);
	SDL_FreeSurface(btnRemove);
	SDL_FreeSurface(btnDone);

	setBoard(board, 0, setupBoardX, setupBoardY);

	char piecesType [12]= { WHITE_P, WHITE_B, WHITE_R, WHITE_N, WHITE_Q, WHITE_K, BLACK_P, BLACK_B, BLACK_R, BLACK_N, BLACK_Q, BLACK_K };
	int countPiecesByType[12];
	initializePiecesTypeCounterArray(board, countPiecesByType);

	int done = 0;
	SDL_Event event;
	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT:
					done = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
				{
					int x = event.motion.x;
					int y = event.motion.y;
					if ((x >= setupBoardX) && (x <= setupBoardX + 500) && (y >= setupBoardY) && (y <= setupBoardY + 500)) //inside the board
					{
						int i = (x - setupBoardX) / 60;
						int j = 7 - ((y - setupBoardY) / 60);
						if (removeSelected) //remove mode
						{
							if (board[i][j] != EMPTY)
							{
								countPiecesByType[getIndexOfPieceTypeInCounterArray(board[i][j], piecesType)]--;
								board[i][j] = EMPTY;
								setBoard(board, 0, setupBoardX, setupBoardY);
							}
						}
						else //add mode
						{
							if ((countPiecesByType[0] == 8) && (countPiecesByType[1] == 2) &&
								(countPiecesByType[2] == 2) && (countPiecesByType[3] == 2) &&
								(countPiecesByType[4] == 1) && (countPiecesByType[5] == 1) &&
								(countPiecesByType[6] == 8) && (countPiecesByType[7] == 2) &&
								(countPiecesByType[8] == 2) && (countPiecesByType[9] == 2) &&
								(countPiecesByType[10] == 1) && (countPiecesByType[11] == 1))
								break;
							int pieceTypeIndex = getIndexOfPieceTypeInCounterArray(board[i][j], piecesType);
							if (pieceTypeIndex == 11)
								pieceTypeIndex = 0;
							else
								pieceTypeIndex++;
							while (!addPieceFromTypeCounterArray(countPiecesByType, piecesType[pieceTypeIndex]))
							{
								if (pieceTypeIndex == 11)
									pieceTypeIndex = 0;
								else
									pieceTypeIndex++;
							}
							if (board[i][j]!=EMPTY)
								countPiecesByType[getIndexOfPieceTypeInCounterArray(board[i][j], piecesType)]--;
							board[i][j] = piecesType[pieceTypeIndex];
							setBoard(board, 0, setupBoardX, setupBoardY);
						}
					}
					if ((x >= 150) && (x <= 250) && (y >= 600) && (y <= 650)) //addButton Pressed
					{
						SDL_Surface *btnAdd = NULL;
						SDL_Surface *btnRemove = NULL;
						btnAdd = load_image("Buttons/btnAddSelected.png", 0);
						btnRemove = load_image("Buttons/btnRemove.png", 0);
						apply_surface(150, 600, btnAdd, screen, NULL);
						apply_surface(280, 600, btnRemove, screen, NULL);
						SDL_FreeSurface(btnAdd);
						SDL_FreeSurface(btnRemove);
						removeSelected = 0;
						refresh();
					}
					if ((x >= 280) && (x <= 380) && (y >= 600) && (y <= 650)) //addButton Pressed
					{
						SDL_Surface *btnAdd = NULL;
						SDL_Surface *btnRemove = NULL;
						btnAdd = load_image("Buttons/btnAdd.png", 0);
						btnRemove = load_image("Buttons/btnRemoveSelected.png", 0);
						apply_surface(150, 600, btnAdd, screen, NULL);
						apply_surface(280, 600, btnRemove, screen, NULL);
						SDL_FreeSurface(btnAdd);
						SDL_FreeSurface(btnRemove);
						removeSelected = 1;
						refresh();
					}
					if ((x >= 215) && (x <= 315) && (y >= 670) && (y <= 720)) //btnDone Pressed
					{
						//check if white and black king are present
						if ((countPiecesByType[getIndexOfPieceTypeInCounterArray(WHITE_K, piecesType)] == 0)
							|| (countPiecesByType[getIndexOfPieceTypeInCounterArray(BLACK_K, piecesType)] == 0))
						{

						}
						else
							guiEnterGameState(board, nextColor, 0);
					}
					break;
				}

				default:
					break;
			}
		}
	}

}



void guiSetForGameState(char board[BOARD_SIZE][BOARD_SIZE], color nextColor, int needsSpriteInitialization)
{
	if (needsSpriteInitialization) //enters if didn't go through setup state
	{
		int i = 0, j = 0, k = 0;
		/* set up game piece clip */
		for (i = 0; i< 2; i++){
			for (j = 0; j < 6; j++){
				clip[k].x = j * 60;
				clip[k].y = i * 60;
				clip[k].w = 60;
				clip[k].h = 60;
				k++;
			}
		}
	}

	SDL_Surface *backgroud = NULL;
	SDL_Surface *btnSave = NULL;
	SDL_Surface *btnGetBestMove = NULL;
	SDL_Surface *btnMainMenu = NULL;
	SDL_Surface *btnQuit = NULL;
	backgroud = load_image("defaultBackground.png", 0);
	btnSave = load_image("Buttons/btnSave.png", 0);
	btnGetBestMove = load_image("Buttons/btnGetBestMove.png", 0);
	btnMainMenu = load_image("Buttons/btnMainMenu.png", 0);
	btnQuit = load_image("Buttons/btnQuitSmall.png", 0);
	apply_surface(0, 0, backgroud, screen, NULL);
	apply_surface(550, 100, btnSave, screen, NULL);
	apply_surface(550, 170, btnGetBestMove, screen, NULL);
	apply_surface(550, 240, btnMainMenu, screen, NULL);
	apply_surface(550, 310, btnQuit, screen, NULL);
	SDL_FreeSurface(backgroud);
	SDL_FreeSurface(btnSave);
	SDL_FreeSurface(btnGetBestMove);
	SDL_FreeSurface(btnMainMenu);
	SDL_FreeSurface(btnQuit);

	setBoard(board, 0,gameBoardX,gameBoardY);


}




void guiSetMainStage(char board[BOARD_SIZE][BOARD_SIZE])
{
	/* surfaces: */

	SDL_Surface *background = NULL;
	SDL_Surface *quit = NULL;
	SDL_Surface *guiBoard = NULL;
	SDL_Surface *startGame = NULL;



	/* Load up board image */
	quit = load_image("quit.bmp", 0);
	background = load_image("mainBackground1.png", 0);
	guiBoard = load_image("board.png", 0);


	/* Apply images to screen */
	apply_surface(0, 0, background, screen, NULL);
	apply_surface(0, 0, guiBoard, screen, NULL);
	apply_surface(500, 700, quit, screen, NULL);

	setBoard(board, 0,0,0); /* convert cmd board to GUI board */

	refresh();

	/* free loaded images */
	SDL_FreeSurface(guiBoard);
	SDL_FreeSurface(background);
	SDL_FreeSurface(quit);
		

}

SDL_Surface *load_image(char *filename, int transparent)
{
	/* Temporary storage for the image that's loaded */
	SDL_Surface* loadedImage = NULL;

	/* The optimized image that will be used */
	SDL_Surface* optimizedImage = NULL;


	/* Load the image */
	loadedImage = IMG_Load( filename );
	if (loadedImage == NULL) 
	{
		printf("ERROR: failed to load image: %s\n", SDL_GetError());
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}
	/* Create an optimized image */
	optimizedImage = SDL_DisplayFormat(loadedImage);
	if (optimizedImage == NULL){
		printf("ERROR: failed to format image: %s\n", SDL_GetError());
		SDL_FreeSurface(loadedImage);
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}

	/* set all white pixels to transparent, if transparent == 1 */
	if (transparent == 1){
		SDL_SetColorKey(optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB(optimizedImage->format, 0xFF, 0, 0xFF));

	}
	/* Free the old image */
	SDL_FreeSurface(loadedImage);

	return optimizedImage;

}

void apply_surface(int x, int  y, SDL_Surface *source, SDL_Surface *destination, SDL_Rect* clip){

	/* make temporary rectangle to hold the offsets */
	SDL_Rect offset;

	/* give offset to the rectangle */
	offset.x = x;
	offset.y = y;

	/* Blit the surface */
	SDL_BlitSurface(source, clip, destination, &offset);

}

void initWindow(){


	/*  start SDL */
	if (SDL_Init(SDL_INIT_VIDEO) == -1){
		printf("ERROR: unable to init SDL: %s\n", SDL_GetError());
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}

	/* set up screen */
	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if (screen == NULL){
		printf("ERROR: failed to set video mode: %s\n", SDL_GetError());
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}



}

//TODO: change name and change implementation
void setBoard(char board[BOARD_SIZE][BOARD_SIZE], int piecesOnly, int x, int y)
{
	int i, j;
	SDL_Surface *guiBoard = NULL;

	guiBoard = load_image("board.png", 0);
	tools = load_image("sprites3.png", 1);
	if (!piecesOnly)
		apply_surface(x, y, guiBoard, screen, NULL);

	for (i = 0; i< BOARD_SIZE; i++){
		for (j = 0; j< BOARD_SIZE; j++){

			if (board[i][j] == WHITE_B)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[8]);
			else if (board[i][j] == WHITE_R)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[10]);
			else if (board[i][j] == WHITE_K)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[6]);
			else if (board[i][j] == WHITE_N)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[9]);
			else if (board[i][j] == WHITE_P)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[11]);
			else if (board[i][j] == WHITE_Q)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[7]);

			else if (board[i][j] == BLACK_B)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[2]);
			else if (board[i][j] == BLACK_R)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[4]);
			else if (board[i][j] == BLACK_K)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[0]);
			else if (board[i][j] == BLACK_N)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[3]);
			else if (board[i][j] == BLACK_P)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[5]);
			else if (board[i][j] == BLACK_Q)
				apply_surface(x+i * 60, y+(7 - j) * 60, tools, screen, &clip[1]);
			continue;

		}
	}
	refresh();
	SDL_FreeSurface(guiBoard);
	SDL_FreeSurface(tools);

}

/* main screen
*
* TITLE
* NEW GAME
* LOAD GAME
* QUIT
*
*/
int guiMainMenu(char board[BOARD_SIZE][BOARD_SIZE], color nextPlayer)
{
	//declare local variables
	int x, y;
	SDL_Event event;
	SDL_Surface *background = NULL;
	SDL_Surface *btnNewGame = NULL;
	SDL_Surface *btnLoadGame = NULL;
	SDL_Surface *btnQuit = NULL;
	int btnNewGameInverted = 0;
	int btnLoadGameInverted = 0;
	int btnQuitInverted = 0;
	initWindow();

	SDL_WM_SetCaption("Chess Game", NULL);

	background = load_image("defaultBackground.png", 0);
	btnNewGame = load_image("Buttons/btnNewGame.png", 0);
	btnLoadGame = load_image("Buttons/btnLoadGame.png", 0);
	btnQuit = load_image("Buttons/btnQuitBig.png", 0);
	apply_surface(0, 0, background, screen, NULL);
	apply_surface(150, 150, btnNewGame, screen, NULL);
	apply_surface(150, 350, btnLoadGame, screen, NULL);
	apply_surface(150, 550, btnQuit, screen, NULL);
	SDL_FreeSurface(background);
	SDL_FreeSurface(btnNewGame);
	SDL_FreeSurface(btnLoadGame);
	SDL_FreeSurface(btnQuit);

	refresh();

	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT: 
				done = 1;
				break;
			case SDL_MOUSEMOTION:
				x = event.motion.x;
				y = event.motion.y;
				if ((x >= 150) && (x<=650) && (y>=150) && (y <= 250))
				{
					if (!btnNewGameInverted)
					{
						btnNewGameInverted = 1;
						btnNewGame = load_image("Buttons/btnNewGameInverted.png", 0);
						apply_surface(150, 150, btnNewGame, screen, NULL);
						SDL_FreeSurface(btnNewGame);
						refresh();
					}
				}
				else if ((x >= 150) && (x<=650) && (y>=350) && (y <= 450))
				{
					if (!btnLoadGameInverted)
					{
						btnLoadGameInverted = 1;
						btnLoadGame = load_image("Buttons/btnLoadGameInverted.png", 0);
						apply_surface(150, 350, btnLoadGame, screen, NULL);
						SDL_FreeSurface(btnLoadGame);
						refresh();
					}
				}
				else if ((x >= 150) && (x<=650) && (y>=550) && (y <= 650))
				{
					if (!btnQuitInverted)
					{
						btnQuitInverted = 1;
						btnQuit = load_image("Buttons/btnQuitInvertedBig.png", 0);
						apply_surface(150, 550, btnQuit, screen, NULL);
						SDL_FreeSurface(btnQuit);
						refresh();
					}
				}
				else
				{
					if (btnNewGameInverted)
					{
						btnNewGameInverted = 0;
						btnNewGame = load_image("Buttons/btnNewGame.png", 0);
						apply_surface(150, 150, btnNewGame, screen, NULL);
						SDL_FreeSurface(btnNewGame);
						refresh();
					}
					else if (btnLoadGameInverted)
					{
						btnLoadGameInverted = 0;
						btnLoadGame = load_image("Buttons/btnLoadGame.png", 0);
						apply_surface(150, 350, btnLoadGame, screen, NULL);
						SDL_FreeSurface(btnLoadGame);
						refresh();
					}
					else if (btnQuitInverted)
					{
						btnQuitInverted = 0;
						btnQuit = load_image("Buttons/btnQuitBig.png", 0);
						apply_surface(150, 550, btnQuit, screen, NULL);
						SDL_FreeSurface(btnQuit);
						refresh();
					}
				}
				break;
			case SDL_MOUSEBUTTONUP:
				x = event.motion.x;
				y = event.motion.y;
				if ((x >= 150) && (x <= 650) && (y >= 550) && (y <= 650)) //Quit button pressed
				{
					done = 1;
				}
				else if ((x >= 150) && (x <= 650) && (y >= 350) && (y <= 450)) //Load button pressed
				{
					guiLoadScreen(board, nextPlayer); 
					/* back from load or type screen, redraw splash screen */
					background = load_image("splashScreen.png", 0);
					apply_surface(0, 0, background, screen, NULL);
					SDL_FreeSurface(background);
					refresh();
				}
				else if ((x >= 150) && (x <= 650) && (y >= 150) && (y <= 250)) //Start button pressed
				{
					/* go to choose game type */
					guiChangeSettings(board, nextPlayer);
					/* back from load or type screen, redraw splash screen */
					background = load_image("splashScreen.png", 0);
					apply_surface(0, 0, background, screen, NULL);
					SDL_FreeSurface(background);
					refresh();
				}

				break;
			
			default:
				
				break;


			}


		}
	}

	/* Quit SDL */

	SDL_Quit();

	return 1;

}

void drawLoadScreen ()
{
	int i, height;
	SDL_Surface *background = NULL;
	SDL_Surface *title = NULL;
	SDL_Surface *loadSlot[7];
	for (int i = 0; i < 7; i++)
		loadSlot[i] = NULL;
	SDL_Surface *back = NULL;

	background = load_image("saveScreenBackground.png", 0);
	back = load_image("Buttons/btnBack.png", 0);
	title = load_image("Labels/lblSelectSlot.png", 0);
	apply_surface(100, 100, background, screen, NULL);
	apply_surface(120, 120, title, screen, NULL);
	apply_surface(110, 620, back, screen, NULL);

	loadSlot[0] = load_image("Buttons/btnSlot1.png", 0);
	apply_surface(300, 200, loadSlot[0], screen, NULL);
	SDL_FreeSurface(loadSlot[0]);
	loadSlot[1] = load_image("Buttons/btnSlot2.png", 0);
	apply_surface(300, 200 + 70, loadSlot[1], screen, NULL);
	SDL_FreeSurface(loadSlot[1]);
	loadSlot[2] = load_image("Buttons/btnSlot3.png", 0);
	apply_surface(300, 200 + 70 * 2, loadSlot[2], screen, NULL);
	SDL_FreeSurface(loadSlot[2]);
	loadSlot[3] = load_image("Buttons/btnSlot4.png", 0);
	apply_surface(300, 200 + 70 * 3, loadSlot[3], screen, NULL);
	SDL_FreeSurface(loadSlot[3]);
	loadSlot[4] = load_image("Buttons/btnSlot5.png", 0);
	apply_surface(300, 200 + 70 * 4, loadSlot[4], screen, NULL);
	SDL_FreeSurface(loadSlot[4]);
	loadSlot[5] = load_image("Buttons/btnSlot6.png", 0);
	apply_surface(300, 200 + 70 * 5, loadSlot[5], screen, NULL);
	SDL_FreeSurface(loadSlot[5]);
	loadSlot[6] = load_image("Buttons/btnSlot7.png", 0);
	apply_surface(300, 200 + 70 * 6, loadSlot[6], screen, NULL);
	SDL_FreeSurface(loadSlot[6]);

	SDL_FreeSurface(background);
	SDL_FreeSurface(back);

	refresh();
}


void guiLoadScreen(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color_pointer){

	int  x, y, i = 0, height, temp;
	SDL_Event event;
	drawLoadScreen();
	while (!done)
	{	
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT: 
					done = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					x = event.motion.x;
					y = event.motion.y;
					for (i = 0; i < 7; i++)
					{
						for (i = 0; i < 7; i++)
						{
							if ((x >= 300) && (x <= 400) && (y >= 200 + 70 * i) && (y <= 250 + 70 * i)) //save slot pressed
							{
								loadState(board, i + 1, current_player_color_pointer);
								drawLoadScreen();
								return (i + 1);
							}
						}
						if ((x >= 110) && (x <= 160) && (y >= 620) && (y <= 670)) //btnBackPressed
							return;
					}
					break;

			default:
				break;

			}


		}
	}


}



/* gui, choosing load state */
void loadState(char board[BOARD_SIZE][BOARD_SIZE], int state, color current_player_color_pointer){


	SDL_Surface *popUpError = NULL;
	char fName[11];
	fName[10] = '\0';
	/* choose what file we want to open out of the 5 */
	switch (state)
	{
		case 1: strcpy(fName, "state1.xml"); break;
		case 2: strcpy(fName, "state2.xml"); break;
		case 3: strcpy(fName, "state3.xml"); break;
		case 4: strcpy(fName, "state4.xml"); break;
		case 5: strcpy(fName, "state5.xml"); break;
		case 6: strcpy(fName, "state6.xml"); break;
		case 7: strcpy(fName, "state7.xml"); break;
	}



	if (loadGame(board, fName, current_player_color_pointer) == 0)
	{
		/* error handling, incase we choose a non existing file */
		popUpError = load_image("Labels\lblNotValidFile.png", 0);
		apply_surface(300, 350, popUpError, screen, NULL);
		SDL_FreeSurface(popUpError);

		/*update screen */
		refresh(); /* file error has happened */

		/* Pause */
		SDL_Delay(1000);


	}
	else
	{


		LOAD = 1; /* we are starting game from load point, use correct player color, not white */

		guiEnterGameState(board, current_player_color_pointer);
	}



	return;


}

/* gui for settings in game */
void guiSettingsState(char board[BOARD_SIZE][BOARD_SIZE]){

	int x, y;
	SDL_Surface *check50 = NULL;
	SDL_Surface *check100 = NULL;
	SDL_Surface *settingsCover1 = NULL;
	SDL_Surface *settingsCover2 = NULL;
	SDL_Event event;


	SDL_Surface *settingsScreen = NULL;

	/* load images for this screen */
	check50 = load_image("check50.png", 1);
	check100 = load_image("check100.png", 1);
	settingsScreen = load_image("settingsScreen.png", 0);
	settingsCover1 = load_image("settingsCover1.png", 0);
	settingsCover2 = load_image("settingsCover2.png", 0);
	/* set up screen */

	apply_surface(0, 0, settingsScreen, screen, NULL);

	/*update screen */
	refresh();

	while (!done){


		while (SDL_PollEvent(&event)){

			switch (event.type){

			case SDL_QUIT: done = 1;

				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x;
				y = event.motion.y;

				if ((x >= 50) && (x <= 100) && (y <= 350) && (y >= 300)){
					/* difficulty 1 */
					minimaxDepth = 1;
					initDepth = minimaxDepth;
					/* tell the user what they just did */
					apply_surface(50, 300, settingsCover1, screen, NULL);
					apply_surface(50, 300, check50, screen, NULL);
					refresh();

				}
				else if ((x >= 150) && (x <= 200) && (y <= 350) && (y >= 300)){

					/* difficulty 2 */
					minimaxDepth = 2;
					initDepth = minimaxDepth;
					/* tell the user what they just did */
					apply_surface(50, 300, settingsCover1, screen, NULL);
					apply_surface(150, 300, check50, screen, NULL);						
					refresh();

				}
				else if ((x >= 250) && (x <= 300) && (y <= 350) && (y >= 300)){
					/* difficulty 3 */
					minimaxDepth = 3;
					initDepth = minimaxDepth;
					/* tell the user what they just did */
					apply_surface(50, 300, settingsCover1, screen, NULL);
					apply_surface(250, 300, check50, screen, NULL);
					refresh();

				}
				else if ((x >= 350) && (x <= 400) && (y <= 350) && (y >= 300)){
					/* difficulty 4 */
					minimaxDepth = 4;
					initDepth = minimaxDepth;
					/* tell the user what they just did */
					apply_surface(50, 300, settingsCover1, screen, NULL);
					apply_surface(350, 300, check50, screen, NULL);
					refresh();

				}
				else if ((x >= 150) && (x <= 300) && (y <= 450) && (y >= 400)){
					/* difficulty best */
					minimaxDepth = -1;
					/* tell the user what they just did */
					apply_surface(50, 300, settingsCover1, screen, NULL);
					apply_surface(200, 400, check50, screen, NULL);
					refresh();

				}
				else if ((x >= 100) && (x <= 350) && (y <= 550) && (y >= 500)){
					/* start game */

					SDL_FreeSurface(check50);
					SDL_FreeSurface(check100);
					SDL_FreeSurface(settingsScreen);
					SDL_FreeSurface(settingsCover1);
					SDL_FreeSurface(settingsCover2);
					guiSetForGameState(board, WHITE, 1);
				}
				else if ((x >= 150) && (x <= 250) && (y <= 750) && (y >= 700)){
					/* back to main splash screen */
					goto JUMP2;
				}
				else if ((x >= 450) && (x <= 550) && (y <= 350) && (y >= 300)){
					/* choose white color */
					user_color = WHITE;
					/* tell the user what they just did */
					apply_surface(450, 300, settingsCover2, screen, NULL);
					apply_surface(450, 300, check100, screen, NULL);
					refresh();


				}
				else if ((x >= 650) && (x <= 750) && (y <= 350) && (y >= 300)){
					/* choose black color */
					user_color = BLACK;
					/* tell the user what they just did */
					apply_surface(450, 300, settingsCover2, screen, NULL);
					apply_surface(650, 300, check100, screen, NULL);
					refresh();


				}

				break;

			default:
				break;

			}


		}
	}

JUMP2:
	SDL_FreeSurface(check50);
	SDL_FreeSurface(check100);
	SDL_FreeSurface(settingsScreen);
	SDL_FreeSurface(settingsCover1);
	SDL_FreeSurface(settingsCover2);

	return;


}





/* flip screen */
void refresh(){
	if (SDL_Flip(screen) == -1){
		printf("ERROR: failed to flip buffer: %s\n", SDL_GetError());
		freeAllRepositories();
		freeTree(gameTree);
		exit(0);
	}

}

//displays on screen the game settings according to the input integer
void displayGameSettings()
{
	if (game_mode == 2)
	{
		//depth choice
		SDL_Surface *lblDepth = NULL;
		SDL_Surface *lblChooseDepth = NULL;
		SDL_Surface *btnChangeDepth = NULL;
		lblDepth = load_image("Labels/lblDepth1.png", 0);
		lblChooseDepth = load_image("Labels/lblChooseDepth.png", 0);
		btnChangeDepth = load_image("Buttons/btnChangeSetting.jpg", 1);
		apply_surface(10, 150, lblChooseDepth, screen, NULL);
		apply_surface(230, 150, lblDepth, screen, NULL);
		apply_surface(300, 150, btnChangeDepth, screen, NULL);
		SDL_FreeSurface(lblDepth);
		SDL_FreeSurface(lblChooseDepth);
		SDL_FreeSurface(btnChangeDepth);
		//user color
		SDL_Surface *lblUserColor = NULL;
		SDL_Surface *lblChooseUserColor = NULL;
		SDL_Surface *btnChangeUserColor = NULL;
		lblUserColor = load_image("Labels/lblWhiteColor.png", 0);
		lblChooseUserColor = load_image("Labels/lblChangeUserColor.png", 0);
		btnChangeUserColor = load_image("Buttons/btnChangeSetting.jpg", 1);
		apply_surface(10, 220, lblChooseUserColor, screen, NULL);
		apply_surface(230, 220, lblUserColor, screen, NULL);
		apply_surface(350, 220, btnChangeUserColor, screen, NULL);
		SDL_FreeSurface(lblChooseUserColor);
		SDL_FreeSurface(lblUserColor);
		SDL_FreeSurface(btnChangeUserColor);
	}
	//do next player
	SDL_Surface *lblChooseNextPlayerColor = NULL;
	SDL_Surface *lblNextPlayerColor = NULL;
	SDL_Surface *btnChangeNextColor = NULL;

	SDL_Surface *lblSetup = NULL;
	SDL_Surface *lblSetupAnswer = NULL;
	SDL_Surface *btnChangeSetup = NULL;

	lblNextPlayerColor = load_image("Labels/lblWhiteColor.png", 0);
	lblChooseNextPlayerColor = load_image("Labels/lblChooseNextPlayer.png", 0);
	btnChangeNextColor = load_image("Buttons/btnChangeSetting.jpg", 1);

	lblSetupAnswer = load_image("Labels/lblYes.png", 0);
	lblSetup = load_image("Labels/lblSetup.png", 0);
	btnChangeSetup = load_image("Buttons/btnChangeSetting.jpg", 1);

	if (game_mode == 2)
	{
		apply_surface(10, 300, lblChooseNextPlayerColor, screen, NULL);
		apply_surface(280, 300, lblNextPlayerColor, screen, NULL);
		apply_surface(400, 300, btnChangeNextColor, screen, NULL);

		apply_surface(10, 370, lblSetup, screen, NULL);
		apply_surface(140, 370, lblSetupAnswer, screen, NULL);
		apply_surface(210, 370, btnChangeSetup, screen, NULL);

	}
	else
	{
		apply_surface(10, 150, lblChooseNextPlayerColor, screen, NULL);
		apply_surface(280, 150, lblNextPlayerColor, screen, NULL);
		apply_surface(400, 150, btnChangeNextColor, screen, NULL);

		apply_surface(10, 220, lblSetup, screen, NULL);
		apply_surface(140, 220, lblSetupAnswer, screen, NULL);
		apply_surface(210, 220, btnChangeSetup, screen, NULL);

	}
	SDL_FreeSurface(lblChooseNextPlayerColor);
	SDL_FreeSurface(lblNextPlayerColor);
	SDL_FreeSurface(btnChangeNextColor);

	SDL_FreeSurface(lblSetupAnswer);
	SDL_FreeSurface(lblSetup);
	SDL_FreeSurface(btnChangeSetup);
}

void displaySettingsScreenBase()
{
	SDL_Surface *lblTitle = NULL;
	SDL_Surface *background = NULL;
	SDL_Surface *btnBack = NULL;
	SDL_Surface *btnSwitchMode = NULL;
	SDL_Surface *lblGameMode = NULL;
	SDL_Surface *btnStart = NULL;

	/* load images for this screen */
	lblTitle = load_image("Labels/lblChooseGameType.png", 0);
	background = load_image("defaultBackground.png", 0);
	btnBack = load_image("Buttons/btnBack.png", 1);
	btnSwitchMode = load_image("Buttons/btnSwitchMode.png", 1);
	if (game_mode==2)
		lblGameMode = load_image("Labels/lblSinglePlayer.png", 0);
	else
		lblGameMode = load_image("Labels/lblTwoPlayers.png", 0);
	btnStart = load_image("Buttons/btnStart.png", 0);

	/* set up screen */
	apply_surface(0, 0, background, screen, NULL);
	apply_surface(20, 730, btnBack, screen, NULL);
	apply_surface(230, 70, lblGameMode, screen, NULL);
	apply_surface(450, 70, btnSwitchMode, screen, NULL);
	apply_surface(10, 10, lblTitle, screen, NULL);
	apply_surface(300, 730, btnStart, screen, NULL);

	SDL_FreeSurface(btnBack);
	SDL_FreeSurface(lblGameMode);
	SDL_FreeSurface(lblTitle);
	SDL_FreeSurface(background);
	SDL_FreeSurface(btnStart);

	displayGameSettings();
}

/* gui for choosing amount of players */
void guiChangeSettings(char board[BOARD_SIZE][BOARD_SIZE], color nextColor)
{
	int x, y;
	int enterSetupState = 1;
	nextColor=WHITE;
	minimaxDepth = 1;
	user_color = WHITE;
	SDL_Event event;
	game_mode = 2;
	displaySettingsScreenBase();

	refresh();

	init_board_(board);

	/*update screen */
	refresh();
	while (!done)
	{
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_QUIT: done = 1;
					break;
				case SDL_MOUSEBUTTONDOWN:
					x = event.motion.x;
					y = event.motion.y;

					if ((x >= 450) && (x <= 565) && (y >= 70) && (y <= 120)) //btnSwitchMode
					{
						if (game_mode==1)
							game_mode = 2;
						else
							game_mode = 1;
						nextColor = WHITE;
						minimaxDepth = 1;
						user_color = WHITE;
						enterSetupState = 1;
						displaySettingsScreenBase();
						refresh();
					}

					if (game_mode == 2)
					{
						if ((x >= 300) && (x <= 350) && (y >= 150) && (y <= 200)) //btnChangeDepth
						{
							char *lblDepthPath = "";
							switch (minimaxDepth)
							{
								case -1:
									minimaxDepth = 1;
									lblDepthPath = "Labels/lblDepth1.png";
									break;
								case 1:
									minimaxDepth = 2;
									lblDepthPath = "Labels/lblDepth2.png";
									break;
								case 2:
									minimaxDepth = 3;
									lblDepthPath = "Labels/lblDepth3.png";
									break;
								case 3:
									minimaxDepth = 4;
									lblDepthPath = "Labels/lblDepth4.png";
									break;
								case 4:
									minimaxDepth = -1;
									lblDepthPath = "Labels/lblDepthBest.png";
									break;
								default:
									break;
							}
							SDL_Surface *lblDepth = NULL;
							lblDepth = load_image(lblDepthPath, 0);
							apply_surface(230, 150, lblDepth, screen, NULL);
							SDL_FreeSurface(lblDepth);
							refresh();
						}
						if ((x >= 350) && (x <= 400) && (y >= 220) && (y <= 270)) //btnChangeUserColor
						{
							SDL_Surface *lblUserColor = NULL;
							if (user_color == WHITE)
							{
								user_color = BLACK;
								lblUserColor = load_image("Labels/lblBlackColor.png", 0);
							}
							else 
							{
								user_color = WHITE;
								lblUserColor = load_image("Labels/lblWhiteColor.png", 0);
							}
							apply_surface(230, 220, lblUserColor, screen, NULL);
							SDL_FreeSurface(lblUserColor);
							refresh();
						}
						if ((x >= 400) && (x <= 450) && (y >= 300) && (y <= 350)) //btnChangeNextColor
						{
							SDL_Surface *lblNextColor = NULL;
							if (nextColor == WHITE)
							{
								nextColor = BLACK;
								lblNextColor = load_image("Labels/lblBlackColor.png", 0);
							}
							else
							{
								nextColor = WHITE;
								lblNextColor = load_image("Labels/lblWhiteColor.png", 0);
							}
							apply_surface(280, 300, lblNextColor, screen, NULL);
							SDL_FreeSurface(lblNextColor);
							refresh();
						}
						if ((x >= 210) && (x <= 260) && (y >= 370) && (y <= 420)) //btnChangeSetup
						{
							SDL_Surface *lblSetup = NULL;
							if (enterSetupState)
							{
								enterSetupState = 0;
								lblSetup = load_image("Labels/lblNo.png", 0);
							}
							else
							{
								enterSetupState = 1;
								lblSetup = load_image("Labels/lblYes.png", 0);
							}
							apply_surface(140, 370, lblSetup, screen, NULL);
							SDL_FreeSurface(lblSetup);
							refresh();
						}
					}
					else //2 player mode
					{
						if ((x >= 400) && (x <= 450) && (y >= 150) && (y <= 200)) //btnChangeNextColor
						{
							SDL_Surface *lblNextColor = NULL;
							if (nextColor == WHITE)
							{
								nextColor = BLACK;
								lblNextColor = load_image("Labels/lblBlackColor.png", 0);
							}
							else
							{
								nextColor = WHITE;
								lblNextColor = load_image("Labels/lblWhiteColor.png", 0);
							}
							apply_surface(280, 150, lblNextColor, screen, NULL);
							SDL_FreeSurface(lblNextColor);
							refresh();
						}
						if ((x >= 210) && (x <= 260) && (y >= 220) && (y <= 270)) //btnChangeSetup
						{
							SDL_Surface *lblSetup = NULL;
							if (enterSetupState)
							{
								enterSetupState = 0;
								lblSetup = load_image("Labels/lblNo.png", 0);
							}
							else
							{
								enterSetupState = 1;
								lblSetup = load_image("Labels/lblYes.png", 0);
							}
							apply_surface(140, 220, lblSetup, screen, NULL);
							SDL_FreeSurface(lblSetup);
							refresh();
						}
					}
					if ((x >= 20) && (x <= 70) && (y >= 730) && (y <= 780)) //btnBack
					{
						guiMainMenu(board, nextColor);
					}
					if ((x >= 300) && (x <= 400) && (y >= 730) && (y <= 780)) //btnStart
					{
						if (enterSetupState == 1)
							guiSetupState(board, user_color, nextColor);
						else
							guiEnterGameState(board, user_color, 1);
					}
				default:
					break;
				}

		}
	}

JUMP3:


	return;

}



/* leaf gui function, prints out winner */
void guiWinMessage(color c, char board[BOARD_SIZE][BOARD_SIZE], color current_player_color){

	int x, y;

	SDL_Event event; /* event handling */
	SDL_Surface *blackWinner = NULL;
	SDL_Surface *whiteWinner = NULL;

	blackWinner = load_image("blackWinner.bmp", 0);
	whiteWinner = load_image("whiteWinner.bmp", 0);


	if (c == BLACK)
		apply_surface(50, 550, blackWinner, screen, NULL);
	else
		apply_surface(50, 550, whiteWinner, screen, NULL);

	SDL_FreeSurface(whiteWinner);
	SDL_FreeSurface(blackWinner);

	/*update screen */
	refresh();

	while (!done){
		
		while (SDL_PollEvent(&event)){
			switch (event.type){
			case SDL_QUIT: done = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x;
				y = event.motion.y;

				if ((x >= 550) && (x <= 750) && (y <= 150) && (y >= 50)){


					/* go to main menu */

					SDL_Quit();
					freeAllRepositories();
					freeTree(gameTree);
					main(0, NULL); /*no need to reset GUI flag, already 1 */
					
					/*update screen */
					refresh();

				}
				else if ((x >= 550) && (x <= 750) && (y <= 300) && (y >= 200)){
					/* save game */
					/* get file name by giving user option to choose */
					saveState(board, current_player_color);
					guiSetMainStage(board,0);
					setMessageToUser(current_player_color,TURN_MESSAGE);
					/*setBoard(board, 0);*/

					blackWinner = load_image("blackWinner.bmp", 0);
					whiteWinner = load_image("whiteWinner.bmp", 0);


					if (c == BLACK)
						apply_surface(50, 550, blackWinner, screen, NULL);
					else
						apply_surface(50, 550, whiteWinner, screen, NULL);

					SDL_FreeSurface(whiteWinner);
					SDL_FreeSurface(blackWinner);


					/*update screen */
					refresh();

				}
				else if ((x >= 550) && (x <= 750) && (y <= 450) && (y >= 350)){
					/* restart game */

					freeAllRepositories();
					freeTree(gameTree);
					init_board_(board);
					setBoard(board, 0,0,0);

					game_state(board, WHITE);

					/*update screen */
					refresh();

				}
				else if ((x >= 500) && (x <= 700) && (y <= 750) && (y >= 700)){
					/* quit game */

					done = 1;
					freeAllRepositories(); /* possible way to exit program */
					freeTree(gameTree);

				}

				break;

			default:
				break;

			}

		}
	}

	SDL_Quit();


}


void saveState(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color){

	/* get slot choice from user */
	int slot = guiGetSaveSlot();
	char fName[11];
	fName[10] = '\0';

	/* choose what file we want to open out of the 5 */
	switch (slot)
	{
		case 1: strcpy(fName, "state1.xml"); break;
		case 2: strcpy(fName, "state2.xml"); break;
		case 3: strcpy(fName, "state3.xml"); break;
		case 4: strcpy(fName, "state4.xml"); break;
		case 5: strcpy(fName, "state5.xml"); break;
		case 6: strcpy(fName, "state6.xml"); break;
		case 7: strcpy(fName, "state7.xml"); break;

		case -1:setBoard(board, 0,0,0); return; /* back was clicked, dont save */
	}

	saveGame(board, fName, current_player_color);

}

void drawSaveScreen()
{
	int i, height;
	SDL_Surface *background = NULL;
	SDL_Surface *title = NULL;
	SDL_Surface *saveSlot[7];
	for (int i = 0; i < 7; i++)
		saveSlot[i] = NULL;
	SDL_Surface *back = NULL;

	background = load_image("saveScreenBackground.png", 0);
	back = load_image("Buttons/btnBack.png", 0);
	title = load_image("Labels/lblSelectSlot.png", 0);
	apply_surface(100, 100, background, screen, NULL);
	apply_surface(120, 120, title, screen, NULL);
	apply_surface(110, 620, back, screen, NULL);

	saveSlot[0] = load_image("Buttons/btnSlot1.png", 0);
	apply_surface(300, 200, saveSlot[0], screen, NULL);
	SDL_FreeSurface(saveSlot[0]);
	saveSlot[1] = load_image("Buttons/btnSlot2.png", 0);
	apply_surface(300, 200 + 70, saveSlot[1], screen, NULL);
	SDL_FreeSurface(saveSlot[1]);
	saveSlot[2] = load_image("Buttons/btnSlot3.png", 0);
	apply_surface(300, 200 + 70*2, saveSlot[2], screen, NULL);
	SDL_FreeSurface(saveSlot[2]);
	saveSlot[3] = load_image("Buttons/btnSlot4.png", 0);
	apply_surface(300, 200 + 70*3, saveSlot[3], screen, NULL);
	SDL_FreeSurface(saveSlot[3]);
	saveSlot[4] = load_image("Buttons/btnSlot5.png", 0);
	apply_surface(300, 200 + 70*4, saveSlot[4], screen, NULL);
	SDL_FreeSurface(saveSlot[4]);
	saveSlot[5] = load_image("Buttons/btnSlot6.png", 0);
	apply_surface(300, 200 + 70*5, saveSlot[5], screen, NULL);
	SDL_FreeSurface(saveSlot[5]);
	saveSlot[6] = load_image("Buttons/btnSlot7.png", 0);
	apply_surface(300, 200 + 70*6, saveSlot[6], screen, NULL);
	SDL_FreeSurface(saveSlot[6]);

	SDL_FreeSurface(background);
	SDL_FreeSurface(back);
	refresh();
}

int guiGetSaveSlot()
{
	SDL_Surface *saveSuccessful = NULL;
	SDL_Event event;
	int x, y, i = 0, height, temp;

	drawSaveScreen();

	/*update screen */
	refresh();

	while (!done){

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_QUIT:
				done = 1;
				SDL_Quit();
				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x;
				y = event.motion.y;
				for (i = 0; i < 7; i++)
				{
					if ((x >= 300) && (x <= 400) && (y >= 200 + 70 * i) && (y <= 250 + 70 * i)) //save slot pressed
					{
						return (i+1);
					}
				}
				if ((x >= 110) && (x <= 160) && (y >= 620) && (y <= 670)) //btnBackPressed
					return -1;
				break;

			default:
				break;

			}
		}
	}
	return -1;  /* never gets here */

}

/* gets move from user clicks, return char* of move
* returns NULL on quit
*/
char * guiGetMove(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color)
{
	SDL_Surface *checkerChosen = NULL;
	SDL_Event event;
	int toMove = 0, x, y, pos_x, pos_y, pos_w, pos_z;
	char *move = allocWrapper(calloc(20, sizeof(char)), "calloc");


	strcpy(move, "move <x,y> to <x,y>");
	
	/* Update screen */
	refresh();

	while (!done){

		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{

			case SDL_QUIT:
				/*freeAllRepositories();
				freeTree(gameTree);
				gameTree = NULL; */
				SDL_Quit();
				done = 1;

				/*exit(0); */
				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x;
				y = event.motion.y;
				if ((x >= gameBoardX) && (x <= gameBoardX + 500) && (y >= gameBoardY) && (y <= gameBoardY + 500)) //inside board click
				{ /* click inside board borders */
					//pos_x = 8 - (int)((480 - x) / 60);
					//pos_y = 1 + (int)((480 - y) / 60);
					int pos_x = (x - gameBoardX) / 60+1;
					int pos_y = 7 - ((y - gameBoardY) / 60)+1;
					toMove = (toMove + 1) % 2;
					if (toMove == 0)
					{
						/* even amount of clicks, try to make it into a move */
						move[6] = (char)(48 + pos_w + 48); /* from */
						move[8] = (char)(48 + pos_z);
						move[15] = (char)(48 + pos_x + 48); /* to */
						move[17] = (char)(48 + pos_y);

						setBoard(board, 0, gameBoardX, gameBoardY);

						return move;
					}
					pos_w = pos_x;/* save odd click location */
					pos_z = pos_y;

					checkerChosen = load_image("selectedPlace.png", 0);
					//TODO: mark available moves
					apply_surface(((pos_x-1) * 60) + gameBoardX, ((8 - pos_y) * 60) + gameBoardY, checkerChosen, screen, NULL);
					setBoard(board, 1, gameBoardX, gameBoardY); /* set only pieces, not board itself */
					SDL_FreeSurface(checkerChosen);
					/* Update screen */
					refresh();
				}

				else if ((x >= 550) && (x <= 750) && (y >= 240) && (y <= 290)) //btnMainMenu pressed
				{
					/* go to main menu */

					/*freeAllRepositories(); */
					SDL_Quit();
					main(0, NULL); /* no need to reset gui flag, already 1 */
				}
				else if ((x >= 550) && (x <= 750) && (y >= 100) && (y <= 150)) //btnSaveGame
				{
					saveState(board, current_player_color);
					guiSetForGameState(board, 0,gameBoardX,gameBoardY); /* reset screen after save screen */
					setMessageToUser(current_player_color,TURN_MESSAGE);
					/* Update screen */
					refresh();
				}
				else if ((x >= 550) && (x <= 750) && (y >= 450) && (y <= 350))
				{
					/* restart game */

					freeAllRepositories();
					init_board_(board);
					setBoard(board, 0,0,0);

					game_state(board, WHITE);
					/* Update screen */
					refresh();


				}
				else if ((x >= 500) && (x <= 700) && (y >= 310) && (y <= 360)) //btnQuit pressed
				{
					SDL_Quit();
					done = 1;
				}
				else if ((x >= 500) && (x <= 700) && (y >= 170) && (y <= 210)) //btnGetBestMove presseed
				{
					//TODO: implement
				}
				break;
			default:
				break;
			}
		}
	}


	free(move); move = NULL;
	return NULL; /* we hit quit */


}

char * choosePromotion(){


	char *result = allocWrapper(calloc(10, sizeof(char)), "calloc");
	SDL_Surface *background = NULL;
	SDL_Event event;
	int x, y;


	tools = load_image("sprites3.png", 1);
	background = load_image("mainBackground.png", 0);

	apply_surface(0, 0, background, screen, NULL);
	apply_surface(300, 300, tools, screen, &clip[3]);/* knight */
	apply_surface(500, 300, tools, screen, &clip[4]);/* rook */
	apply_surface(300, 500, tools, screen, &clip[2]);/* bishop */
	apply_surface(500, 500, tools, screen, &clip[1]);/* queen */

	SDL_FreeSurface(tools);
	SDL_FreeSurface(background);

	/* Update screen */
	refresh();

	while (!done){

		while (SDL_PollEvent(&event)){
			switch (event.type){

			case SDL_QUIT: done = 1;
				break;
			case SDL_MOUSEBUTTONDOWN:
				x = event.motion.x;
				y = event.motion.y;
				if ((x >= 300) && (x <= 360) && (y <= 360) && (y >= 300)){
					/* knight */
					strcpy(result, " knight\0");
					return result;
				}
				else if ((x >= 500) && (x <= 560) && (y <= 360) && (y >= 300)){
					/* rook */
					strcpy(result, " rook\0");
					return result;
				}
				else if ((x >= 300) && (x <= 360) && (y <= 560) && (y >= 500)){
					/* bishop */
					strcpy(result, " bishop\0");
					return result;
				}
				else if ((x >= 500) && (x <= 560) && (y <= 560) && (y >= 500)){
					/* queen */
					strcpy(result, " queen\0");
					return result;
				}
				break;
			default:
				break;
			}
		}
	}
	free(result);
	return NULL; /* we hit x */

}

/* show a message in case of check */
void guiCheckMessage(char board[BOARD_SIZE][BOARD_SIZE]){

	SDL_Surface *check = NULL;

	check = load_image("checkMessage.bmp", 0);

	apply_surface(140, 140, check, screen, NULL);

	SDL_FreeSurface(check);

	refresh();
	SDL_Delay(1000);

	setBoard(board, 0,0,0);

}


void guiUserTurn(char board[BOARD_SIZE][BOARD_SIZE], color current_player_color)
{
	char  str[STRING_SIZE], theMove[STRING_SIZE], *temp;

	checker srcChkr;
	checker destChkr;
	legalMovesRepository re;
	legalMovesRepository* rep = &re;


	/* initialize repository */
	rep->legalMoves = NULL;
	rep->pos = 0; rep->size = 0;
	addRepToArrayOfReps(rep);

	/* creating a repository of all legal available moves */
	getAllLegalMoves(board, current_player_color, rep, 1, 0);



	while (1)
	{ /* loop. incase of error, we get to try again */
		/* get move */
		setMessageToUser(current_player_color,TURN_MESSAGE);
		temp = guiGetMove(board, current_player_color);
		if (temp == NULL){
			/* we got NULL, this means we hit quit or x in the getMove window */
			break;
		}
		strncpy(theMove, temp + 5, 15);
		free(temp);



		if (theMove == NULL){

			setMessageToUser(WHITE,INVALID_MOVE);
			continue;
		}

		/* check legal source checker */
		strncpy(str, theMove, 5); /* str = source checker <x,y> */
		srcChkr = stringToChecker(str);
		if (srcChkr.y == 0){

			setMessageToUser(WHITE, INVALID_MOVE);
			continue;
		}

		/* check legal dest checker */
		strncpy(str, theMove + 9, 5); /* str = dest checker <x,y> */
		destChkr = stringToChecker(str);
		if (destChkr.y == 0){

			setMessageToUser(WHITE,INVALID_MOVE);
			continue;
		}

		/* checking if the user has a tool on the source checker */
		if (!(((current_player_color == WHITE) &&
			WHITE_TOOL(board[srcChkr.x - 97][srcChkr.y - 1])) ||
			((current_player_color == BLACK) &&
			BLACK_TOOL(board[srcChkr.x - 97][srcChkr.y - 1]))))
		{

			setMessageToUser(WHITE,INVALID_MOVE); /* no tool in source checker */
			continue;
		}

		/* getting the extra string if a pawn has to be promoted */
		if (((board[srcChkr.x - 97][srcChkr.y - 1] == WHITE_P && destChkr.y == BOARD_SIZE) ||
			(board[srcChkr.x - 97][srcChkr.y - 1] == BLACK_P && destChkr.y == 1)) /*&& isItInRepository(rep,theMove)*/)
		{
			/* GUI, choose what you want your pawn to be */
			temp = choosePromotion();/* gui function for choosing promotion */

			if (temp == NULL)
				break; /* x was hit in promotion screen */
			strcat(theMove, temp);
			free(temp);

			guiSetMainStage(board,0);
		}

		/* checking if it a legal move by searching in the repository */

		if (isItInRepository(rep, theMove) == 0){

			setMessageToUser(WHITE,INVALID_MOVE); /*move not in repository */
			continue;

		}

		/* the move is legal - update the board */

		simulateTurn(board, theMove, board);
		/*setBoard(board,0);*/

		/*not taking care of castling in GUI yet */


		break; /* break the while loop, we have a legal move */

	}

	freeRepository(rep);

}



void setMessageToUser(color nextColor, messageType msg)
{
	switch (msg)
	{
		case (TURN_MESSAGE):
		{
			SDL_Surface *turnColor = NULL;
			SDL_Surface *turnID = NULL;

			if (nextColor == WHITE)
				turnColor = load_image("Labels/lblWhiteTurn.png", 0);
			else
				turnColor = load_image("Labels/lblBlackTurn.png", 0);

			apply_surface(300, 600, turnColor, screen, NULL);

			if (nextColor == user_color)
				turnID = load_image("Labels/lblUserMove.png", 0);
			else
			{
				turnID = load_image("Labels/lblComputerMove.png", 0);
			}
			apply_surface(300, 670, turnID, screen, NULL);

			refresh();
			SDL_FreeSurface(turnColor);
			SDL_FreeSurface(turnID);
			break;
		}
		case (INVALID_MOVE):
		{
			SDL_Surface *illegalMove = NULL;
			illegalMove = load_image("Labels/lblIllegalMove.png", 0);
			apply_surface(60, 600, illegalMove, screen, NULL);
			SDL_FreeSurface(illegalMove);
			refresh();
			break;
		}
	}



}
