#include "RuntimeApp.h"

int main(int argc, char** argv) {
    RuntimeApp app;
    app.run("data.json");  // Could also read from argv[1]
    return 0;
}