#include <iostream>
#include <vector>
#include <unordered_set>
#include <fstream>
#include <string>
#include <unordered_map>
#include <random>
#include <conio.h>
#include <thread>
#include <chrono>
#include <atomic>
using namespace std;

unordered_set<string> dictionary;

//Used to read the text file which contails all words in the oxford dicitonary
void loadDictionary(const string& path) {
	ifstream f(path);

	if (!f.is_open()) {
		cerr << "Error opening the file!";
		exit(10);
	}

	string word;

	//loads each word into the unordered map
	while (getline(f, word)) {
		dictionary.insert(word);
	}
}

//When a decision is made between a vowel and a consonant different text options are displayed, this is to prevent duplication of code
bool vowelOrConsonant(int& mainChoice, const int& secondaryChoice, int limit, string type, string secondaryType) {

	if (mainChoice == limit) {
		cout << "ERROR: " << type << "s exceed maximum limit, you must have at least " << secondaryType << "\n";
		return false;
	}

	mainChoice++;

	int optionsLeft = 9 - (mainChoice + secondaryChoice);
	cout << type << " chosen " << optionsLeft;

	if (optionsLeft == 1) {
		cout << " choice remaining\n";
	}
	else {
		cout << " choices remaining\n";
	}

	return true;

}

//This randomly selects a letter from the vowel or consonant vector
char letterGenarator(vector<pair<char, int>>& letterSet, mt19937& seed) {
	int total = 0;

	//Counts the total amount of the remaining letters in a given vector
	for (const auto& pair : letterSet) {
		total += pair.second;
	}

	//Creates a random number distribution from 1 to the total with equal probabilty for each number, the number is the position of a letter in the vector
	uniform_int_distribution<> dist(1, total);
	int rnd = dist(seed);

	//loops through the given vector adding the totals of each letter
	int letterTotal = 0;
	for (auto& pair : letterSet) {
		letterTotal += pair.second;

		//When letterTotal exceeds rnd decrease the amount of letters by 1 and return the letter that exceeded the total
		if (rnd <= letterTotal) {
			pair.second--;

			char result = pair.first;

			//if the total of a specific letter is 0 remove it from the vector
			if (pair.second == 0) {
				pair = letterSet.back();
				letterSet.pop_back();
			}

			return result;
		}
	}


	return NULL;
}

//This makes it safe to change in one thread and read another
atomic<bool> timeUp(false);

//This waits 30 seconds and sets timeUp to false, this runs in a seprate thread so the main loop can continue running as the timer go's
void timerThread() {
	this_thread::sleep_for(chrono::seconds(30));
	timeUp = true;
}


int main() {

	//The two vectors hold all the vwels and consonnants, I took inspiration from scrabble letter distributions, as countdown was inspired by them
	vector<pair<char, int>> vowels{ {'A', 15}, {'E', 21}, {'I', 13}, {'O', 13}, {'U', 5} };
	vector<pair<char, int>> consonants{ {'B', 2}, {'C', 3}, {'D', 6}, {'F', 3}, {'G', 3}, {'J', 1}, {'K', 1}, {'L', 5}, {'M', 4}, {'N', 8},
	{'P', 4}, {'Q', 1}, {'S', 9}, {'T', 9}, {'V', 1}, {'X', 1}, {'Z', 1}, {'H', 2}, {'R', 9}, {'W', 1}, {'Y', 1} };

	//loads all the words into the unordered map
	loadDictionary("words_alpha.txt");

	while (true) {
		char letter;

		cout << "Hello and welcome to Countdown.\n\nYou will have to choose 9 letters of either vowels or consonants.\n";
		cout << "There is a minimum requirement of at least 3 vowels and at least 4 consonants from the chosen 9 letters.\n";
		cout << "You will be given a score based on your longest valid word.\nYou will get 1 point per letter, if you use all 9 letters you will get 18 points.\n";
		cout << "You will have 4 rounds to play, each round your score will be added to the total goodluck.\n";
		cout << "When you are ready to play type 'y': ";

		cin >> letter;

		if (letter == 'y') {
			break;
		}
	}

	int totalScore = 0;

	//Loops for 4 rounds
	for (size_t indx = 0; indx < 4; indx++) {
		cout << "\n\n\n";

		cout << "Round " << indx + 1 << endl;

		int vowelsCount = 0;
		int consonantsCount = 0;

		cout << "Choose either a vowel or a consonant for your 9 letters (there must be at least 3 vowels and at least 4 consonants) \n";

		char vOrC;

		random_device rd;
		mt19937 seed(rd());

		string finL = "";

		unordered_map<char, int> currentCombination;

		cout << "\n\n\n";

		//this ensures that you cannot exceed 9 letters
		while (vowelsCount + consonantsCount != 9) {
			cout << "'v' for vowel and 'c' for consonant: ";

			cin >> vOrC;

			cout << "\n\n\n";

			if (vOrC != 'v' && vOrC != 'c') {
				cout << "ERROR: Invalid choice please try again\n";
			}

			//gets the choice form the user and puts it through the function and ensures that the minimum amount of consonants and vowels are chosen
			else if (vOrC == 'v') {
				
				if (vowelOrConsonant(vowelsCount, consonantsCount, 5, "Vowel", "4 Consonants")) {
					char chosenLetter = letterGenarator(vowels, seed);

					cout << "You have got: " << chosenLetter << " for your vowel\n";

					finL += chosenLetter;

					currentCombination[chosenLetter]++;

					cout << "Your letters are: " << finL << endl;
				}

			}
			else if (vOrC == 'c') {

				if (vowelOrConsonant(consonantsCount, vowelsCount, 6, "Consonant", "3 Vowels")) {
					char chosenLetter = letterGenarator(consonants, seed);

					cout << "You have got: " << chosenLetter << " for your consonant\n";

					finL += chosenLetter;

					currentCombination[chosenLetter]++;

					cout << "Your letters are: " << finL << endl;
				}

			}
			cout << "\n\n\n";
		}

		cout << "You have finished letter selection you final combination is:\n";
		cout << "                           " << finL << "\n\n";

		cout << "Your 30 seconds starts now, type in valid words using the letters you have been given, your longest word will be scored (enter to submit)\n\n";

		string inputtedWord = "";
		string currentWord;

		//30 second timer starts now
		thread timer(timerThread);

		//duplicates the chosen given combinations of letters
		unordered_map<char, int> tempCombination = currentCombination;

		//Runs until timeUp is true
		while (!timeUp) {
			//Checks if a key is pressed and is non blocking
			if (_kbhit()) {
				//Gets the key instantly 
				char ch = _getch();

				//if user hits enter
				if (ch == '\r') {
					//makes sure that there is a word
					if (!currentWord.empty()) {
						//if the word is larger then 9 characters it denies submission and clears the word
						if (currentWord.size() > 9) {
							cout << "Invalid word! A word cannot contain more letters the the allocated set: " << finL << "\n\n";
							currentWord.clear();
						}
						//If the word does not exist in the unordered map deny submission and clear the word
						else if (dictionary.find(currentWord) == dictionary.end()) {
							cout << endl;
							cout << "Invalid word! Please try again\n\n";

							currentWord.clear();
							//resets the used letter in tempCombination as each letter used decrease its count to prevent exceed the amount of availible letters
							tempCombination = currentCombination;
						}
						//If it passes all checks and the word is valid we get here
						else {
							//If the word is larger then the previous word it gets stored
							if (currentWord.size() > inputtedWord.size()) {
								inputtedWord = currentWord;
							}
							cout << "\nWord submitted: " << currentWord << "\n";
							currentWord.clear();
							//resets the used letter in tempCombination as each letter used decrease its count to prevent exceed the amount of availible letters
							tempCombination = currentCombination;
						}
					}
				}
				//If backspace is pressed
				else if (ch == '\b') { 
					if (!currentWord.empty()) {
						//Add 1 to the count of the leter which was removed
						tempCombination[toupper(currentWord[currentWord.size() - 1])]++;
						//remove the last word entered in the string
						currentWord.pop_back();
						//visually display the backspace by removing the letter
						cout << "\b \b";
					}
				}
				//Ensures that only the valid letters can be inputted
				else if (toupper(ch) != finL[0] && toupper(ch) != finL[1] && toupper(ch) != finL[2] && toupper(ch) != finL[3] && toupper(ch) != finL[4] && toupper(ch) != finL[5]
					&& toupper(ch) != finL[6] && toupper(ch) != finL[7] && toupper(ch) != finL[8]) {
					cout << endl;
					cout << "Invalid option! Choose from the correct selection of letters: " << finL << "\n\n";
					cout << currentWord;
				}
				else {
					//If a valid letter is chosen and its use does not exceed the amount
					if (tempCombination[toupper(ch)] > 0) {
						//decrease the amount of the letter by 1
						tempCombination[toupper(ch)]--;
						//add the letter to the word
						currentWord += tolower(ch);
						//display the letter
						cout << ch;
					}
					//If the use of a valid letter has exceeded its amount
					else {
						cout << endl;
						cout << "Invalid option! You have exceeded the use of this letter, check here: " << finL << "\n\n";
						cout << currentWord;
					}

				}
			}
			//This line pauses the current thread for 0.01 seconds to avoid the loop from running too fast prevent 100% cpu usage 
			this_thread::sleep_for(chrono::milliseconds(10));
		}

		//Gets the size of the largest word
		int points = inputtedWord.size();
		
		//if its a 9 letter word make it 18 points
		if (points == 9) {
			points *= 2;
		}

		//adds the points to the total score
		totalScore += points;

		if (points == 0) {
			cout << "\n\nTime's up! You did not manage to write a valid word this means you scored " << points << " this round!\n";
		}
		else {
			cout << "\n\nTime's up! Your highest score word was: " << inputtedWord << " scoring you " << points << " this round!\n";
		}

		cout << "Your current total score is: " << totalScore << endl;

		//waits for the timer thread to finish to prevent errors
		timer.join();
		//resets timeUp back to false for the next loop around
		timeUp = false;
	}

	cout << "\n\n\n";

	cout << "You have finished the game, you have score a total of " << totalScore << " points!\n";

	char enter = ' ';

	cout << "Push the 'enter' key to exit";

	//Allows user to see total score before choosing to leave
	while (enter != '\r') {
		if (_kbhit())
			enter = _getch();
	}

	return 0;
}