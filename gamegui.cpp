#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <limits.h>
#include "game.h"
#include "AI_Name.h"

using namespace std;

Point (*BlackAI)(const NodeType board[BoardSize][BoardSize], NodeType yourType);
const char* BlackAIName;
Point (*WhiteAI)(const NodeType board[BoardSize][BoardSize], NodeType yourType);
const char* WhiteAIName;

//extern "C" S_AI blackAI;
//extern "C" S_AI whiteAI;

static bool argIsValid;

GameSettings::GameSettings()
{
    isPrint      = true;
    isRandFirst  = true;
    sleepTime    = 1;
    repeatTime   = INT_MAX;
    printLength  = 1;
    isNormal     = true;
    isEval       = false;
    savePath[0]  = '\0'; 
}

Game::Game()
{
    for (int i = 0; i < BoardSize; i++) {
        for (int j = 0; j < BoardSize; j++) {
            board[i][j] = Empty;
        }
    }

    // Draw board
    int drawSize = BoardSize*2;
    boardChar[1][1]                   = "┌";
    boardChar[1][drawSize-1]          = "┐";
    boardChar[drawSize-1][1]          = "└";
    boardChar[drawSize-1][drawSize-1] = "┘";
    for (int i = 2; i < drawSize-1; ++i) {
        if (i%2 == 1) {
            boardChar[1][i]          = "┬";
            boardChar[drawSize-1][i] = "┴";
            boardChar[i][1]          = "├";
            boardChar[i][drawSize-1] = "┤";
        }
        else {
            boardChar[1][i]          = "─";
            boardChar[drawSize-1][i] = "─";
            boardChar[i][1]          = "│";
            boardChar[i][drawSize-1] = "│";

        }
    }
    for (int i = 2; i < drawSize - 1; ++i) {
        for (int j = 2; j < drawSize - 1; ++j) {
            if (i%2 == 0) {
                if (j%2 == 0)
                    boardChar[i][j] = " ";
                else
                    boardChar[i][j] = "│";
            } else {
                if (j%2 == 0)
                    boardChar[i][j] = "─";
                else
                    boardChar[i][j] = "┼";
            }
        }
    }

    for (int i = 0; i < MAX_AI_NUM; ++i) {
        struct S_AI ai = AIList[i];
        if (ai.id > 0 && ai.id <= MAX_AI_NUM) {
            AI_Map::iterator it = aiMap.find(ai.id);
            if (it == aiMap.end()) {
                printf("Loaded AI: %d. %s\n", ai.id, ai.name);
                aiMap[ai.id] = AIList[i];
            } else {
                printf("AI id conflicts at %d!\n", ai.id);
                exit(0);
            }
        }
    }
}
void Game::Initialize()
{
    for (int i = 0; i < BoardSize; i++) {
        for (int j = 0; j < BoardSize; j++) {
            board[i][j] = Empty;
        }
    }
    isBlackPlaying = true;
    winner = Empty;
}
NodeType Game::Play(S_AI blackAI, S_AI whiteAI) {
    Initialize();
    if (!settings.isNormal)
        PreSetBoard();
    while (1) {
        Point p;
        p = blackAI.func(board, Black);
        if (Move(p) == false) {
            if (settings.isPrint)
                printf("Stupid AI %s(%s) made a move at (%i, %i)\n", blackAI.name, blackChar, p.x, p.y);
            return White;
        }
        if (settings.isPrint) {
            PrintBoard();
            printf("%s made a move at (%d, %d)\n", blackChar, p.x, p.y);
            sleep(settings.sleepTime);
        }
        if (CheckVictory(Black) == Black) {
            return Black;
        }
        
        p = whiteAI.func(board, White);
        if (Move(p) == false) {
            if (settings.isPrint)
                printf("Stupid AI %s(%s) made a move at (%i, %i)\n", whiteAI.name, whiteChar, p.x, p.y);
            return Black;
        }
        if (settings.isPrint) {
            PrintBoard();
            printf("%s made a move at (%d, %d)\n", whiteChar, p.x, p.y);
            sleep(settings.sleepTime);
        }
        if (CheckVictory(White) == White) {
            return White;
        }
    }
}
bool Game::Move(Point p)
{
    if (p.x < 0 || p.x >= BoardSize ||
            p.y < 0 || p.y >= BoardSize) {
        return false;
    }
    if (board[p.x][p.y] == Empty) {
        if (isBlackPlaying)
            board[p.x][p.y] = Black;
        else
            board[p.x][p.y] = White;
        isBlackPlaying = !isBlackPlaying;
        return true;
    } else {
        return false;
    }
}
NodeType Game::GetType(Point p)
{
    assert(0 <= p.x && p.x < BoardSize);
    assert(0 <= p.y && p.y < BoardSize);
    return board[p.x][p.y];
}
NodeType Game::CheckVictory(NodeType t)
{
    bool isFull = true;
    for (int i = 0; i < BoardSize; ++i) {
        for (int j = 0; j < BoardSize; ++j) {
            Point p(i,j);
            NodeType winType = CheckNode(p);
            if (winType == Black) {
                winner = Black; 
                return Black;
            } else if (winType == White) {
                winner = White;
                return White;
            }
            if (board[p.x][p.y] == Empty)
                isFull = false;
        }
    }
    if (isFull == true) {
        winner = t;
        return t;
    }
    return Empty;
}
// We only check right, down and right-down
NodeType Game::CheckNode(Point p)
{
    NodeType curType = GetType(p);
    // right
    bool vic = true;
    for (int i = 1; i < 5; i++) {
        Point nextP(p.x+i, p.y);
        if (p.x+i >= BoardSize || GetType(nextP) != curType) {
            vic = false;
            break;
        }
    }
    if (vic == true)
        return curType;
    // down
    vic = true;
    for (int i = 1; i < 5; i++) {
        Point nextP(p.x, p.y+i);
        if (p.y+i >= BoardSize || GetType(nextP) != curType) {
            vic = false;
            break;
        }
    }
    if (vic == true)
        return curType;
    // right down
    vic = true;
    for (int i = 1; i < 5; i++) {
        Point nextP(p.x+i, p.y+i);
        if (p.x+i >= BoardSize || p.y+i >= BoardSize || GetType(nextP) != curType) {
            vic = false;
            break;
        }
    }
    if (vic == true)
        return curType;
    //left down
    vic = true;
    for (int i = 1; i < 5; i++) {
        Point nextP(p.x-i, p.y+i);
        if (p.x-i < 0 || p.y+i >= BoardSize || GetType(nextP) != curType) {
            vic = false;
            break;
        }
    }
    if (vic == true)
        return curType;

    return Empty;
}
void Game::PrintBoard()
{
    //system("clear");
    printf(" ");
    for (int i = 0; i < BoardSize; ++i) {
        printf("%2d", i);
    }
    printf("\n");
    for (int i = 0; i < BoardSize; ++i) {
        printf("%2d", i);
        for (int j = 0; j < BoardSize; ++j) {
            Point p(i,j);
            if (GetType(p) == Empty)
                cout<<boardChar[i*2+1][j*2]<<boardChar[i*2+1][j*2+1];
            else if (GetType(p) == White)
                cout<<boardChar[i*2+1][j*2]<<whiteChar;
            else if (GetType(p) == Black)
                cout<<boardChar[i*2+1][j*2]<<blackChar;
        }
        printf("\n");
    }
    printf("%s: %s | ", blackChar, blackAI.name);
    printf("%s: %s\n", whiteChar, whiteAI.name);
}
void Game::PreSetBoard()
{
    int x = 0;
    int y = 0;
    NodeType t;
    for (int i = 0; i <= 2; ++i) {
        t = (i == 1) ? Black : White;
        x = (BoardSize - 9) / 2 + rand() % 9;
        y = (BoardSize - 9) / 2 + rand() % 9;
        if (board[x][y] == Empty)
            board[x][y] = t;
    }
}
// We assume both IDs exist
void Game::SetGamerAI(int gamer1ID, int gamer2ID)
{
    S_AI ai1;
    S_AI ai2;
    ai1 = aiMap[gamer1ID];
    ai2 = aiMap[gamer2ID];

    if (gamer1ID == 10)
        ai1.id = 10;
    else if (gamer2ID == 10)
        ai2.id = 10;

    if (!settings.isRandFirst) {
        blackAI = ai1;
        whiteAI = ai2;
    } else {
        if (rand() % 2 == 0) {
            blackAI = ai1;
            whiteAI = ai2;
        } else {
            blackAI = ai2;
            whiteAI = ai1;
        }
    }
}
void Game::PrintEval(int total, const std::vector< std::vector<int> >& blackWinArray, const std::vector<double>& timeCostArray) 
{
    int IDnum = playerIDList.size();
    if (settings.savePath[0] != '\0') {
        FILE *pFile;
        pFile = fopen(settings.savePath, "w");
        if (pFile == NULL) {
            printf("Filename %s invalid!\n", settings.savePath);
            exit(1);
        }
        fprintf(pFile, "%d\n", total);
        fprintf(pFile, "%d\n", IDnum);
        for (int i = 0; i < IDnum; ++i) {
            fprintf(pFile, "%s\n", aiMap[playerIDList[i]].name);
        }
        for (int i = 0; i < IDnum; ++i) {
            for (int j = 0; j < IDnum; ++j) {
                int blackWinTimes = blackWinArray[i][j];
                fprintf(pFile, "%d\n", blackWinTimes);
            }
        }
        fclose(pFile);
    } else {
        printf("Total round: %d\n", total);
        printf("First\\Last ");
        for (int i = 0; i < IDnum; ++i) {
            printf("| %10.10s", aiMap[playerIDList[i]].name);
        }
        printf("\n");

        for (int i = 0; i < IDnum; ++i) {
            printf("%10.10s ", aiMap[playerIDList[i]].name);
            for (int j = 0; j < IDnum; ++j) {
                int blackWinTimes = blackWinArray[i][j];
                printf("|    %6.2f%%", (float)blackWinTimes*100/total);
            }
            printf(" Time Cost: %6.3fs\n", timeCostArray[i]/total);
        }
    }
} 
void Game::Evaluate()
{
    int IDnum = playerIDList.size();
    clock_t begin = 0;
    clock_t end   = 0;
    std::vector< std::vector<int> > blackWinTimesArray(IDnum, vector<int>(IDnum));
    std::vector<double> timeCostArray(IDnum);
    settings.isRandFirst = false;
    settings.isPrint     = false;
    settings.isNormal    = false;
    settings.sleepTime   = 0;
    for (int repeat = 1; repeat <= settings.repeatTime; ++repeat) {
        for (int i = 0; i < IDnum; ++i) {
            int  IDBlack = playerIDList[i];
            for (int j = 0; j < IDnum; ++j) {
                int      IDWhite = playerIDList[j];
                NodeType result  = Empty;
                SetGamerAI(IDBlack, IDWhite);
                begin  = clock();
                result = Play(blackAI, whiteAI);
                end    = clock();
                timeCostArray[i] += (double)(end - begin) / CLOCKS_PER_SEC;
                if (result == Black)
                    blackWinTimesArray[i][j]++;
            }
        }
        if (repeat % settings.printLength == 0) {
            PrintEval(repeat, blackWinTimesArray, timeCostArray);
        }
    }

}
void CheckBoolArguments(char* arg, const char* c, bool& dest)
{
    int length = strlen(c);
    if (!strncmp(arg+1, c, length)) {
        if (arg[length+1] == ':') {
            if (arg[length+2] == '0') {
                dest = false;
            } else if (arg[length+2] == '1') {
                dest = true;
            } else {
                printf("Error: Invalud argument: %s\n", arg);
                exit(1);
            }
            argIsValid = true;
        } 
    }
}

void CheckIntArguments(char* arg, const char* c, int& dest)
{
    int length = strlen(c);
    if (!strncmp((arg+1), c, length)) {
        if (arg[length+1] == ':') {
            dest = atoi(arg+length+2);            
            argIsValid = true;
        }
    }
}

void CheckStringArguments(char* arg, const char* c, char* dest)
{
    int length = strlen(c);
    if (!strncmp((arg+1), c, length)) {
        if (arg[length+1] == ':') {
            dest = strcpy(dest, arg+length+2);
            argIsValid = true;
        }
    }
}

/*int main(int argc, char* argv[]) 
{
    Game game;
    game.Initialize();
    srand(time(NULL));

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            argIsValid = false;
            CheckIntArguments (argv[i], "sleep", game.settings.sleepTime);
            CheckIntArguments (argv[i], "eval_round", game.settings.repeatTime);
            CheckIntArguments (argv[i], "eval_print_length", game.settings.printLength);
            CheckBoolArguments(argv[i], "print", game.settings.isPrint);
            CheckBoolArguments(argv[i], "rand_first", game.settings.isRandFirst);
            CheckBoolArguments(argv[i], "normal", game.settings.isNormal);
            CheckBoolArguments(argv[i], "eval_enable", game.settings.isEval);
            CheckStringArguments(argv[i], "file", game.settings.savePath);
            if (!argIsValid) {
                printf("Error: Unknown argument %s\n", argv[i]);
                exit(1);
            }
        } else {
            int ID = atoi(argv[i]);
            AI_Map::iterator it = game.aiMap.find(ID);
            if (it != game.aiMap.end()) {
                game.playerIDList.push_back(ID);
            } else {
                printf("ID: %d does not exist!\n", ID);
                exit(1);
            }
        }
    }

    if (!game.settings.isEval) {
        if (game.playerIDList.size() != 2) {
            printf("We need two IDs for a non-evaluation game!\n");
            exit(1);
        } else {
            game.SetGamerAI(game.playerIDList[0], game.playerIDList[1]);
            NodeType result = game.Play(blackAI, whiteAI);
            if (result == Black) {
                printf("%s Wins! AI: \"%s\" beat AI: \"%s\"\n", blackChar, blackAI.name, whiteAI.name);
            } else if (result == White) {
                printf("%s Wins! AI: \"%s\" beat AI: \"%s\"\n", whiteChar, whiteAI.name, blackAI.name);
            }
        }
    } else {
        game.Evaluate(); 
    }

    return 0;
}*/
