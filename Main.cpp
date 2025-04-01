#include "TSlimX2Pump.h"
#include "UserInterface.h"
#include <iostream>
#include <memory>

int main() {
    std::cout << "t:slim X2 Insulin Pump Simulator" << std::endl;
    std::cout << "================================" << std::endl;
    
    // Initialize the pump with default settings
    auto pump = std::make_shared<TSlimX2Pump>();
    
    // Initialize the user interface with a reference to the pump
    UserInterface ui(pump);
    
    // Main program loop
    ui.run();
    
    return 0;
}
