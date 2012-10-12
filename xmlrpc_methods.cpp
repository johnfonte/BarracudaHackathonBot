//
// T-1000
// John Richards <jrichards@barracuda.com>
//
// Implemented by
// @Authors:
// John Fonte
// Michael Hayter
//
// Post-Competition Rework by
// John Fonte

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
#include <deque>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include "xmlrpc_methods.hpp"

using namespace std;
using namespace xmlrpc_c;

struct Point {

private:
    int x;
    int y;

    int square;

    bool owned;
    string owner;

public:
    Point() {
        this->x = -1;
        this->y = -1;
        this->square = -1;
        this->owned = false;
        this->owner = "";

    }
    Point(int x, int y, int square) {
        this->x = x;
        this->y = y;
        this->square = square;
        this->owned = false;
        this->owner = "";
    }
    void setOwner(string owner) {
        if(owner != "") {
            this->owned = true;
            this->owner = owner;
        } else {
            this->owned = false;
            this->owner = "";
        }
    }
    string getOwner() {
        return this->owner;
    }
    bool getIsOwned() {
        return this->owned;
    }
    int getX() {
        return this->x;
    }
    int getY() {
        return this->y;
    }
    int getSquare() {
        return this->square;
    }
};

struct Board
{
    map< int, vector<Point*> > bins;
    vector< vector<Point*> > grid;
    int boardSize;
    int credits;
    int myID;
    int opponentID;
    int bid;
    int squareChoice;
    bool validPath;
    bool textOutputAllowed;
    Board() {
        this->boardSize = 7;
        for (int i = 0; i < boardSize; i++) {
            this->bins[i].resize(this->boardSize);
        }
        this->credits = 0;
        textOutputAllowed = false;
    }
    void set(int myID, int opponentID, int credits, vector< vector<value> > originalBoard, bool validPath, bool textOutputAllowed) {
        this->boardSize = 7;
        this->myID = myID;
        this->opponentID = opponentID;
        this->credits = credits;
        this->validPath = validPath;
        this->textOutputAllowed = textOutputAllowed;

        Point* temp;
        for (int i = 0; i < originalBoard.size(); i++) {
            for (int j = 0; j < originalBoard[i].size(); j++) {
                int key = value_int(originalBoard.at(i).at(j));
                if(this->myID == 0) {
                    temp = new Point(i, j, key);
                } else { 
                    temp = new Point(j, i, key);
                }
                this->bins[key/boardSize].at(key%boardSize) = temp;
                this->grid[i].at(j) = temp;
                if(textOutputAllowed) cout << key << endl;
            }
        }

    }
    Point* getWest(Point* p) {
        if(p != NULL && p->getX() > 0) {
            return grid.at(p->getX()-1).at(p->getY());
        }
        return NULL;
    }
    Point* getNorthWest(Point* p) {
        if(p != NULL && p->getX() > 0 && p->getY() > 0) {
            return grid.at(p->getX()-1).at(p->getY()-1);
        }
        return NULL;
    }
    Point* getNorth(Point* p) {
        if(p != NULL && p->getY() > 0) {
            return grid.at(p->getX()).at(p->getY()-1);
        }
        return NULL;
    }
    Point* getNorthEast(Point* p) {
        if(p != NULL && p->getX() < this->boardSize && p->getY() > 0) {
            return grid.at(p->getX()+1).at(p->getY()-1);
        }
        return NULL;
    }
    Point* getEast(Point* p) {
        if(p != NULL && p->getX() < this->boardSize) {
            return grid.at(p->getX()+1).at(p->getY());
        }
        return NULL;
    }
    Point* getSouthEast(Point* p) {
        if(p != NULL && p->getX() < this->boardSize && p->getY() < this->boardSize) {
            return grid.at(p->getX()+1).at(p->getY()+1);
        }
        return NULL;
    }
    Point* getSouth(Point* p) {
        if(p != NULL && p->getY() < this->boardSize) {
            return grid.at(p->getX()).at(p->getY()+1);
        }
        return NULL;
    }
    Point* getSouthWest(Point* p) {
        if(p != NULL && p->getX() > 0 && p->getY() < this->boardSize) {
            return grid.at(p->getX()-1).at(p->getY()+1);
        }
        return NULL;
    }

};

struct Step
{
    Step* last;
    Point* node;
    int length;
    Step(Step* last, Point* node, bool nodeIsOwned) {
        this->last = last;
        this->node = node;
        if(this->last == NULL && nodeIsOwned) {
            this->length = 0;
        } else if(this->last == NULL && !nodeIsOwned) {
            this->length = 1;
        } else if(nodeIsOwned) {
            this->length = this->last->length;
        } else {
            this->length = this->last->length + 1;
        }
    }
};


struct Path{
    deque<Step*> bfsqueue;
    vector<int> temporary;
    vector<int> final;
    vector<int> bids;
    int bidLength;
    int temporarySize;
    int finalSize;
    Path() {
        this->temporarySize = 0;
        this->finalSize = 0;
        this->bidLength = 0;
    }
    void createBids(int credits) {
        this->bids.resize(this->bidLength);
        for(int i=0; i<this->bids.size(); i++) {
            if(i==this->bids.size()-1) {
                this->bids[i] = credits;
            } else {
                this->bids[i] = credits/this->bids.size();
                credits = credits - this->bids[i];
            }

            if(this->bids[i] == 0) {
                //failsafe
                //set any bid to be number of credits left if we cannot finish the path
                this->bids[i] = credits;
            }
        }
    }
};

///////////////////////// END NEW STRUCTS ////////////////////////////////////



///////////////////////// BEGIN GLOBALS ///////////////////////////////////////

Board board;

Path path;

const int MAX_DEPTH = 10;

bool isReachable[7][7];

///////////////// END GLOBALS ///////////////////////////////////////////////


/////////////////////// BEGIN OUR FUNCTIONS ////////////////////////



int nextGroup(int groupId) {
    return ((groupId+1)%board.boardSize);
}

bool works () {
    // for each thing in path
    // BFS


    for(int i = 0; i < path.temporarySize; i++) {
        // always set temppath to reachable
        int key = path.temporary[i];

        int x = board.bins[key/board.boardSize][key%board.boardSize]->getX();
        int y = board.bins[key/board.boardSize][key%board.boardSize]->getY();
        
        isReachable[x][y] = true;
    }

    for (int i = 0; i < board.boardSize; i++ ) {
        // start BFS on first column/row
        // add to deque
        if(isReachable[0][i])
            path.bfsqueue.push_back(new Step(NULL, board.grid[0].at(i), board.grid[0].at(i)->getIsOwned()));
    }

    Step* s;
    while(!path.bfsqueue.empty()) {
        s = path.bfsqueue.back();
        path.bfsqueue.pop_back();
        int sX = s->node->getX();
        int sY = s->node->getY();
        isReachable[sX][sY] = false; // remove this piece from further searches, cannot double back

        if(s->node->getX() == board.boardSize) {
            // got across! put everything (order does not matter) into final path
            path.bidLength = s->length;
            while(s != NULL) {
                path.final.push_back(s->node->getSquare());
                s = s->last;
            }
            return true;
        } else {
            Point* tempPoint = board.getWest(s->node);
            int tX = tempPoint->getX();
            int tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getNorthWest(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getNorth(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getNorthEast(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getEast(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getSouthEast(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getSouth(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }

            tempPoint = board.getSouthWest(s->node);
            tX = tempPoint->getX();
            tY = tempPoint->getY();
            if(tempPoint != NULL && isReachable[tX][tY] && tempPoint->getIsOwned()) {
                path.bfsqueue.push_back(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto back for low cost option -- we own it
            } else {
                path.bfsqueue.push_front(new Step(s, tempPoint, tempPoint->getIsOwned())); // push onto front for high cost option -- it's open
            }
        }
    }
    //no path found, queue empty
    return false;

}

void search(int groupId, int depth, int depthLimit) {
    if(board.validPath) return;

    if(depth >= depthLimit) { // termination because solution is too long
        return;
    }
    if(path.temporarySize >= board.boardSize) { //only check if path is possibly long enough
        if(works()) {
            board.validPath = true;
            path.finalSize = path.final.size();
            path.createBids(board.credits);
            return;
        }
    }
    if(board.textOutputAllowed) cout << "in search: " << groupId << " " << depth << " " << depthLimit << endl;

    int squaresUsed = 0;
    // descend into tree with unused elements in an attempt to find path
    for(int i=0; i<board.boardSize; i++) {
        int key = i+(groupId*board.boardSize);
        if(!board.bins[key/board.boardSize][key%board.boardSize]->getIsOwned()) {
            path.temporary.push_back(key);
            path.temporarySize++;
            board.bins[key/board.boardSize][key%board.boardSize]->setOwner("us");
            search(nextGroup(groupId), depth+1, depthLimit);
            board.bins[key/board.boardSize][key%board.boardSize]->setOwner("");
            path.temporary.pop_back();
            path.temporarySize--;
        } else {
            // track if all squares in block are occupied
            squaresUsed++;
        }
    }
    // if all squares in block are used, move to next block, do not increase depth, edge case untested
    if(squaresUsed == 7) {
        search(nextGroup(groupId), depth, depthLimit);
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
    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    map<string, value> gstate = paramList.getStruct(0);

    // The 'board' needs a little transformation:
    vector<value> unprocessed_board = value_array(gstate["board"]).vectorValueValue();
    vector< vector<value> > originalBoard;
    for (vector<value>::iterator i = unprocessed_board.begin(); i != unprocessed_board.end(); ++i) {
        originalBoard.push_back(value_array(*i).vectorValueValue());
    }

    // You can access items from the board like this:
    // int someNum = value_int(board[0][0]);

    ///////////////// END BARRACUDA CODE ///////////////////////

    // Access params like this:
    int myID = value_int(gstate["idx"]);
    int opponentID = value_int(gstate["opponent_id"]); // this will affect which direction we use
    int credits = value_int(gstate["credits"]); // sets starting credits
    bool validPath = false;
    bool textOutputAllowed = false;

    board.set(myID, opponentID, credits, originalBoard, validPath, textOutputAllowed);

    if(textOutputAllowed) cout << "initgame, game id: " << value_int(gstate["id"]) << endl;

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

    int groupId = value_int(offer[0])/board.boardSize;

    board.bid = 0;

    for(int i=0; i<board.boardSize; i++) {
        for(int j=0; j<board.boardSize; j++) {
            if(board.grid[i].at(j)->getOwner() == "us") {
                path.temporary.push_back(board.grid[i].at(j)->getSquare());
                path.temporarySize++;
            }
        }
    }

    for(int i=0; i<MAX_DEPTH; i++) {
        search(groupId, 0, i);
    }

    board.squareChoice = -1;
    for (int i = 0; i < path.finalSize; i++) {
        int key = path.final.at(i);
        if(path.final.at(i)/board.boardSize == groupId && !board.bins[key/board.boardSize][key%board.boardSize]->getIsOwned()) {
            board.squareChoice = key;
            break;
        }
    }

    if(board.validPath && board.squareChoice != -1) {
        board.bid = path.bids[0];
    } else {
        board.bid = 0;
    }
    *retval = value_int(board.bid);
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
    // make choice based on previous calculation

    ///////////////// BEGIN BARRACUDA CODE /////////////////////
    vector<value> offer = paramList.getArray(0);
    map<string, value> gstate = paramList.getStruct(1);
    ///////////////// END BARRACUDA CODE ///////////////////////

    board.credits = board.credits - board.bid;
    // See InitGameMethod::execute and GetBidMethod::execute for info on how to use params.
    if(board.textOutputAllowed) cout << "making choice: " << board.squareChoice << endl;

    *retval = value_int(board.squareChoice);
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
    if(board.textOutputAllowed) cout << "SOLUTION RESET" << "\n\n";
    path.temporary.clear();
    path.temporarySize = 0;
    board.validPath = false;
    if (result_string == "you_chose") {
        board.bins[choice/board.boardSize][choice%board.boardSize]->setOwner("us");
    } else if(result_string == "opponent_chose") {
        board.bins[choice/board.boardSize][choice%board.boardSize]->setOwner("them");
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
    if(board.textOutputAllowed) cout << "Game Ended:\n";
    if(board.textOutputAllowed) cout << "Winner:\t" << value_int(result["winner"]) << endl;
    *retval = value_boolean(true);

}
