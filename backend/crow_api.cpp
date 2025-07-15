#include "crow_all.h"
#include "train_logic.cpp"  // Your original logic file without main()

// Book ticket wrapper
string bookTicketAPI(string from, string to, string name, int age) {
    int trainIndex = searchTrain(from, to);
    if (trainIndex == -1) {
        return "No train available from " + from + " to " + to + ".";
    }

    int pnr = generatePNR();
    TrainData& currentTrain = trainData[trainIndex];

    if (currentTrain.seatCount < MAX_SEATS) {
        currentTrain.names[currentTrain.seatCount] = name;
        currentTrain.ages[currentTrain.seatCount] = age;
        currentTrain.pnrs[currentTrain.seatCount] = pnr;
        currentTrain.seatCount++;
        return "Ticket booked. PNR: " + to_string(pnr);
    } else if (currentTrain.waitNames.size() < WAITING_LIMIT) {
        currentTrain.waitNames.push(name);
        currentTrain.waitAges.push(age);
        currentTrain.waitPnrs.push(pnr);
        return "All seats full. Added to waiting list. PNR: " + to_string(pnr);
    } else {
        return "Waiting list full. Cannot book ticket.";
    }
}

// Cancel ticket wrapper
string cancelTicketAPI(int pnr) {
    // Search through all trains
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        // Check confirmed passengers
        for (int i = 0; i < currentTrain.seatCount; i++) {
            if (currentTrain.pnrs[i] == pnr) {
                // Shift remaining passengers
                for (int j = i; j < currentTrain.seatCount - 1; j++) {
                    currentTrain.names[j] = currentTrain.names[j + 1];
                    currentTrain.ages[j] = currentTrain.ages[j + 1];
                    currentTrain.pnrs[j] = currentTrain.pnrs[j + 1];
                }
                currentTrain.seatCount--;

                string result = "Ticket cancelled for PNR: " + to_string(pnr) + " on " + trains[trainIdx][0] + "\n";
                
                // Move waiting passenger to confirmed if available
                if (!currentTrain.waitNames.empty()) {
                    currentTrain.names[currentTrain.seatCount] = currentTrain.waitNames.front(); 
                    currentTrain.waitNames.pop();
                    currentTrain.ages[currentTrain.seatCount] = currentTrain.waitAges.front(); 
                    currentTrain.waitAges.pop();
                    currentTrain.pnrs[currentTrain.seatCount] = currentTrain.waitPnrs.front(); 
                    currentTrain.waitPnrs.pop();
                    currentTrain.seatCount++;
                    result += "First waiting passenger confirmed.\n";
                }
                return result;
            }
        }
    }
    return "PNR not found.";
}

// Show passengers wrapper
string showPassengersAPI() {
    string result;
    result += "Confirmed List:\n\n";
    
    bool hasPassengers = false;
    
    // Show confirmed passengers grouped by train
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        if (currentTrain.seatCount > 0) {
            result += "--- " + trains[trainIdx][0] + " (" + trains[trainIdx][1] + " to " + trains[trainIdx][2] + ") ---\n";
            
            for (int i = 0; i < currentTrain.seatCount; i++) {
                hasPassengers = true;
                result += "Passenger " + to_string(i + 1) + ":\n";
                result += "Name: " + currentTrain.names[i] + "\n";
                result += "Age: " + to_string(currentTrain.ages[i]) + "\n";
                result += "PNR: " + to_string(currentTrain.pnrs[i]) + "\n";
                result += "Train: " + trains[trainIdx][0] + "\n\n";
            }
        }
    }
    
    if (!hasPassengers) {
        result += "No confirmed passengers.\n\n";
    }

    result += "Waiting List:\n\n";
    bool hasWaitingPassengers = false;
    
    // Show waiting passengers grouped by train
    for (int trainIdx = 0; trainIdx < TRAIN_COUNT; trainIdx++) {
        TrainData& currentTrain = trainData[trainIdx];
        
        if (!currentTrain.waitNames.empty()) {
            result += "--- " + trains[trainIdx][0] + " Waiting List ---\n";
            
            queue<string> tempNames = currentTrain.waitNames;
            queue<int> tempAges = currentTrain.waitAges;
            queue<int> tempPnrs = currentTrain.waitPnrs;
            
            int waitingIndex = 1;
            while (!tempNames.empty()) {
                hasWaitingPassengers = true;
                result += "Passenger " + to_string(waitingIndex++) + ":\n";
                result += "Name: " + tempNames.front() + "\n";
                result += "Age: " + to_string(tempAges.front()) + "\n";
                result += "PNR: " + to_string(tempPnrs.front()) + "\n";
                result += "Train: " + trains[trainIdx][0] + "\n\n";

                tempNames.pop();
                tempAges.pop();
                tempPnrs.pop();
            }
        }
    }
    
    if (!hasWaitingPassengers) {
        result += "No passengers in waiting list.\n";
    }

    return result;
}

// View trains wrapper
string viewTrainsAPI() {
    string result;
    for (int i = 0; i < TRAIN_COUNT; i++) {
        result += trains[i][0] + " (" + trains[i][1] + " to " + trains[i][2] + ")\n";
    }
    return result;
}

// Search trains wrapper (validates route without booking)
string searchTrainsAPI(string from, string to) {
    int trainIndex = searchTrain(from, to);
    if (trainIndex == -1) {
        return "No train available from " + from + " to " + to + ".";
    }
    return trains[trainIndex][0] + " (" + trains[trainIndex][1] + " to " + trains[trainIndex][2] + ")";
}

// CORS middleware
struct CORS {
    struct context {};

    void before_handle(crow::request& req, crow::response& res, context&) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");

        if (req.method == "OPTIONS"_method) {
            res.code = 204;
            res.end();
        }
    }

    void after_handle(crow::request&, crow::response& res, context&) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Accept");
    }
};

// CROW server main()
int main() {
    srand(time(0));  // Initialize random seed for unique PNRs
    initializeTrains();  // Initialize per-train data structures
    crow::App<CORS> app;

    CROW_ROUTE(app, "/book").methods("POST"_method)
    ([](const crow::request& req){
        std::cout << "📩 /book request: " << req.body << std::endl;

        auto data = crow::json::load(req.body);
        if (!data) {
            std::cout << "❌ JSON parsing failed in /book\n";
            return crow::response(400, "Invalid JSON");
        }

        try {
            string from = data["from"].s();
            string to = data["to"].s();
            string name = data["name"].s();
            int age = data["age"].i();
            std::cout << "✅ Booking ticket for " << name << " from " << from << " to " << to << ", age " << age << "\n";
            return crow::response(bookTicketAPI(from, to, name, age));
        } catch (...) {
            std::cout << "❌ Exception occurred in /book route\n";
            return crow::response(500, "Internal Server Error");
        }
    });

    CROW_ROUTE(app, "/cancel").methods("POST"_method)
    ([](const crow::request& req){
        std::cout << "📩 /cancel request: " << req.body << std::endl;

        auto data = crow::json::load(req.body);
        if (!data) {
            std::cout << "❌ JSON parsing failed in /cancel\n";
            return crow::response(400, "Invalid JSON");
        }

        try {
            int pnr = data["pnr"].i();
            std::cout << "✅ Canceling PNR: " << pnr << std::endl;
            return crow::response(cancelTicketAPI(pnr));
        } catch (...) {
            std::cout << "❌ Exception occurred in /cancel route\n";
            return crow::response(500, "Internal Server Error");
        }
    });

    CROW_ROUTE(app, "/passengers")
    ([](){
        return crow::response(showPassengersAPI());
    });

    CROW_ROUTE(app, "/trains")
    ([](){
        return crow::response(viewTrainsAPI());
    });

    CROW_ROUTE(app, "/search").methods("POST"_method)
    ([](const crow::request& req){
        std::cout << "📩 /search request: " << req.body << std::endl;

        auto data = crow::json::load(req.body);
        if (!data) {
            std::cout << "❌ JSON parsing failed in /search\n";
            return crow::response(400, "Invalid JSON");
        }

        try {
            string from = data["from"].s();
            string to = data["to"].s();
            std::cout << "✅ Searching trains from " << from << " to " << to << "\n";
            return crow::response(searchTrainsAPI(from, to));
        } catch (...) {
            std::cout << "❌ Exception occurred in /search route\n";
            return crow::response(500, "Internal Server Error");
        }
    });

    app.port(18080).multithreaded().run();
}
