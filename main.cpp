//=================================================================
// Author       : Daniel Geers
// Mail         : geers[at]cs[dot]uni-frankfurt[dot]de
// Created      : 27.12.2014
// Description  : Game Of Life in OOP as C++ implementation 
//=================================================================

#include <iostream>
#include <string>
#include <windows.h> // for sleep() between tasks - only works on windows OS !
using namespace std;


// class to create a random number
class Zufall
{
	public:
		Zufall(unsigned int anfang);
		unsigned int ziehe_zahl();
	private:
		unsigned int x;
};

// random numbers, compare to D. Knuth
Zufall::Zufall(unsigned int Startwert){ x = Startwert; }

unsigned int Zufall::ziehe_zahl() {
	unsigned int a = 16807; // a = 7^5
	unsigned int q = 127773, r = 2836, k;
	k = x / q;
	x = a*(x - k*q) - r*k;
	return x;
}

// class for the game of life simulation
class GameOfLife{
public:
	GameOfLife(int rows, int cols);
	~GameOfLife();
	void clear();
	void set(int row, int col, int value);
	void set(int row, int col, const char* values);
	int get(int row, int col);
	void print();
	void print(char background); // for more fun
	void advance();
	void set_periodic(bool periodic);
	bool is_periodic();
	void randomize(Zufall z);

	// HELPERs and/or FUN:
	bool isValid(int row, int col);  // additional helper function vs repeating myself
	int abs(int number); // helper to return positive numbers only
	char getBackground();
	void setBackground(char background);

private:
	int row;
	int col;
	int** gameboard;
	bool m_periodic = false;
	char background = '.'; // custom fun
};

// class constructor, allocates memory for gameboard[rows][cols]
GameOfLife::GameOfLife(int rows, int cols){
	row = rows; col = cols;
	gameboard = new int*[rows];
	for (int i = 0; i < rows; i++){
		gameboard[i] = new int[rows];
	}
	// initialize the gameboard with 0's
	clear();
}

// class destructor
GameOfLife::~GameOfLife(){

	for (int i = 0; i < row; i++){
		if (gameboard[i] != 0) gameboard[i] = 0;
		delete gameboard[i];
		gameboard[i] = nullptr;
	}
}

// creates random entries into the gameboard
void GameOfLife::randomize(Zufall z){
	cout << z.ziehe_zahl() % 2 << endl;
	for (int i = 0; i < row; i++){
		for (int j = 0; j < col; j++){
			gameboard[i][j] = z.ziehe_zahl() % 2;
		}
	}
}

// clears the gameboard (= sets all field values to 0)
void GameOfLife::clear(){
	for (int i = 0; i < row; i++){
		for (int j = 0; j < col; j++){
			gameboard[i][j] = 0;
		}
	}
}

// HELPER: returns true, if rows and cols are in range of the instanciated object
bool GameOfLife::isValid(int row, int col){
	// are rows / cols valid?:
	if ((row < 0) || (row >= this->row)){
		//cout << "[debug:] row does not fit !" << endl;
		return false;
	}
	if ((col < 0) || (col >= this->col)){
		//cout << "[debug:] col does not fit !" << endl;
		return false;
	}
	return true;
}

// sets a boolean value at a given position (row, col); terminates if col or row is out of bounds

void GameOfLife::set(int row, int col, int value){
	if (is_periodic() == false){
		if (isValid(row, col) && (value == 0 || value == 1)){
			gameboard[row][col] = value;
			// cout << "\t[SET():] gameboard[" << row << "][" << col << "] was set to " << this->get(row, col) << endl;
		}
		// case: out of bounds
		else { return; }
	}
	// for periodic playfields:
	/* first attempt, does not work that way.
	else {gameboard[abs(row % (this->row))][abs(col % (this->col))] = value;}
	*/
		if(row >= this->row){row = row%this->row;}
		else if(row < 0){row = this->row+row;}

		if(col >= this->col){col = col%this->col;}
		else if(col < 0){col = this->col+col;}

		if(value == 0){ gameboard[row][col] = 0;}
		if(value == 1){ gameboard[row][col] = 1;}

}

// parses a string to set multiple values at a time
void GameOfLife::set(int row, int col, const char* values){
	// are rows / cols valid?:
	if (this->is_periodic() == false){
		if (!isValid(row, col)){ cout << "ERROR" << endl; return; } // stop if something is wrong
		else {
			string parseString = (string)values;
			for (int i = 0; i <= parseString.length() - 1; i++){
				if (parseString[i] == 'O'){ set(row, col + i, 1); }
				else if (parseString[i] == '.'){ set(row, col + i, 0); }
				else { this->set(row, i + 1, 'E'); } // for debugging purpose only: errorcase (should not happen!)
			}
		}
	}
	else {
		string parseString = (string)values;
		for (int i = 0; i <= parseString.length() - 1; i++){
			if (parseString[i] == 'O'){ set(row, col + i, 1); }
			else if (parseString[i] == '.'){ set(row, col + i, 0); }
			else { this->set(row, i + 1, 9); } // for debugging purpose only: errorcase (should not happen!)
		}
	}

}

// helper: return number as positive |number|
int GameOfLife::abs(int number){
	if (number < 0){ return -number; }
	else return number;
}

// returns the int value (0 or 1) of a gameboard tile !! IMPORTANT FEATURE: RETURNS 0 IF out of bounce !!
int GameOfLife::get(int row, int col){
	// for non-periodic gameboards:
	if (is_periodic() == false){
		if (isValid(row, col)){return gameboard[row][col];}
		else {return 0;}
	}
	// for periodic gameboards:
	else{

		if(row >= this->row){row = row%this->row;}
		else if(row < 0){row = this->row+row;}

		if(col >= this->col){col = col%this->col;}
		else if(col < 0){col = this->col+col;}
		return gameboard[row][col];

		// return gameboard[abs(row%(this->row))][abs(col%(this->col))];

	}

}

// evolve the gameboard for the next turn
void GameOfLife::advance(){
	// create a new gameboard where the future step(s) will be saved:
	GameOfLife temp(this->row, this->col);
	// check for each cell, if it would survive next round:
	// for each tile, compute the survival chance and put it into the tempboard:
	for (int i = 0; i < row; i++){
		for (int j = 0; j < col; j++){
			                 // left              // right              // upper left             //upper center        // upper right            // bottom left            // bottom center      // bottom right
			int neighbors = this->get(i, j - 1) + this->get(i, j + 1) + this->get(i - 1, j - 1) + this->get(i - 1, j) + this->get(i - 1, j + 1) + this->get(i + 1, j - 1) + this->get(i + 1, j) + this->get(i + 1, j + 1);
			// if (neighbors > 0) {cout << "array["<<i<<","<<j<<"] has "<<neighbors<<" neighbors" << endl;} // DEBUG
 			if (this->get(i, j) == 0 && neighbors == 3){
				temp.set(i, j, 1); // these tiles will survive next round
			}
			else if (this->get(i, j) == 1 && (neighbors > 3 || neighbors < 2)){
				temp.set(i, j, 0);   // this tile will not survive next round
			}
			else temp.set(i, j, this->get(i, j));
		}
	}

	// copy computed values from temp array into target array:
	for (int i = 0; i < row; i++){
		for (int j = 0; j < col; j++){
			this->set(i, j, temp.get(i, j));
		}
	}

}

// prints out the current gameboard
void GameOfLife::print(){
	// cout << "Printing out current " << this->row << " x " << this->col << " board:" << endl << endl;
	for (int i = 0; i < this->row; i++){
		for (int j = 0; j < this->col; j++){
			// cout << "\trow=" << i << " col=" << j << endl;
			if (gameboard[i][j] == 0){ cout << ".";}
			else if (gameboard[i][j] == 1){cout << "O";}
			else { cout << "E"; } // E means: Error - just for debugging!
		}
		cout << endl; // next line after each rowx
	}
	cout << endl;
}

// prints out the current gameboard with a custom background
void GameOfLife::print(char background){
	// cout << "Printing out current " << this->row << " x " << this->col << " board:" << endl << endl;
	for (int i = 0; i < this->row; i++){
		for (int j = 0; j < this->col; j++){
			// cout << "\trow=" << i << " col=" << j << endl;
			if (gameboard[i][j] == 0){ cout << background; }
			else if (gameboard[i][j] == 1){ cout << "O"; }
			else { cout << "E"; } // E means: Error - just for debugging!
		}
		cout << endl; // next line after each rowx
	}
	cout << endl;
}

// gets the current background:
char GameOfLife::getBackground(){
	return this->background;
}

// sets the current background:
void GameOfLife::setBackground(char background){
	this->background = background;
}

// changes periodic to be true or false
void GameOfLife::set_periodic(bool periodic){
	this->m_periodic = periodic;
}

// checks and returns true if the gameboard is periodic; returns false otherwise
bool GameOfLife::is_periodic(){
	if (this->m_periodic == true) return true;
	else return false;
}

// starts GameOfLife and defines the amount of iterations to advance
void PlayGameOfLife(GameOfLife& g, int iterations){
	g.print();
	for (int i = 0; i < iterations; ++i){
		g.advance();
		cout << endl;
		// for custom fun:
		if (g.getBackground() != '.'){ g.print(g.getBackground()); }
		else g.print();
		Sleep(250);
	}
}

int main(){


	Sleep(1500);
	cout << "Creating Game of Life with 5 iterations" << endl;

	GameOfLife g(10, 10);
	g.set(2, 2, ".O..O.");
	g.set(3, 2, "O.OO.O");
	g.set(4, 2, ".O..O.");
	g.set(5, 2, ".O..O.");
	g.set(6, 2, "O.OO.O");
	g.set(7, 2, ".O..O.");
	PlayGameOfLife(g, 5);


	cout << "Creating Game of Life (5x5, 20 iterations) with Toric Playfield: " << endl;
	Sleep(2000);

	GameOfLife toricGoL(5, 5);
	toricGoL.set_periodic(true);
	toricGoL.set(-3, -3, ".O.");
	toricGoL.set(-2, -3, "..O");
	toricGoL.set(-1, -3, "OOO");
	PlayGameOfLife(toricGoL, 20);

	cout << "##### AUFGABE 3 (komplett:) #####" << endl;
	Sleep(2000);
	GameOfLife randomGOF(30, 30);
	//randomGOF.set_periodic(true);
	randomGOF.randomize(Zufall(3));
	PlayGameOfLife(randomGOF, 150);

	Sleep(1000);
	cout << "Creating AWESOME Game of Life customizable background: " << endl;
	Sleep(1000);
	GameOfLife z(31, 31);
	z.setBackground(' ');
	z.set(5, 0, "OOOOOOOOOOOOO.....OOOOOOOOOOOOO");
	z.set(16, 0, "OOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO");
	PlayGameOfLife(z, 300);


	return 0;
}
