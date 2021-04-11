#include "../headers/main.h"
#include "../headers/types.h"
#include "../headers/t3p_client.h"
#include "../headers/tests.h"

using namespace std;

int main()
{
    #if defined(DEBUG_UDP) || defined(DEBUG_MENUS) || defined(DEBUG_TCP) || defined(DEBUG_MATCH)
        status_t status;
        if ((status = run_tests()) != STATUS_OK){
            cout << "Ending with status_t = " << status << "\n";
            return EXIT_FAILURE;
        }
        cout << "Ending SUCCESSFULLY with status_t = " << status << "\n";
        return EXIT_SUCCESS;
    #else
        status_t status;
        if ((status = t3p_client()) != STATUS_OK)
            return EXIT_FAILURE;
        return EXIT_SUCCESS;
    #endif

    
}