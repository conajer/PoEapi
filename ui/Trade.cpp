/*
* Trade.cpp, 10/22/2020 11:55 AM
*/

class Trade : public Sell {
public:

    Trade(addrtype address) : Sell(address) {
        path = {3, 1, 0, 0};
    }
};
