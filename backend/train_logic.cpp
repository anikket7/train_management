#include <iostream>
#include <string>
#include <queue>
#include <cstdlib>
#include <ctime>
#include <algorithm>
using namespace std;

const int MAX_SEATS = 5;
const int WAITING_LIMIT = 3;
const int TRAIN_COUNT = 10;

// Train format: [Train Name, Source, Destination]
string trains[TRAIN_COUNT][3] = {
    {"Patna Express", "Patna", "Delhi"},
    {"Rajdhani", "Delhi", "Mumbai"},
    {"Goa Superfast", "Mumbai", "Goa"},
    {"Chennai Mail", "Chennai", "Bangalore"},
    {"Punjab Mail", "Amritsar", "Delhi"},
    {"Himalayan Queen", "Shimla", "Delhi"},
    {"Garib Rath", "Kolkata", "Lucknow"},
    {"Duronto Express", "Hyderabad", "Pune"},
    {"Shatabdi", "Bhopal", "Indore"},
    {"Intercity", "Ahmedabad", "Surat"}
};

// Train-specific passenger tracking
struct TrainData {
    string names[MAX_SEATS];
    int ages[MAX_SEATS];
    int pnrs[MAX_SEATS];
    int seatCount;
    queue<string> waitNames;
    queue<int> waitAges;
    queue<int> waitPnrs;
};

TrainData trainData[TRAIN_COUNT];

// Initialize all trains with empty data
void initializeTrains() {
    for (int i = 0; i < TRAIN_COUNT; i++) {
        trainData[i].seatCount = 0;
    }
}

// Helper: convert string to lowercase
string toLower(string s) {
    transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Random PNR generator with uniqueness check
int generatePNR() {
    int pnr;
    bool isUnique;
    
    do {
        isUnique = true;
        pnr = rand() % 90000 + 10000;
        
        // Check if PNR already exists in any train
        for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
            TrainData& currentTrain = trainData[trainIdx];
            
            // Check confirmed passengers
            for (int i = 0; i < currentTrain.seatCount; i++) {
                if (currentTrain.pnrs[i] == pnr) {
                    isUnique = false;
                    break;
                }
            }
            
            if (!isUnique) break;
            
            // Check waiting list
            queue<int> tempPnrs = currentTrain.waitPnrs;
            while (!tempPnrs.empty()) {
                if (tempPnrs.front() == pnr) {
                    isUnique = false;
                    break;
                }
                tempPnrs.pop();
            }
            
            if (!isUnique) break;
        }
    } while (!isUnique);
    
    return pnr;
}

// Find train index from source → destination
int searchTrain(string from, string to) {
    for (int i = 0; i < TRAIN_COUNT; i++) {
        if (toLower(trains[i][1]) == toLower(from) && toLower(trains[i][2]) == toLower(to)) {
            return i;
        }
    }
    return -1;
}

// Book a ticket
void bookTicket() {
    string from, to;
    cout << "\nEnter From Station: ";
    cin >> from;
    cout << "Enter To Station: ";
    cin >> to;

    int trainIndex = searchTrain(from, to);
    if (trainIndex == -1) {
        cout << "No train available from " << from << " to " << to << ".\n";
        return;
    }

    cout << "Train Found: " << trains[trainIndex][0]
         << " (" << trains[trainIndex][1] << " to " << trains[trainIndex][2] << ")\n";

    char confirm;
    cout << "Do you want to book a ticket on this train? (y/n): ";
    cin >> confirm;

    if (confirm != 'y' && confirm != 'Y') {
        cout << "Booking cancelled.\n";
        return;
    }

    string name;
    int age, pnr;

    cout << "Enter Passenger Name: ";
    cin >> name;
    cout << "Enter Age: ";
    cin >> age;

    pnr = generatePNR();

    TrainData& currentTrain = trainData[trainIndex];

    if (currentTrain.seatCount < MAX_SEATS) {
        currentTrain.names[currentTrain.seatCount] = name;
        currentTrain.ages[currentTrain.seatCount] = age;
        currentTrain.pnrs[currentTrain.seatCount] = pnr;
        currentTrain.seatCount++;
        cout << "Ticket Booked. PNR: " << pnr << "\n";
    } else if (currentTrain.waitNames.size() < WAITING_LIMIT) {
        currentTrain.waitNames.push(name);
        currentTrain.waitAges.push(age);
        currentTrain.waitPnrs.push(pnr);
        cout << "All seats full. Added to waiting list. PNR: " << pnr << "\n";
    } else {
        cout << "Waiting list is also full. Cannot book ticket.\n";
    }
}

// Cancel a ticket by PNR
void cancelTicket() {
    int pnr;
    cout << "\nEnter PNR to Cancel: ";
    cin >> pnr;

    bool found = false;
    
    // Search through all trains
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        // Check confirmed passengers
        for (int i = 0; i < currentTrain.seatCount; i++) {
            if (currentTrain.pnrs[i] == pnr) {
                found = true;
                cout << "Ticket Cancelled for PNR: " << pnr << " on " << trains[trainIdx][0] << "\n";

                // Shift remaining passengers
                for (int j = i; j < currentTrain.seatCount - 1; j++) {
                    currentTrain.names[j] = currentTrain.names[j + 1];
                    currentTrain.ages[j] = currentTrain.ages[j + 1];
                    currentTrain.pnrs[j] = currentTrain.pnrs[j + 1];
                }
                currentTrain.seatCount--;

                // Move waiting passenger to confirmed if available
                if (!currentTrain.waitNames.empty()) {
                    currentTrain.names[currentTrain.seatCount] = currentTrain.waitNames.front(); 
                    currentTrain.waitNames.pop();
                    currentTrain.ages[currentTrain.seatCount] = currentTrain.waitAges.front(); 
                    currentTrain.waitAges.pop();
                    currentTrain.pnrs[currentTrain.seatCount] = currentTrain.waitPnrs.front(); 
                    currentTrain.waitPnrs.pop();
                    currentTrain.seatCount++;
                    cout << "First waiting passenger confirmed.\n";
                }
                return;
            }
        }
    }

    if (!found) {
        cout << "PNR not found in confirmed list.\n";
    }
}

// Show confirmed and waiting passengers
void showPassengers() {
    cout << "\n===== All Passengers =====\n";
    cout << "Confirmed List:\n\n";
    
    bool hasPassengers = false;
    
    // Show confirmed passengers grouped by train
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        if (currentTrain.seatCount > 0) {
            cout << "--- " << trains[trainIdx][0] << " (" << trains[trainIdx][1] << " to " << trains[trainIdx][2] << ") ---\n";
            
            for (int i = 0; i < currentTrain.seatCount; i++) {
                hasPassengers = true;
                cout << "Passenger " << (i + 1) << ":\n";
                cout << "Name: " << currentTrain.names[i] << "\n";
                cout << "Age: " << currentTrain.ages[i] << "\n";
                cout << "PNR: " << currentTrain.pnrs[i] << "\n";
                cout << "Train: " << trains[trainIdx][0] << "\n\n";
            }
        }
    }
    
    if (!hasPassengers) {
        cout << "No confirmed passengers.\n\n";
    }

    cout << "Waiting List:\n\n";
    bool hasWaitingPassengers = false;
    
    // Show waiting passengers grouped by train
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        if (!currentTrain.waitNames.empty()) {
            if (!hasWaitingPassengers) {
                // Only show train header for first train with waiting passengers
            }
            cout << "--- " << trains[trainIdx][0] << " Waiting List ---\n";
            
            queue<string> tempNames = currentTrain.waitNames;
            queue<int> tempAges = currentTrain.waitAges;
            queue<int> tempPnrs = currentTrain.waitPnrs;
            
            int waitingIndex = 1;
            while (!tempNames.empty()) {
                hasWaitingPassengers = true;
                cout << "Passenger " << waitingIndex++ << ":\n";
                cout << "Name: " << tempNames.front() << "\n";
                cout << "Age: " << tempAges.front() << "\n";
                cout << "PNR: " << tempPnrs.front() << "\n";
                cout << "Train: " << trains[trainIdx][0] << "\n\n";

                tempNames.pop();
                tempAges.pop();
                tempPnrs.pop();
            }
        }
    }
    
    if (!hasWaitingPassengers) {
        cout << "No passengers in waiting list.\n";
    }
}


// View all train routes
void viewAllTrains() {
    cout << "\nList of Available Trains:\n";
    for (int i = 0; i < TRAIN_COUNT; i++) {
        cout << i + 1 << ". " << trains[i][0]
             << " (" << trains[i][1] << " to " << trains[i][2] << ")\n";
    }
}

// Main menu
// int main() {

//     int choice;

//     do {
//         cout << "\n===== Train Management System =====\n";
//         cout << "1. Search & Book Ticket\n";
//         cout << "2. Cancel Ticket\n";
//         cout << "3. Show All Passengers\n";
//         cout << "4. View All Trains\n";
//         cout << "5. Exit\n";
//         cout << "Enter your choice: ";
//         cin >> choice;

//         if (cin.fail()) {
//             cin.clear();
//             cin.ignore(1000, '\n');
//             cout << "Invalid input. Please enter a number.\n";
//             continue;
//         }

//         switch (choice) {
//             case 1: bookTicket(); break;
//             case 2: cancelTicket(); break;
//             case 3: showPassengers(); break;
//             case 4: viewAllTrains(); break;
//             case 5: cout << "Exiting. Thank you.\n"; break;
//             default: cout << "Invalid choice. Try again.\n";
//         }
//     } while (choice != 5);

//     return 0;
// }
