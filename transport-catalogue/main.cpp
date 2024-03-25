#include <iostream>

#include "request_handler.h"

using namespace std;

int main() {
    transport_catalogue::TransportCatalogue catalogue;
    transport_catalogue::requests::RequestHandler handler(catalogue, cin);
    handler.Render();
    handler.ExecuteStatRequest(cout);
}