#include "movegen.h"
#include "position.h"

int main() {
    initLookup();
    Position position = {"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"};
    position.displayBoard();
    return 0;
}
