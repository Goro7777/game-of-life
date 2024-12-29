#include <iostream>
#include <cstdlib>
#include <ctime>
using namespace std;

// constants used as default parameters for class World
// can be avoided/replaced if other values are passed to World constructor
const int ROWS = 20;
const int COLS = 20;
const int INITIAL_DBS = 5;
const int INITIAL_ANTS = 100;

// constants used inside classes
// cannot be avoided/replaced
const char EMPTY_SYMBOL = '-';
const char DB_SYMBOL = 'X';
const char ANT_SYMBOL = 'o';
const int STEPS_TO_STARVE = 3;
const int STEPS_TO_BREED_ANT = 3;
const int STEPS_TO_BREED_DB = 8;

class Organism {
    public:
        // Organism: initializes breeding-related variables and organism symbol
        Organism(int theStepsToBreed, char theSymbol) 
            : stepsToBreed(theStepsToBreed), curStepsToBreed(0), symbol(theSymbol) {}
        
        char getSymbol() { return symbol; }
        
        // move: moves the organism to an adjacent (up/right/down/left) empty cell
        //       If several empty cells available, selects one randomly
        // Input: current location of the organism and 
        //       count of organism that can serve as food (needed when overriding the function in a derived class)
        // Output: returns the index in the grid after moving
        virtual int move(Organism** grid, int rows, int cols, int ind, int& totalFoodCount);

        // breedControl: contorls breeding properties and make the organism breed when ready
        void breedControl(Organism** grid, int rows, int cols, int ind, int& orgCount);

        // findAdjacentTargetInds: finds all adjacent (up/right/left/down) cells containing the given target
        // Input: current location of the organism and target symbol
        // Output: returns an array of adjacent cell indexes containing target 
        //         and updates the number of targets/indexes found
        int* findAdjacentTargetInds(Organism** grid, int rows, int cols, int ind, 
                                int& targetsFound, char targetSymbol = EMPTY_SYMBOL);

    private:
        char symbol;
        int stepsToBreed;
        int curStepsToBreed;

        // breed: if finds adjacent empty cells, selects one randomly and creates a new organism in it
        // Output: updates the organism count (when finds an empty cell to breed in)
        void breed(Organism** grid, int rows, int cols, int ind, int& orgCount);

        bool orgHasSymbol(Organism* org, char symbol);
};

class Doodlebug : public Organism {
    public:
        // Doodlebug: initializes starving-related and breeding-related variables and organism symbol
        Doodlebug() 
            : Organism(STEPS_TO_BREED_DB, DB_SYMBOL), stepsToStarve(STEPS_TO_STARVE), curStepsToStarve(0) {}
        
        // move: if finds food in adjacent cell, moves the organism there to eat
        //       if cannot find food, makes the organism starve and possibly die (moves it off the grid)
        //       if the organism does not die, moves it to an emtpy cell (if finds one)
        //       if multiples food/empty cell indexes found, selects one randomly
        // Input: current location of the organism, food count 
        // Output: returns index in the grid after moving
        //         returns -1 if the organism has died
        //         decrements food count if the organism eats 
        virtual int move(Organism** grid, int rows, int cols, int ind, int& ants);
        
        // eat: moves the organism to cell containing food and deletes the food
        int eat(Organism** grid, int rows, int cols, int ind, int& totalFoodCount, char foodSymbol = ANT_SYMBOL);
        
        bool hasStarved() { return curStepsToStarve >= stepsToStarve; }
    
    private:
        int stepsToStarve;
        int curStepsToStarve;
};

class Ant : public Organism {
    public:
        Ant() : Organism(STEPS_TO_BREED_ANT, ANT_SYMBOL) {}
};

class World {
    public:
        World(int theRows = ROWS, int theCOls = COLS, int initialDoodlebugs = INITIAL_DBS, 
              int initialAnts = INITIAL_ANTS);
        ~World();
        
        // step: processes all doodlebugs
        //       processes all ants
        //       prints the grid
        void step();

    private:
        int rows;
        int cols;
        int doodlebugs;
        int ants;
        int time;
        Organism** grid;

        void printGrid();
        
        // doodlebugsStep: each doodlebug
        //          - tries to eat
        //          - if cannot eat, tries to move to an empty cell
        //          - if hungry for long enough, starves/dies
        //          - if doesn't starve/die, breeds if ready
        void doodlebugsStep();
        
        // antsStep: each ant
        //          - tries to move to an empty cell 
        //          - breeds if ready
        void antsStep();  
        
        // findAllOrgInds: finds all the organisms with the given symbol in the grid
        // Output: returns an array of indexes of target organisms 
        int* findAllOrgInds(char symbol, int count);
};

// Prerequisite: random number generator has been seeded
int getRandIntInInterval(int low, int high);
int* getRandIntsInInterval(int low, int high, int count);

int main() {
    srand(time(0));

    World world;
    while (true) {
        cin.ignore();
        world.step();
    }    
}

World::World(int theRows, int theCOls, int initialDoodlebugs, int initialAnts) 
: rows(theRows), cols(theCOls), doodlebugs(initialDoodlebugs), ants(initialAnts), time(0) {
    int cells = rows * cols;
    grid = new Organism*[cells] {nullptr};

    int totalOrganisms = doodlebugs + ants;
    int* organismsIndexes = getRandIntsInInterval(0, cells - 1, totalOrganisms);

    int i = 0;
    while (i < doodlebugs) {
        grid[organismsIndexes[i]] = new Doodlebug;
        i++;
    }
    while (i < totalOrganisms) {
        grid[organismsIndexes[i]] = new Ant;
        i++;
    }

    delete [] organismsIndexes;

    printGrid();
    cout << endl << "Press ENTER to continue\n";
}

World::~World() {
    int cells = rows * cols;
    for (int i = 0; i < cells; i++) {
        if (grid[i] != nullptr)
            delete grid[i];
    }
    delete [] grid;
}

void World::printGrid() {
    cout << "World at time " << time << ":\n\n";
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int ind = i * rows + j;
            if (grid[ind] != nullptr) 
                cout << grid[ind]->getSymbol() << " ";
            else 
                cout << EMPTY_SYMBOL << " ";
        }
        cout << endl;
    }
}

void World::step() {
    time++;
    doodlebugsStep();
    antsStep();
    printGrid();
    cout << endl << "Press ENTER to continue\n";
}

void World::doodlebugsStep() {
    int dbsBeforeBreed = doodlebugs;
    int* dbsToMoveInds = findAllOrgInds(DB_SYMBOL, dbsBeforeBreed);
    for (int i = 0; i < dbsBeforeBreed; i++) {
        int ind = dbsToMoveInds[i];
        int newInd = grid[ind]->move(grid, rows, cols, ind, ants);
        if (newInd >= 0)
            grid[newInd]->breedControl(grid, rows, cols, newInd, doodlebugs);
        else {
            delete grid[ind];
            grid[ind] = nullptr;
            doodlebugs--;
        }
    }

    delete [] dbsToMoveInds;
}

void World::antsStep() {
    int antsBeforeBreed = ants;
    int* antsToMoveInds = findAllOrgInds(ANT_SYMBOL, antsBeforeBreed);
    for (int i = 0; i < antsBeforeBreed; i++) {
        int ind = antsToMoveInds[i];
        ind = grid[ind]->move(grid, rows, cols, ind, ants);
        if (ind >= 0)
            grid[ind]->breedControl(grid, rows, cols, ind, ants);
    }

    delete [] antsToMoveInds;
}

int* World::findAllOrgInds(char symbol, int count) {
    int cells = rows * cols;
    int* inds = new int[count];
    int j = 0;
    for (int i = 0; i < cells && j < count; i++) {
        if (grid[i] != nullptr && grid[i]->getSymbol() == symbol) 
            inds[j++] = i;
    }

    return inds;
}

int Organism::move(Organism** grid, int rows, int cols, int ind, int& totalFoodCount) {
    int adjacentEmptyCellsCount;
    int* adjacentEmptyCellsInds = findAdjacentTargetInds(grid, rows, cols, ind, adjacentEmptyCellsCount);
    int nextInd = ind;

    if (adjacentEmptyCellsCount) {
        int tempInd = getRandIntInInterval(0, adjacentEmptyCellsCount - 1);
        nextInd = adjacentEmptyCellsInds[tempInd];
        
        grid[nextInd] = grid[ind];
        grid[ind] = nullptr;
    }

    delete [] adjacentEmptyCellsInds;
    return nextInd;
}

void Organism::breedControl(Organism** grid, int rows, int cols, int ind, int& orgCount) {
    curStepsToBreed++;
    if (curStepsToBreed >= stepsToBreed) 
        breed(grid, rows, cols, ind, orgCount);
}

void Organism::breed(Organism** grid, int rows, int cols, int ind, int& orgCount) {
    int adjacentEmptyCellsCount;
    int* adjacentEmptyCellsInds = findAdjacentTargetInds(grid, rows, cols, ind, adjacentEmptyCellsCount);
    
    if (adjacentEmptyCellsCount > 0) {
        int tempInd = getRandIntInInterval(0, adjacentEmptyCellsCount - 1);
        int childInd = adjacentEmptyCellsInds[tempInd];
        
        if (grid[ind]->getSymbol() == DB_SYMBOL)
            grid[childInd] = new Doodlebug;
        else if (grid[ind]->getSymbol() == ANT_SYMBOL)
            grid[childInd] = new Ant;
        
        orgCount++;
        curStepsToBreed = 0;
    }

    delete [] adjacentEmptyCellsInds;
}

int* Organism::findAdjacentTargetInds(Organism** grid, int rows, int cols, int ind, int& targetsFound, char targetSymbol) {
    int uppInd, rightInd, downInd, leftInd;
    targetsFound = 0;
    int* possibleMoveIndexes = new int[4];
    
    uppInd = ind - cols;
    if ((uppInd >= 0) && orgHasSymbol(grid[uppInd], targetSymbol))
        possibleMoveIndexes[targetsFound++] = uppInd;
    
    rightInd = ind + 1;
    if ((rightInd % cols != 0) && orgHasSymbol(grid[rightInd], targetSymbol))
        possibleMoveIndexes[targetsFound++] = rightInd;

    downInd = ind + cols;
    if ((downInd < rows * cols) && orgHasSymbol(grid[downInd], targetSymbol))
        possibleMoveIndexes[targetsFound++] = downInd; 

    leftInd = ind - 1;
    if ((ind % cols != 0) && orgHasSymbol(grid[leftInd], targetSymbol))
        possibleMoveIndexes[targetsFound++] = leftInd;
    
    return possibleMoveIndexes;
}

bool Organism::orgHasSymbol(Organism* org, char symbol) {
    if (org == nullptr)
        return (symbol == EMPTY_SYMBOL);
    
    return org->getSymbol() == symbol;
}

int Doodlebug::move(Organism** grid, int rows, int cols, int ind, int& ants) {
    int newInd = eat(grid, rows, cols, ind, ants);
    
    if (newInd == ind) {
        curStepsToStarve++;
        if (hasStarved()) 
            newInd = -1;
        else
            newInd = Organism::move(grid, rows, cols, ind, ants);
    } 

    return newInd;
}

int Doodlebug::eat(Organism** grid, int rows, int cols, int ind, int& totalFoodCount, char foodSymbol) {
    int adjacentFoodCellsCount;
    int* adjacentFoodCellsInds = findAdjacentTargetInds(grid, rows, cols, ind, adjacentFoodCellsCount, foodSymbol);
    int foodInd = -1;

    if (adjacentFoodCellsCount) {
        int tempInd = getRandIntInInterval(0, adjacentFoodCellsCount - 1);
        foodInd = adjacentFoodCellsInds[tempInd];
        
        delete grid[foodInd];
        grid[foodInd] = grid[ind];
        grid[ind] = nullptr;

        totalFoodCount--;
        curStepsToStarve = 0;
    }
    
    delete [] adjacentFoodCellsInds;
    return (foodInd == -1 ? ind : foodInd);
}

int getRandIntInInterval(int low, int high) {
    return (rand() % (high - low + 1) + low);
}

int* getRandIntsInInterval(int low, int high, int count) {  
    int* randInts = new int[count];
    int curCount = 0;
    int nextNum;

    while (curCount < count) {
        bool hasBeenSelected = false;
        nextNum = getRandIntInInterval(low, high);
        for (int i = 0; i < curCount; i++) {
            if (randInts[i] == nextNum) {
                hasBeenSelected = true;
                break;
            }
        }

        if (!hasBeenSelected) 
            randInts[curCount++] = nextNum;
    }

    return randInts;
}
