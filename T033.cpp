#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

#define TABLE_SIZE 27

//CONSOLE PROPERTY
void setColor(int color){
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void gotoxy(int x, int y){
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

//MALLOC PROPERTY
struct Node{
	char username[101];
	char password[101];
	int score;
	int clearedRow;
	int totalBlock;
	
	struct Node* next;
};

struct Node* hashTable[TABLE_SIZE];
struct Node* createNewNode(char username[], char password[], int score, int clearedRow, int totalBlock){
	struct Node* newNode = (struct Node*)malloc(sizeof(struct Node));
	strcpy(newNode->username, username);
	strcpy(newNode->password, password);
	
	newNode->score = score;
	newNode->clearedRow = clearedRow;
	newNode->totalBlock = totalBlock;
	
	newNode->next = NULL;
	
	return newNode;
}

//HASH TABLE
int hashFunction(char name[]){
	char firstChar = tolower(name[0]);
	
	if(isalpha(firstChar)) return (firstChar - 'a');
	else return 26;
}

void insertUser(char username[], char password[], int score, int clearedRow, int totalBlock){
	int index = hashFunction(username);
	struct Node* newNode = createNewNode(username, password, score, clearedRow, totalBlock);
	
	struct Node* curr = hashTable[index];
	struct Node* prev = NULL;
	
	while(curr != NULL && curr->score >= newNode->score){
		prev = curr;
		
		curr = curr->next;
	}
	if(prev == NULL){
		//Beggining
		newNode->next = hashTable[index];
		hashTable[index] = newNode;
	}else{
		//Mid
		prev->next = newNode;
		newNode->next = curr;
	}
}

struct Node* searchUser(char username[]){
	int index = hashFunction(username);
	struct Node* curr = hashTable[index];
	
	while(curr != NULL){
		if(strcmp(curr->username, username) == 0){
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

void displayMenu();
void printTetris();

//LEADER BOARD
void displayUser(){
    const int showUser = 17;
	
	struct Node* userList[1000];
	int totalUser = 0;
	for(int i = 0; i < TABLE_SIZE; i++){
		struct Node* curr = hashTable[i];
		while(curr != NULL){
			userList[totalUser] = curr;
			totalUser++;
			
			curr = curr->next;
		}
	}
	
	int page = 0;
	int totalPages = (totalUser + showUser - 1) / showUser;
	printTetris();
	puts("");
	
	while(1){
	    //Header
	    for(int i = 6; i <= 8; i++){
	    	gotoxy(1, i);
			switch(i){
	    		case 6: printf("  +-----------------------------------------------------+\n"); break;
	    		case 7: printf("  | %s | %-21s | %-5s | %-5s | %-5s |\n", "No.", "Username", "Score", "Clear", "Total"); break;
	    		case 8: printf("  +-----------------------------------------------------+\n"); break;
			}
		}
	
		int startIndex = page * showUser;
		int endIndex = startIndex + showUser;
	    int num = 1;
		
	    //Content
	    for(int i = startIndex; i < endIndex; i++){
			gotoxy(1, 9 + (i - startIndex));
			if(i < totalUser){
	            printf("  | %03d | %-21s | %-5d | %-5d | %-5d |\n", i+1, userList[i]->username, userList[i]->score, userList[i]->clearedRow, userList[i]->totalBlock);
	        }else{
	    		printf("  | %03d |                       |       |       |       |\n", i + 1);
			}
	    }
	    
	    //Footer
	    gotoxy(1, 26);
	    printf("  +-----------------------------------------------------+\n");
	    
	    //Ingfo
	    gotoxy(1, 27); printf("  Page %d / %d", page + 1, totalPages);
	    gotoxy(1, 28); puts("  Input 'a' and 'd' to navigate ('q' to exit): ");
	    
	    while(1){
	    	if(_kbhit()){
	    		char ch = getch();
				if(ch == 'a'){
					page = (page == 0) ? totalPages - 1 : page - 1;
					break;
				}else if(ch == 'd'){
					page = (page == totalPages - 1) ? 0 : page + 1;
					break;
				}else if(ch == 'q'){
					displayMenu();
					return;
				}
			}
		}
	}	
}

//FILE READER
void readUser(){
	FILE* file = fopen("users.txt", "r");
	char buffer[256];
	while(fgets(buffer, sizeof(buffer), file)){
		//username, password, score, cleared row, total block
		//s, s, d, d, d
		char username[101];
		char password[101];
		int score, clearedRow, totalBlock;
		
		sscanf(buffer, "%[^,],%[^,],%d,%d,%d", username, password, &score, &clearedRow, &totalBlock);
		insertUser(username, password, score, clearedRow, totalBlock);
	}
	fclose(file);
}

#define BLOCK_ROWS 6
#define BLOCK_COLS 6
struct BlockNode{
	char shape[BLOCK_ROWS][BLOCK_COLS];
	int color;
	
	struct BlockNode* next;
	struct BlockNode* prev;
};

struct BlockNode* head = NULL;
struct BlockNode* tail = NULL;

struct BlockNode* createNewBlockNode(char shape[BLOCK_ROWS][BLOCK_COLS]){
	struct BlockNode* newBlockNode = (struct BlockNode*)malloc(sizeof(struct BlockNode));
	
	memcpy(newBlockNode->shape, shape, sizeof(newBlockNode->shape));
	newBlockNode->next = NULL;
	newBlockNode->prev = NULL;
	
	return newBlockNode;
}

void pushBlock(char shape[BLOCK_ROWS][BLOCK_COLS]){
	struct BlockNode* newNode = createNewBlockNode(shape);
    if(head == NULL){
        head = tail = newNode;
        head->next = head;
        head->prev = head;
    }else{
        tail->next = newNode;
        newNode->prev = tail;
        newNode->next = head;
        head->prev = newNode;
        tail = newNode;
    }
}

void adminPage();
void readBlock(){
	FILE* file = fopen("block.txt", "r");
	char bufferBlock[BLOCK_ROWS][BLOCK_COLS];
	char buffer[BLOCK_COLS + 2];
	int row = 0, found = 0;
	
	while(fgets(buffer, sizeof(buffer), file)){
		if(buffer[0] == '='){
			if(row > 0){
				pushBlock(bufferBlock);
				found = 1;
			}
			memset(bufferBlock, 0, sizeof(bufferBlock));
			row = 0;
		}else if(row < BLOCK_ROWS){
			strncpy(bufferBlock[row], buffer, BLOCK_COLS);
			row++;
		}
	}
	
	if(row > 0){
		pushBlock(bufferBlock);
		found = 1;
	}
	
	fclose(file);
	
	if(found != 1){
		puts("Wait for admind");
		_sleep(100);
		adminPage();
		return;
	}
}

//void merge(struct Node* node, int left, int mid, int right){
//	int n1 = mid - left + 1; 
//	int n2 = right - mid; 
//	int leftArr[n1], rightArr[n2];
//	
//	for(int i = 0; i < n1; i++){
//		leftArr[i] = arr[left + i];
//	}
//	for(int i = 0; i < n2; i++){
//		rightArr[i] = arr[mid + 1 + i];
//	}
//	
//	int i = 0, j = 0, k = left;
//	while(i < n1 && j < n2){
//		if(leftArr[i] <= rightArr[j]){
//			arr[k] = leftArr[i];
//			i++;
//		}else{
//			arr[k] = rightArr[i];
//			j++;
//		}
//		k++;
//	}
//	
//	while(i < n1){
//		arr[k] = leftArr[i];
//		i++;	
//		k++;
//	}
//	while(j < n2){
//		arr[k] = rightArr[j];
//		j++;	
//		k++;
//	}
//}
//
//void mergeSort(struct Node* node, int left, int right){
//	if(left < right){
//		int mid = (left + right) / 2;
//		
//		mergeSort(node, left, mid);
//		mergeSort(node, mid + 1, right);
//		
//		merge(node, left, mid, right);
//	}
//}

//MISC
void clearScreen(){
	for(int i = 0; i < 120; i++){
		gotoxy(0, i);
		printf("                                                                                                                  ");
	}
	gotoxy(0, 0);
}
void printTetris(){
	system("cls");
	gotoxy(0, 0);
	puts("|  \\/  || ___ \\   |_   _||  ___||_   _|| ___ \\|_   _|/  ___|");
	puts("| .  . || |_/ /     | |  | |__    | |  | |_/ /  | |  \\ `--.");
	puts("| |\\/| ||    /      | |  |  __|   | |  |    /   | |   `--. \\");
	puts("| |  | || |\\ \\  _   | |  | |___   | |  | |\\ \\  _| |_ /\\__/ /");
	puts("\\_|  |_/\\_| \\_|(_)  \\_/  \\____/   \\_/  \\_| \\_| \\___/ \\____/");
}
void printJargon(){
	printTetris();
	
	char line1[] = "\"Confront the challenges of learning and";
	char line2[] = "outgrow the boundaries together\"";
	int line1Len = strlen(line1);
	int line2Len = strlen(line2);
	
	gotoxy(10, 12);
	for(int i = 0; i < line1Len; i++){
		printf("%c", line1[i]);
		_sleep(20);
	}
	gotoxy(14, 13);
	for(int i = 0; i < line2Len; i++){
		printf("%c", line2[i]);
		_sleep(20);
	}
	gotoxy(17, 15);
	puts("Press enter to continue...");	
}

void GAMEPLAY();
void drawNewBlock(){
	printTetris();
	
	char canvas[6][6];
	int brushX = 0; int brushY = 0;
	int startX = 5; int startY = 12;
	char tool = '#';
	
	for(int i = 0; i < 6; i++){
        for(int j = 0; j < 6; j++){
            canvas[i][j] = '.';
        }
    }
    
    while(1){
    	gotoxy(startX, startY);
    	printf("+------+\n");
    	for(int i = 0; i < 6; i++){
            gotoxy(startX, startY + i + 1);
            printf("|");
            for(int j = 0; j < 6; j++){
                if(i == brushY && j == brushX){
                	(tool == '#') ? setColor(10) : setColor(12);
                    printf("%c", tool);
                    setColor(7);
                }else{
                    printf("%c", canvas[i][j]);
                }
            }
            printf("|");
        }
        gotoxy(startX, startY + 7);
        printf("+------+\n");

        gotoxy(startX + 13, startY + 0); printf("Draw your unique block!");
        gotoxy(startX + 13, startY + 1); printf("  w, a, s, d\t-> control position");
        gotoxy(startX + 13, startY + 2); printf("  e\t\t-> change mode (erase / draw)");
        gotoxy(startX + 13, startY + 3); printf("  space\t-> draw '#'");
        gotoxy(startX + 13, startY + 4); printf("  q\t\t-> save and exit");
        
        char ch = getch();

        if(ch == 'w' && brushY > 0) brushY--;
        if(ch == 's' && brushY < 5) brushY++;
        if(ch == 'a' && brushX > 0) brushX--;
        if(ch == 'd' && brushX < 5) brushX++;

        if(ch == 'e') tool = (tool == '#') ? '.' : '#';
        if(ch == ' ') canvas[brushY][brushX] = tool;
        if(ch == 'q') break;
	}
	adminPage();
}

void adminPage(){ //Still unfinished
	printTetris();
	
	gotoxy(0, 6); puts("1. Play Game");
	gotoxy(0, 7); puts("2. Add new block");
	gotoxy(0, 8); puts("3. Remove block");
	gotoxy(0, 9); puts("4. Log out and save");
	printf(">> ");
	
	char input[256];
	while(1){
		scanf("%s", input);
		if(strcmp(input, "1") == 0){
			printJargon();
			while(1){
				if(_kbhit()){
					char ch = getch();
					if(ch == 13) break;
				}
			}
			GAMEPLAY();
		}
		else if(strcmp(input, "2") == 0){
			drawNewBlock();
		}
		else if(strcmp(input, "3") == 0){
			system("cls");
			puts("Remove block, no time");
		}
		else{
			gotoxy(0, 11); puts("Invalid input");
			_sleep(150);
			adminPage();
			break;
		}
	}
	
	/*
		+------+
		¦#.....¦
		¦......¦
		¦......¦
		¦......¦
		¦......¦
		¦......¦
		+------+
	*/
		
}

#define FIELD_ROWS 30
#define FIELD_COLS 50
char playfield[FIELD_ROWS][FIELD_COLS];
void initPlayfield(){
	//i = y
	//j = x
	
    for(int i = 0; i < FIELD_ROWS; i++){
        for(int j = 0; j < FIELD_COLS; j++){
        	if(i < 9) continue;
        	
            if(j < 15 || j > 15 + 29) playfield[i][j] = ' ';
            else if(j == 15 || j == 15 + 29) playfield[i][j] = '|';
            else if(i == FIELD_ROWS - 2) playfield[i][j] = '=';
            else playfield[i][j] = '.';
        }
    }
}
void debugPlayField(){
    system("cls");  

    for(int i = 0; i < FIELD_ROWS; i++){
        for(int j = 0; j <= FIELD_COLS; j++){
            printf("%c", playfield[i][j]);
        }
        puts("");
    }
}

void drawNextBlock(){
	if(head == NULL || head->next == NULL) return;

    struct BlockNode* nextBlock = head->next;
    int startRow = 9;
    int startCol = 50;

    for(int i = 0; i < BLOCK_ROWS; i++){
        gotoxy(startCol, startRow + i);
        for(int j = 0; j < BLOCK_COLS; j++){
            if(nextBlock->shape[i][j] == '#')printf("#");
            else printf(".");
        }
    }
}

//GAMEPLAY(SCREEN)
int score = 0, totalClearedRows = 0, blockCount = 0;
char save_name[101]; char save_password[101];
void leftSide(){
	gotoxy(1, 6); puts("Score");
	gotoxy(1, 7); printf("+---------+");
	gotoxy(1, 8); printf("|  %5d  |", score);	
	gotoxy(1, 9); printf("+---------+");
	
	gotoxy(1, 11); puts("Clear row");
	gotoxy(1, 12); printf("+---------+");
	gotoxy(1, 13); printf("|  %5d  |", totalClearedRows);	
	gotoxy(1, 14); printf("+---------+");
	
	gotoxy(1, 16); puts("Block");
	gotoxy(1, 17); printf("+---------+");
	gotoxy(1, 18); printf("|  %5d  |", blockCount / 2);	
	gotoxy(1, 19); printf("+---------+");
	
	gotoxy(1, 21); puts("Controls");
	gotoxy(1, 22); printf("--------");
	gotoxy(1, 23); printf("'a' left");	
	gotoxy(1, 24); printf("'d' right");	
	gotoxy(1, 25); printf("'s' down");	
	gotoxy(1, 26); printf("'w' rotate");	
	gotoxy(1, 27); printf("' ' hard");	
	gotoxy(5, 28); printf("drop");	
}

void rightSide(){
	gotoxy(47, 6); puts("+----------+");
	gotoxy(47, 7); puts("|   NEXT   |");
	gotoxy(47, 8); puts("+----------+");
	for(int i = 9; i <= 14; i++){
        gotoxy(47, i); puts("|..      ..|");
    }
    gotoxy(47, 15); puts("+----------+");
    drawNextBlock();
	
	gotoxy(47, 17); printf("notes: ");
	gotoxy(47, 18); printf("-------");
	gotoxy(47, 19); printf("> press 'p'");
	gotoxy(47, 20); printf("  to pause");
	gotoxy(47, 21); printf("  the game");
	
	gotoxy(47, 23); printf("> press 'l'");
	gotoxy(47, 24); printf("  to move");
	gotoxy(47, 25); printf("  the next");
	gotoxy(47, 26); printf("  block");
}

void midSide(){
	//Border
	for(int i = 9; i <= 28; i++){
		gotoxy(15, i); printf("|");
		gotoxy(44, i); printf("|");
	}
	
	//Inside
	for(int i = 9; i < 28; i++){
        for(int j = 16; j < 44; j++){
            gotoxy(j, i);
            printf("%c", playfield[i][j]);
        }
    }
	
	gotoxy(16, 28); printf("============================");
}

int x = 25;
int y = 9;

int lastTetrisRow = -1;  
int lastTetrisX = -1;
char lastTetrisShape[BLOCK_ROWS][BLOCK_COLS];

void spawnTetris(){
    struct BlockNode* curr = head;
    if(curr == NULL) return;

    int startRow = y;
    int startCol = x;
    
    if(lastTetrisRow != -1){
        for(int i = 0; i < BLOCK_ROWS; i++){
            for(int j = 0; j < BLOCK_COLS; j++){
                int prevRow = lastTetrisRow + i;
                int prevCol = lastTetrisX + j;

                if(lastTetrisShape[i][j] == '#' && playfield[prevRow][prevCol] == '.'){
                    gotoxy(prevCol, prevRow);
                    printf(".");
                }
            }
        }
    }
    
    setColor(curr->color);
    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            int row = startRow + i;
            int col = startCol + j;

            if(curr->shape[i][j] == '#' && playfield[row][col] == '.'){
                gotoxy(col, row);
                printf("#");
                lastTetrisShape[i][j] = '#'; 
            }else{
                lastTetrisShape[i][j] = '.';
            }
        }
    }

	setColor(7);
    lastTetrisRow = startRow;
    lastTetrisX = startCol;
}

int calcLanding(int startRow, int startCol, char block[BLOCK_ROWS][BLOCK_COLS]){
    int landingRow = startRow;

    while(landingRow + BLOCK_ROWS < FIELD_ROWS){
        for(int i = 0; i < BLOCK_ROWS; i++){
            for(int j = 0; j < BLOCK_COLS; j++){
                if(block[i][j] == '#'){
                    int nextRow = landingRow + i + 1;
                    int nextCol = startCol + j;

                    if(nextRow >= FIELD_ROWS || playfield[nextRow][nextCol] != '.'){
                        return landingRow;
                    }
                }
            }
        }
        landingRow++;  // Move down
    }
    
    return landingRow;
}

int lastPreviewRow = -1;  
int lastPreviewX = -1;
char lastPreviewShape[BLOCK_ROWS][BLOCK_COLS];

void spawnPreview(){
    struct BlockNode* curr = head;
    if(curr == NULL) return;

    int landingRow = calcLanding(y, x, curr->shape);
    if(landingRow == y) return;

    if(lastPreviewRow != -1){
        for(int i = 0; i < BLOCK_ROWS; i++){
            for(int j = 0; j < BLOCK_COLS; j++){
                if(lastPreviewShape[i][j] == '*' && playfield[lastPreviewRow + i][lastPreviewX + j] == '.'){
                    gotoxy(lastPreviewX + j, lastPreviewRow + i);
                    printf(".");
                }
            }
        }
    }

    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            if(curr->shape[i][j] == '#' && playfield[landingRow + i][x + j] == '.'){
                gotoxy(x + j, landingRow + i);
                printf("\033[90m*\033[0m");
                lastPreviewShape[i][j] = '*';
            }else{
                lastPreviewShape[i][j] = '.';
            }
        }
    }

    lastPreviewRow = landingRow;
    lastPreviewX = x;
}

void spawnNextBlock(){
	static int isFirstRun = 1;
	if(!isFirstRun){
		if(head != NULL){
			head = head->next;
		}
	}
	isFirstRun = 0;	
	
	if(head == NULL) return;
	head->color = 1 + (rand() % 15);
	x = 25;
	y = 9;
}
int colorField[FIELD_ROWS][FIELD_COLS];
void renderAll(){
	leftSide();
	rightSide();
	midSide();
	
	for(int i = 0; i < FIELD_ROWS; i++){
        for(int j = 0; j < FIELD_COLS; j++){
            if(playfield[i][j] == '#'){
                setColor(colorField[i][j]);
                gotoxy(j, i);
                printf("#");
            }
        }
    }
    setColor(7);

    spawnTetris();
    spawnPreview();
}

//GAMEPLAY
void placeBlock(int x, int y, char block[BLOCK_ROWS][BLOCK_COLS]){
	struct BlockNode* curr = head;
    if(curr == NULL) return;
    
	for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            if(block[i][j] == '#'){
                playfield[y + i][x + j] = block[i][j];
                colorField[y + i][x + j] = curr->color;
            }
        }
    }
    
    blockCount++;
}

void rotateTetris(){
	struct BlockNode* curr = head;
	if(curr == NULL) return;
	
	char temp[BLOCK_ROWS][BLOCK_COLS];

    //Transpose and reverse rows to rotate 90 degrees
    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            temp[j][BLOCK_ROWS - 1 - i] = curr->shape[i][j];
        }
    }

    //Copy back rotated shape
    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            curr->shape[i][j] = temp[i][j];
        }
    }
}

int canMoveLeft(int x, int y, char block[BLOCK_ROWS][BLOCK_COLS]){
    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = 0; j < BLOCK_COLS; j++){
            if(block[i][j] == '#' && (x + j - 1 < 0 || playfield[y + i][x + j - 1] != '.')){
                return 0;
            }
        }
    }
    return 1;
}

int canMoveRight(int x, int y, char block[BLOCK_ROWS][BLOCK_COLS]){
    for(int i = 0; i < BLOCK_ROWS; i++){
        for(int j = BLOCK_COLS - 1; j >= 0; j--){
            if(block[i][j] == '#' && (x + j + 1 >= FIELD_COLS || playfield[y + i][x + j + 1] != '.')){
                return 0;
            }
        }
    }
    return 1;
}

int canMoveDown(int x, int y, char block[BLOCK_ROWS][BLOCK_COLS]){
    for(int i = BLOCK_ROWS - 1; i >= 0; i--){
        for(int j = 0; j < BLOCK_COLS; j++){
            if(block[i][j] == '#'){
                int newY = y + i + 1;
                if(newY >= FIELD_ROWS || playfield[newY][x + j] != '.'){
                    return 0;
                }
            }
        }
    }
    return 1;
}

void gameOver(){
//	clearScreen();
	int startX = 0;
	int startY = 6;
	int rows = 23; int cols = 6;
	
	 for(int i = 0; i < rows; i++){
        for(int j = 0; j < cols; j++){
            gotoxy(startX + (j * 12), startY + i);
            printf("   GAME OVER   ");
        }
        _sleep(200);
    }
	
	printTetris();
	gotoxy(10, 12);
    puts("You lost...");
	gotoxy(10, 13);
    puts("Press enter to continue");
    
    while(1){
    	if(_kbhit()){
    		char ch = getch();
			if(ch == 13) break;
		}
	}
	
    //Adsking for confirmation
    printTetris();
    struct Node* sNode = searchUser(save_name);
    int prevScore = 0, prevRow = 0, prevBlock = 0;
	if(sNode != NULL){
		prevScore = sNode->score; prevRow = sNode->clearedRow; prevBlock = sNode->totalBlock;
	}
	
    gotoxy(10, 14); puts("+--------------------+");
    gotoxy(10, 15); puts("| Current Stats      |");
    gotoxy(10, 16); puts("+--------------------+");
    gotoxy(10, 17); printf("| Final Score: %-5d |", score);    
    gotoxy(10, 18); printf("| Cleared row: %-5d |", totalClearedRows);    
    gotoxy(10, 19); printf("| Block count: %-5d |", blockCount);    
    gotoxy(10, 20); puts("+--------------------+");
    
    gotoxy(40, 14); puts("+--------------------+");
    gotoxy(40, 15); puts("| Previous Stats     |");
    gotoxy(40, 16); puts("+--------------------+");
    gotoxy(40, 17); printf("| Final Score: %-5d |", prevScore);    
    gotoxy(40, 18); printf("| Cleared row: %-5d |", prevRow);    
    gotoxy(40, 19); printf("| Block count: %-5d |", prevBlock);    
    gotoxy(40, 20); puts("+--------------------+");
    
    gotoxy(10, 22); puts("!! Previous data will be lost !!");
    gotoxy(10, 23); puts("Do you want to save your stats [Y | N]?");
    
    while(1){
    	if(_kbhit()){
    		char ch = getch();
			if(ch == 'y'){
			    //Save progress
			    FILE* file = fopen("users.txt", "a");
			    fprintf(file, "%s,%s,%d,%d,%d\n", save_name, save_password, score, totalClearedRows, blockCount);
			    fclose(file);
				break;
			}
			if(ch == 'n') break;
		}
	}
    score = 0; totalClearedRows = 0; blockCount = 0;
    strcpy(save_name, ""); strcpy(save_password, "");
    initPlayfield();
	displayMenu();
}

void moveTetris(){
	struct BlockNode* curr = head;
	if(curr == NULL) return;
	
	clock_t lastMoveTime = clock();
	int isQuickNoScoping = 0;
	
	while(1){
		if(_kbhit()){
			char ch = getch();
	        
	        int blockWidth = 0;
	        int blockHeight = 0;
	        for(int i = 0; i < BLOCK_ROWS; i++){
                for(int j = 0; j < BLOCK_COLS; j++){
                    if(curr->shape[i][j] == '#'){
                        if(j > blockWidth) blockWidth = j;
                        if(i > blockHeight) blockHeight = i;
                    }
                }
            }
            blockWidth++; blockHeight++;
			
			switch(ch){
				case 'a': if(canMoveLeft(x, y, curr->shape)) x--; break;
				case 'd': if(canMoveRight(x, y, curr->shape)) x++; break;
				case 's': if(canMoveDown(x, y, curr->shape)) y++; break;
				case 'w':{
					rotateTetris();	
//					if(x < 15) x = 15;
//					if(x + BLOCK_COLS > 44) x = 44 - BLOCK_COLS;
					break;
				}
				case ' ':{
					isQuickNoScoping = 1;
					int landingY = calcLanding(y, x, curr->shape);
					while(y < landingY){
						y++;
						spawnTetris();
						_sleep(15);
					}
					placeBlock(x, y, curr->shape);
					return;
				} 
				case '0': debugPlayField(); break;
				case 'p':{
					//Pause gmame
					printTetris();
					gotoxy(10, 12);
					puts("Are you sure you want to exit [Y / N]?");
					while(1){
						if(_kbhit()){
							char ch = getch();
							if(ch == 'y'){
								renderAll();
								gameOver();
							}
							if(ch == 'n'){
								printTetris();
								renderAll();
								break;
							}
						}
					}
					
					break;
				}
				case 'l':{
					//Next linked list
					if(head != NULL && head->next != NULL){
						head->next = head->next->next;
						head->next->prev = head;
				        drawNextBlock();
					}
					break;
				}
			}
			
			spawnTetris();
			spawnPreview();
		}
		
        if((clock() - lastMoveTime) >= CLOCKS_PER_SEC && !isQuickNoScoping){
            lastMoveTime = clock();
            if(canMoveDown(x, y, curr->shape)){
                y++;
                spawnTetris();
            }else{
                placeBlock(x, y, curr->shape);
                break;
            }
        }
	}
}

//Please send help
int clearFullLines(){
    int clearedRows = 0;

	//i = row, j = col
    for(int i = FIELD_ROWS - 3; i >= 9; i--){
        int isFull = 1;
        
        for(int j = 16; j < 15 + 28; j++){
            if(playfield[i][j] != '#'){
                isFull = 0;
                break;
            }
        }

        if(isFull == 1){
            clearedRows++;

            for(int shiftRow = i; shiftRow > 9; shiftRow--){
                for(int j = 16; j < 15 + 28; j++) {
                    playfield[shiftRow][j] = playfield[shiftRow - 1][j];
                    colorField[shiftRow][j] = colorField[shiftRow - 1][j];
                }
            }

            for(int j = 16; j < 15 + 28; j++){
                playfield[0][j] = '.';
                colorField[0][j] = 7;
            }

            i++;
        }
    }
    return clearedRows;
}

void updateScore(int n){
	int point = (n / 2.00) * (200 + (n - 1) * 20);
	score += point;
}

int ISGAMEOVER(){
	for(int i = 16; i < 15 + 28; i++){
		if(playfield[y + 1][i] == '#') return 1;
	}
	return 0;
}

void GAMEPLAY(){
	//Initialization
	readBlock();
	initPlayfield();
	spawnNextBlock();
	
	//Render UI
	printTetris();
	
	//Loopy doopy
	while(1){
        if(head == NULL) break;
		if(ISGAMEOVER()){
			gameOver();
			return;
		}

        x = 25; y = 9;
        renderAll();
		moveTetris();
		placeBlock(x, y, head->shape);
		
		int clearedRows = clearFullLines();
		totalClearedRows += clearedRows;
		
		if(clearedRows > 0) updateScore(clearedRows);
		
		spawnNextBlock();
	}
}

void login(struct Node* sNode){
	printTetris();
	
	for(int i = 10; i <= 13; i++){
		gotoxy(10, i);	
		switch(i){
			case 10: puts("Password: ('0' to go back)"); break;
			case 11: puts("=========================="); break;
			case 13: puts("=========================="); break;
		}
	}
	
	char password[24] = {0};
	int passLen = strlen(password);
	
	gotoxy(10, 14);
	printf("                                                    ");
	gotoxy(10, 12);
	printf("                                                    ");
		
	while(1){
		gotoxy(10, 12);
		
		char key = getch();
		if(key == 8 && passLen > 0){
			password[--passLen] = '\0';
			gotoxy(10, 12);
			printf("                                                    ");
		}
		else if(key >= 32 && key <= 126 && passLen < 24){
            password[passLen++] = key;
            password[passLen] = '\0';
        }
        
        gotoxy(10, 12);
		for(int i = 0; i < passLen; i++){
			printf("*");
		}
		
		if(key == 13){
			if(strcmp(password, "0") == 0){
				displayMenu();
				return;
			}
			
			if(strcmp(sNode->password, password) == 0){
				printJargon();
				while(1){
					if(_kbhit()){
						char ch = getch();
						if(ch == 13){
							strcpy(save_name, sNode->username);
							strcpy(save_password, sNode->password);
							GAMEPLAY();
							return;
						}
					}
				}
			}else{
				gotoxy(10, 14);
				puts("*Incorrect password");
				gotoxy(10, 12);
				printf("                                                    ");
				
				strcpy(password, "");
				passLen = 0;
			}
		}
	}
}

void playGame(){
	printTetris();
	
	char username[101];
	for(int i = 10; i <= 13; i++){
		gotoxy(10, i);		
		switch(i){
			case 10: puts("Username: ('0' to go back)"); break;
			case 11: puts("=========================="); break;
			case 13: puts("=========================="); break;
		}
	}
	
	while(1){
		gotoxy(10, 12);
		printf("                                                    ");
		gotoxy(10, 12);
		
		fgets(username, sizeof(username), stdin);
		username[strcspn(username, "\n")] = '\0';
		
		int isValidUsername = strspn(username, "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-_") == strlen(username);
		int nameLen = strlen(username);
		
		gotoxy(10, 14);
		printf("                                                    ");
		gotoxy(10, 14);
		
		//Special case
		struct Node* sNode = searchUser(username);
		if(strcmp(username, "0") == 0){
			displayMenu();
			return;
		}else if(strcmp(username, "Admin") == 0){
			adminPage();
			return;
		}else if(sNode != NULL){
			puts("User already registered! Want to login? [Y / N]?");
			while(1){
				if(_kbhit()){
					char ch = getch();
					
					if(ch == 'y'){
						login(sNode);
						return;
					}else if(ch == 'n'){
						displayMenu();
						return;
					}
				}
			}
		}
		
		//Requirement
		if(nameLen == 0){
			puts("*Username cannot be empty");
			strcpy(username, "");
		}
		else if(nameLen < 4 || nameLen > 20){
			puts("*Username must be between 4-20 (inclusive)");
			strcpy(username, "");
		}
		else if(!isValidUsername){
			puts("*Username must consist of only alphanumeric, '.', '-', and '_' character");
			strcpy(username, "");
		}else{
			//Valid
			break;
		}
	}
	
	gotoxy(10, 14);
	printf("                                                    ");
	gotoxy(10, 14);
	
	puts("Register new user! Want to register? [Y / N]");
	while(1){
		if(_kbhit()){
			char ch = getch();
			if(ch == 'y'){
				for(int i = 10; i <= 18; i++){
					gotoxy(10, i);	
					switch(i){
						case 10: puts("Password: ('0' to go back)"); break;
						case 11: puts("=========================="); break;
						case 13: puts("=========================="); break;
						case 15: puts("[ ] Length 8-24 (Inclusive)"); break;
						case 16: puts("[ ] Contains upper case letter"); break;
						case 17: puts("[ ] Contains lower case letter"); break;
						case 18: puts("[ ] Contains symbol / numeric character"); break;
					}
				}
					
				char password[24] = {0};
				int passLen = strlen(password);
				
				gotoxy(10, 14);
				printf("                                                    ");
				gotoxy(10, 12);
				printf("                                                    ");
				gotoxy(10, 12);
				
				while(1){
					char key = getch();
					
					if(key == 8 && passLen > 0){
						password[--passLen] = '\0';
						gotoxy(10, 12);
						printf("                                                    ");
					}
					else if(key >= 32 && key <= 126 && passLen < 24){
                        password[passLen++] = key;
                        password[passLen] = '\0';
                    }
					
					int req1 = (passLen > 8 && passLen < 24);
					int req2 = 0, req3 = 0, req4 = 0;
					
					for(int i = 0; i < passLen; i++){
						if(isupper(password[i])) req2 = 1;
						if(islower(password[i])) req3 = 1;
						if(isdigit(password[i]) || !isalnum(password[i])) req4 = 1;
					}
					
					gotoxy(10, 12);
					for(int i = 0; i < passLen; i++){
						printf("*");
					}
					
					for(int i = 15; i <= 18; i++){
						gotoxy(10, i);	
						switch(i){
							case 15: printf("[%c] Length 8-24 (Inclusive)", (req1) ? 'V' : ' '); break;
							case 16: printf("[%c] Contains upper case letter", (req2) ? 'V' : ' '); break;
							case 17: printf("[%c] Contains lower case letter", (req3) ? 'V' : ' '); break;
							case 18: printf("[%c] Contains symbol / numeric character", (req4) ? 'V' : ' '); break;
						}
					}
					
					if(key == 13){
						if(strcmp(password, "0") == 0){
							displayMenu();
							return;
						}
						
						if(req1 && req2 && req3 && req4){
							for(int i = 10; i < 20; i++){
								gotoxy(10, i);
								printf("                                                    ");
							}
							
							printJargon();
							while(1){
								if(_kbhit()){
									char ch = getch();
									if(ch == 13){
										insertUser(username, password, 0, 0, 0);
										displayMenu();
										return;
									}
								}
							}
							break;
						}
					}
				}
			}else if(ch == 'n'){
				displayMenu();
				break;
			}
		}
	}
}

void viewUser(){
	printJargon();
	while(1){
		if(_kbhit()){
			char ch = getch();
			if(ch == 13) break;
		}
	}
	
	displayUser();	
}

void exitGame(){
	system("cls");

    gotoxy(10, 10);
    printf("Exiting the game...");

    _sleep(1000);
    exit(0);
}

void displayMenu(){
	printTetris();
	
	gotoxy(0, 6); puts("1. Play");
	gotoxy(0, 7); puts("2. View User");
	gotoxy(0, 8); puts("3. Exit");
	printf(">> ");
	
	char input[256];
	while(1){
		scanf("%s", input);
		if(strcmp(input, "1") == 0) playGame();
		else if(strcmp(input, "2") == 0) viewUser();
		else if(strcmp(input, "3") == 0) exitGame();
		else{
			gotoxy(0, 11); puts("Invalid input");
			_sleep(150);
			displayMenu();
			break;
		}
	}
}

int main(){
	srand(time(0));
	
	readUser();
	insertUser("Natan", "aka", 0, 0, 0);
//	gameOver();
//	GAMEPLAY();
	
//	initPlayfield(); debugPlayField();
	
//	displayUser();
	displayMenu();
//	adminPage();
	return 0;
}

