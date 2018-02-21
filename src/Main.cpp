#include "App.hpp"
 
int main(int argc, char* args[]) {
    try {
        App app;
        app.main_loop();
        app.cleanup();
    } catch(std::string& error) {
        std::cout << error << std::endl;
    }
    
    return 0;
}
