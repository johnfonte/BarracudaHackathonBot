//
// T-1000
// John Richards <jrichards@barracuda.com>
//
// Implemented by
// @Authors:
// John Fonte
// Michael Hayter
//

#include <map>
#include <string>
#include <vector>
#include <cmath>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "xmlrpc_methods.hpp"

using namespace std;
using namespace xmlrpc_c;

struct Point {
    Point() {
        this->x = -1;
        this->y = -1;
    }
    Point(int x, int y) {
        this->x = x;
        this->y = y;
    }

    int x;
    int y;
};

///////////////////////// BEGIN GLOBALS ///////////////////////////////////////
int boardSize = 7;

map< int, vector<Point> > modifiedBoard; // map is 7 bins of 7

map< int, bool> isUsedUs; // map is 0 - 48
map< int, bool> isUsedThem; // map is 0 - 48

int squaresOwned;

vector<int> tempPath; // temporary path for recursive search
int tempPathSize = 0;

vector<int> winningPath; // store winning path

bool validPath = false; // to skip search

vector<int> pathFinder[7];

int myID = -1;
int opponentID = -1;

int numCredits = -1;
const int MAX_DEPTH = 10;
int pathDepth = 0;

int isReachable[7][7];

int squareChoice = 0;

int solutionCounter = 0;
int SOLUTION_MAX = 10; 
int solutionHeur[49];

int bid;

///////////////// END GLOBALS ///////////////////////////////////////////////


/////////////////////// BEGIN OUR FUNCTIONS ////////////////////////

int nextGroup(int groupId) {
    return ((groupId+1)%boardSize);
}

int dy[] = {-1,0, 1};
int dx[] = {1, 1, 1};

bool works () {
    //for each thing in path
    //   if (exists and is true) andvance

    for (int i=0; i < boardSize; i++) 
        for (int j=0; j<boardSize;j++) 
            isReachable[i][j] = 0;

    for(int i = 0; i < tempPath.size(); i++ ) {
        int key = tempPath[i];

//        cout << "tempPath " << tempPath[i] << endl;
        int x = modifiedBoard[key/boardSize][key%boardSize].x;
        int y = modifiedBoard[key/boardSize][key%boardSize].y;
//        cout << "y = " << y << " x= " << x << endl;
        
        isReachable[y][x] = -1;
    }

/*
    cout << "Reachable Grid 1\n";
    for (int i=0; i < boardSize; i++) {
        for (int j=0; j<boardSize;j++){ 
            cout << (isReachable[i][j])  << " ";
        }
        cout << endl;
    }
*/
    for (int i = 0; i < boardSize; i++ ) {
        if (isReachable[i][0] == -1)
            isReachable[i][0] = 1;
    }
    for (int j = 0; j <boardSize; j++) {
        for (int i = 0; i < boardSize; i++) {
            if (isReachable[i][j] == 1) {
                for(int k=0; k< 3; k++) {
                    int ny = i + dy[k];
                    int nx = j + dx[k];

                    if (isReachable[ny][nx] == -1) {
                        isReachable[ny][nx] = 1;
                    }
                }
            }
        }
    }
/*
    cout << "Reachable Grid 2\n";
    for (int i=0; i < boardSize; i++) {
        for (int j=0; j<boardSize;j++){ 
            cout << (isReachable[i][j])  << " ";
        }
        cout << endl;
    }
*/
    //check if last col has a 1 in it
    for (int i = 0; i <boardSize; i++) {
        if (isReachable[i][boardSize-1] == 1) {
            return true;
        }
    }
    return false;

}

void search(int groupId, int depth, int depthLimit, int currentCredits) {
    if(validPath) return;
    if(solutionCounter>=SOLUTION_MAX) {
        return;
    }
    if(depth > depthLimit) { // base case, termination
        return;
    }
    if(tempPathSize >= boardSize) {
        if(works()) {
            winningPath = tempPath;
            validPath = true;
            pathDepth = winningPath.size();
            solutionCounter++;
//            cout << "winning path size: " << pathDepth << endl;
            for (int i = 0; i < pathDepth; i++) {
                int key = winningPath.at(i);
                int x = modifiedBoard[key/boardSize].at(key%boardSize).x;
                int y = modifiedBoard[key/boardSize].at(key%boardSize).y;
                cout << "i= " << i <<  " x: " << x << " y: " << y << endl;
            }
            return;
        }
    }
//    cout << "in search: " << groupId << " " << depth << " " << depthLimit << " " << currentCredits << endl;
    int currentBid = currentCredits/20;
    if(currentCredits <= 0) {
        return;
    }
    // if found path, return (when 7 or more elements)
    for(int i=0; i<boardSize; i++) {
        int squareNumber = i+(groupId*boardSize);
//        cout << "squareNumber: " << squareNumber << endl;
        if(!isUsedThem[squareNumber] && !isUsedUs[squareNumber]) {
            tempPath.push_back(squareNumber);
            tempPathSize++;
            isUsedUs[squareNumber] = true;
            search(nextGroup(groupId), depth+1, depthLimit, currentCredits - currentBid);
            isUsedUs[squareNumber] = false;
            tempPath.pop_back();
            tempPathSize--;
        } else {
            // do nothing, continue checking for search
        }
    }
}



// ------------------------------------------------------------------------
// PingMethod
// ------------------------------------------------------------------------

PingMethod::PingMethod()
{
}

// ------------------------------------------------------------------------

PingMethod::~PingMethod()
{
}

// ------------------------------------------------------------------------

void
PingMethod::execute(const paramList& paramList, value* const retval)
{
    *retval = value_boolean(true);
}

// ------------------------------------------------------------------------
// InitGameMethod
// ------------------------------------------------------------------------

InitGameMethod::InitGameMethod()
{
}

// ------------------------------------------------------------------------

InitGameMethod::~InitGameMethod()
{
}

// ------------------------------------------------------------------------

void
InitGameMethod::execute(const paramList& paramList, value* const retval)
{
    map<string, value> gstate = paramList.getStruct(0);

    // Access params like this:
    myID = value_int(gstate["idx"]);
    opponentID = value_int(gstate["opponent_id"]); // this will affect which direction we use

    numCredits = value_int(gstate["credits"]); // sets global with starting credits

    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    // The 'board' needs a little transformation:
    vector<value> unprocessed_board = value_array(gstate["board"]).vectorValueValue();
    vector< vector<value> > originalBoard;
    for (vector<value>::iterator i = unprocessed_board.begin(); i != unprocessed_board.end(); ++i) {
        originalBoard.push_back(value_array(*i).vectorValueValue());
    }
    ///////////////// END BARRACUDA CODE ///////////////////////



    for (int i = 0; i < boardSize; i++) {
        modifiedBoard[i].resize(boardSize);
    }

    //Create modified board

    for (int i = 0; i < originalBoard.size(); i++) {
        for (int j = 0; j < originalBoard[i].size(); j++) {
            int key = value_int(originalBoard.at(i).at(j));
            if(myID == 0) {
                modifiedBoard[key/boardSize].at(key%boardSize).x = j;
                modifiedBoard[key/boardSize].at(key%boardSize).y = i;
            } else { 
                modifiedBoard[key/boardSize].at(key%boardSize).x = i;
                modifiedBoard[key/boardSize].at(key%boardSize).y = j;
            }
            isUsedUs[key] = false;
            isUsedThem[key] = false;
//            cout << key << endl;
        }
    }

    int currentElement;
    cout << "initgame, game id: " << value_int(gstate["id"]) << endl;
/*    for(int i=0; i<boardSize; i++) {
        for (int j = 0; j < boardSize; j++)
        {
            cout << "row: " << i << "\tcolumn: " << j << "\toriginal value: ";
            currentElement = value_int(originalBoard.at(i).at(j));
            cout << currentElement;
            currentElement = modifiedBoard[i].at(j).x;
            cout << "\nx of modified: " << currentElement << "\ty of modified: ";
            currentElement = modifiedBoard[i].at(j).y;
            cout << currentElement << "\tmodified value: " << i*7+j << "\n\n";
        }
    }
*/
    validPath = false;
    tempPath.clear();
    tempPathSize = 0;
    solutionCounter = 0;

/*    for(int i=0; i<7; i++) {
        for (int j = 0; j < 20; j++) {
            for(int k=0; k<20; k++) {
                for(int l=0; l<100; l++) {
                    alreadyVisited[i][j][k][l] = false;
                }
            }
        }
    }
*/

    // You can access items from the board like this:
//    int someNum = value_int(board[0][0]);

    *retval = value_boolean(true);
}

// ------------------------------------------------------------------------
// GetBidMethod
// ------------------------------------------------------------------------

GetBidMethod::GetBidMethod()
{
}

// ------------------------------------------------------------------------

GetBidMethod::~GetBidMethod()
{
}

// ------------------------------------------------------------------------

void
GetBidMethod::execute(const paramList& paramList, value* const retval)
{
    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    vector<value> offer = paramList.getArray(0);
    map<string, value> gstate = paramList.getStruct(1);
    ///////////////// END BARRACUDA CODE ///////////////////////

    // See InitGameMethod::execute for info on how to use params.
    // Access the offer vector like this:

    int currentDepth = 0;
    int groupId = value_int(offer[0])/boardSize;

    bid = 0;


    int key;
    for (int i = 0; i < boardSize; i++) {
        for(int j = 0; j < boardSize; j++) {
            key = i+boardSize*j;
            if(isUsedUs[key] == true)
                tempPath.push_back(key);
                tempPathSize++;
        }
    }
    for(int i=0; i<MAX_DEPTH; i++) {
        search(groupId, 0, i, numCredits);
    }
    if(validPath) {
        cout << "Winning Path:\t";
        for(int i = 0; i<winningPath.size(); i++) {
            cout << winningPath.at(i) << "\t";
        }
        cout << endl;
    }

    squareChoice = -1;
    for (int i = 0; i < winningPath.size(); i++) {
        if(winningPath.at(i)/boardSize == groupId && !isUsedUs[winningPath.at(i)] && !isUsedThem[winningPath.at(i)]) {
            squareChoice = winningPath.at(i);
            break;
        }
    }

    if(validPath && squareChoice != -1) {
        if((pathDepth-squaresOwned) > 0) {
            bid = min(numCredits, numCredits / (pathDepth-squaresOwned));
        } else if(pathDepth == 2) {
            bid = numCredits/2 - 1;
        } else if(pathDepth==1){
            bid = numCredits;
        } else {
            bid = min(numCredits, 9);
        }
    } else {
        bid = 0;
    }
    *retval = value_int(bid);
}

// ------------------------------------------------------------------------
// MakeChoiceMethod
// ------------------------------------------------------------------------

MakeChoiceMethod::MakeChoiceMethod()
{
}

// ------------------------------------------------------------------------

MakeChoiceMethod::~MakeChoiceMethod()
{
}

// ------------------------------------------------------------------------

void
MakeChoiceMethod::execute(const paramList& paramList, value* const retval)
{
    // update isUsedUs
    // do tree calculations for next time

    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    vector<value> offer = paramList.getArray(0);
    map<string, value> gstate = paramList.getStruct(1);
    ///////////////// END BARRACUDA CODE ///////////////////////

    numCredits = numCredits - bid;
    // See InitGameMethod::execute and GetBidMethod::execute for info on how to use params.
//    cout << "making choice: " << squareChoice << endl;
    squaresOwned++;

    *retval = value_int(squareChoice);
}

// ------------------------------------------------------------------------
// MoveResultMethod
// ------------------------------------------------------------------------

MoveResultMethod::MoveResultMethod()
{
}

// ------------------------------------------------------------------------

MoveResultMethod::~MoveResultMethod()
{
}

// ------------------------------------------------------------------------

void
MoveResultMethod::execute(const paramList& paramList, value* const retval)
{
    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    map<string, value> result = paramList.getStruct(0);
    ///////////////// END BARRACUDA CODE ///////////////////////

    // Use the result struct like you would use the game state struct.
    // See other execute methods for more info.
    string result_string = value_string(result["result"]);
    int choice = value_int(result["choice"]);
//    cout << "SOLUTION RESET" << "\n\n";
    tempPath.clear();
    tempPathSize = 0;
    validPath = false;
    if (result_string == "you_chose") {
        isUsedUs[choice] = true;
    } else if(result_string == "opponent_chose") {
        isUsedThem[choice] = true;
    }

    *retval = value_boolean(true);
}

// ------------------------------------------------------------------------
// GameResultMethod
// ------------------------------------------------------------------------

GameResultMethod::GameResultMethod()
{
}

// ------------------------------------------------------------------------

GameResultMethod::~GameResultMethod()
{
}

// ------------------------------------------------------------------------

void
GameResultMethod::execute(const paramList& paramList, value* const retval)
{
    map<string, value> result = paramList.getStruct(0);
    cout << "Game Ended:\n";
    cout << "Winner:\t" << value_int(result["winner"]) << endl;
    *retval = value_boolean(true);

}
