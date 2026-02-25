// ProjectTwo.cpp
// CS-300 Project Two: Advising Assistance Program
// Daniel Mitchell
// daniel.mitchell1@snhu.edu
// February 22, 2026
//
// Single-file solution (no CSVParser headers) per project guidance.

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

// ----------------------------
// Data Model
// ----------------------------
struct Course {
    string courseNumber;              // e.g., CSCI400
    string title;                     // e.g., Large Software Development
    vector<string> prerequisites;     // e.g., ["CSCI301", "CSCI350"]
};

// ----------------------------
// BST Node
// ----------------------------
struct TreeNode {
    Course course;
    TreeNode* left;
    TreeNode* right;

    TreeNode(const Course& c) : course(c), left(nullptr), right(nullptr) {}
};

// ----------------------------
// String Helpers
// ----------------------------
static inline string trim(const string& s) {
    size_t start = 0;
    while (start < s.size() && isspace(static_cast<unsigned char>(s[start]))) {
        start++;
    }

    size_t end = s.size();
    while (end > start && isspace(static_cast<unsigned char>(s[end - 1]))) {
        end--;
    }

    return s.substr(start, end - start);
}

static inline string toUpper(string s) {
    for (char& ch : s) {
        ch = static_cast<char>(toupper(static_cast<unsigned char>(ch)));
    }
    return s;
}

static vector<string> splitCSVLine(const string& line) {
    // Simple CSV split: this project’s input format is comma-separated without quoted commas.
    vector<string> tokens;
    string token;
    stringstream ss(line);

    while (getline(ss, token, ',')) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

// ----------------------------
// BST Operations
// ----------------------------
static TreeNode* insertNode(TreeNode* root, const Course& course) {
    if (root == nullptr) {
        return new TreeNode(course);
    }

    if (course.courseNumber < root->course.courseNumber) {
        root->left = insertNode(root->left, course);
    }
    else if (course.courseNumber > root->course.courseNumber) {
        root->right = insertNode(root->right, course);
    }
    else {
        // If duplicate key appears, overwrite with latest
        root->course = course;
    }

    return root;
}

static const Course* searchCourse(const TreeNode* root, const string& courseNumber) {
    if (root == nullptr) {
        return nullptr;
    }

    if (courseNumber == root->course.courseNumber) {
        return &root->course;
    }
    else if (courseNumber < root->course.courseNumber) {
        return searchCourse(root->left, courseNumber);
    }
    else {
        return searchCourse(root->right, courseNumber);
    }
}

static void inOrderPrint(const TreeNode* root) {
    if (root == nullptr) {
        return;
    }
    inOrderPrint(root->left);
    cout << root->course.courseNumber << ", " << root->course.title << endl;
    inOrderPrint(root->right);
}

static void freeTree(TreeNode* root) {
    if (root == nullptr) {
        return;
    }
    freeTree(root->left);
    freeTree(root->right);
    delete root;
}

// ----------------------------
// Validation Helpers
// ----------------------------
static bool containsCourseNumber(const vector<string>& allCourseNumbers, const string& courseNumber) {
    return find(allCourseNumbers.begin(), allCourseNumbers.end(), courseNumber) != allCourseNumbers.end();
}

// ----------------------------
// File Loading
// ----------------------------
static bool loadCoursesFromFileBST(const string& filename, TreeNode*& root, string& errorMessage) {
    errorMessage.clear();

    ifstream file(filename);
    if (!file.is_open()) {
        errorMessage = "ERROR: Could not open file.";
        return false;
    }

    vector<Course> tempCourses;
    vector<string> allCourseNumbers;

    string line;
    while (getline(file, line)) {
        line = trim(line);
        if (line.empty()) {
            continue;
        }

        vector<string> tokens = splitCSVLine(line);
        if (tokens.size() < 2) {
            errorMessage = "ERROR: Invalid line format: " + line;
            return false;
        }

        Course c;
        c.courseNumber = toUpper(tokens[0]);
        c.title = tokens[1];

        for (size_t i = 2; i < tokens.size(); i++) {
            string prereq = toUpper(tokens[i]);
            if (!prereq.empty()) {
                c.prerequisites.push_back(prereq);
            }
        }

        tempCourses.push_back(c);
        allCourseNumbers.push_back(c.courseNumber);
    }

    // Validate that all prerequisites exist in the file.
    for (const Course& c : tempCourses) {
        for (const string& prereq : c.prerequisites) {
            if (!containsCourseNumber(allCourseNumbers, prereq)) {
                errorMessage = "ERROR: Prerequisite not found: " + prereq;
                return false;
            }
        }
    }

    // Clear any previously-loaded data, then rebuild the tree.
    freeTree(root);
    root = nullptr;

    for (const Course& c : tempCourses) {
        root = insertNode(root, c);
    }

    return true;
}

// ----------------------------
// Printing One Course
// ----------------------------
static void printSingleCourse(const TreeNode* root, const string& userInputCourseNumber) {
    string courseNumber = toUpper(trim(userInputCourseNumber));
    const Course* course = searchCourse(root, courseNumber);

    if (course == nullptr) {
        cout << "Course not found." << endl;
        return;
    }

    cout << course->courseNumber << ", " << course->title << endl;

    if (course->prerequisites.empty()) {
        cout << "Prerequisites: None" << endl;
        return;
    }
    
    cout << "Prerequisites: ";
    for (size_t i = 0; i < course->prerequisites.size(); i++) {
        const Course* prereqCourse = searchCourse(root, course->prerequisites[i]);
        if (prereqCourse != nullptr) {
            cout << prereqCourse->courseNumber << " (" << prereqCourse->title << ")";
        } else {
            cout << course->prerequisites[i] << " (Title not found)";
        }

        if (i + 1 < course->prerequisites.size()) {
            cout << ", ";
        }
    }
    cout << endl;
    
}

// ----------------------------
// Menu
// ----------------------------
static void printMenu() {
    cout << "1. Load Data Structure." << endl;
    cout << "2. Print Course List." << endl;
    cout << "3. Print Course." << endl;
    cout << "9. Exit" << endl;
}

static int readMenuChoice() {
    // Read full line to avoid input stream issues.
    string input;
    getline(cin, input);
    input = trim(input);

    // Try to parse as int.
    try {
        return stoi(input);
    }
    catch (...) {
        return -1; // invalid
    }
}

// ----------------------------
// Main
// ----------------------------
int main() {
    cout << "Welcome to the course planner." << endl;

    TreeNode* root = nullptr;
    bool dataLoaded = false;

    while (true) {
        cout << endl;
        printMenu();
        cout << endl;
        cout << "What would you like to do? ";

        int choice = readMenuChoice();

        if (choice == 1) {
            cout << "Enter the file name: ";
            string filename;
            getline(cin, filename);
            filename = trim(filename);

            string errorMessage;
            bool ok = loadCoursesFromFileBST(filename, root, errorMessage);

            if (ok) {
                dataLoaded = true;
            }
            else {
                dataLoaded = false;
                cout << errorMessage << endl;
            }
        }
        else if (choice == 2) {
            if (!dataLoaded) {
                cout << "Please load data first (option 1)." << endl;
                continue;
            }

            cout << "Here is a sample schedule:" << endl;
            cout << endl;
            inOrderPrint(root);
            cout << endl;

        }
        else if (choice == 3) {
            if (!dataLoaded) {
                cout << "Please load data first (option 1)." << endl;
                continue;
            }

            cout << "What course do you want to know about? ";
            string courseNumber;
            getline(cin, courseNumber);

            printSingleCourse(root, courseNumber);

            cout << endl;

        }
        else if (choice == 9) {
            cout << "Thank you for using the course planner!" << endl;
            break;
        }
        else {
            // echo invalid option number if numeric, otherwise generic.
            if (choice >= 0) {
                cout << choice << " is not a valid option." << endl;
            }
            else {
                cout << "That is not a valid option." << endl;
            }
        }
    }

    freeTree(root);
    return 0;
}
