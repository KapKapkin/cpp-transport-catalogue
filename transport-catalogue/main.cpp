#include <iostream>
#include <string>

#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {
    TransportCatalogue catalogue;
    input::InputReader reader;
    reader.Load(cin, catalogue);
    statisctics::ParseInputAndPrintStat(cin, cout, catalogue);
}